/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>

int
main(void)
{
	void *handle;
	handle = dlopen("/opt/vmware/lib64/libvmcaclient.so.0.0.0", 
						RTLD_LAZY);
	if( handle == NULL )
	{
		printf("errno[%d], errmsg[%s]\n", errno, dlerror());
		return -1;
	}
	else
	{
		printf("\nvmca Lib loaded successfully");
	}
	dlclose(handle);
	return 0;
}
