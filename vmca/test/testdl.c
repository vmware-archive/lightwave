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
