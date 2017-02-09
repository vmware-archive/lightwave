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

#include "stdafx.h"


static
DWORD
LogAccessInfo(
    HANDLE hToken,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    DWORD dwAccessDesired
    );

//
// Following function performs AccessCheck, checking if identity in IDL_handle
// is allowed dwAccessDesired for a resource whose SD is pSD.
//

BOOL
IsRpcOperationAllowed(
    handle_t IDL_handle,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    DWORD dwAccessDesired
    )
{
   NTSTATUS        ntStatus = ERROR_SUCCESS;
   DWORD           dwError = ERROR_SUCCESS;
   PACCESS_TOKEN   hToken = NULL;
   ACCESS_MASK     accessGranted;
   BOOLEAN         fAccessGranted = FALSE;
   GENERIC_MAPPING GenericMapping;

   rpc_binding_inq_access_token_caller(IDL_handle, &hToken, &dwError);

    if (dwError != ERROR_SUCCESS) {

        VMEVENT_LOG_ERROR("Error in rpc_binding_inq_access_token_caller: %u\n",
                 dwError);
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

   dwError = LogAccessInfo(hToken, pSD, dwAccessDesired);
   BAIL_ON_VMEVENT_ERROR(dwError);

   // Initialize generic mapping structure to map all.
   memset(&GenericMapping, 0xff, sizeof(GENERIC_MAPPING));
   GenericMapping.GenericRead = GENERIC_READ;
   GenericMapping.GenericWrite = GENERIC_WRITE;
   GenericMapping.GenericExecute = 0;
   GenericMapping.GenericAll = GENERIC_READ | GENERIC_WRITE;

   // This only does something if we want to use generic access
   // rights, like GENERIC_ALL, in our call to AccessCheck().
   RtlMapGenericMask(&dwAccessDesired, &GenericMapping);

   // Make the AccessCheck() call.
   fAccessGranted = RtlAccessCheck(pSD, hToken, dwAccessDesired,
                                   0, &GenericMapping,
                                   &accessGranted, &ntStatus);
   dwError = LwNtStatusToWin32Error(ntStatus);
   if (dwError != ERROR_SUCCESS && dwError != ERROR_ACCESS_DENIED) {

      VMEVENT_LOG_ERROR("Error in RtlAccessCheck : %u\n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

error:
   RtlReleaseAccessToken(&hToken);

   if (fAccessGranted) {
      VMEVENT_LOG_DEBUG("RPC Access GRANTED!\n");
   } else {
      VMEVENT_LOG_ERROR("RPC Access DENIED!\n");
   }

   return fAccessGranted;
}

// Log subject SID, resource SD, and desired access.
DWORD
LogAccessInfo(
    HANDLE hToken,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    DWORD dwAccessDesired
    )
{
   DWORD           dwError = ERROR_SUCCESS;
   PTOKEN_USER     ptuUser  = NULL;
   PTOKEN_GROUPS   pGroupInfo = NULL;
   DWORD           cbBuffer = 0;
   PSTR            pszUserStrSid=NULL;
   PSTR            pszStrSD = NULL;
   PSTR            pszGroupStrSid=NULL;
   PSECURITY_DESCRIPTOR_RELATIVE pRelSD=NULL;
   unsigned int    i=0;

   VMEVENT_LOG_DEBUG("Access desired = %u.\n", dwAccessDesired);

   // Get SID from TokenUser

   // Determine required size of buffer for token information.
   if ((dwError = LwNtStatusToWin32Error(RtlQueryAccessTokenInformation(hToken,
                  TokenUser, NULL, 0,
                  &cbBuffer))) != ERROR_INSUFFICIENT_BUFFER) {
      // Call should have failed with INSUFFICIENT_BUFFER error due to
      // zero-length buffer
      dwError = ERROR_GEN_FAILURE;
      VMEVENT_LOG_ERROR("Error %u:RtlQueryAccessTokenInformation should have failed, "
               "but it did not. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   } else {
       dwError = ERROR_SUCCESS;
   }

   dwError = EventLogAllocateMemory(cbBuffer, (PVOID *) &ptuUser);
   BAIL_ON_VMEVENT_ERROR(dwError);

   if ((dwError = LwNtStatusToWin32Error(RtlQueryAccessTokenInformation(hToken,
                  TokenUser, ptuUser, cbBuffer,
                  &cbBuffer))) != ERROR_SUCCESS) {

      VMEVENT_LOG_ERROR("Error %u:RtlQueryAccessTokenInformation", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   if ((dwError = LwNtStatusToWin32Error(RtlAllocateCStringFromSid(
                  &pszUserStrSid, ptuUser->User.Sid))) != ERROR_SUCCESS) {

      VMEVENT_LOG_ERROR("Error %u:RtlAllocateCStringFromSid", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   VMEVENT_LOG_DEBUG("Subject sid = %s.\n", pszUserStrSid);

   // Get Group memberships from TokenGroups

   // Determine required size of buffer for token information.
   if ((dwError = LwNtStatusToWin32Error(RtlQueryAccessTokenInformation(hToken,
                  TokenGroups, NULL, 0,
                  &cbBuffer))) != ERROR_INSUFFICIENT_BUFFER) {
      // Call should have failed due to zero-length buffer.
      dwError = ERROR_GEN_FAILURE;
      VMEVENT_LOG_ERROR("Error %u:RtlQueryAccessTokenInformation should have failed, "
               "but it did not. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   } else {
       dwError = ERROR_SUCCESS;
   }

   dwError = EventLogAllocateMemory(cbBuffer, (PVOID *) &pGroupInfo);
   BAIL_ON_VMEVENT_ERROR(dwError);

   if ((dwError = LwNtStatusToWin32Error(RtlQueryAccessTokenInformation(hToken,
                  TokenGroups, pGroupInfo, cbBuffer,
                  &cbBuffer))) != ERROR_SUCCESS) {

      VMEVENT_LOG_ERROR("Error %u:RtlQueryAccessTokenInformation", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   for (i=0; i<pGroupInfo->GroupCount; i++) {
      if ((dwError = LwNtStatusToWin32Error(RtlAllocateCStringFromSid(
                     &pszGroupStrSid,
                     pGroupInfo->Groups[i].Sid))) != ERROR_SUCCESS) {

          VMEVENT_LOG_ERROR("Error %u:RtlAllocateCStringFromSid", dwError);
          BAIL_ON_VMEVENT_ERROR(dwError);
       }

      VMEVENT_LOG_DEBUG("Member of group sid = %s.\n", pszGroupStrSid);
      RTL_FREE(&pszGroupStrSid);
      pszGroupStrSid = NULL;
   }

   cbBuffer = 0; // It seems this variable has to be set to 0 for the following
                 // function not to return INVALID_PARAM (87) error.
   if ((dwError = LwNtStatusToWin32Error(RtlAbsoluteToSelfRelativeSD(pSD,
                  NULL, &cbBuffer))) != ERROR_INSUFFICIENT_BUFFER) {
      // Call should have failed due to zero-length buffer.
      dwError = ERROR_GEN_FAILURE;
      VMEVENT_LOG_ERROR("Error %u:RtlAbsoluteToSelfRelativeSD should have failed, "
               "but it did not. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   } else {
       dwError = ERROR_SUCCESS;
   }

   dwError = EventLogAllocateMemory(cbBuffer, (PVOID *) &pRelSD);
   BAIL_ON_VMEVENT_ERROR(dwError);

   if ((dwError = LwNtStatusToWin32Error(RtlAbsoluteToSelfRelativeSD(pSD,
                  pRelSD, &cbBuffer))) != ERROR_SUCCESS) {

      VMEVENT_LOG_ERROR("Error %u:RtlAbsoluteToSelfRelativeSD", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   if ((dwError = LwNtStatusToWin32Error(
                  RtlAllocateSddlCStringFromSecurityDescriptor(&pszStrSD,
                  pRelSD, SDDL_REVISION_1, 255))) != ERROR_SUCCESS) {

      VMEVENT_LOG_ERROR("Error in RtlAllocateSddlCStringFromSecurityDescriptor: %u\n",
               dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   VMEVENT_LOG_DEBUG("String SD = %s.\n", pszStrSD);

error:
   RTL_FREE(&pszUserStrSid);
   RTL_FREE(&pszGroupStrSid);
   RTL_FREE(&pszStrSD); // or LwRtlCStringFree ?
   EventLogFreeMemory(ptuUser);
   EventLogFreeMemory(pGroupInfo);
   EventLogFreeMemory(pRelSD);

   return dwError;
}

// Construct basic (without any ACLs) SD
DWORD
ConstructBasicSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE * ppSD
    )
{
   DWORD dwError = 0;
   PSID pSid = NULL;
   // Well known Sid for Built-in Administrators.
   PSTR pWellKnownBASid = "S-1-5-32-544";
   PSECURITY_DESCRIPTOR_ABSOLUTE pSD = NULL;

   dwError = EventLogAllocateMemory(
                   SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE,
                   (PVOID *) &pSD);
   BAIL_ON_VMEVENT_ERROR(dwError);

   if ((dwError = LwNtStatusToWin32Error(
                       RtlCreateSecurityDescriptorAbsolute(
                               pSD,
                               SECURITY_DESCRIPTOR_REVISION))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlCreateSecurityDescriptorAbsolute. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   // Just set Administrators group as Owner & Group
   if ((dwError = LwNtStatusToWin32Error(
                       RtlAllocateSidFromCString(
                           &pSid,
                           pWellKnownBASid))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlAllocateSidFromCString", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   if ((dwError = LwNtStatusToWin32Error(
                       RtlSetOwnerSecurityDescriptor(
                               pSD,
                               pSid,
                               FALSE))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlSetUserSecurityDescriptor", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   if ((dwError = LwNtStatusToWin32Error(
                       RtlSetGroupSecurityDescriptor(
                               pSD,
                               pSid,
                               FALSE))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlSetGroupSecurityDescriptor", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   *ppSD = pSD;

cleanup:

   return dwError;

error:

    *ppSD = NULL;

    if (pSD)
    {
        EventLogFreeMemory(pSD);
    }

    RTL_FREE(&pSid);

    goto cleanup;
}

// Construct SD for EventLog-Service resources.
//
// Current ACLs are:
// - Allow READ access to all Authenticated users.
// - Allow READ-WRITE access to Builtin admins

DWORD
ConstructSDForEventLogServ(
    PSECURITY_DESCRIPTOR_ABSOLUTE * ppSD
    )
{
   DWORD dwError = 0;
   PACL pDacl = NULL;
   DWORD dwDaclSize = 0;

   union {
        SID sid;
        BYTE buffer[SID_MAX_SIZE];
   } authenticatedUsersSID;

   union {
           SID sid;
           BYTE buffer[SID_MAX_SIZE];
  } builtinAdminsSID;

   ULONG sidSize = 0;
   DWORD dwAccessMask = 0;
   PSECURITY_DESCRIPTOR_ABSOLUTE pSD = NULL;

   dwError = ConstructBasicSD(&pSD);
   BAIL_ON_VMEVENT_ERROR(dwError);

   // obtain a sid for the Authenticated Users Group
   sidSize = sizeof(authenticatedUsersSID.buffer);
   if ((dwError = LwNtStatusToWin32Error(
                       RtlCreateWellKnownSid(
                               WinAuthenticatedUserSid,
                               NULL,
                               &authenticatedUsersSID.sid,
                               &sidSize))) != ERROR_SUCCESS)
   {
        VMEVENT_LOG_ERROR("Error %u:RtlCreateWellKnownSid", dwError);
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

   // obtain a sid for the Builtin Admins Group
  sidSize = sizeof(builtinAdminsSID.buffer);
  if ((dwError = LwNtStatusToWin32Error(
                      RtlCreateWellKnownSid(
                              WinBuiltinAdministratorsSid,
                              NULL,
                              &builtinAdminsSID.sid,
                              &sidSize))) != ERROR_SUCCESS)
   {
     VMEVENT_LOG_ERROR("Error %u:RtlCreateWellKnownSid", dwError);
     BAIL_ON_VMEVENT_ERROR(dwError);
   }

   // Compute size needed for the ACL.
    dwDaclSize = ACL_HEADER_SIZE +
                 sizeof(ACCESS_ALLOWED_ACE) + SID_MAX_SIZE +
                 sizeof(ACCESS_ALLOWED_ACE) + SID_MAX_SIZE;

   dwError = EventLogAllocateMemory(dwDaclSize, (PVOID *) &pDacl);
   BAIL_ON_VMEVENT_ERROR(dwError);

   if ((dwError = LwNtStatusToWin32Error(
                       RtlCreateAcl(
                           pDacl,
                           dwDaclSize,
                           ACL_REVISION))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlCreateAcl. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   // Add the READ access allowed to Authenticated users ACE to the DACL.
   dwAccessMask = GENERIC_READ;
   if ((dwError = LwNtStatusToWin32Error(
                       RtlAddAccessAllowedAceEx(
                               pDacl,
                               ACL_REVISION,
                               0,
                               dwAccessMask,
                               &authenticatedUsersSID.sid))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlAddAccessAllowedAceEx. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   // Add the READ-WRITE access allowed to Builtin admins ACE to the DACL.
  dwAccessMask = GENERIC_READ | GENERIC_WRITE;
  if ((dwError = LwNtStatusToWin32Error(
                      RtlAddAccessAllowedAceEx(
                              pDacl,
                              ACL_REVISION,
                              0,
                              dwAccessMask,
                              &builtinAdminsSID.sid))) != ERROR_SUCCESS)
  {
      VMEVENT_LOG_ERROR("Error %u:RtlAddAccessAllowedAceEx. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
  }

   // Set our DACL to the SD.
   if ((dwError = LwNtStatusToWin32Error(
                       RtlSetDaclSecurityDescriptor(
                               pSD,
                               TRUE,
                               pDacl,
                               FALSE))) != ERROR_SUCCESS)
   {
      VMEVENT_LOG_ERROR("Error %u:RtlSetDaclSecurityDescriptor. \n", dwError);
      BAIL_ON_VMEVENT_ERROR(dwError);
   }

   *ppSD = pSD;

cleanup:

   return dwError;

error:

    *ppSD = NULL;

    if (pSD)
    {
        EventLogFreeMemory(pSD);
    }

    if (pDacl)
    {
        EventLogFreeMemory(pDacl);
    }

    goto cleanup;
}
