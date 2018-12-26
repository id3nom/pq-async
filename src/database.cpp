/*
    This file is part of libpq-async++
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

database::database(
    sp_event_strand<int> strand,
    const std::string& connection_string)
    :_connection_string(connection_string),
    _conn(NULL),
    _strand(strand),
    _lock()
{
    pq_async_log_trace("ptr: %p", this);
}

database::~database()
{
    pq_async_log_trace("ptr: %p", this);
    this->close();
}

void database::exec_queries(const std::string& sql, const async_cb& cb)
{
    std::vector< std::string > queries;
    split_queries(sql, queries);
    
    throw pq_async::exception("not implemented!");
}

void database::exec_queries(const std::string& sql)
{
    this->wait_for_sync();
    
    open_connection();
    connection_lock conn_lock(_conn);
    
    std::vector< std::string > queries;
    split_queries(sql, queries);
    
    pq_async_log_debug(
        "splitting sql queries:\noriginal:\n%s\n\nresults:\n\n%s", 
        sql.c_str(), pq_async::join(queries, "\n\n--next-query--\n")
        );
    
    for(const std::string& qry: queries){
        pq_async_log_debug("Executing query:\n%s", qry.c_str());
        PGresult* res = PQexec(_conn->conn(), qry.c_str());
        
        int result_status = PQresultStatus(res);
        
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
}

void database::split_queries(
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
    
    for(std::string::size_type i = 0; i < sql.size(); ++i) {
        cur_char = sql[i];
        next_char = i < sql.size() -1 ? sql[i+1] : '\0';
        
        if(i == sql.size() -1){
            if(cur_char != ';')
                cur_qry += cur_char;
            
            pq_async::trim(cur_qry);
            if(cur_qry.size() > 0)
                queries.push_back(cur_qry);
            continue;
        }
        
        if(in_comment){
            if(cur_char == '\n')
                in_comment = false;
            continue;
        }
        
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
        
        if(in_dollar_quote){
            if(cur_char == '$' &&
                dollar_quote_string == sql.substr(i, dollar_quote_string.size())
            ){
                cur_qry += dollar_quote_string;
                i += dollar_quote_string.size();
                in_dollar_quote = false;
                continue;
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
            pq_async::trim(cur_qry);
            if(cur_qry.size() > 0)
                queries.push_back(cur_qry);
            cur_qry = "";
            continue;
        }
        
        cur_qry += cur_char;
    }
}



int database::_process_execute_result(PGresult* res)
{
    pq_async_log_trace("");
    
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


sp_data_table database::_process_query_result(PGresult* res)
{
    pq_async_log_trace("");
    
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
    
    sp_data_table table(new pq_async::data_table());
    
    int fieldCount = PQnfields(res);
    for(int i = 0; i < fieldCount; ++i){
        int oid = PQftype(res, i);
        std::string name(PQfname(res, i));
        int fmt = PQfformat(res, i);
        
        sp_data_column col(new data_column(oid, i, name, fmt));
        
        table->get_columns()->emplace_back(col);
    }

    int row_count = PQntuples(res);
    table->reserve(row_count);
    for(int i = 0; i < row_count; ++i){
        sp_data_row row(new data_row(table->get_columns(), res, i));
        table->emplace_back(row);
    }

    PQclear(res);

    return table;
}


sp_data_row database::_process_query_single_result(PGresult* res)
{
    pq_async_log_trace("");
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

    sp_data_table table(new pq_async::data_table());
         
    int fieldCount = PQnfields(res);
    for(int i = 0; i < fieldCount; ++i){
        int oid = PQftype(res, i);
        std::string name(PQfname(res, i));
        int fmt = PQfformat(res, i);
        sp_data_column col(new data_column(oid, i, name, fmt));
        
        table->get_columns()->emplace_back(col);
    }
    
    int row_count = PQntuples(res);
    table->reserve(row_count);
    for(int i = 0; i < row_count; ++i){
        sp_data_row row(new data_row(table->get_columns(), res, i));
        table->emplace_back(row);

        PQclear(res);
        return row;
    }

    PQclear(res);

    return sp_data_row();
}

sp_data_prepared database::_process_send_prepare_result(
    const std::string& name, bool auto_deallocate,
    sp_connection_lock lock, PGresult* res)
{
    pq_async_log_trace("");
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
    return sp_data_prepared(
        new data_prepared(
            this->shared_from_this(), name, auto_deallocate, lock
        )
    );
}

sp_data_prepared database::_new_prepared(
    const char* name, bool auto_deallocate,
    sp_connection_lock lock)
{
    return sp_data_prepared(
        new data_prepared(
            this->shared_from_this(), name, auto_deallocate,
            lock
        )
    );
}


sp_data_large_object database::create_lo()
{
    open_connection();
    connection_lock conn_lock(_conn);
    
    pq_async::oid lo_oid = lo_create(_conn->conn(), pq_async::oid());
    return get_lo(lo_oid);
}

sp_data_large_object database::get_lo(const pq_async::oid& lo_oid)
{
    return sp_data_large_object(
        new data_large_object(
            this->shared_from_this(), lo_oid
        )
    );
}


} //ns: pq_async