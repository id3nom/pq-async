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

#ifndef _libpq_async_version_h
#define _libpq_async_version_h

#define QU(x) #x
#define QUH(x) QU(x)

#define _LPQASYNC_VERSION_MAJOR_ 0
#define _LPQASYNC_VERSION_MINOR_ 1
#define _LPQASYNC_VERSION_PATCH_ 0

#define _LPQASYNC_VERSION_ QUH(_LPQASYNC_VERSION_MAJOR_) "." QUH(_LPQASYNC_VERSION_MINOR_) "." QUH(_LPQASYNC_VERSION_PATCH_)
#define _LPQASYNC_APP_ pq_async
#define _LPQASYNC_SHORTNAME_ QUH(_LPQASYNC_APP_) " / v" _LPQASYNC_VERSION_
#define _LPQASYNC_FULLNAME_ "libpq-async++ Alpha V" _LPQASYNC_VERSION_

#endif //_libpq_async_version_h
