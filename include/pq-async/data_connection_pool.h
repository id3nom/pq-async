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

#ifndef _libpq_async_connection_pool_h
#define _libpq_async_connection_pool_h


#include "data_common.h"

namespace pq_async{

class connection_lock;
typedef std::shared_ptr<pq_async::connection_lock> sp_connection_lock;

class connection_task;
class connection_pool;
class database;

typedef std::shared_ptr< pq_async::connection_task > sp_connection_task;


class connection
{
    friend class connection_pool;
    friend class connection_lock;
    friend class database;
    
private:
    connection(connection_pool* pool, std::string connection_string
        ): _res(0), _running(0), _id(md::num_to_str(++s_next_id)),
        _pool(pool),_connection_string(connection_string), 
        is_in_transaction(false), 
        _conn(NULL), _sock_fd(-1),
        _owner(NULL),
        _last_modification_date(std::chrono::system_clock::now())
    {
    }
    
    ~connection()
    {
        close_connection();
        _owner = NULL;
    }
    
    void set_notice_processor();
    
public:
    
    const std::string& id(){return _id;}

    PGconn* conn(){return _conn;};
    
    bool is_opened()
    {
        return _conn && PQstatus(_conn) == CONNECTION_OK;
    }
    
    bool lock();
    void release();
    
    md::sp_event_strand<int> strand();
    
    void touch()
    {
        _last_modification_date = std::chrono::system_clock::now();
    }
    
    bool is_dead();
        
    void open_connection()
    {
        touch();
        
        if(_conn != NULL && PQstatus(_conn) != CONNECTION_OK)
            close_connection();

        if(_conn != NULL)
            return;

        _conn = PQconnectdb(_connection_string.c_str());
        
        if(PQstatus(_conn) == CONNECTION_OK){
            // set notice processor
            set_notice_processor();
            
            // enable non blocking mode
            if(PQsetnonblocking(_conn, 1)){
                throw pq_async::exception(
                    "Unable to set connection to non blocking"
                );
            }
            
            _sock_fd = PQsocket(_conn);
            return;
        }
        
        std::string errMsg = PQerrorMessage(_conn);
        PQfinish(_conn);
        _conn = NULL;

        throw pq_async::exception(errMsg);
    }

    void close_connection()
    {
        touch();

        if(_conn == NULL)
            return;
    
        PQfinish(_conn);
        _conn = NULL;
        is_in_transaction.store(false);
    }
    
    void reserve(){
        _running.store(2);
        touch();
    }
    
    void start_work(){
        if(_running.load() == 0 || _running.load() == 2)
            _running.store(1);
        touch();
    }
    
    void stop_work(){
        _running.store(0);
        touch();
    }
    
    bool running(){ return _running.load() != 0;}
    
    bool can_be_stolen(){
        return !is_in_transaction.load() && _running.load() == 0;
    }
    
    bool in_transaction(){ return is_in_transaction.load();}
    
    void begin_transaction()
    {
        if(is_in_transaction.load())
            throw pq_async::exception("Already in a transaction!");
        
        open_connection();
        
        PGresult* res = PQexec(_conn, "BEGIN");
        if(PQresultStatus(res) == PGRES_COMMAND_OK){
            is_in_transaction.store(true);
            PQclear(res);
            return;
        }
        
        std::string errMsg = PQerrorMessage(_conn);
        PQclear(res);
        throw pq_async::exception(errMsg);
    }
    
    void commit_transaction()
    {
        touch();
        
        if(!is_in_transaction.load())
            throw pq_async::exception("Not in a transaction!");
        
        PGresult* res = PQexec(_conn, "COMMIT");
        if(PQresultStatus(res) == PGRES_COMMAND_OK){
            is_in_transaction.store(false);
            PQclear(res);
            return;
        }
        
        std::string errMsg = PQerrorMessage(_conn);
        PQclear(res);
        throw pq_async::exception(errMsg);
    }
    
    
    void rollback_transaction()
    {
        touch();
        
        if(!is_in_transaction.load())	
            throw pq_async::exception("Not in a transaction!");
        
        PGresult* res = PQexec(_conn, "ROLLBACK");
        if(PQresultStatus(res) == PGRES_COMMAND_OK){
            is_in_transaction.store(false);
            PQclear(res);
            return;
        }

        std::string errMsg = PQerrorMessage(_conn);
        PQclear(res);
        throw pq_async::exception(errMsg);
    }



