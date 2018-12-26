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

#ifndef _libpq_async_data_prepared_h
#define _libpq_async_data_prepared_h

#include "data_common.h"
#include "log.h"

#include "data_connection_pool.h"
#include "data_reader.h"
#include "database.h"

#include "utils.h"

namespace pq_async {

#define _PQ_ASYNC_SEND_QRY_PREP_BODY_PARAMS(__val, __process_fn, __def_val) \
    parameters p; \
    p.push_back<sizeof...(PARAMS) -1>(args...); \
     \
    value_cb<__val> cb; \
    assign_value_cb<value_cb<__val>, __val>(cb, get_last(args...)); \
     \
    this->_db->open_connection( \
    [self=this->shared_from_this(), \
        _p = std::move(p), \
        cb] \
    (const cb_error& err, sp_connection_lock lock){ \
        if(err){ \
            cb(err, __def_val); \
            return; \
        } \
         \
        try{ \
            auto ct = std::make_shared<connection_task>( \
                self->_db->_strand.get(), self->_db, lock, \
            [self, cb]( \
                const cb_error& err, PGresult* r \
            )-> void { \
                if(err){ \
                    cb(err, __def_val); \
                    return; \
                } \
                 \
                try{ \
                    cb(nullptr, self->_db->__process_fn(r)); \
                }catch(const std::exception& err){ \
                    cb(pq_async::cb_error(err), __def_val); \
                } \
            }); \
            ct->send_query_prepared(self->_name.c_str(), _p); \
            self->_db->_strand->push_back(ct); \
             \
        }catch(const std::exception& err){ \
            cb(pq_async::cb_error(err), __def_val); \
        } \
    });

#define _PQ_ASYNC_SEND_QRY_PREP_BODY_T(__val, __process_fn, __def_val) \
    value_cb<__val> cb; \
    assign_value_cb<value_cb<__val>, __val>(cb, acb); \
    this->_db->open_connection( \
    [self=this->shared_from_this(), \
        _p = std::move(p), \
        cb] \
    (const cb_error& err, sp_connection_lock lock){ \
        if(err){ \
            cb(err, __def_val); \
            return; \
        } \
         \
        try{ \
            auto ct = std::make_shared<connection_task>( \
                self->_db->_strand.get(), self->_db, lock, \
            [self, cb]( \
                const cb_error& err, PGresult* r \
            )-> void { \
                if(err){ \
                    cb(err, __def_val); \
                    return; \
                } \
                 \
                try{ \
                    cb(nullptr, self->_db->__process_fn(r)); \
                }catch(const std::exception& err){ \
                    cb(pq_async::cb_error(err), __def_val); \
                } \
            }); \
            ct->send_query_prepared(self->_name.c_str(), _p); \
            self->_db->_strand->push_back(ct); \
             \
        }catch(const std::exception& err){ \
            cb(pq_async::cb_error(err), __def_val); \
        } \
    });

#define _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(__process_fn) \
    this->_db->wait_for_sync(); \
    auto lock = this->_db->open_connection(); \
    connection_task ct( \
        this->_db->_strand.get(), this->_db, lock \
    ); \
    ct.send_query_prepared(_name.c_str(), p); \
    return this->_db->__process_fn(ct.run_now());




class data_prepared
    : public std::enable_shared_from_this<data_prepared>
{
    friend database;
    
    data_prepared(
        sp_database db, const std::string& name, bool auto_deallocate,
        sp_connection_lock lock)
        : _db(db), _name(name), _auto_deallocate(auto_deallocate), _lock(lock)
    {
    }
    
public:
    
    ~data_prepared()
    {
        if(_auto_deallocate)
            _db->deallocate_prepared(this->_name.c_str());
    }
    
    sp_database db(){ return _db;}
    
    
    
    /*!
     * \brief asynchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(int) 
     * \param args query parameters, the last parameter is the query callback
     * pq_async::value_cb<int>
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(int)>
    void execute(const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_PARAMS(
            int, _process_execute_result, -1
        );
    }

    /*!
     * \brief asynchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, int) 
     * \param p query parameters
     * \param acb completion void(const cb_error&, int) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, int)>
    void execute(const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_T(
            int, _process_execute_result, -1
        );
    }
    
    /*!
     * \brief synchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(int) 
     * \param args query parameters
     * \return int32_t the number of record processed
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(int)>
    int32_t execute(const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_execute_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \return int32_t the number of record processed
     */
    int32_t execute()
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_execute_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \param p query parameters
     * \return int32_t the number of record processed
     */
    int32_t execute(const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_execute_result);
    }

    
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_table) 
     * \param args query parameters, the last parameter is 
     * the completion void(const cb_error&, sp_data_table) callback
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_table)>
    void query(const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_PARAMS(
            sp_data_table, _process_query_result, sp_data_table()
        );
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_table) 
     * \param p query parameters
     * \param acb completion void(const cb_error&, sp_data_table) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_table)>
    void query(const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_T(
            sp_data_table, _process_query_result, sp_data_table()
        );
    }
    
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_table) 
     * \param args query parameters
     * \return sp_data_table 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_table)>
    sp_data_table query(const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \return sp_data_table 
     */
    sp_data_table query()
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \param p query parameters
     * \return sp_data_table 
     */
    sp_data_table query(const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_result);
    }
    
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_row) 
     * \param args query parameters, the last parameter is
     * the completion void(const cb_error&, sp_data_row) callback
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_row)>
    void query_single(const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_PARAMS(
            sp_data_row, _process_query_single_result, sp_data_row()
        );
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_row) 
     * \param p query parameters
     * \param acb the completion void(const cb_error&, sp_data_row) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_row)>
    void query_single(const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_T(
            sp_data_row, _process_query_single_result, sp_data_row()
        );
    }
    
    
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_row) 
     * \param args query parameters
     * \return sp_data_row 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_row)>
    sp_data_row query_single(const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_single_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \return sp_data_row 
     */
    sp_data_row query_single()
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_single_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \param p query parameters
     * \return sp_data_row 
     */
    sp_data_row query_single(const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_single_result);
    }
    
    /*!
     * \brief asynchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R the result type
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(R) 
     * \param args query parameters, the last parameter is the query callback
     * pq_async::value_cb<R>
     */
    template<typename R, typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(R)>
    void query_value(const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_PARAMS(
            R, _process_query_value_result<R>, R()
        );
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, R) 
     * \param p query parameters
     * \param acb completion void(const cb_error&, R) callback
     */
    template<
        typename R, typename T,
        PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, R)
    >
    void query_value(const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_T(
            R, _process_query_value_result<R>, R()
        );
    }
    
    /*!
     * \brief synchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R the result type
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(R) 
     * \param args query parameters
     * \return R the scalar value
     */
    template<typename R, typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(R)>
    R query_value(const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_value_result<R>);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R 
     * \return R 
     */
    template<typename R>
    R query_value()
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_value_result<R>);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R 
     * \param p query parameters
     * \return R 
     */
    template<typename R>
    R query_value(const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC(_process_query_value_result<R>);
    }
    
    
    
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_reader) 
     * \param args query parameters, the last parameter is the query callback
     * pq_async::value_cb<sp_data_reader>
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_reader)>
    void query_reader(const PARAMS&... args)
    {
        parameters p;
        p.push_back<sizeof...(PARAMS) -1>(args...);
        
        value_cb<sp_data_reader> cb;
        assign_value_cb<value_cb<sp_data_reader>, sp_data_reader>(
            cb, get_last(args...)
        );
        
        this->_db->open_connection(
        [self=this->shared_from_this(),
            _p = std::move(p),
            cb]
        (const cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err, sp_data_reader());
                return;
            }
            
            try{
                auto ct = std::make_shared<reader_connection_task>(
                    self->_db->_strand.get(), self->_db, lock
                );
                cb(nullptr, std::shared_ptr<data_reader>(new data_reader(ct)));
                ct->send_query_prepared(self->_name.c_str(), _p);
                self->_db->_strand->push_back(ct);
                
            }catch(const std::exception& err){
                cb(pq_async::cb_error(err), sp_data_reader());
            }
        });
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_reader) 
     * \param p query parameters
     * \param acb completion void(const cb_error&, sp_data_reader) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_reader)>
    void query_reader(const parameters& p, const T& acb)
    {
        value_cb<sp_data_reader> cb;
        assign_value_cb<value_cb<sp_data_reader>, sp_data_reader>(cb, acb);

        this->_db->open_connection(
        [self=this->shared_from_this(),
            _p = std::move(p),
            cb]
        (const cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err, sp_data_reader());
                return;
            }
            
            try{
                auto ct = std::make_shared<reader_connection_task>(
                    self->_db->_strand.get(), self->_db, lock
                );
                cb(nullptr, std::shared_ptr<data_reader>(new data_reader(ct)));
                ct->send_query_prepared(self->_name.c_str(), _p);
                self->_db->_strand->push_back(ct);
                
            }catch(const std::exception& err){
                cb(pq_async::cb_error(err), sp_data_reader());
            }
        });
    }
    
    
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_reader) 
     * \param args query parameters
     * \return sp_data_reader 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_reader)>
    sp_data_reader query_reader(const PARAMS&... args)
    {
        this->_db->wait_for_sync();
        auto lock = this->_db->open_connection();
        parameters p(args...);
        auto ct = std::make_shared<reader_connection_task>(
            this->_db->_strand.get(), this->_db, lock
        );
        ct->send_query_prepared(_name.c_str(), p);
        return std::shared_ptr<data_reader>(new data_reader(ct));
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \return sp_data_reader 
     */
    sp_data_reader query_reader()
    {
        this->_db->wait_for_sync();
        auto lock = this->_db->open_connection();
        parameters p;
        auto ct = std::make_shared<reader_connection_task>(
            this->_db->_strand.get(), this->_db, lock
        );
        ct->send_query_prepared(_name.c_str(), p);
        return std::shared_ptr<data_reader>(new data_reader(ct));
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \param p query parameters
     * \return sp_data_reader 
     */
    sp_data_reader query_reader(const parameters& p)
    {
        this->_db->wait_for_sync();
        auto lock = this->_db->open_connection();
        auto ct = std::make_shared<reader_connection_task>(
            this->_db->_strand.get(), this->_db, lock
        );
        ct->send_query_prepared(_name.c_str(), p);
        return std::shared_ptr<data_reader>(new data_reader(ct));
    }
    
    
private:
    sp_database _db;
    std::string _name;
    bool _auto_deallocate;
    sp_connection_lock _lock;
};


#undef _PQ_ASYNC_SEND_QRY_PREP_BODY_PARAMS
#undef _PQ_ASYNC_SEND_QRY_PREP_BODY_T
#undef _PQ_ASYNC_SEND_QRY_PREP_BODY_SYNC

} // ns: pq_async
#endif //_libpq_async_data_prepared_h
