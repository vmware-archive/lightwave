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
package com.vmware.identity.saml.impl;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.regex.Pattern;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.InvalidPrincipalException;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.PrincipalAttribute;
import com.vmware.identity.saml.PrincipalAttributeDefinition;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.TokenValidator;

/**
 * This class validates that already issued SAML token is valid as of now.
 * (assertion statements: all statements within the assertion are validated)
 */
public final class TokenValidatorImpl implements TokenValidator {

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(TokenValidatorImpl.class);
   private static final String GROUP = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";

   private final Collection<PrincipalAttributeDefinition> supportedAttributes;
   private final PrincipalAttributesExtractor principalAttributesExtractor;

   private final TokenValidator authnTokenValidator;
   /**
    * @param authnTokenValidator
    *        token validator performing authn only token validation. not {@code null}.
    * @param principalAttributesExtractor
    *           required
    */
   public TokenValidatorImpl(TokenValidator authnTokenValidator,
      PrincipalAttributesExtractor principalAttributesExtractor) {
	  Validate.notNull(authnTokenValidator);
	  Validate.notNull(principalAttributesExtractor);

      this.authnTokenValidator = authnTokenValidator;
      this.principalAttributesExtractor = principalAttributesExtractor;
      this.supportedAttributes = principalAttributesExtractor
         .getAllAttributeDefinitions();
   }

   @Override
   public ServerValidatableSamlToken validate(ServerValidatableSamlToken token)
      throws InvalidSignatureException, InvalidTokenException, SystemException {

      logger.debug("Validating token.");
      final ServerValidatableSamlToken result = this.authnTokenValidator.validate(token);
      logger.debug("Token validated");
      logger.debug( String.format("Token is from external idp: [%s]", result.isExternal()) );

      if( result.isExternal() == true )
      {
          logger.warn("Cannot perform online validation for token from external idp.");
          throw new InvalidTokenException(
              "External tokens cannot be on-line validated. They should be validtade through their issuer."
          );
      }
      if ( ( result.getSubject().subjectValidation() != SubjectValidation.Regular ) || (result.getSubject().subjectUpn() == null) )
      {
          logger.warn("Token's subject is invalid. [SubjectValidation={}]", result.getSubject().subjectValidation());
          throw new InvalidTokenException(
              "Token's subject is invalid."
          );
      }

      // TODO [866694][849937] make this check on all attributes which are
      // parameterized
      // TODO check [866704] for details
      checkGroupList(token.getGroupList(), token.getSubject().subjectUpn());
      logger.debug("Group list is verified.");

      // TODO [866694] check for other attributes (first, last name,
      // isSolution)?

      logger.info("Token {} for principal {} successfully validated.",
         token.getId(), token.getSubject().subjectUpn());

       return result;
   }

   /**
    * Checks if the group list from the token is equal to the current group list
    * for the given principal
    *
    * @param groupList
    *           required
    * @param subject
    *           required
    * @throws InvalidTokenException
    *            if the token group list differs from the current group list
    */
   private void checkGroupList(List<PrincipalId> groupList,
      PrincipalId subject) throws InvalidTokenException, SystemException {

      // TODO pass attributes for which validation should be performed
      PrincipalAttributeDefinition groupAttributeDefinition = getSupportedAttributeDefinition(GROUP);
      if (groupAttributeDefinition == null) {
         throw new InvalidTokenException(GROUP
            + " is not supported attribute in token attributes.");
      }

      final PrincipalAttribute groupAttribute;
      try {
         groupAttribute = getAttribute(subject, groupAttributeDefinition);
      } catch (InvalidPrincipalException e) {
         throw new InvalidTokenException(
            String.format(
               "Principal %s not found, race condition suspected since the principal has just been validated.",
               subject), e);
      }

      List<PrincipalId> currentGroupList = toGroupList(groupAttribute
         .getValues());
      List<PrincipalId> tokenGroupList = groupList;
      if (tokenGroupList.size() != currentGroupList.size()
         || !tokenGroupList.containsAll(currentGroupList)) {
         throw new InvalidTokenException("Current group membership of "
            + "the principal is different from the one stated in the token");
      }

   }

   private PrincipalAttribute getAttribute(PrincipalId subject,
      PrincipalAttributeDefinition attributeDefinition)
      throws InvalidPrincipalException, SystemException {
      assert subject != null;
      assert attributeDefinition != null;

      Collection<PrincipalAttributeDefinition> attributeDefinitions = new HashSet<PrincipalAttributeDefinition>();
      attributeDefinitions.add(attributeDefinition);
      final Set<PrincipalAttribute> attributes = principalAttributesExtractor
         .getAttributes(subject, attributeDefinitions);
      if (attributes == null || attributes.size() != 1) {
         throw new IllegalStateException(String.format(
            "No single attribute found for %s searching for %s", subject,
            attributeDefinition));
      }

      PrincipalAttribute attribute = attributes.iterator().next();
      assert attribute != null;
      return attribute;
   }

   /**
    * @param attributeName
    *           not null, name of attribute
    * @return attribute definition of the attribute with given name or null if
    *         this attribute is not supported
    */
   private PrincipalAttributeDefinition getSupportedAttributeDefinition(
      String attributeName) {
      assert attributeName != null;

      PrincipalAttributeDefinition result = null;
      for (PrincipalAttributeDefinition attr : supportedAttributes) {
         if (attr.getName().equalsIgnoreCase(attributeName)) {
            result = attr;
            break;
         }
      }

      return result;
   }

   private PrincipalId parseGroupId(String groupId) {
      Validate.notEmpty(groupId, "groupId");

      final PrincipalId group;
      if (groupId.contains("\\")) {
         String[] parts = splitInTwo(groupId, '\\');
         group = new PrincipalId(parts[1], parts[0]);

      } else {
         String[] parts = splitInTwo(groupId, '/');
         group = new PrincipalId(parts[0], parts[1]);
      }

      return group;
   }

   /**
    * @param value
    *           required
    * @param separator
    *           required
    * @return
    */
   private String[] splitInTwo(String value, char separator) {

      Pattern splitter = Pattern.compile(Pattern.quote(String
         .valueOf(separator)));
      String split[] = splitter.split(value, 3);
      if (split.length != 2 || split[0].isEmpty() || split[1].isEmpty()) {
         throw new IllegalStateException(
            String.format(
               "Invalid principal value: `%s' (incorrect number of fields)",
               value));
      }

      return split;
   }

   /**
    * @param groups
    *           can be null
    * @return the converted list or empty list
    */
   private List<PrincipalId> toGroupList(String[] groups) {
      List<PrincipalId> result = new ArrayList<PrincipalId>();
      if (groups != null) {
         for (String groupId : groups) {
            result.add(parseGroupId(groupId));
         }
      }
      return result;
   }
}
