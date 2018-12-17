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


#ifndef _libpq_async_data_large_object_h
#define _libpq_async_data_large_object_h

#include "stable_headers.h"

#include "data_common.h"
#include "log.h"

#include "data_connection_pool.h"
#include "database.h"

extern "C" {
#include INCLUDE_FILE(LIBPQ_POSTGRESQL_TYPE_INCLUDE_DIR,libpq/libpq-fs.h)
}

namespace pq_async{

enum class lo_mode
{
	read = INV_READ,
	write = INV_WRITE,
};

enum class lo_whence
{
	seek_start = SEEK_SET,
	seek_cur = SEEK_CUR,
	seek_end = SEEK_END,
};

class data_large_object
{
	friend class pq_async::database;
	
	data_large_object(const sp_database& db, const pq_async::oid& oid)
		: _db(db), _oid(oid), _opened_read(false), _opened_write(false), _fd(-1)
	{
	}
public:
	inline bool opened()
	{
		return _opened_read || _opened_write;
	}
	inline bool opened_read()
	{
		return _opened_read;
	}
	inline bool opened_write()
	{
		return _opened_write;
	}
	
	void open(lo_mode m);
	
	int32_t read(char* buf, int32_t len);
	int32_t write(const char* buf, int32_t len);
	
	int64_t tell();
	int64_t seek(int64_t offset, lo_whence w = lo_whence::seek_cur);
	
	void resize(int64_t new_size);
	
	void close();
	void unlink();
	
private:
	sp_database _db;
	pq_async::oid _oid;
	bool _opened_read;
	bool _opened_write;
	int _fd;
};

} // ns: pq_async

#endif //_libpq_async_data_large_object_h