    void set_savepoint(const char* savepoint_name)
    {
        touch();
        
        if(!is_in_transaction.load())
            throw pq_async::exception("Not in a transaction!");
        
        std::string sql("SAVEPOINT ");
        sql += savepoint_name;
        PGresult* res = PQexec(_conn, sql.c_str());
        if(PQresultStatus(res) == PGRES_COMMAND_OK){
            PQclear(res);
            return;
        }

        std::string errMsg = PQerrorMessage(_conn);
        PQclear(res);
        throw pq_async::exception(errMsg);
    }

    void release_savepoint(const char* savepoint_name)
    {
        touch();
        
        if(!is_in_transaction.load())
            throw pq_async::exception("Not in a transaction!");
        
        std::string sql("RELEASE SAVEPOINT ");
        sql += savepoint_name;
        PGresult* res = PQexec(_conn, sql.c_str());
        if(PQresultStatus(res) == PGRES_COMMAND_OK){
            PQclear(res);
            return;
        }

        std::string errMsg = PQerrorMessage(_conn);
        PQclear(res);
        throw pq_async::exception(errMsg);
    }
    
    void rollback_savepoint(const char* savepoint_name)
    {
        touch();
        
        if(!is_in_transaction.load())
            throw pq_async::exception("Not in a transaction!");
        
        std::string sql("ROLLBACK TO SAVEPOINT ");
        sql += savepoint_name;
        PGresult* res = PQexec(_conn, sql.c_str());
        if(PQresultStatus(res) == PGRES_COMMAND_OK){
            PQclear(res);
            return;
        }
    }

private:
    
    static std::atomic<int> s_next_id;
    
    std::atomic<int> _res;
    std::atomic<int> _running;
    
    std::string _id;
    
    connection_pool* _pool;
    std::string _connection_string;
    
    std::atomic<bool> is_in_transaction;
    PGconn* _conn;
    int _sock_fd;
    
    database* _owner;
    
    std::chrono::system_clock::time_point _last_modification_date;
};

class connection_lock
{
public:
    connection_lock(connection* conn):_conn(conn)
    {
        if(!_conn)
            return;
        
        if(_conn->_running.load() == 1){
            pq_async::default_logger()->error(
                "unable to lock the connection because it's already locked"
            );
            
            std::string err_msg(
                "unable to lock the connection because it's already locked"
            );
            throw pq_async::exception(err_msg);
        }
        
        _conn->start_work();
    }
    
    ~connection_lock()
    {
        if(!_conn)
            return;
        
        _conn->stop_work();
    }
    
