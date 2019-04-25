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

#ifndef _libpq_async_database_h
#define _libpq_async_database_h

#include "data_common.h"
#include "log.h"

#include "data_connection_pool.h"
#include "data_table.h"
#include "data_reader.h"

#include "utils.h"

namespace pq_async{

#if PQ_ASYNC_BUILD_DEBUG == 1
#define PQ_ASYNC_DEFAULT_TIMEOUT_MS 300000
#else
#define PQ_ASYNC_DEFAULT_TIMEOUT_MS 5000
#endif


#define _PQ_ASYNC_SEND_QRY_BODY_PARAMS(__val, __process_fn, __def_val) \
    parameters p; \
    p.push_back<sizeof...(PARAMS) -1>(args...); \
     \
    md::callback::value_cb<__val> cb; \
    md::callback::assign_value_cb<md::callback::value_cb<__val>, __val>(cb, md::get_last(args...)); \
     \
    this->open_connection( \
    [self=this->shared_from_this(), \
        _sql = std::string(sql),_p = std::move(p), \
        cb] \
    (const md::callback::cb_error& err, sp_connection_lock lock){ \
        if(err){ \
            cb(err, __def_val); \
            return; \
        } \
         \
        try{ \
            auto ct = std::make_shared<connection_task>( \
                self->_strand.get(), self, lock, \
            [self, cb]( \
                const md::callback::cb_error& err, PGresult* r \
            )-> void { \
                if(err){ \
                    cb(err, __def_val); \
                    return; \
                } \
                 \
                try{ \
                    cb(nullptr, self->__process_fn(r)); \
                }catch(const std::exception& err){ \
                    cb(md::callback::cb_error(err), __def_val); \
                } \
            }); \
            PQ_ASYNC_DBG(self->_log, \
                "queuing query: {}\ndb: {:p}, ct: {:p}, lock: {:p}, conn: {:p}", \
                _sql.c_str(), (void*)self.get(), (void*)ct.get(), (void*)lock.get(), (void*)lock->conn() \
            ); \
            ct->send_query(_sql.c_str(), _p); \
            self->_strand->push_front(ct); \
             \
        }catch(const std::exception& err){ \
            cb(md::callback::cb_error(err), __def_val); \
        } \
    });

#define _PQ_ASYNC_SEND_QRY_BODY_T(__val, __process_fn, __def_val) \
    md::callback::value_cb<__val> cb; \
    md::callback::assign_value_cb<md::callback::value_cb<__val>, __val>(cb, acb); \
    this->open_connection( \
    [self=this->shared_from_this(), \
        _sql = std::string(sql),_p = std::move(p), \
        cb] \
    (const md::callback::cb_error& err, sp_connection_lock lock){ \
        if(err){ \
            cb(err, __def_val); \
            return; \
        } \
         \
        try{ \
            auto ct = std::make_shared<connection_task>( \
                self->_strand.get(), self, lock, \
            [self, cb]( \
                const md::callback::cb_error& err, PGresult* r \
            )-> void { \
                if(err){ \
                    cb(err, __def_val); \
                    return; \
                } \
                 \
                try{ \
                    cb(nullptr, self->__process_fn(r)); \
                }catch(const std::exception& err){ \
                    cb(md::callback::cb_error(err), __def_val); \
                } \
            }); \
            ct->send_query(_sql.c_str(), _p); \
            self->_strand->push_back(ct); \
             \
        }catch(const std::exception& err){ \
            cb(md::callback::cb_error(err), __def_val); \
        } \
    });

#define _PQ_ASYNC_SEND_QRY_BODY_SYNC(__process_fn) \
    this->wait_for_sync(); \
    auto lock = open_connection(); \
    connection_task ct( \
        this->_strand.get(), this->shared_from_this(), lock \
    ); \
    ct.send_query(sql, p); \
    return __process_fn(ct.run_now());

/*!
 * \brief 
 * 
 */
class database
    : public std::enable_shared_from_this<database>
{
    friend class connection;
    friend class connection_task;
    friend class connection_pool;
    friend class data_large_object;
    friend class data_prepared;

    database(
        md::sp_event_strand<int> strand, 
        const std::string& connection_string,
        md::log::sp_logger log
    );
    
public:
    
    virtual ~database();
    
    
    md::sp_event_strand<int> get_strand(){ return _strand;}
    
    md::log::sp_logger log(){ return _log;}
    
    /*!
     * \brief synchronously try to acquire and open a connection
     * 
     * \param timeout_ms milliseconds to wait before failure, default to 5000
     * \return sp_connection_lock 
     */
    sp_connection_lock open_connection(
        int32_t timeout_ms = PQ_ASYNC_DEFAULT_TIMEOUT_MS)
    {
        if(_lock)
            return _lock;
        
        #ifdef PQ_ASYNC_THREAD_SAFE
        std::unique_lock<std::recursive_mutex> lock(
            connection_pool::instance()->conn_pool_mutex
        );
        #endif
        
        if(_conn == NULL)
            _conn = connection_pool::get_connection(
                this, _connection_string, timeout_ms
            );
        
        _conn->reserve();
        if(_conn == NULL)
            _conn = connection_pool::get_connection(
                this, _connection_string, timeout_ms
            );
        
        sp_connection_lock cl(new connection_lock(_conn));
        _conn->open_connection();
        
        #ifdef PQ_ASYNC_THREAD_SAFE
        lock.unlock();
        connection_pool::notify_all();
        #endif
        
        return cl;
    }
    
    /*!
     * \brief asynchronously try to acquire and open a connection
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK_T(CB, bool) 
     * \param acb 
     * \param timeout_ms milliseconds to wait before failure, default to 5000
     */
    template<
        typename CB,
        PQ_ASYNC_VALID_DB_VAL_CALLBACK(CB, sp_connection_lock)
    >
    void open_connection(const CB& acb,
        int32_t timeout_ms = PQ_ASYNC_DEFAULT_TIMEOUT_MS)
    {
        md::callback::value_cb<sp_connection_lock> cb;
        md::callback::assign_value_cb<
            md::callback::value_cb<sp_connection_lock>,
            sp_connection_lock
        >(
            cb, acb
        );
        
        if(_lock){
            this->_strand->push_back(
                std::bind(cb, nullptr, _lock)
            );
            return;
        }
        
        try{
            auto ct = std::make_shared<connection_task>(
                this->_strand.get(), this->shared_from_this(), this->_conn,
            [self=this->shared_from_this(), cb](
                const md::callback::cb_error& err, sp_connection_lock lock
            )-> void {
                if(err){
                    cb(err, sp_connection_lock());
                    return;
                }
                
                try{
                    cb(nullptr, lock);
                }catch(const std::exception& err){
                    cb(md::callback::cb_error(err), sp_connection_lock());
                }
            });
            PQ_ASYNC_DBG(_log,
                "queuing open connection\ndb: {:p}, ct: {:p}",
                (void*)this, (void*)ct.get()
            );
            
            ct->connect(_connection_string, timeout_ms);
            this->_strand->push_back(ct);
            
        }catch(const std::exception& err){
            cb(md::callback::cb_error(err), sp_connection_lock());
        }
    }
    
    /*!
     * \brief release the underlying connection
     * this call is always synchronous
     */
    void close()
    {
        #ifdef PQ_ASYNC_THREAD_SAFE
        std::unique_lock<std::recursive_mutex> lock(
            connection_pool::instance()->conn_pool_mutex
        );
        #endif
        
        if(_conn == NULL)
            return;
        
        _conn->release();
        _conn = NULL;
        
        #ifdef PQ_ASYNC_THREAD_SAFE
        lock.unlock();
        connection_pool::notify_all();
        #endif
    }
    
    /*!
     * \brief returns true if the database is currently working.
     * 
     * \return true 
     * \return false 
     */
    bool working()
    {
        return this->_conn && this->_conn->running();
    }
    
    /*!
     * \brief returns if a transaction is in progress
     * 
     * \return true if a transaction is in progress
     * \return false if no transaction is in progress
     */
    bool in_transaction()
    {
        if(!_conn)
            return false;
        
        return _conn->in_transaction();
    }
    
    /*!
     * \brief synchronously starts a new transaction
     * 
     */
    void begin()
    {
        wait_for_sync();
        _lock = open_connection();
        _conn->begin_transaction();
    }
    
    /*!
     * \brief asynchronously starts a new transaction
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param acb md::callback::async_cb callback to call on completion
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void begin(const CB& acb)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        if(this->in_transaction()){
            this->_strand->push_back(
                std::bind(
                    cb,
                    md::callback::cb_error("Already in a transaction!")
                )
            );
            return;
        }

        this->open_connection(
        [self=this->shared_from_this(), cb]
        (const md::callback::cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err);
                return;
            }
            self->_lock = lock;
            
            self->execute("BEGIN",
            [self, cb](const md::callback::cb_error& err){
                if(err){
                    self->_lock.reset();
                    cb(err);
                    return;
                }
                
                self->_conn->is_in_transaction.store(true);
                cb(nullptr);
            });
        });
    }
    
    /*!
     * \brief synchonously commit current transaction
     * 
     */
    void commit()
    {
        wait_for_sync();
        _conn->commit_transaction();
        _lock.reset();
    }
    
    /*!
     * \brief asynchonously commit current transaction
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param acb 
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void commit(const CB& acb)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        if(!this->in_transaction()){
            this->_strand->push_back(
                std::bind(cb, md::callback::cb_error("Not in a transaction!"))
            );
            return;
        }
        
        this->_conn->touch();
        this->execute("COMMIT",
        [self=this->shared_from_this(), cb](const md::callback::cb_error& err){
            if(err){
                self->_lock.reset();
                cb(err);
                return;
            }
            self->_conn->touch();
            self->_conn->is_in_transaction.store(false);
            cb(nullptr);
            self->_lock.reset();
        });
    }
    
    /*!
     * \brief synchonously rollback current transaction
     * 
     */
    void rollback()
    {
        wait_for_sync();
        _conn->rollback_transaction();
        _lock.reset();
    }
    
    /*!
     * \brief asynchonously rollback current transaction
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param acb 
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void rollback(const CB& acb)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        if(!this->in_transaction()){
            this->_strand->push_back(
                std::bind(cb, md::callback::cb_error("Not in a transaction!"))
            );
            return;
        }
        
        this->_conn->touch();
        this->execute("ROLLBACK",
        [self=this->shared_from_this(), cb](const md::callback::cb_error& err){
            if(err){
                self->_lock.reset();
                cb(err);
                return;
            }
            self->_conn->touch();
            self->_conn->is_in_transaction.store(false);
            cb(nullptr);
            self->_lock.reset();
        });
    }
    
    
    /*!
     * \brief synchronously sets a new savepoint
     * inside the current transaction
     * 
     * \param name the savepoint name
     */
    void set_savepoint(const char* name)
    {
        wait_for_sync();
        _conn->set_savepoint(name);
    }
    
    /*!
     * \brief asynchronously sets a new savepoint
     * inside the current transaction
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param acb 
     * \param name 
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void set_savepoint(const CB& acb, const char* name)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        if(!this->in_transaction()){
            this->_strand->push_back(
                std::bind(cb, md::callback::cb_error("Not in a transaction!"))
            );
            return;
        }
        
        this->_conn->touch();
        std::string s("SAVEPOINT ");
        s += name;
        this->execute(s.c_str(),
        [self=this->shared_from_this(), cb](const md::callback::cb_error& err){
            if(err){
                cb(err);
                return;
            }
            
            self->_conn->touch();
            cb(nullptr);
        });
    }
    
    /*!
     * \brief synchonously release the specified savepoint
     * 
     * \param name 
     */
    void release_savepoint(const char* name)
    {
        wait_for_sync();
        _conn->release_savepoint(name);
    }
    
    /*!
     * \brief asynchonously release the specified savepoint
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param acb 
     * \param name 
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void release_savepoint(const CB& acb, const char* name)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        if(!this->in_transaction()){
            this->_strand->push_back(
                std::bind(cb, md::callback::cb_error("Not in a transaction!"))
            );
            return;
        }
        
        this->_conn->touch();
        std::string s("RELEASE SAVEPOINT ");
        s += name;
        this->execute(s.c_str(),
        [self=this->shared_from_this(), cb](const md::callback::cb_error& err){
            if(err){
                cb(err);
                return;
            }
            self->_conn->touch();
            cb(nullptr);
        });
    }
    
    /*!
     * \brief synchonously rollback the specified savepoint
     * 
     * \param name 
     */
    void rollback_savepoint(const char* name)
    {
        wait_for_sync();
        _conn->rollback_savepoint(name);
    }
    
    /*!
     * \brief asynchonously rollback the specified savepoint
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param acb 
     * \param name 
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void rollback_savepoint(const CB& acb, const char* name)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        if(!this->in_transaction()){
            this->_strand->push_back(
                std::bind(cb, md::callback::cb_error("Not in a transaction!"))
            );
            return;
        }
        
        this->_conn->touch();
        std::string s("ROLLBACK TO SAVEPOINT ");
        s += name;
        this->execute(s.c_str(),
        [self=this->shared_from_this(), cb](const md::callback::cb_error& err){
            if(err){
                cb(err);
                return;
            }
            self->_conn->touch();
            cb(nullptr);
        });
    }
    
    
    
    
    
    
    
    
    
    /*!
     * \brief asynchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(int) 
     * \param sql the SQL query to process
     * \param args query parameters, the last parameter is the query callback
     * pq_async::value_cb<int>
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(int)>
    void execute(const char* sql, const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_BODY_PARAMS(int, _process_execute_result, -1);
    }
    /*!
     * \brief asynchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, int) 
     * \param sql the SQL query to process
     * \param p query parameters
     * \param acb completion void(const md::callback::cb_error&, int) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, int)>
    void execute(const char* sql, const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_BODY_T(int, _process_execute_result, -1);
    }
    
    /*!
     * \brief synchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(int) 
     * \param sql the SQL query to process
     * \param args query parameters
     * \return int32_t the number of record processed
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(int)>
    int32_t execute(const char* sql, const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_execute_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \param sql the SQL query to process
     * \return int32_t the number of record processed
     */
    int32_t execute(const char* sql)
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_execute_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns the number of rows affected by insert, update and delete
     * 
     * \param sql the SQL query to process
     * \param p query parameters
     * \return int32_t the number of record processed
     */
    int32_t execute(const char* sql, const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_execute_result);
    }

    
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_table) 
     * \param sql the SQL query to process
     * \param args query parameters, the last parameter is 
     * the completion void(const md::callback::cb_error&, sp_data_table) callback
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_table)>
    void query(const char* sql, const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_BODY_PARAMS(
            sp_data_table, _process_query_result, sp_data_table()
        );
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_table) 
     * \param sql the SQL query to process
     * \param p query parameters
     * \param acb completion void(const md::callback::cb_error&, sp_data_table) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_table)>
    void query(const char* sql, const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_BODY_T(
            sp_data_table, _process_query_result, sp_data_table()
        );
    }
    
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_table) 
     * \param sql the SQL query to process
     * \param args query parameters
     * \return sp_data_table 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_table)>
    sp_data_table query(const char* sql, const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \param sql the SQL query to process
     * \return sp_data_table 
     */
    sp_data_table query(const char* sql)
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_table as the result
     * 
     * \param sql the SQL query to process
     * \param p query parameters
     * \return sp_data_table 
     */
    sp_data_table query(const char* sql, const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_result);
    }
    
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_row) 
     * \param sql the SQL query to process
     * \param args query parameters, the last parameter is
     * the completion void(const md::callback::cb_error&, sp_data_row) callback
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_row)>
    void query_single(const char* sql, const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_BODY_PARAMS(
            sp_data_row, _process_query_single_result, sp_data_row()
        );
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_row) 
     * \param sql the SQL query to process
     * \param p query parameters
     * \param acb the completion void(const md::callback::cb_error&, sp_data_row) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_row)>
    void query_single(const char* sql, const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_BODY_T(
            sp_data_row, _process_query_single_result, sp_data_row()
        );
    }
    
    
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_row) 
     * \param sql the SQL query to process
     * \param args query parameters
     * \return sp_data_row 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_row)>
    sp_data_row query_single(const char* sql, const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_single_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \param sql 
     * \return sp_data_row 
     */
    sp_data_row query_single(const char* sql)
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_single_result);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_row as the result
     * 
     * \param sql the SQL query to process
     * \param p query parameters
     * \return sp_data_row 
     */
    sp_data_row query_single(const char* sql, const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_single_result);
    }
    
    /*!
     * \brief asynchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R the result type
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(R) 
     * \param sql the SQL query to process
     * \param args query parameters, the last parameter is the query callback
     * pq_async::value_cb<R>
     */
    template<typename R, typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(R)>
    void query_value(const char* sql, const PARAMS&... args)
    {
        _PQ_ASYNC_SEND_QRY_BODY_PARAMS(
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
     * \param sql the SQL query to process
     * \param p query parameters
     * \param acb completion void(const md::callback::cb_error&, R) callback
     */
    template<
        typename R, typename T,
        PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, R)
    >
    void query_value(const char* sql, const parameters& p, const T& acb)
    {
        _PQ_ASYNC_SEND_QRY_BODY_T(
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
     * \param sql the SQL query to process
     * \param args query parameters
     * \return R the scalar value
     */
    template<typename R, typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(R)>
    R query_value(const char* sql, const PARAMS&... args)
    {
        parameters p(args...);
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_value_result<R>);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R 
     * \param sql the SQL query to process
     * \return R 
     */
    template<typename R>
    R query_value(const char* sql)
    {
        parameters p;
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_value_result<R>);
    }
    /*!
     * \brief synchrounously process a query
     * and returns a scalar value as the result
     * 
     * \tparam R 
     * \param sql the SQL query to process
     * \param p query parameters
     * \return R 
     */
    template<typename R>
    R query_value(const char* sql, const parameters& p)
    {
        _PQ_ASYNC_SEND_QRY_BODY_SYNC(_process_query_value_result<R>);
    }
    
    
    
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_reader) 
     * \param sql the SQL query to process
     * \param args query parameters, the last parameter is the query callback
     * pq_async::value_cb<sp_data_reader>
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_reader)>
    void query_reader(const char* sql, const PARAMS&... args)
    {
        parameters p;
        p.push_back<sizeof...(PARAMS) -1>(args...);
        
        md::callback::value_cb<sp_data_reader> cb;
        md::callback::assign_value_cb<
            md::callback::value_cb<sp_data_reader>,
            sp_data_reader
        >(
            cb, md::get_last(args...)
        );
        
        this->open_connection(
        [self=this->shared_from_this(),
            _sql = std::string(sql),_p = std::move(p),
            cb]
        (const md::callback::cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err, sp_data_reader());
                return;
            }
            
            try{
                auto ct = std::make_shared<reader_connection_task>(
                    self->_strand.get(), self, lock
                );
                cb(nullptr, std::shared_ptr<data_reader>(new data_reader(ct)));
                ct->send_query(_sql.c_str(), _p);
                self->_strand->push_back(ct);
                
            }catch(const std::exception& err){
                cb(md::callback::cb_error(err), sp_data_reader());
            }
        });
    }
    /*!
     * \brief asynchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \tparam T 
     * \tparam PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_reader) 
     * \param sql the SQL query to process
     * \param p query parameters
     * \param acb completion void(const md::callback::cb_error&, sp_data_reader) callback
     */
    template<typename T, PQ_ASYNC_VALID_DB_VAL_CALLBACK(T, sp_data_reader)>
    void query_reader(const char* sql, const parameters& p, const T& acb)
    {
        md::callback::value_cb<sp_data_reader> cb;
        md::callback::assign_value_cb<
            md::callback::value_cb<sp_data_reader>, sp_data_reader
        >(cb, acb);
        
        this->open_connection(
        [self=this->shared_from_this(),
            _sql = std::string(sql),_p = std::move(p),
            cb]
        (const md::callback::cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err, sp_data_reader());
                return;
            }
            
            try{
                auto ct = std::make_shared<reader_connection_task>(
                    self->_strand.get(), self, lock
                );
                cb(nullptr, std::shared_ptr<data_reader>(new data_reader(ct)));
                ct->send_query(_sql.c_str(), _p);
                self->_strand->push_back(ct);
                
            }catch(const std::exception& err){
                cb(md::callback::cb_error(err), sp_data_reader());
            }
        });
    }
    
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_reader) 
     * \param sql the SQL query to process
     * \param args query parameters
     * \return sp_data_reader 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_reader)>
    sp_data_reader query_reader(const char* sql, const PARAMS&... args)
    {
        this->wait_for_sync();
        auto lock = open_connection();
        parameters p(args...);
        auto ct = std::make_shared<reader_connection_task>(
            this->_strand.get(), this->shared_from_this(), lock
        );
        ct->send_query(sql, p);
        return std::shared_ptr<data_reader>(new data_reader(ct));
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \param sql the SQL query to process
     * \return sp_data_reader 
     */
    sp_data_reader query_reader(const char* sql)
    {
        this->wait_for_sync();
        auto lock = open_connection();
        parameters p;
        auto ct = std::make_shared<reader_connection_task>(
            this->_strand.get(), this->shared_from_this(), lock
        );
        ct->send_query(sql, p);
        return std::shared_ptr<data_reader>(new data_reader(ct));
    }
    /*!
     * \brief synchrounously process a query
     * and returns a pq_async::data_reader as the result
     * 
     * \param sql the SQL query to process
     * \param p query parameters
     * \return sp_data_reader 
     */
    sp_data_reader query_reader(const char* sql, const parameters& p)
    {
        this->wait_for_sync();
        auto lock = open_connection();
        auto ct = std::make_shared<reader_connection_task>(
            this->_strand.get(), this->shared_from_this(), lock
        );
        ct->send_query(sql, p);
        return std::shared_ptr<data_reader>(new data_reader(ct));
    }

    
    
    
    
    /*!
     * \brief synchronously parse and execute an sql script
     * 
     * \param sql script commands to execute
     */
    void exec_queries(const std::string& sql);
    /*!
     * \brief asynchronously parse and execute an sql script
     * 
     * \param sql script commands to execute
     * \param cb callback to call on completion or error
     * cb: void(const md::callback::cb_error& err)
     */
    void exec_queries(const std::string& sql, const md::callback::async_cb& cb);
    
    /*!
     * \brief Create a large object
     * 
     * \return sp_data_large_object 
     */
    sp_data_large_object create_lo();
    /*!
     * \brief Get the lo object
     * 
     * \param lo_oid 
     * \return sp_data_large_object 
     */
    sp_data_large_object get_lo(const pq_async::oid& lo_oid);
    
    
    
    
    
    
    
    
    /*!
     * \brief synchronously creates a new prepared statement 
     * and returns a data_prepared object
     * 
     * \param name the prepared statement name
     * \param sql the prepared statement query
     * \param auto_deallocate if the prepared statement shoul be delete on
     * \return sp_data_prepared 
     */
    sp_data_prepared prepare(
        const char* name, const char* sql, bool auto_deallocate)
    {
        return prepare(name, sql, false, std::vector<data_type>{});
    }
    
    /*!
     * \brief synchronously creates a new prepared statement 
     * and returns a data_prepared object
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_prepared) 
     * \param name the prepared statement name
     * \param sql the prepared statement query
     * \param auto_deallocate if the prepared statement shoul be delete on
     * \param args list of query parameter's data_type
     * \return sp_data_prepared 
     */
    template<typename... PARAMS, PQ_ASYNC_INVALID_DB_CALLBACK(sp_data_prepared)>
    sp_data_prepared prepare(
        const char* name, const char* sql, bool auto_deallocate,
        const PARAMS&... args)
    {
        std::vector<data_type> v;
        _build_prepared<sizeof...(PARAMS)>(v, args...);
        return prepare(name, sql, auto_deallocate, v);
    }
    
    /*!
     * \brief synchronously creates a new prepared statement 
     * and returns a data_prepared object
     * 
     * \tparam Container 
     * \tparam Args 
     * \param name the prepared statement name
     * \param sql the prepared statement query
     * \param auto_deallocate if the prepared statement shoul be delete on
     * data_prepared object destruction
     * \param types list of query parameter's data_type
     * \return sp_data_prepared 
     */
    template<template<typename ...> typename Container, typename ... Args>
    sp_data_prepared prepare(
        const char* name, const char* sql, bool auto_deallocate,
        const Container<pq_async::data_type, Args...>& types)
    {
        wait_for_sync();
        auto lock = open_connection();
        connection_task ct(
            this->_strand.get(), this->shared_from_this(), lock
        );
        
        if(this->query_value<bool>(
            "select exists"
            "(select 1 from pg_prepared_statements where name = $1)",
            name
        ))
            return this->_new_prepared(name, auto_deallocate, lock);
        
        ct.send_prepare(name, sql, types);
        return _process_send_prepare_result(
            name, auto_deallocate, lock, ct.run_now()
        );
    }
    
    
    /*!
     * \brief asynchronously creates a new prepared statement 
     * 
     * \param name the prepared statement name
     * \param sql the prepared statement query
     * \param auto_deallocate if the prepared statement shoul be delete on
     * \param acb completion void(const md::callback::cb_error&, sp_data_prepared) callback
     */
    void prepare(
        const char* name, const char* sql, bool auto_deallocate,
        md::callback::value_cb<sp_data_prepared> acb)
    {
        prepare(
            name, sql, auto_deallocate, std::vector<data_type>{}, acb
        );
    }
    
    /*!
     * \brief asynchronously creates a new prepared statement 
     * 
     * \tparam PARAMS 
     * \tparam PQ_ASYNC_VALID_DB_CALLBACK(sp_data_prepared) 
     * \param name the prepared statement name
     * \param sql the prepared statement query
     * \param auto_deallocate if the prepared statement shoul be delete on
     * \param args list of query parameter's data_type, the last argument is
     * the completion void(const md::callback::cb_error&, sp_data_prepared) callback
     */
    template<typename... PARAMS, PQ_ASYNC_VALID_DB_CALLBACK(sp_data_prepared)>
    void prepare(
        const char* name, const char* sql, bool auto_deallocate,
        const PARAMS&... args)
    {
        std::vector<data_type> t;
        _build_prepared<sizeof...(PARAMS) -1>(t, args...);
        
        md::callback::value_cb<sp_data_prepared> cb;
        md::callback::assign_value_cb<
            md::callback::value_cb<sp_data_prepared>,
            sp_data_prepared
        >(
            cb, md::get_last(args...)
        );
        
        prepare(name, sql, auto_deallocate, t, cb);
    }
    
    /*!
     * \brief asynchronously creates a new prepared statement 
     * 
     * \param name the prepared statement name
     * \param sql the prepared statement query
     * \param auto_deallocate if the prepared statement shoul be delete on
     * \param types list of query parameter's data_type
     * \param cb completion void(const md::callback::cb_error&, sp_data_prepared) callback
     */
    void prepare(
        const char* name, const char* sql, bool auto_deallocate,
        const std::vector<data_type>& types,
        md::callback::value_cb<sp_data_prepared> cb)
    {
        this->open_connection(
        [self=this->shared_from_this(),
            _name = std::string(name), _sql = std::string(sql),
            auto_deallocate, types, cb]
        (const md::callback::cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err, sp_data_prepared());
                return;
            }
            
            try{
                auto ct = std::make_shared<connection_task>(
                    self->_strand.get(), self, lock,
                [self, lock, _name, auto_deallocate, cb]
                (const md::callback::cb_error& err, PGresult* r)-> void {
                    if(err){
                        cb(err, sp_data_prepared());
                        return;
                    }
                    
                    try{
                        cb(
                            nullptr,
                            self->_process_send_prepare_result(
                                _name.c_str(), auto_deallocate, lock, r
                            )
                        );
                    }catch(const std::exception& err){
                        cb(md::callback::cb_error(err), sp_data_prepared());
                    }
                });
                ct->send_prepare(_name.c_str(), _sql.c_str(), types);
                self->_strand->push_back(ct);
                
            }catch(const std::exception& err){
                cb(md::callback::cb_error(err), sp_data_prepared());
            }
        });
    }
    
    /*!
     * \brief synchronously delete a prepared statement
     * 
     * \param name the prepared statement name
     */
    void deallocate_prepared(const char* name)
    {
        auto lock = this->open_connection();
        char* es_name = PQescapeIdentifier(
            this->_conn->_conn, name, strnlen(name, 255)
        );
        if(!es_name)
            throw pq_async::exception("Deallocate prepared invalid name!");
        std::string sql("DEALLOCATE PREPARE ");
        sql += es_name;
        PQfreemem(es_name);
        lock.reset();
        
        this->execute(sql.c_str());
    }
    
    /*!
     * \brief asynchronously delete a prepared statement
     * 
     * \tparam CB 
     * \tparam PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB) 
     * \param name the prepared statement name
     * \param acb completion void(const md::callback::cb_error&) callback
     */
    template< typename CB, PQ_ASYNC_VALID_DB_ASY_CALLBACK(CB)>
    void deallocate_prepared(const char* name, const CB& acb)
    {
        md::callback::async_cb cb;
        md::callback::assign_async_cb<md::callback::async_cb>(cb, acb);
        
        this->open_connection(
        [self=this->shared_from_this(),
            _name = std::string(name),
            cb]
        (const md::callback::cb_error& err, sp_connection_lock lock){
            if(err){
                cb(err);
                return;
            }
            
            char* es_name = PQescapeIdentifier(
                self->_conn->_conn, _name.c_str(), _name.size()
            );
            if(!es_name){
                cb(md::callback::cb_error(
                    pq_async::exception("Deallocate prepared invalid name!")
                ));
            }
            std::string sql = "DEALLOCATE PREPARE ";
            sql += es_name;
            PQfreemem(es_name);
            lock.reset();
            
            self->execute(sql.c_str(), 
            [self, cb](const md::callback::cb_error& err){
                if(err){
                    cb(err);
                    return;
                }
                
                cb(nullptr);
            });
            
        });
    }
    
    
    
    /*!
     * \brief creates a new database instance using a new strand from 
     * the default event_queue
     * 
     * \param connection_string the connection string used for that instance
     * \return sp_database 
     */
    static sp_database open(
        const std::string& connection_string,
        md::log::sp_logger log = nullptr)
    {
        sp_database dbso(
            new database(
                md::event_queue::get_default()->new_strand<int>(),
                connection_string,
                log
            )
        );
        return dbso;
    }
    
    /*!
     * \brief creates a new database instance usign the specified event_strand
     * 
     * \param strand the pq_async::event_strand to use
     * \param connection_string the connection string used for that instance
     * \return sp_database 
     */
    static sp_database open(
        md::sp_event_strand<int> strand, 
        const std::string& connection_string,
        md::log::sp_logger log = nullptr)
    {
        sp_database dbso(
            new database(strand, connection_string, log)
        );
        return dbso;
    }
    
    
    static void split_queries(
        const std::string& sql, std::vector< std::string >& queries
    );
    
    
