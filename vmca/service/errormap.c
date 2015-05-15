#include "includes.h"

DWORD
VMCADCEGetErrorCode(
    dcethread_exc* pDceException
)
{
    DWORD dwError = 0;
    dwError = dcethread_exc_getstatus (pDceException);
//FixME  AnuE - find right mapping/move to common gobuild component
//    dwError =  1703;
	return dwError;
}

DWORD
VMCAMapDCEErrorCode(
	DWORD dwError
	)
{
//FixME AnuE  - find right mapping/move to common gobuild component
//	return(dwError = 1703);
	return dwError;
}
