#ifndef _WIN32

#if 1 /* TBD: adam */
#include <pthread.h>

#define HEIMDAL_MUTEX pthread_mutex_t
#define HEIMDAL_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define HEIMDAL_MUTEX_init(m) pthread_mutex_init(m, NULL)
#define HEIMDAL_MUTEX_lock(m) pthread_mutex_lock(m)
#define HEIMDAL_MUTEX_unlock(m) pthread_mutex_unlock(m)
#define HEIMDAL_MUTEX_destroy(m) pthread_mutex_destroy(m)
#endif /* if 1 */

#else
/* TBD: Adam-Appears locking is only done in logging layer. Ignore for now */
#define HEIMDAL_MUTEX void *
#define HEIMDAL_MUTEX_INITIALIZER 0
#define HEIMDAL_MUTEX_init(m) 
#define HEIMDAL_MUTEX_lock(m) 
#define HEIMDAL_MUTEX_unlock(m)
#define HEIMDAL_MUTEX_destroy(m)
#endif
