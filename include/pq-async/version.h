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
#define _LPQASYNC_FULLNAME_ "pq-async Alpha V" _LPQASYNC_VERSION_

#endif //_libpq_async_version_h
