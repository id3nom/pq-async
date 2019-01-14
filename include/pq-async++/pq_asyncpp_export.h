
#ifndef PQ_ASYNC_CLIENT_API_H
#define PQ_ASYNC_CLIENT_API_H

#ifdef PQ_ASYNC___STATIC_DEFINE
#  define PQ_ASYNC_CLIENT_API
#  define PQ_ASYNC___NO_EXPORT
#else
#  ifndef PQ_ASYNC_CLIENT_API
#    ifdef pq_async___EXPORTS
        /* We are building this library */
#      define PQ_ASYNC_CLIENT_API 
#    else
        /* We are using this library */
#      define PQ_ASYNC_CLIENT_API 
#    endif
#  endif

#  ifndef PQ_ASYNC___NO_EXPORT
#    define PQ_ASYNC___NO_EXPORT 
#  endif
#endif

#ifndef PQ_ASYNC___DEPRECATED
#  define PQ_ASYNC___DEPRECATED 
#endif

#ifndef PQ_ASYNC___DEPRECATED_EXPORT
#  define PQ_ASYNC___DEPRECATED_EXPORT PQ_ASYNC_CLIENT_API PQ_ASYNC___DEPRECATED
#endif

#ifndef PQ_ASYNC___DEPRECATED_NO_EXPORT
#  define PQ_ASYNC___DEPRECATED_NO_EXPORT PQ_ASYNC___NO_EXPORT PQ_ASYNC___DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef PQ_ASYNC___NO_DEPRECATED
#    define PQ_ASYNC___NO_DEPRECATED
#  endif
#endif

#endif