private:
    sp_data_prepared _new_prepared(
        const char* name, bool auto_deallocate,
        sp_connection_lock lock
    );


    template<typename... PARAMS>
    void _build_prepared(std::vector<data_type>& v, const PARAMS& ...args)
    {
        _build_prepared<sizeof...(PARAMS)>(v, args...);
    }
    
    template<int SIZE = 0,
        typename std::enable_if<(SIZE == 0), int32_t>::type = 0
    >
    void _build_prepared(std::vector<data_type>& v) {}
    
    template<int SIZE, typename T, typename... PARAMS,
        typename std::enable_if<(SIZE == 0), int32_t>::type = 0
    >
    void _build_prepared(
        std::vector<data_type>& v,
        const T& /*value*/, const PARAMS& .../*args*/
    ){}
    
    template<int SIZE, typename T, typename... PARAMS,
        typename std::enable_if<(SIZE > 0), int32_t>::type = 0
    >
    void _build_prepared(
        std::vector<data_type>& v, const T& t, const PARAMS&... args)
    {
        v.emplace_back(t);
        _build_prepared<SIZE -1>(v, args...);
    }
    
    void wait_for_sync()
    {
        while(this->working() && this->_strand->size() > 0){
            this->_strand->run_n();
            usleep(10);
        }
    }
    
    int _process_execute_result(PGresult* res);
    sp_data_table _process_query_result(PGresult* res);
    sp_data_row _process_query_single_result(PGresult* res);
    sp_data_prepared _process_send_prepare_result(
        const std::string& name, bool auto_deallocate,
        sp_connection_lock lock, PGresult* res
    );
    
    template<typename RETURN_T>
    RETURN_T _process_query_value_result(PGresult* res)
    {
        PQ_ASYNC_TRACE(_log, "");
        
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
        
        if(result_status == PGRES_EMPTY_QUERY){
            PQclear(res);
            throw pq_async::exception("No records in the query result!");
        }
        
        if(result_status == PGRES_NONFATAL_ERROR){
            PQclear(res);
            throw pq_async::exception("Non fatal error has occured!");
        }
        
        int oid = PQftype(res, 0);
        char* val = PQgetisnull(res, 0, 0) ? NULL : PQgetvalue(res, 0, 0);
        int len = val == NULL ? -1 : PQgetlength(res, 0, 0);
        int format = PQfformat(res, 0);
        
        RETURN_T value = val_from_pgparam<RETURN_T>(oid, val, len, format);
        
        PQclear(res);
        
        return value;
    }
    
    
    std::string _connection_string;
    
    connection* _conn;
    md::sp_event_strand<int> _strand;
    sp_connection_lock _lock;
    md::log::sp_logger _log;
};


#undef _PQ_ASYNC_SEND_QRY_BODY_PARAMS
#undef _PQ_ASYNC_SEND_QRY_BODY_T
#undef _PQ_ASYNC_SEND_QRY_BODY_SYNC

} // ns: pq_async
#endif //_libpq_async_database_h
