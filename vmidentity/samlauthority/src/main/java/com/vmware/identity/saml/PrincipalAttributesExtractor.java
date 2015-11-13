/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

package com.vmware.identity.saml;

import java.util.Collection;
import java.util.Set;

import com.vmware.identity.idm.PrincipalId;

/**
 * Implementation of this interface is responsible for extraction of the
 * attributes by given principal.
 */
public interface PrincipalAttributesExtractor {

   /**
    * @return all possible attribute definitions
    * @throws SystemException
    */
   Collection<PrincipalAttributeDefinition> getAllAttributeDefinitions() throws SystemException;

   /**
    * @param principalId
    *           principal identifier, required
    * @param attributeDefinitions, required
    * @return not null set of attributes for a given principal.
    * @throws InvalidPrincipalException
    * @throws SystemException
    */
   Set<PrincipalAttribute> getAttributes(PrincipalId principalId,
      Collection<PrincipalAttributeDefinition> attributeDefinitions)
      throws InvalidPrincipalException, SystemException;

   /**
    * Determines whether given principal is not disabled.
    *
    * @param principalId
    *           principal identifier, required
    * @return
    * @throws InvalidPrincipalException
    * @throws SystemException
    */
   boolean isActive(PrincipalId principalId) throws InvalidPrincipalException,
      SystemException;

   /**
    * Looks up active user in system domain.
    *
    * @param attributeName
    *           Name of an attribute to base the search on, required
    * @param attributeValue
    *           The value of the attribute to look for
    * @return Principal Id of the user matching the search criteria
    * @throws InvalidPrincipalException
    * @throws SystemException
    */
   PrincipalId findActiveUser(String attributeName, String attributeValue) throws InvalidPrincipalException,
      SystemException;
}
