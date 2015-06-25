/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : extern.h
 *
 * Abstract :
 *
 *            VMware dns Service
 */

#ifndef EXTERN_H
#define	EXTERN_H

#ifdef	__cplusplus
extern "C" {
#endif

extern VMDNS_RECORD_TYPE_NAME_MAP gRecordTypeMap[];
extern VMDNS_SERVICE_TYPE_NAME_MAP gServiceNameMap[];
extern VMDNS_SERVICE_PROTOCOL_NAME_MAP gProtocolNameMap[];
extern VMDNS_RECORD_METHODS gRecordMethods[];

extern DWORD gRecordTypeMapSize;
extern DWORD gServiceNameMapSize;
extern DWORD gProtocolNameMapSize;
extern DWORD gRecordMethodMapSize;

#ifdef	__cplusplus
}
#endif

#endif	/* EXTERN_H */

