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
package com.vmware.identity.saml.idm;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.InvalidPrincipalException;
import com.vmware.identity.saml.PrincipalAttribute;
import com.vmware.identity.saml.PrincipalAttributeDefinition;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.util.PerfConstants;

/**
 * This class is responsible for extracting principal attributes using the IDM
 * as a data source
 */
public final class IdmPrincipalAttributesExtractor implements
   PrincipalAttributesExtractor {

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(IdmPrincipalAttributesExtractor.class);

   private final static IDiagnosticsLogger perfLog = DiagnosticsLoggerFactory
      .getLogger(PerfConstants.PERF_LOGGER.getClass());

   private final String tenantName;
   private final CasIdmClient idmClient;

   /**
    * @param hostName
    * @param tenantName
    */
   public IdmPrincipalAttributesExtractor(String tenantName,
      CasIdmClient idmClient) {
      assert tenantName != null;
      assert idmClient != null;

      this.tenantName = tenantName;
      this.idmClient = idmClient;
   }

   @Override
   public Collection<PrincipalAttributeDefinition> getAllAttributeDefinitions()
      throws SystemException {
      Collection<Attribute> allAttributeDefinitions = Collections.emptySet();
      try {
         allAttributeDefinitions = idmClient
            .getAttributeDefinitions(tenantName);
      } catch (Exception e) {
         throw new SystemException(e);
      }

      Set<PrincipalAttributeDefinition> result = Collections.emptySet();
      if (allAttributeDefinitions != null) {
         log.trace("{} attribute definitions retrieved",
            allAttributeDefinitions.size());
         result = new HashSet<PrincipalAttributeDefinition>(
            allAttributeDefinitions.size());

         for (Attribute attrDefinition : allAttributeDefinitions) {
            if (attrDefinition == null || attrDefinition.getName() == null
               || attrDefinition.getNameFormat() == null) {
               throw new IllegalStateException(
                  "Missing or invalid attribute definition!");
            }
            final PrincipalAttributeDefinition newDef = new PrincipalAttributeDefinition(
               attrDefinition.getName(), attrDefinition.getNameFormat(),
               attrDefinition.getFriendlyName());
            result.add(newDef);
            log.trace("An attribute definition {} retrieved.", newDef);
         }
      }

      return result;
   }

   @Override
   public Set<PrincipalAttribute> getAttributes(PrincipalId principalId,
      Collection<PrincipalAttributeDefinition> attributeDefinitions)
      throws InvalidPrincipalException, SystemException {
      Validate.notNull(principalId);
      Validate.notNull(attributeDefinitions);

      Set<Attribute> attributeDefsIDM = new HashSet<Attribute>();
      for (PrincipalAttributeDefinition principalAttributeDefinition : attributeDefinitions) {
         attributeDefsIDM
            .add(convertToIDMAttributeDefinition(principalAttributeDefinition));
      }

      Collection<AttributeValuePair> attributes = Collections.emptySet();
      try {
         final long start = System.nanoTime();
         attributes = idmClient.getAttributeValues(tenantName, principalId,
            attributeDefsIDM);
         perfLog.trace("'idmClient.getAttributes' took {} ms.",
            TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - start));
      } catch (com.vmware.identity.idm.InvalidPrincipalException e) {
         throw new InvalidPrincipalException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }

      Set<PrincipalAttribute> result = Collections.emptySet();
      if (attributes != null) {
         log.trace("{} attributes retrieved for {}", attributes.size(),
            principalId);
         result = new HashSet<PrincipalAttribute>(attributes.size());
         for (AttributeValuePair attr : attributes) {
            final Attribute attrDefinition = attr.getAttrDefinition();
            if (attrDefinition == null || attrDefinition.getName() == null
               || attrDefinition.getNameFormat() == null) {
               throw new IllegalStateException(
                  "Missing or invalid attribute definition!");
            }
            List<String> values = attr.getValues();
            final PrincipalAttribute newAttr = new PrincipalAttribute(
               attrDefinition.getName(), attrDefinition.getNameFormat(),
               attrDefinition.getFriendlyName(), values == null
                  || values.size() == 0 ? null
                  : values.toArray(new String[values.size()]));
            result.add(newAttr);
            log.trace("An attribute {} retrieved for {}", newAttr, principalId);
         }
      }
      return result;
   }

   @Override
   public boolean isActive(PrincipalId principalId)
      throws InvalidPrincipalException, SystemException {
      try {
         final long start = System.nanoTime();
         final boolean result = idmClient.isActive(tenantName, principalId);
         perfLog.trace("'idmClient.isActive' took {} ms.",
            TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - start));
         return result;
      } catch (com.vmware.identity.idm.InvalidPrincipalException | NoSuchIdpException e) {
         throw new InvalidPrincipalException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
   }

   @Override
   public PrincipalId findActiveUser(String attributeName, String attributeValue)
           throws InvalidPrincipalException, SystemException
   {
       try {
           final long start = System.nanoTime();
           final PrincipalId result = idmClient.findActiveUserInSystemDomain(tenantName, attributeName, attributeValue);
           perfLog.trace("'idmClient.findActiveUser' took {} ms.",
              TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - start));
           return result;
        } catch (com.vmware.identity.idm.InvalidPrincipalException e) {
           throw new InvalidPrincipalException(e);
        } catch (Exception e) {
           throw new SystemException(e);
        }
   }

   private Attribute convertToIDMAttributeDefinition(
      PrincipalAttributeDefinition principalAttributeDefinition) {
      assert principalAttributeDefinition != null;

      Attribute attribute = new Attribute(principalAttributeDefinition.getName());
      attribute.setFriendlyName(principalAttributeDefinition.getFriendlyName());
      attribute.setNameFormat(principalAttributeDefinition.getNameFormat());

      return attribute;
   }
}
