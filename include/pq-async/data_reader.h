/*
MIT License

Copyright (c) 2011-2019 Michel Dénommée

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _libpq_async_data_reader_h
#define _libpq_async_data_reader_h

#include "data_common.h"
#include "data_table_t.h"
#include "data_connection_pool.h"

namespace pq_async{

class data_reader_t 
    : public std::enable_shared_from_this<data_reader_t>
{
    friend class database_t;
    friend class data_prepared_t;
    
    data_reader_t(sp_connection_task ct)
        : _ct(ct), _table(), _closed(false)
    {
    }
    
public:
    
    ~data_reader_t()
    {
        close();
        _ct.reset();
    }
    
    database db(){ return _ct->db();}
    
    bool closed(){ return _closed;}
    
    template<
        typename T,
        typename std::enable_if<
            !std::is_same<
                data_row,
                typename std::remove_pointer<
                    T
                >::type
            >::value &&
            !std::is_array<
                typename std::remove_pointer<
                    T
                >::type
            >::value &&
            (std::is_invocable_r<
                void,
                typename std::remove_pointer<
                    T
                >::type,
                const md::callback::cb_error&
            >::value ||
            std::is_invocable_r<
                void,
                typename std::remove_pointer<
                    T
                >::type,
                const md::callback::cb_error&,
                data_row
            >::value ||
            std::is_invocable_r<
                void,
                typename std::remove_pointer<
                    T
                >::type
            >::value ||
            std::is_invocable_r<
                void,
                typename std::remove_pointer<
                    T
                >::type,
                data_row
            >::value)
        , int32_t>::type = -1
    >
    void next(T tcb)
    {
        md::callback::value_cb<data_row> cb;
        md::callback::assign_value_cb<
            md::callback::value_cb<data_row>, data_row
        >(
            cb, tcb
        );
        
        _ct->_cb =
        [self=this->shared_from_this(), cb](
            const md::callback::cb_error& err, PGresult* res
        )-> void {
            if(err){
                cb(err, data_row());
                self->_ct->_cb = nullptr;
                return;
            }
            
            try{
                if(!res){
                    self->_closed = true;
                    self->close();
                    cb(nullptr, data_row());
                    self->_ct->_cb = nullptr;
                    return;
                }
                if(self->_closed)
                    throw pq_async::exception("The reader is closed!");
                
                self->init_table(res);
                
                auto res_status = PQresultStatus(res);
                if(res_status != PGRES_COMMAND_OK && 
                    res_status != PGRES_TUPLES_OK &&
                    res_status != PGRES_SINGLE_TUPLE
                    ){
                    std::string errMsg = PQerrorMessage(self->_ct->conn());
                    PQclear(res);
                    res = nullptr;
                    throw pq_async::exception(errMsg);
                }
                
                if(PQntuples(res)){
                    data_row row(
                        new data_row_t(self->_table->get_columns(), res, 0)
                    );
                    self->_table->emplace_back(row);
                }
                PQclear(res);
                res = nullptr;
                
                if(self->_table->size() > 0)
                    cb(md::callback::cb_error(err), (*self->_table)[0]);
            }catch(const std::exception& err){
                cb(md::callback::cb_error(err), data_row());
                self->_ct->_cb = nullptr;
            }
        };
    }
    
    data_row next()
    {
        if(_closed)
            throw pq_async::exception("The reader is closed!");
        
        auto res = _ct->run_now();
        if(!res){
            this->_closed = true;
            this->close();
            
            return data_row();
        }
        
        init_table(res);
        
        auto res_status = PQresultStatus(res);
        if(res_status != PGRES_COMMAND_OK && 
            res_status != PGRES_TUPLES_OK &&
            res_status != PGRES_SINGLE_TUPLE
            ){
            std::string errMsg = PQerrorMessage(_ct->conn());
            PQclear(res);
            res = nullptr;
            throw pq_async::exception(errMsg);
        }
        
        if(PQntuples(res)){
            data_row row(new data_row_t(_table->get_columns(), res, 0));
            _table->emplace_back(row);
        }
        PQclear(res);
        res = nullptr;
        
        if(_table->size() > 0)
            return (*_table)[0];
        
        while(PGresult* r = PQgetResult(_ct->conn())){
            PQclear(r);
        };
        _closed = true;
        
        return data_row();
    }
    
    void close()
    {
        if(_closed)
            return;
        
        char errbuf[256];
        PGcancel* c = PQgetCancel(_ct->conn());
        if(c){
            if(!PQcancel(c, errbuf, 256))
                pq_async::default_logger()->error(MD_ERR(
                    "Unable to cancel current query: {:p}",
                    (void*)c
                ));
            PQfreeCancel(c);
        }
        while(PGresult* r = PQgetResult(_ct->conn())){
            PQclear(r);
        };
        _closed = true;
    }
    
private:
    
    void init_table(PGresult* res)
    {
        if(_table){
            _table->clear();
            return;
        }
        
        int res_status = PQresultStatus(res);
        if(res_status != PGRES_COMMAND_OK && 
            res_status != PGRES_TUPLES_OK &&
            res_status != PGRES_SINGLE_TUPLE
        ){
            std::string errMsg = PQerrorMessage(_ct->conn());
            PQclear(res);
            res = nullptr;
            throw pq_async::exception(errMsg);
        }

        _table = data_table(new pq_async::data_table_t());

        int fieldCount = PQnfields(res);
        for(int i = 0; i < fieldCount; ++i){
            int oid = PQftype(res, i);
            std::string name(PQfname(res, i));
            int fmt = PQfformat(res, i);

            data_column col(
                new data_column_t(oid, i, name, fmt)
            );
            
            _table->get_columns()->emplace_back(col);
        }
    }
    
    sp_connection_task _ct;
    data_table _table;
    bool _closed;
};

} //namespace pq_async

#endif //_libpq_async_data_reader_h
