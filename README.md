## Async c++14/17 wrapper for the PostgreSQL libpq 'C' library.

[![Build Status](https://travis-ci.org/micheldenommee/pq-async.svg?branch=master)](https://travis-ci.org/micheldenommee/pq-async)

## Notice

The project is in it's alpha phase and bugs can still be present,

It havn't been tested on any other distro than Debian Stretch.

So if you want to use that lib do it at your own risk as long that you
respect the GNU LGPL v2.1 license.

## Configuration

### Required development libraries

#### Debian based distribution:

~~~bash
$ sudo apt-get install libpq-dev postgresql-server-dev-all \
libcurl4-gnutls-dev libboost-dev uuid-dev \
libboost-filesystem-dev libboost-program-options-dev \
libboost-iostreams-dev libboost-regex-dev \
libboost-thread-dev libboost-locale-dev
~~~

#### Required dependencies

- CMake 3.0.0 and up.
- A compiler supporting, at least, c++14.
- [The boost::multi_array library](https://www.boost.org/doc/libs/1_62_0/libs/multi_array/doc/user.html).
- The date library from 
Howard Hinnant @ https://github.com/HowardHinnant/date 


to download the required libraries you must run the following command after cloning
this repo:
~~~bash
$ git submodule update --init --recursive
~~~

## basic usage

~~~cpp

// initialize the connection pool
// For more info on the "do_ssl" and "do_crypto" arguments
// see https://www.postgresql.org/docs/current/libpq-ssl.html#LIBPQ-SSL-INITIALIZE
bool do_ssl = true;
bool do_crypto = true;
int max_connection_pool_count = 5; // the default is 20 connections
                                   // per connection strings
pq_async::connection_pool::init(
    max_connection_pool_count, do_ssl, do_crypto
);

// get a database instance
auto db = pq_async::database::open("connection string");

// execute a synchronous that return the number of touched records use:
db->execute("insert into my_table (name) values ('my name')");

// to execute the same query in async mode:
db->execute("insert into my_table (name) values ('my name')",
[](const md::callback::cb_error& err){
    if(err){
        // an error occured.
    }
    // ...
});

// for async work to be execute the following command should be run
pq_async::event_queue::get_default()->run();

// use database::query function to get multiple records result
sp_data_table tbl = db->query("my sql statement");
// use database::query_single function to get a single record result
sp_data_row row = db->query_single("my sql statement");
// use database::query_value function to get a single value result
std::string name = db->query_value<std::string>("my sql statement");
// use database::query_reader function to get a reader value result
std::string name = db->query_value<std::string>("my sql statement");

// all query function are variadic templates that support parameterized query
// e.g:
std::string name = db->query_value<std::string>(
    "select $1", std::string("my name")
    );
// to see a list of the supported types goto 
// "https://micheldenommee.github.io/libpq-async/index.html#autotoc_md24"

// all of the previous function have async versions that support callback as
// there last parameter e.g:
db->query_value<std::string>(
    "select $1", std::string("my name"),
[](const md::callback::cb_error& err, const std::string& name){
    if(err){
        // ...
    }
    
    std::cout << name << std::endl;
});

// all callback are not required to have the "const md::callback::cb_error& err" argument
// in that case if an error occured it is sent to the unhandle error handler
// this is legal:
db->query_value<std::string>(
    "select $1", std::string("my name"),
[](const std::string& name){
    std::cout << name << std::endl;
});
// this too:
db->query_value<std::string>(
    "select $1", std::string("my name"),
[](){
    std::cout << "do nothing with the result!" << std::endl;
});
// and auto are legal too:
db->query_value<std::string>(
    "select $1", std::string("my name"),
[](auto err, auto name){
    std::cout << name << std::endl;
});



// release resource on program exit
pq_async::connection_pool::destroy();

~~~

for more usage example look at the documentation
https://micheldenommee.github.io/libpq-async


## Thanks and Credits

- PostgreSQL, https://www.postgresql.org/, PostgreSQL License
- date lib from Howard Hinnant, https://github.com/HowardHinnant/date, MIT license
- json lib from Niels Lohmann, https://github.com/nlohmann/json, MIT license