    const connection* conn(){return _conn;}
    
private:
    connection* _conn;

};

class connection_task
    : public md::event_task_base, 
    public std::enable_shared_from_this< connection_task >
{
    friend data_reader;
protected:
    enum class command_type
    {
        none = 0,
        connect = 1,
        query = 2,
        prepare = 3,
        query_prepared = 4,
        cancel = 5,
        sent = 6,
    };
    
public:
    connection_task(
        md::event_queue* owner, sp_database db, sp_connection_lock lock,
        const md::callback::value_cb<PGresult*>& cb
    );
    
    connection_task(
        md::event_queue* owner, sp_database db, connection* conn,
        const md::callback::value_cb<sp_connection_lock>& cb
    );
    
    connection_task(
        md::event_queue* owner, sp_database db, sp_connection_lock lock
    );
    connection_task(md::event_queue* owner, sp_database db, connection* conn);
    
    sp_database db(){ return _db;}
    
    void connect(const std::string& connection_string, int32_t timeout_ms)
    {
        _cmd_type = command_type::connect;
        _sql = connection_string;
        
        std::chrono::system_clock::time_point
            timeout_date(
                std::chrono::system_clock::now() +
                std::chrono::milliseconds(timeout_ms)
            );
        _format = timeout_date.time_since_epoch().count();
    }

    void send_query(const char* sql, const parameters& p,
        int format = PG_BIN_FORMAT)
    {
        _cmd_type = command_type::query;
        _sql = sql;
        _p = p;
        _format = format;
    }
    
    void send_query(const char* sql, parameters& p, int format = PG_BIN_FORMAT)
    {
        _cmd_type = command_type::query;
        _sql = sql;
        _p = std::move(p);
        _format = format;
    }
    
    void send_prepare(
        const char* name, const char* sql, const std::vector<data_type>& t)
    {
        _cmd_type = command_type::prepare;
        _name = name;
        _sql = sql;
        _t = t;
    }
    
    void send_query_prepared(
        const char* name, parameters& p, int format = PG_BIN_FORMAT)
    {
        _cmd_type = command_type::query_prepared;
        _name = name;
        _p = std::move(p);
        _format = format;
    }
    void send_query_prepared(
        const char* name, const parameters& p, int format = PG_BIN_FORMAT)
    {
        _cmd_type = command_type::query_prepared;
        _name = name;
        _p = p;
        _format = format;
    }
    
    void cancel()
    {
        if(_cmd_type != command_type::sent || _completed)
            throw pq_async::exception("No command in progress!");
        
        _cmd_type = command_type::cancel;
    }
    
    virtual PGresult* run_now()
    {
        if(_cmd_type == command_type::none)
            return nullptr;
        
        if(_cmd_type != command_type::sent){
            switch(_cmd_type){
                case command_type::connect:
                    _connect();
                    break;
                case command_type::query:
                    _send_query();
                    break;
                case command_type::prepare:
                    _send_prepare();
                    break;
                    
                case command_type::query_prepared:
                    _send_query_prepared();
                    break;
                    
                default:
                    break;
            }
            _cmd_type = command_type::sent;
        }
        
        PGresult* last = nullptr;
        while(PGresult* r = PQgetResult(this->conn())){
            if(last)
                PQclear(last);
            last = r;
        }
        _completed = true;
        
        return last;
    }
    
    virtual void run_task()
    {
        if(!_cb && !_lock_cb)
            return;
        
        try{
            if(_cmd_type == command_type::none)
                return;
            
            if(_cmd_type != command_type::sent){
                // start waiting for IO
                
                switch(_cmd_type){
                    case command_type::connect:
                        _connect();
                        return;
                    case command_type::query:
                        _send_query();
                        break;
                    case command_type::prepare:
                        _send_prepare();
                        break;
                        
                    case command_type::query_prepared:
                        _send_query_prepared();
                        break;
                        
                    case command_type::cancel:
                        _cancel_command();
                        break;

                    default:
                        break;
                }
                _cmd_type = command_type::sent;
                return;
            }
            
            if(!this->_consume_data())
                return;
            _completed = true;
            while(PGresult* r = PQgetResult(this->conn())){
                _cb(nullptr, r);
            }
        }catch(const std::exception& err){
            _cb(md::callback::cb_error(err), nullptr);
        }
    }
    
    virtual md::event_requeue_pos requeue()
    {
        if(_completed)
            return md::event_requeue_pos::none;
        return md::event_requeue_pos::front;
    }
    virtual size_t size() const
    {
        return 0;
    }
    
    
    PGconn* conn();
    
protected:
    
    void _connect();
    
    void _send_query()
    {
        PQ_ASYNC_DEF_DBG(
            "sending query: {}\nct: {:p}, db: {:p}",
            _sql, (void*)this, (void*)_db.get()
        );
        
        if(PQsendQueryParams(
            this->conn(), _sql.c_str(), _p.size(), _p.types(), 
            _p.values(), _p.lengths(), _p.formats(), 
            _format
        ))
            return;
        std::string errMsg = PQerrorMessage(this->conn());
        throw pq_async::exception(errMsg);
    }
    
    void _send_prepare()
    {
        PQ_ASYNC_DEF_DBG(
            "sending prepare query, name: {}, sql: {}\nct: {:p}, db: {:p}",
            _name, _sql, (void*)this, (void*)_db.get()
        );
        
        Oid* _types = new Oid[_t.size()];
        for(size_t i = 0; i < _t.size(); ++i)
            _types[i] = (Oid)_t[i];
        
        if(PQsendPrepare(
            this->conn(), _name.c_str(), _sql.c_str(), _t.size(), _types
        )){
            delete[] _types;
            return;
        }
        delete[] _types;
        std::string errMsg = PQerrorMessage(this->conn());
        throw pq_async::exception(errMsg);
    }
    
    void _send_query_prepared()
    {
        PQ_ASYNC_DEF_DBG(
            "sending query prepared: {}\nct: {:p}, db: {:p}",
            _name, (void*)this, (void*)_db.get()
        );
        
        if(PQsendQueryPrepared(
            this->conn(), _name.c_str(), _p.size(),
            _p.values(), _p.lengths(), _p.formats(), 
            _format
        ))
            return;
        std::string errMsg = PQerrorMessage(this->conn());
        throw pq_async::exception(errMsg);
    }
    
    void _cancel_command()
    {
        PQ_ASYNC_DEF_DBG(
            "canceling current command\nct: {:p}, db: {:p}",
            (void*)this, (void*)_db.get()
        );
        
        char errbuf[256];
        PGcancel* c = PQgetCancel(this->conn());
        if(c){
            if(!PQcancel(c, errbuf, 256))
                throw pq_async::exception(errbuf);
        }
    }
    
    bool _consume_data()
    {
        if(!PQconsumeInput(this->conn()))
            throw pq_async::exception("Unable to consume input data");
        
        #if PQ_ASYNC_BUILD_DEBUG == 1
        bool busy = !PQisBusy(this->conn());
        PQ_ASYNC_DEF_TRACE(
            "is busy: {}\nct: {:p}, db: {:p}",
            busy ? "false" : "true", (void*)this, (void*)_db.get()
        );
        return busy;
        #else
            return !PQisBusy(this->conn());
        #endif
    }
    
    command_type _cmd_type;
    
    std::string _name;
    std::string _sql;
    parameters _p;
    std::vector<data_type> _t;
    int64_t _format;
    
    bool _completed;
    sp_database _db;

    connection* _conn;
    md::callback::value_cb<sp_connection_lock> _lock_cb;

    sp_connection_lock _lock;
    md::callback::value_cb<PGresult*> _cb;
};

class reader_connection_task
    : public connection_task
{
public:
    reader_connection_task(
        md::event_queue* owner, sp_database db, sp_connection_lock lock
    );
    
    virtual PGresult* run_now()
    {
        if(_cmd_type == command_type::none)
            return nullptr;
        
        if(_cmd_type != command_type::sent){
            switch(_cmd_type){
                case command_type::query:
                    _send_query();
                    break;
                case command_type::prepare:
                    _send_prepare();
                    break;
                    
                case command_type::query_prepared:
                    _send_query_prepared();
                    break;
                default:
                    break;
            }
            
            if(!PQsetSingleRowMode(this->conn())){
                std::string errMsg = PQerrorMessage(this->conn());
                throw pq_async::exception(errMsg);
            }
            
            _cmd_type = command_type::sent;
        }
        
        PGresult* r = PQgetResult(this->conn());
        if(!r)
            _completed = true;
        
        return r;
    }
    
    virtual void run_task()
    {
        try{
            if(_cmd_type == command_type::none)
                return;
            
            if(_cmd_type != command_type::sent){
                // start waiting for IO
                
                
                switch(_cmd_type){
                    case command_type::query:
                        _send_query();
                        break;
                    case command_type::prepare:
                        _send_prepare();
                        break;
                        
                    case command_type::query_prepared:
                        _send_query_prepared();
                        break;
                    default:
                        break;
                }
                
                if(_cmd_type != command_type::cancel){
                    if(!PQsetSingleRowMode(this->conn())){
                        std::string errMsg = PQerrorMessage(this->conn());
                        throw pq_async::exception(errMsg);
                    }
                }
                
                _cmd_type = command_type::sent;
                return;
            }
            
            if(!this->_consume_data())
                return;
            
            PGresult* r = PQgetResult(this->conn());
            if(!r)
                _completed = true;
            _cb(nullptr, r);
            
        }catch(const std::exception& err){
            _cb(md::callback::cb_error(err), nullptr);
        }
    }

};

class connection_pool
{
    //friend class connection;
private:
    connection_pool()
        : _max_conn(DEFAULT_CONNECTION_POOL_MAX_CONN)
    {
    }

