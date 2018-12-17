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

#include "data_large_object.h"

namespace pq_async {
	
void data_large_object::open(lo_mode m)
{
	if(this->opened())
		throw exception("Alreaddy opened!");
	
	_db->open_connection();
	connection_lock conn_lock(_db->_conn);
	_db->begin();
	
	this->_fd = lo_open(
		_db->_conn->conn(),
		this->_oid.data(),
		(int)m
	);
	
	if(this->_fd == -1){
		_db->rollback();
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
	
	_opened_read = ((int)m & (int)lo_mode::read) != 0;
	_opened_write = ((int)m & (int)lo_mode::write) != 0;
}

int32_t data_large_object::read(char* buf, int32_t len)
{
	if(!this->opened())
		throw exception("Unable to read when large object is closed!");
	
	connection_lock conn_lock(_db->_conn);
	int32_t r = lo_read(_db->_conn->conn(), this->_fd, buf, len);
	if(r == -1){
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
	return r;
}

int32_t data_large_object::write(const char* buf, int32_t len)
{
	if(!this->opened())
		throw exception("Unable to write when large object is closed!");
	
	connection_lock conn_lock(_db->_conn);
	int32_t r = lo_write(_db->_conn->conn(), this->_fd, buf, len);
	if(r == -1){
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
	return r;
}

int64_t data_large_object::tell()
{
	if(!this->opened())
		throw exception("Unable to tell when large object is closed!");
	
	connection_lock conn_lock(_db->_conn);
	int64_t r = lo_tell64(_db->_conn->conn(), this->_fd);
	if(r == -1){
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
	return r;
}

int64_t data_large_object::seek(
	int64_t offset, lo_whence w)
{
	if(!this->opened())
		throw exception("Unable to seek when large object is closed!");
	
	connection_lock conn_lock(_db->_conn);
	int64_t r = lo_lseek64(_db->_conn->conn(), this->_fd, offset, (int)w);
	if(r == -1){
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
	return r;
}

void data_large_object::resize(int64_t new_size)
{
	if(!this->opened())
		throw exception("Unable to resize when large object is closed!");
	
	connection_lock conn_lock(_db->_conn);
	int r = lo_truncate64(_db->_conn->conn(), this->_fd, new_size);
	if(r == -1){
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
}

void data_large_object::close()
{
	if(!this->opened())
		return;
	
	connection_lock conn_lock(_db->_conn);
	int r = lo_close(_db->_conn->conn(), this->_fd);
	this->_fd = -1;
	_opened_read = _opened_write = false;
	
	if(r == -1){
		_db->rollback();
		std::string err_msg = PQerrorMessage(_db->_conn->conn());
		throw pq_async::exception(err_msg);
	}
	_db->commit();
}

void data_large_object::unlink()
{
	if(this->opened())
		throw exception("Large object must be closed to be delete!");
	
	_db->open_connection();
	connection_lock conn_lock(_db->_conn);
	lo_unlink(_db->_conn->conn(), this->_oid);
}


} // ns: pq_async