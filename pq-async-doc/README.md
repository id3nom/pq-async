[TOC]

# About pq-async

This library is a c++ wrapper for the PostgreSQL libpq 'C' library.

It facilitate the usage of the library by the use of variadic templates,

By default, all data are fetch in binary format, which in theory should be
faster than text format, for both transfer and convertion of the data,
performance tests are on the way to validate that theory.

The other purpose of that library is to use the non-blocking IO 
of the libpq library,
allowing developpers to use it with any event base library e.g.: 
libevent, libev, libuv, boost::asio and so on.

## Notice

The project is in it's alpha phase and bugs can still be present,

It havn't been tested on any other distro than Debian Stretch.

So if you want to use that lib do it at your own risk as long that you
respect the MIT license.

## Configuration

### Required development libraries

#### Debian based distribution:

~~~{.sh}
$ sudo apt-get install libpq-dev postgresql-server-dev-all \
libcurl4-gnutls-dev libboost-dev uuid-dev
~~~

#### Required dependencies

- CMake 3.0.0 and up.
- A compiler supporting, at least, c++14.
- [The boost::multi_array library](https://www.boost.org/doc/libs/1_62_0/libs/multi_array/doc/user.html).
- The date library from 
Howard Hinnant @ https://github.com/HowardHinnant/date 


to download the required libraries you must run the following command after cloning
this repo:
~~~{.sh}
$ git submodule update --init --recursive
~~~

## Testing requirements:

It is required to have a postgresql database available for testing program
to work.

~~~{.sql}
-- e.g.: To create a testing database named libpq_async_tests, 
-- login to postgresql as superuser and execute the following queries.
postgres=# create role the_username with login;
--CREATE ROLE
postgres=# create database libpq_async_tests with owner the_username;
--CREATE DATABASE
postgres=# \c libpq_async_test
--You are now connected to database "libpq_async_tests" as user "postgres".
libpq_async_tests=# create extension ltree;
--CREATE EXTENSION
~~~

### Testing command

~~~{.sh}
./pq-async_tests --db "dbname='libpq_async_tests' user='...'" --pool-size 4
~~~

## Limitations

* Some PostgreSQL data types do not provide binary output function, 
for example if you try to select an ltree column in binary format PostgreSQL
will throw the following exception: 
'ERROR:  no binary output function available for type ltree'.

* Composite Types are not supported at the moment.

* Is only tested on linux, other platforms support 
may be added in future release.

**Note that one way to use unsupported type, is to cast it as a supported type like 'text' for single value type and 'jsonb' for more complex one.**

# Library usage

## initialisation and cleanup

First the library's pq_async::connection_pool must be initialized, 
by calling one of the two available init function,
by default the maximum connection count is 20.

~~~{.cpp}
// to init with default connection count use that signature
bool do_ssl = true;
bool do_crypto = true;
pq_async::connection_pool::init(do_ssl, do_crypto);
// to init with specific connection count use that one
int max_connection_pool_count = 5;
pq_async::connection_pool::init(
    max_connection_pool_count, do_ssl, do_crypto
);
~~~

For more info on the "do_ssl" and "do_crypto" arguments
see https://www.postgresql.org/docs/current/libpq-ssl.html#LIBPQ-SSL-INITIALIZE


On program exit call the "destroy" static function to cleanup 
"connection_pool" ressources
~~~{.cpp}
pq_async::connection_pool::destroy();
~~~


## The database

To work with postgresql, an instance of the pq_async::database object is required.

Note: this call will never block because the connection is not open on
"database" instantiation, to open the connection you can call "open" function.
~~~{.cpp}
// to obtain an instance, call the static function "open"
auto db = pq_async::database::open("connection string");

// to test if the connection is valid you can force the connection to open with
// "open" function of the instance, please note that the open call will block
// until the connection is opened or an error occured.
db->open();

// to prevent blocking you can call the open async function.
auto cb = [](cb_error& err){
    // will be called on connection or error.
};
db->open(async_cb);

~~~

## Synchronous Mode



### Synchronous Queries

Queries can by any valid SQL query, all query functions are variadic template
which accept any supported type of parameters
(see Supported Types list below).

~~~{.cpp}
// to execute a statement without results
std::string sql("insert into tbl_name(col_id_a, col_str_b) values ($1, $2)");
auto n = db->execute(sql, 10, "some text value");
// "n" is the number of record processed with the query

// to fetch a single value
std::string sql("select col_id_a form tbl_name where col_str_b = $1");
auto id_a = db->query_value<int32_t>(sql, "some text value");

// to fetch a single row
// if multiple records are returned from PostgreSQL, only the first one is used
std::string sql("select * form tbl_name where col_str_b = $1 limit 1");
auto row = db->query_single(sql, "some text value");

// to fetch a table
std::string sql("select * form tbl_name where id > $1");
auto tbl = db->query(sql, 3);

// to create a data reader
std::string sql("select * form tbl_name where id > $1");
auto reader = db->query_reader(sql, 3);

~~~

### Synchronous Data Reader


~~~{.cpp}

// to create a data reader
std::string sql("select * form tbl_name where id > $1");
auto reader = db->query_reader(sql, 3);

while(auto row = reader->next()){
    // do row stuff ...
}

// if you need to explicitly close the "data_reader" 
// (even if it close itself on destruction) you can call "close()"
reader->close();

~~~

### Synchronous Transactions and Savepoints

For more information on transactions go to:
https://www.postgresql.org/docs/11/sql-begin.html

And for more information on savepoints go to:
https://www.postgresql.org/docs/current/sql-savepoint.html

~~~{.cpp}
// to start a transaction use the "begin" function
db->begin();
// the transaction_mode can be specified as the first argument
db->begin(pq_async::transaction_mode::serializable);

// to discard all updates made after the begining of the transaction
// use the "rollback" function
db->rollback();

// make all changes visible to others and garanteed 
// to be durable if a crash occurs, call the "commit" function
db->commit();

// to create a savepoint call "set_savepoint" function
db->set_savepoint("my savepoint name");
// to rollback a savepoint call "rollback_savepoint"
db->rollback_savepoint("my savepoint name");
// release a savepoint call "release_savepoint"
db->release_savepoint("my savepoint name");

~~~


### Synchronous Prepared Statements

~~~{.cpp}
// to create a prepared statement
std::string name("unique statement name");
std::string statement("select $1");
bool auto_deallocate = false;
auto ps = db->prepare(name, statement, auto_deallocate);

// to get a previously prepared statement
bool auto_deallocate = false;
auto ps = db->prepare("name", auto_deallocate);

// to destroy the prepared statement
db->deallocate("name");

// exec functions
auto n = ps->execute(-1);
auto i = ps->query_value<int32_t>(-1);
auto r = ps->query_single(-1);
auto t = ps->query(-1);
auto dr = ps->query_reader(-1);

auto params = new pq_async::parameters(-1);
auto n = ps->execute(-1);
auto i = ps->query_value<int32_t>(params);
auto r = ps->query_single(params);
auto t = ps->query(params);
auto dr = ps->query_reader(params);

~~~

### Synchronous Large Object

To get more info on large objects
https://www.postgresql.org/docs/current/largeobjects.html

~~~{.cpp}

// to create a new large_object
auto lo = db->create_lo();
// the oid() function can be used to retrive the large object id and stored it
auto lo_oid = lo->oid();

// to get an existing large_object
auto lo = db->get_lo(lo_oid);

// to open the large_object for reading, writing or both
pq_async::lo_mode m = pq_async::lo_mode::read | pq_async::lo_mode::write;
if(!lo->is_opened())
    lo->open(lo_oid, m);

// to read from the lo
int nb = 1024;
char* buf[1024];
// if file is end is reached "rb" will be less then "nb"
int32_t rb = lo->read(buf, nb);

// to write to the lo use
int nb = 1024;
char* buf[1024];
int32_t wb = lo->write(buf, nb);

// to get the read write cursor location
int64_t cur_pos = lo->tell();

// to set the read write cursor location
int64_t offset = 0x5ca1ab1e;
pq_async::lo_whence w = lo_whence::seek_start;
int64_t new_pos = lo->seek(offset, w);

// to resize the lo
int64_t new_size = 0xacce1e28;
pq_async::lo_whence w = lo_whence::seek_start;
int64_t new_pos = lo->seek(new_size);

// to close the lo
lo->close();

// to delete the lo
lo->unlink();

~~~

## Asynchronous Mode

To learn more about async mode look at the code in
./tests/queue_tests/queue_test.cpp ,
./tests/db_tests/database_test.cpp and
./tests/db_tests/data_reader_test.cpp

To run async queries you need to call the run function from the event_queue
e.g.:

~~~{.cpp}
pq_async::event_queue::get_default()->run();
~~~

this call will block until all work is done, and should be put into
you main program loop.

if you prefer you can call "run_n(max_number_of_tasks)", this will only call
block for the number of task requested.

in next release there will be a function called "run_t(max_timeout_ms)"
this call will block for the specified duration, note that if 10ms is specified
and the next call takes 1 second to complete the call to "run_t" will block
1 second.

### Asynchronous queries

~~~{.cpp}

// to execute a statement without results
std::string sql("insert into tbl_name(col_id_a, col_str_b) values ($1, $2)");
db->execute(sql, 10, "some text value",
[](const md::callback::cb_error& err, int n){
    if(err){
        //...
    }
    // "n" is the number of record processed with the query
});

// to fetch a single value
std::string sql("select col_id_a form tbl_name where col_str_b = $1");
db->query_value<int32_t>(sql, "some text value",
[](const md::callback::cb_error& err, int32_t id_a){
    if(err){
        //...
    }
});

// to fetch a single row
// if multiple records are returned from PostgreSQL, only the first one is used
std::string sql("select * form tbl_name where col_str_b = $1 limit 1");
db->query_single(sql, "some text value",
[](const md::callback::cb_error& err, sp_data_row row){
    if(err){
        //...
    }
    // use the row...
});

// to fetch a table
std::string sql("select * form tbl_name where id > $1");
db->query(sql, 3,
[](const md::callback::cb_error& err, sp_data_table tbl){
    if(err){
        //...
    }
    // use the table...
});

// to create a data reader
std::string sql("select * form tbl_name where id > $1");
db->query_reader(sql, 3,
[](const md::callback::cb_error& err, sp_data_reader reader){
    if(err){
        //...
    }
    // use the reader...
});

~~~


### Asynchronous Data Reader

~~~{.cpp}

// to create a data reader
std::string sql("select * form tbl_name where id > $1");
db->query_reader(sql, 3,
[](const md::callback::cb_error& err, sp_data_reader reader){
    if(err){
        //...
    }
    // use the reader...
    reader->next([scb, reader]
    (const md::callback::cb_error& err, sp_data_row r){	// this callback will be called
        if(err){							// until there is no more row to
            scb(err);						// be processed
            return;
        }
        
        if(!r){
            // if r is empty the reader is closed
            return;
        }
        
        // if you need to explicitly close the "data_reader" 
        // (even if it close itself on destruction) you can call "close()"
        reader->close(); 	// NOTE that after this call the callback 
                            // will be called one more time with an empty row.
        
        // use the data row
        std::string v = *(*r)["value"];
        std::cout
            << "id: " << r->as_int64("id")
            << ", value: " << v << std::endl;
    });
});


~~~

### Asynchronous Transactions and Savepoints

~~~{.cpp}

For more information on transactions go to:
https://www.postgresql.org/docs/11/sql-begin.html

And for more information on savepoints go to:
https://www.postgresql.org/docs/current/sql-savepoint.html

~~~{.cpp}
// to start a transaction use the "begin" function
db->begin([](const md::callback::cb_error& err){
    if(err){
        // ...
    }
    
});

// will be implement in a future version
// the transaction_mode can be specified as the first argument
//db->begin(pq_async::transaction_mode::serializable);

// to discard all updates made after the begining of the transaction
// use the "rollback" function
db->rollback([](const md::callback::cb_error& err){
    if(err){
        // ...
    }
    
});

// make all changes visible to others and garanteed 
// to be durable if a crash occurs, call the "commit" function
db->commit([](const md::callback::cb_error& err){
    if(err){
        // ...
    }
    
});

// to create a savepoint call "set_savepoint" function
db->set_savepoint("my savepoint name", [](const md::callback::cb_error& err){
    if(err){
        // ...
    }
    
});

// to rollback a savepoint call "rollback_savepoint"
db->rollback_savepoint("my savepoint name", [](const md::callback::cb_error& err){
    if(err){
        // ...
    }
    
});

// release a savepoint call "release_savepoint"
db->release_savepoint("my savepoint name", [](const md::callback::cb_error& err){
    if(err){
        // ...
    }
    
});

~~~



### Asynchronous Prepared Statements


~~~{.cpp}



~~~


## Table

The pq_async::data_table class is the representation of multiple records.

~~~{.cpp}



~~~

## Row

The pq_async::data_row class is the representation of a record.

~~~{.cpp}
// given the following query to fetch a single row
std::string sql("select * form tbl_name where col_str_b = $1 limit 1");
auto row = db->query_single(sql, "some text value");

// here is multiple ways to retrieve values from the row

// with columns name
auto id = row->as<int64_t>("col_id_a");
auto id = row->as_int64("col_id_a");
int64_t id = *(row)["col_id_a"];

// with columns id
auto txt = row->as<std::string>(1);
auto txt = row->as_string(1);
std::string txt = *(row)[1];

~~~


## Data Reader Instance

TODO: document this

~~~{.cpp}



~~~

# Supported Features

## Supported Types

### Arrays

Array for any of the supported types.

### Binary data types:

| postgres type | OID      | CPP type            |
| ------------- | -------- | ------------------- |
| bytea         | BYTEAOID | std::vector<int8_t> |

### boolean type:

| postgres type | OID     | CPP type |
| ------------- | ------- | -------- |
| boolean       | BOOLOID | bool     |

### Character types:

| postgres type | OID        | CPP type       |
| ------------- | ---------- | -------------- |
| varchar(n)    | VARCHAROID | std::string    |
| char(n)       | BPCHAROID  | std::string    |
| text          | TEXTOID    | std::string    |

### date/time types:

| postgres type               | OID            | CPP type               |
| --------------------------- | -------------- | ---------------------- |
| timestamp without time zone | TIMESTAMPOID   | pq_async::timestamp    |
| timestamp with time zone    | TIMESTAMPTZOID | pq_async::timestamp_tz |
| date                        | DATEOID        | pq_async::date         |
| time without time zone      | TIMEOID        | pq_async::time         |
| time with time zone         | TIMETZOID      | pq_async::time_tz      |
| interval                    | INTERVALOID    | pq_async::interval     |

### Geometric types:

| postgres type | OID        | CPP type          |
| ------------- | ---------- | ----------------- |
| point         | POINTOID   | pq_async::point   |
| line          | LINEOID    | pq_async::line    |
| lseg          | LSEGOID    | pq_async::lseg    |
| box           | BOXOID     | pq_async::box     |
| path closed   | PATHOID    | pq_async::path    |
| path open     | PATHOID    | pq_async::path    |
| polygon       | POLYGONOID | pq_async::polygon |
| circle        | CIRCLEOID  | pq_async::circle  |

### JSON types:

| postgres type | OID      | CPP type       |
| ------------- | -------- | -------------- |
| json          | JSONOID  | pq_async::json |
| jsonb         | JSONBOID | pq_async::json |

### Monetary data types:

| postgres type | OID     | CPP type        |
| ------------- | ------- | --------------- |
| money         | CASHOID | pq_async::money |

### Network types:

| postgres type | OID         | CPP type           |
| ------------- | ----------- | ------------------ |
| cidr          | CIDROID     | pq_async::cidr     |
| inet          | INETOID     | pq_async::inet     |
| macaddr       | MACADDROID  | pq_async::macaddr  |
| macaddr8      | MACADDR8OID | pq_async::macaddr8 |

### Numeric types:

| postgres type    | OID        | CPP type          |
| ---------------- | ---------- | ----------------- |
| smallint         | INT2OID    | int16_t            |
| integer          | INT4OID    | int32_t           |
| bigint           | INT8OID    | int64_t           |
| numeric          | NUMERICOID | pq_async::numeric |
| real             | FLOAT4OID  | float             |
| double precision | FLOAT8OID  | double            |

### Range type:

| postgres type | OID          | CPP type            |
| ------------- | ------------ | ------------------- |
| int4range     | INT4RANGEOID | pq_async::int4range |
| int8range     | INT8RANGEOID | pq_async::int8range |
| numrange      | NUMRANGEOID  | pq_async::numrange  |
| tsrange       | TSRANGEOID   | pq_async::tsrange   |
| tstzrange     | TSTZRANGEOID | pq_async::tstzrange |
| daterange     | DATERANGEOID | pq_async::daterange |

### System types:

| postgres type | OID     | CPP type       |
| ------------- | ------- | -------------- |
| oid           | OIDOID  | pq_async::oid  |

### UUID type:

| postgres type | OID     | CPP type       |
| ------------- | ------- | -------------- |
| uuid          | UUIDOID | pq_async::uuid |



# Unsupported features

## Asynchronous Large Object

Async large_object are not supported for the moment.

## Unsupported types

Note: __some of these may be supported in future release__

### System types:

| postgres type | OID     | CPP type       |
| ------------- | ------- | -------------- |
| tid           | TIDOID  | pq_async::tid  |
| xid           | XIDOID  | pq_async::xid  |
| cid           | CIDOID  | pq_async::cid  |
| char          | CHAROID | char           |
| name          | NAMEOID | pq_async::name |

### User types:

| postgres type | OID                 | CPP type           |
| ------------- | ------------------- | ------------------ |
| enum          | Defined in postgres | pq_async::enum     |
| complex types | Defined in postgres | pq_async::complexe |


### Logger:

TODO: accept a callback on which logging info will be pass.


## Thanks and Credits

- PostgreSQL, https://www.postgresql.org/, PostgreSQL License
- date lib from Howard Hinnant, https://github.com/HowardHinnant/date, MIT license
- json lib from Niels Lohmann, https://github.com/nlohmann/json, MIT license


## Known problems

- Please note that, because of a bug in the gcc's libstdc++ v6.3, 
the Howard Hinnant date testit target, can't be build.
<br/>
[see that post for more info](https://github.com/HowardHinnant/date/issues/340),
__Please note that it is not affecting the usage of the library itself, 
only the tests are affected.__

