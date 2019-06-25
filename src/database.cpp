/*
    This file is part of pq-async
    Copyright (C) 2011-2018 Michel Denommee (and other contributing authors)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "database.h"
#include "data_large_object.h"
#include "data_prepared.h"

namespace pq_async{

database open(
    const std::string& connection_string,
    md::log::logger log)
{
    database dbso(
        new database_t(
            md::event_queue_t::get_default()->new_strand<int>(),
            connection_string,
            log
        )
    );
    return dbso;
}

database open(
    md::event_strand<int> strand, 
    const std::string& connection_string,
    md::log::logger log)
{
    database dbso(
        new database_t(strand, connection_string, log)
    );
    return dbso;
}

database_t::database_t(
    md::event_strand<int> strand,
    const std::string& connection_string,
    md::log::logger log = nullptr)
    :_connection_string(connection_string),
    _conn(NULL),
    _strand(strand),
    _lock(),
    _log(log ? log : pq_async::default_logger())
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    strand->enable_activate_on_requeue(false);
}

database_t::~database_t()
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    this->close();
}

void database_t::exec_queries(
    const std::string& sql, const md::callback::async_cb& cb)
{
    std::vector< std::string > queries;
    split_queries(sql, queries);
    
    throw pq_async::exception("not implemented!");
}

//https://stackoverflow.com/questions/53930596/how-can-i-insert-multi-entry-array-using-libpg-copy-from-stdin-method-in-pos

//https://www.postgresql.org/docs/current/libpq-copy.html

class pg_socket_block
{
public:
    pg_socket_block(PGconn* conn)
        :_conn(conn),
        _unblock(PQisnonblocking(conn) == 1)
    {
        if(_unblock)
            PQsetnonblocking(_conn, 0);
    }
    
    ~pg_socket_block()
    {
        if(_unblock)
            PQsetnonblocking(_conn, 1);
    }
    
private:
    PGconn* _conn;
    bool _unblock;
};

void database_t::exec_queries(const std::string& sql)
{
    this->wait_for_sync();
    // switch the connection to blocking mode until exec_queries is completed.
    pg_socket_block conn_block(_conn->conn());
    
    bool local_trans = false;
    if(!this->in_transaction()){
        local_trans = true;
        this->begin();
    }
    
    std::vector< std::string > queries;
    split_queries(sql, queries);
    
    PQ_ASYNC_DBG(
        _log,
        "splitting sql queries:\noriginal:\n{}\n\nresults:\n\n{}", 
        sql, md::join(queries, "\n\n--next-query--\n")
    );
    
    bool in_copy_mode = false;
    for(const std::string& qry: queries){
        #if PQ_ASYNC_BUILD_DEBUG
        std::vector<std::string> qry_lines;
        boost::algorithm::split(
            qry_lines, qry, boost::is_any_of("\n")
        );
        std::string num_qry;
        for(size_t i = 0; i < qry_lines.size(); ++i){
            std::string num = md::num_to_str(i+1);
            for(size_t j = num.size(); j < 4; ++j)
                num += " ";
            num_qry += num + " " + qry_lines[i] + "\n";
        }
        
        PQ_ASYNC_DBG(
            _log,
            "Executing query:\n{}", num_qry
        );
        #else
        PQ_ASYNC_DBG(
            _log,
            "Executing query:\n{}", qry
        );
        #endif//PQ_ASYNC_BUILD_DEBUG
        
        PGresult* res = nullptr;
        int result_status = 0;
        if(!in_copy_mode){
            res = PQexec(_conn->conn(), qry.c_str());
            result_status = PQresultStatus(res);
            if((result_status & PGRES_COPY_IN) == PGRES_COPY_IN){
                // verify if copy mode is required
                std::string cp_qry = md::replace_substring_copy(
                    qry, "\n", " "
                );
                if(
                    boost::algorithm::istarts_with(cp_qry, "copy ") &&
                    boost::algorithm::iends_with(cp_qry, " stdin")
                )
                    in_copy_mode = true;
            }
            
        }else{
            int copy_status = PQputCopyData(
                _conn->conn(), qry.c_str(), qry.size()
            );
            if(copy_status == -1){
                throw MD_ERR(
                    "PQputCopyData failed!\n{}",
                    PQerrorMessage(_conn->conn())
                );
            }
            
            copy_status = PQputCopyEnd(_conn->conn(), nullptr);
            if(copy_status == -1){
                throw MD_ERR(
                    "PQputCopyEnd failed!\n{}",
                    PQerrorMessage(_conn->conn())
                );
            }
            res = PQgetResult(_conn->conn());
            result_status = PQresultStatus(res);
            in_copy_mode = false;
        }
        
        if(in_copy_mode){
            PQ_ASYNC_DBG(
                _log,
                "Switched to copy mode"
            );
            PQclear(res);
            continue;
        }
        
        if(result_status == PGRES_COMMAND_OK ||
            result_status == PGRES_TUPLES_OK
        ){
            PQclear(res);
            
        } else if (result_status == PGRES_EMPTY_QUERY ||
            result_status == PGRES_NONFATAL_ERROR
        ){
            PQclear(res);
            
        } else {
            std::string errMsg = PQresStatus((ExecStatusType)result_status);
            errMsg += ": ";
            errMsg += std::string(PQerrorMessage(_conn->conn()));
            
            PQclear(res);
            throw pq_async::exception(errMsg);
        }
    }
    
    if(local_trans)
        this->commit();
}

void database_t::split_queries(
    const std::string& sql, std::vector< std::string >& queries)
{
    std::string cur_qry;
    char cur_char = '\0';
    char next_char = '\0';
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool in_dollar_quote = false;
    std::string dollar_quote_string;
    bool in_comment = false;
    bool in_copy_mode = false;
    
    for(std::string::size_type i = 0; i < sql.size(); ++i) {
        cur_char = sql[i];
        next_char = i < sql.size() -1 ? sql[i+1] : '\0';
        
        // it's the end
        if(i == sql.size() -1){
            if(cur_char != ';')
                cur_qry += cur_char;
            
            md::trim(cur_qry);
            if(cur_qry.size() > 0)
                queries.push_back(cur_qry);
            continue;
        }
        
        // test for comment end
        if(in_comment){
            if(cur_char == '\n')
                in_comment = false;
            continue;
        }
        
        // test for end of single quote
        if(in_single_quote){
            if(cur_char == '\'' && next_char != '\''){
                cur_qry += cur_char;
                in_single_quote = false;
                continue;
                
            // escaping the "'" char
            } else if(cur_char == '\'' && next_char == '\''){
                cur_qry += cur_char;
                cur_qry += next_char;
                ++i;
                continue;
            }
            
            cur_qry += cur_char;
            continue;
        }
        
        // test for end of double quote
        if(in_double_quote){
            if(cur_char == '"' && next_char != '"'){
                cur_qry += cur_char;
                in_double_quote = false;
                continue;
            // escaping the '"' char
            } else if(cur_char == '"' && next_char == '"'){
                cur_qry += cur_char;
                cur_qry += next_char;
                ++i;
                continue;
            }
            
            cur_qry += cur_char;
            continue;
        }
        
        // test for end of dollar quote
        if(in_dollar_quote){
            if(cur_char == '$' &&
                dollar_quote_string == sql.substr(i, dollar_quote_string.size())
            ){
                cur_qry += dollar_quote_string;
                i += dollar_quote_string.size() -1;
                in_dollar_quote = false;
                continue;
            }
            
            cur_qry += cur_char;
            continue;
        }
        
        // test for end of copy mode
        if(in_copy_mode){
            if((cur_char == '\n' || cur_char == '\r') && next_char == '\\'){
                char third_char = i < sql.size() -2 ? sql[i+2] : '\0';
                if(third_char == '.'){
                    md::trim(cur_qry);
                    //cur_qry += "\n\\.";
                    i += 2;
                    queries.push_back(cur_qry);
                    cur_qry.clear();
                    in_copy_mode = false;
                    continue;
                }
            }
            cur_qry += cur_char;
            continue;
        }
        
        // is a comment
        if(cur_char == '-' && next_char == '-'){
            in_comment = true;
            continue;
        }
        
        // dollar block start
        if(cur_char == '$'){
            dollar_quote_string = "$";
            while(true && i < sql.size() -1){
                cur_char = sql[++i];
                dollar_quote_string += cur_char;
                if(cur_char == '$')
                    break;
            };
            cur_qry += dollar_quote_string;
            in_dollar_quote = true;
            continue;
        }
        
        // text block start
        if(cur_char == '\''){
            cur_qry += cur_char;
            in_single_quote = true;
            continue;
        }
        
        // escaped item definition
        if(cur_char == '"'){
            cur_qry += cur_char;
            in_double_quote = true;
            continue;
        }
        
        // end of query
        if(cur_char == ';'){
            md::trim(cur_qry);
            if(cur_qry.size() > 0){
                // verify if copy mode is required
                std::string cp_qry = md::replace_substring_copy(
                    cur_qry, "\n", " "
                );
                if(
                    boost::algorithm::istarts_with(cp_qry, "copy ") &&
                    boost::algorithm::iends_with(cp_qry, " stdin")
                )
                    in_copy_mode = true;
                
                queries.push_back(cur_qry);
            }
            cur_qry.clear();
            continue;
        }
        
        cur_qry += cur_char;
    }
}



int database_t::_process_execute_result(PGresult* res)
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    
    if(!_conn){
        std::string err_msg("connection is dead!");
        throw pq_async::exception(err_msg);
    }
    
    int result_status = PQresultStatus(res);
    
    if(result_status == PGRES_COMMAND_OK ||
        result_status == PGRES_SINGLE_TUPLE ||
        result_status == PGRES_TUPLES_OK
    ){
        int affectedRows = atoi(PQcmdTuples(res)); 

        PQclear(res);

        return affectedRows;

    } else if(
        result_status == PGRES_EMPTY_QUERY || 
        result_status == PGRES_NONFATAL_ERROR
    ){
        PQclear(res);
        return 0;
    
    } else {
        std::string errMsg = PQerrorMessage(_conn->conn());
        PQclear(res);
        throw pq_async::exception(errMsg);
    }
}


data_table database_t::_process_query_result(PGresult* res)
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    
    if(!_conn){
        std::string err_msg("connection is dead!");
        throw pq_async::exception(err_msg);
    }
    
    int result_status = PQresultStatus(res);
    
    if(result_status != PGRES_COMMAND_OK &&
        result_status != PGRES_SINGLE_TUPLE &&
        result_status != PGRES_TUPLES_OK
    ){
        std::string errMsg = PQerrorMessage(_conn->conn());
        PQclear(res);
        throw pq_async::exception(errMsg);
    }
    
    data_table table(new pq_async::data_table_t());
    
    int fieldCount = PQnfields(res);
    for(int i = 0; i < fieldCount; ++i){
        int oid = PQftype(res, i);
        std::string name(PQfname(res, i));
        int fmt = PQfformat(res, i);
        
        data_column col(new data_column_t(oid, i, name, fmt));
        
        table->get_columns()->emplace_back(col);
    }

    int row_count = PQntuples(res);
    table->reserve(row_count);
    for(int i = 0; i < row_count; ++i){
        data_row row(new data_row_t(table->get_columns(), res, i));
        table->emplace_back(row);
    }

    PQclear(res);

    return table;
}


data_row database_t::_process_query_single_result(PGresult* res)
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    if(!_conn){
        std::string err_msg("connection is dead!");
        throw pq_async::exception(err_msg);
    }
    
    int result_status = PQresultStatus(res);
    
    if(result_status != PGRES_COMMAND_OK &&
        result_status != PGRES_TUPLES_OK &&
        result_status != PGRES_SINGLE_TUPLE
    ){
        std::string errMsg = PQerrorMessage(_conn->conn());
        PQclear(res);
        throw pq_async::exception(errMsg);
    }

    data_table table(new pq_async::data_table_t());
         
    int fieldCount = PQnfields(res);
    for(int i = 0; i < fieldCount; ++i){
        int oid = PQftype(res, i);
        std::string name(PQfname(res, i));
        int fmt = PQfformat(res, i);
        data_column col(new data_column_t(oid, i, name, fmt));
        
        table->get_columns()->emplace_back(col);
    }
    
    int row_count = PQntuples(res);
    table->reserve(row_count);
    for(int i = 0; i < row_count; ++i){
        data_row row(new data_row_t(table->get_columns(), res, i));
        table->emplace_back(row);

        PQclear(res);
        return row;
    }

    PQclear(res);

    return data_row();
}

data_prepared database_t::_process_send_prepare_result(
    const std::string& name, bool auto_deallocate,
    connection_lock lock, PGresult* res)
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    if(!_conn){
        std::string err_msg("connection is dead!");
        throw pq_async::exception(err_msg);
    }
    
    int result_status = PQresultStatus(res);
    
    if(result_status != PGRES_COMMAND_OK){
        std::string errMsg = PQerrorMessage(_conn->conn());
        PQclear(res);
        throw pq_async::exception(errMsg);
    }
    
    PQclear(res);
    return data_prepared(
        new data_prepared_t(
            this->shared_from_this(), name, auto_deallocate, lock
        )
    );
}

data_prepared database_t::_new_prepared(
    const char* name, bool auto_deallocate,
    connection_lock lock)
{
    return data_prepared(
        new data_prepared_t(
            this->shared_from_this(), name, auto_deallocate,
            lock
        )
    );
}


data_large_object database_t::create_lo()
{
    open_connection();
    connection_lock_t conn_lock(_conn);
    
    pq_async::oid lo_oid = lo_create(_conn->conn(), pq_async::oid());
    return get_lo(lo_oid);
}

data_large_object database_t::get_lo(const pq_async::oid& lo_oid)
{
    return data_large_object(
        new data_large_object_t(
            this->shared_from_this(), lo_oid
        )
    );
}


} //ns: pq_async