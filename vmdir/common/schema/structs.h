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

typedef enum
{
    VDIR_LDAP_DEFINITION_TYPE_AT = 0,
    VDIR_LDAP_DEFINITION_TYPE_OC,
    VDIR_LDAP_DEFINITION_TYPE_CR,
    VDIR_LDAP_DEFINITION_TYPE_SR,
    VDIR_LDAP_DEFINITION_TYPE_NF

} VDIR_LDAP_DEFINITION_TYPE;

typedef union _VDIR_LDAP_DEFINITION_DATA
{
    VDIR_LDAP_ATTRIBUTE_TYPE    at;
    VDIR_LDAP_OBJECT_CLASS      oc;
    VDIR_LDAP_CONTENT_RULE      cr;
    VDIR_LDAP_STRUCTURE_RULE    sr;
    VDIR_LDAP_NAME_FORM         nf;

} VDIR_LDAP_DEFINITION_DATA, *PVDIR_LDAP_DEFINITION_DATA;

/*
 * VDIR_LDAP_SCHEMA secretly maintains linked lists of different versions
 * of each definition. When there is no longer any reference to a version,
 * it frees the version and its node in the linked list.
 */
typedef struct _VDIR_LDAP_DEFINITION_LIST
{
    PVDIR_LINKED_LIST   pList;
    PVMDIR_MUTEX        mutex;

} VDIR_LDAP_DEFINITION_LIST, *PVDIR_LDAP_DEFINITION_LIST;

typedef struct _VDIR_LDAP_DEFINITION
{
    VDIR_LDAP_DEFINITION_DATA           data;

    // metadata for maintaining linked list
    VDIR_LDAP_DEFINITION_TYPE           type;
    PVDIR_LINKED_LIST_NODE              pNode;
    PVDIR_LDAP_DEFINITION_LIST          pList;
    int                                 iRefCount;
    PSTR                                pszName;

} VDIR_LDAP_DEFINITION, *PVDIR_LDAP_DEFINITION;
