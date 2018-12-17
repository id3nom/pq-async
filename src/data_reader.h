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

#ifndef _libpq_async_data_reader_h
#define _libpq_async_data_reader_h

#include "data_common.h"
#include "data_table.h"
#include "data_connection_pool.h"

namespace pq_async{

class data_reader 
	: public std::enable_shared_from_this<data_reader>
{
	friend class database;
	friend class data_prepared;
	
	data_reader(sp_connection_task ct)
		: _ct(ct), _table(), _closed(false)
	{
	}
	
public:
	
	~data_reader()
	{
		close();
		_ct.reset();
	}
	
	sp_database db(){ return _ct->db();}
	
	bool closed(){ return _closed;}
	
	template<
		typename T,
		typename std::enable_if<
			!std::is_same<
				sp_data_row,
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
				const cb_error&
			>::value ||
			std::is_invocable_r<
				void,
				typename std::remove_pointer<
					T
				>::type,
				const cb_error&,
				sp_data_row
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
				sp_data_row
			>::value)
		, int32_t>::type = -1
	>
	void next(T tcb)
	{
		value_cb<sp_data_row> cb;
		assign_value_cb<value_cb<sp_data_row>, sp_data_row>(
			cb, tcb
		);
		
		_ct->_cb =
		[self=this->shared_from_this(), cb](
			const cb_error& err, PGresult* res
		)-> void {
			if(err){
				cb(err, sp_data_row());
				self->_ct->_cb = nullptr;
				return;
			}
			
			try{
				if(!res){
					self->_closed = true;
					self->close();
					cb(nullptr, sp_data_row());
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
					sp_data_row row(
						new data_row(self->_table->get_columns(), res, 0)
					);
					self->_table->emplace_back(row);
				}
				PQclear(res);
				res = nullptr;
				
				if(self->_table->size() > 0)
					cb(pq_async::cb_error(err), (*self->_table)[0]);
			}catch(const std::exception& err){
				cb(pq_async::cb_error(err), sp_data_row());
				self->_ct->_cb = nullptr;
			}
		};
	}
	
	sp_data_row next()
	{
		if(_closed)
			throw pq_async::exception("The reader is closed!");
		
		auto res = _ct->run_now();
		if(!res){
			this->_closed = true;
			this->close();
			
			return sp_data_row();
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
			sp_data_row row(new data_row(_table->get_columns(), res, 0));
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
		
		return sp_data_row();
	}
	
	void close()
	{
		if(_closed)
			return;
		
		char errbuf[256];
		PGcancel* c = PQgetCancel(_ct->conn());
		if(c){
			if(!PQcancel(c, errbuf, 256))
				pq_async_log_error("%c", c);
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

		_table = sp_data_table(new pq_async::data_table());

		int fieldCount = PQnfields(res);
		for(int i = 0; i < fieldCount; ++i){
			int oid = PQftype(res, i);
			std::string name(PQfname(res, i));
			int fmt = PQfformat(res, i);

			sp_data_column col(
				new data_column(oid, i, name, fmt)
			);
			
			_table->get_columns()->emplace_back(col);
		}
	}
	
	sp_connection_task _ct;
	sp_data_table _table;
	bool _closed;
};

} //namespace pq_async

#endif //_libpq_async_data_reader_h