    connection_pool(int max_connection_pool_count)
        : _max_conn(max_connection_pool_count)
    {
    }

    connection* _get_connection(
        database* owner, const std::string& connection_string,
        int32_t timeout_ms
    );
    int32_t _get_opened_connection_count(const std::string& connection_string);

public:
    
    virtual ~connection_pool();
    
    static void init(bool init_ssl, bool init_crypto);
    static void init(
        int max_connection_pool_count, bool init_ssl, bool init_crypto
    );
    
    static void destroy()
    {
        if(s_instance)
            delete s_instance;
        s_init = false;
    }
    
    static connection_pool* instance(){ return s_instance;}
    
    static int get_max_conn(){ return instance()->_max_conn;}
    static connection* get_connection(
        pq_async::database* owner, const std::string& connection_string,
        int32_t timeout_ms = 5000
        )
    {
        return instance()->_get_connection(
            owner, connection_string, timeout_ms
        );
    }
    static int32_t get_opened_connection_count(
        const std::string& connection_string)
    { 
        return instance()->_get_opened_connection_count(connection_string);
    }
    
    #ifdef PQ_ASYNC_THREAD_SAFE
    template<typename Lock>
    static void wait(Lock& lock){ instance()->cv.wait(lock);}
    static void notify_one(){ instance()->cv.notify_one();}
    static void notify_all(){ instance()->cv.notify_all();}
    #endif
    
private:
    static connection_pool* s_instance;
    static bool s_init;
    
    static std::string last_stolen_conn_id;
    
public:
    #ifdef PQ_ASYNC_THREAD_SAFE
    std::recursive_mutex conn_pool_mutex;
    std::condition_variable_any cv;
    #endif
    
private:

    int _max_conn;
    std::map< std::string, std::vector< connection* >* > _pools;
};

} //namespace pq_async

#endif //_libpq_async_connection_pool_h
