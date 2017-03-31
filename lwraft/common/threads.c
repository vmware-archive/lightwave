/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


#include "includes.h"

#if !defined(_WIN32) || defined(HAVE_PTHREADS_WIN32)
#  include "threading.c"
#else  // Windows native threading case
#  include "win/threading.c"
#  ifdef WIN2008
#    include "win/condvar2008.c"
#  else
#    include "win/condvar2003.c"
#  endif /* WIN2008 */
#endif
