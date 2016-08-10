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
package com.vmware.identity.samlservice.impl;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.saml2.core.AuthnContext;
import org.opensaml.saml2.core.AuthnContextClassRef;
import org.opensaml.saml2.core.AuthnContextComparisonTypeEnumeration;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Condition;
import org.opensaml.saml2.core.Conditions;
import org.opensaml.saml2.core.IDPEntry;
import org.opensaml.saml2.core.IDPList;
import org.opensaml.saml2.core.NameIDPolicy;
import org.opensaml.saml2.core.RequestedAuthnContext;
import org.opensaml.saml2.core.Scoping;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.saml.ext.DelegableType;
import com.vmware.identity.saml.ext.RenewableType;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.AuthnTypesSupported;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlValidator;
import com.vmware.identity.samlservice.Shared;

/**
 * Validator for AuthnRequestState
 *
 */
public class AuthnRequestStateValidator implements
		SamlValidator<AuthnRequestState> {
	private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AuthnRequestStateValidator.class);

	/* (non-Javadoc)
	 * @see com.vmware.identity.samlservice.SamlValidator#validate(java.lang.Object)
	 */
	@Override
	public ValidationResult validate(
			AuthnRequestState t) {
		log.debug("Validating request {}", t);

		ValidationResult vr = null;

		try {
			Validate.notNull(t);

			HttpServletRequest httpRequest = t.getRequest();
			Validate.notNull(httpRequest);

			AuthnRequest request = t.getAuthnRequest();
			Validate.notNull(request);
			Validate.notNull(request.getIssuer());

			IdmAccessor accessor = t.getIdmAccessor();
			Validate.notNull(accessor);
			Validate.notNull(accessor.getTenant());

			// Validate assertion consumer service first, if that is valid, we can send SAML replies
            try {
                boolean validateACSWithMetadata = !this.isRequestSigned(t);
                String acsUrl = accessor.getAcsForRelyingParty(request
                        .getIssuer().getValue(), request
                        .getAssertionConsumerServiceIndex(), request
                        .getAssertionConsumerServiceURL(), request
                        .getProtocolBinding(), validateACSWithMetadata);

                t.setAcsUrl(acsUrl);
            } catch (IllegalStateException e) {
				// set validation result to 400
				log.debug("Caught illegal state exception while Validating {} returning 400",e.toString());
				vr = new ValidationResult(
						HttpServletResponse.SC_BAD_REQUEST,
						e.getMessage(),
						null);
			}

			// Validate ID
			if (vr == null && request.getID() == null) {
				vr = new ValidationResult(OasisNames.REQUESTER);
				log.debug("Validation FAILED - Request ID is missing");
			}

			// Validate version
			if (vr == null) {
				SAMLVersion version = request.getVersion();
				if ((version.getMajorVersion() > Shared.REQUIRED_SAML_VERSION.getMajorVersion()) ||
						version.getMajorVersion() == Shared.REQUIRED_SAML_VERSION.getMajorVersion() &&
						version.getMinorVersion() > Shared.REQUIRED_SAML_VERSION.getMinorVersion()) {
					// version too high
					vr = new ValidationResult(OasisNames.VERSION_MISMATCH,
							OasisNames.REQUEST_VERSION_TOO_HIGH);
					log.debug("Validation FAILED - Version is too high");
				} else if ((version.getMajorVersion() < Shared.REQUIRED_SAML_VERSION.getMajorVersion()) ||
						version.getMajorVersion() == Shared.REQUIRED_SAML_VERSION.getMajorVersion() &&
						version.getMinorVersion() < Shared.REQUIRED_SAML_VERSION.getMinorVersion()) {
					// version too low
					vr = new ValidationResult(OasisNames.VERSION_MISMATCH,
							OasisNames.REQUEST_VERSION_TOO_LOW);
					log.debug("Validation FAILED - Version is too low");
				}
			}

			// Validate IssueInstant only if this is a new request (i.e. it had not pass been validated)
			if (vr == null && !t.isExistingRequest()) {
				DateTime dtPlus = request.getIssueInstant();
				DateTime dtMinus = request.getIssueInstant();
				DateTime instant = new DateTime();
				long clockTolerance = accessor.getClockTolerance();
				if (dtPlus == null) {
					vr = new ValidationResult(OasisNames.REQUESTER);
					log.debug("Validation FAILED - Issue Instant is missing");
				} else {
					dtPlus = dtPlus.plus(clockTolerance);
					dtMinus = dtMinus.minus(clockTolerance);
					// dtPlus must be after now and dtMinus must be before now
					//	in order to satisfy clock tolerance
					if (dtPlus.isBefore(instant) || dtMinus.isAfter(instant)) {
						vr = new ValidationResult(OasisNames.REQUESTER);
						log.debug("Validation FAILED - Issue Instant outside of clock tolerance");
						log.debug("clockTolerance {}" , clockTolerance);
						log.debug("now {}" , instant);
						log.debug("dtPlus {}" , dtPlus.toString());
						log.debug("dtMinus {}" , dtMinus.toString());
					}
				}
			}

			// Destination URL skipped, this is already done by OpenSAML when parsing

			// validate scoping if presenet
            if (vr == null) {
                vr = validateScoping(t);
            }

			// signature must NOT be included
			if (vr == null) {
				if (request.getSignature() != null) {
					log.debug("Validation FAILED - Signature MUST NOT be present");
					vr = new ValidationResult(
						OasisNames.REQUESTER,
						OasisNames.REQUEST_UNSUPPORTED);
				}
			}

			// ensure that we don't accept unsigned requests if configuration requires signing
			if (vr == null) {

				try {
					boolean mustBeSigned =
							accessor.getAuthnRequestsSignedForRelyingParty(
									request.getIssuer().getValue());
					this.validateSigning(mustBeSigned, t);
				} catch (IllegalStateException e) {
					// set validation result to request denied
					log.error("Validation FAILED - unsigned request detected, signing required");
					vr = new ValidationResult(
							OasisNames.RESPONDER,
							OasisNames.REQUEST_DENIED);
				}
			}

			// validate NameIDPolicy if present
			if (vr == null) {
				NameIDPolicy policy = request.getNameIDPolicy();
				if (policy != null) {
					String format = policy.getFormat();
					if (format != null &&
							!format.equals(OasisNames.PERSISTENT) &&
							!format.equals(OasisNames.EMAIL_ADDRESS) &&
							!format.equals(SAMLNames.IDFORMAT_VAL_UPN.toString())) {
						log.error("Validation FAILED - unknown NameIDPolicy Format");
						vr = new ValidationResult(
								OasisNames.REQUESTER,
								OasisNames.INVALID_NAMEID_POLICY);
					}
				}
			}

			// validate conditions
			if (vr == null) {
			    Conditions conditions = request.getConditions();
			    if (conditions != null) {
			        // notBefore processing
			        DateTime notBefore = conditions.getNotBefore();
			        if (notBefore != null) {
			            // no additional validation, we'll use whatever client wants
			            t.setStartTime(notBefore.toDate());
			        }
			        // delegable and renewable conditions
			        for (Condition c : conditions.getConditions()) {
			            if (c == null) {
			                continue;
			            }
			            if (c instanceof RenewableType) {
			                t.setRenewable(true);
			            }
			            if (c instanceof DelegableType) {
			                t.setDelegable(true);
			            }
			        }
			    }
			}
			if (vr == null) {
				computeSupportedAuthnTypes(t, request);
			}

            // validation done
            if (vr == null) {
                log.info("Authentication request validation succeeded");
                vr = new ValidationResult(); // success

                // check if we need to convert a principal into emailAddress
                if (request.getNameIDPolicy() != null &&
                        request.getNameIDPolicy().getFormat() != null &&
                        request.getNameIDPolicy().getFormat().equals(OasisNames.EMAIL_ADDRESS)) {
                    t.setIdentityFormat(OasisNames.IDENTITY_FORMAT_EMAIL_ADDRESS);
                } else {
                    t.setIdentityFormat(OasisNames.IDENTITY_FORMAT_UPN);
                }
            }

        } catch (Exception e) {
            vr = new ValidationResult(
                     HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            log.debug("Caught exception while Validating " + e.toString() + ", returning 400");
        }
        return vr;
	}

    /**
     * validate the Scoping element in the SAML request and setup proxy
     * parameters.
     *
     * @param AuthnRequestState
     * @return ValidationResult
     */
    public ValidationResult validateScoping(AuthnRequestState t) {
        ValidationResult vr = null;
        IdmAccessor accessor = t.getIdmAccessor();
        String tenant = accessor.getTenant();

        List<String> eligibleLocalIdpList = getEligibleLocalIdpList(t);
        boolean isIDPSelectionEnabled = t.isIDPSelectionEnabled(tenant);
        List<String> eligibleExternalIdpList = getEligibleExternalIdpList(t);

        if (eligibleLocalIdpList.isEmpty() && eligibleExternalIdpList.isEmpty()) {
            return new ValidationResult(OasisNames.REQUESTER,
                    OasisNames.NO_SUPPORTED_IDP);
        }

        // 3 cases:
        // 1. If no eligible external idps, do local authentication
        // 2. If there are eligible external idps but idp selection is not enabled, do external idp authentication
        // 3. If there are eligible external idps and idp selection is enabled, need to choose idp
        if (eligibleExternalIdpList.isEmpty()) {
            log.debug("No eligible external idps. Do local authentication.");
            t.setProxying(false);
            t.setNeedChooseIDPView(false);
        } else if (!isIDPSelectionEnabled) {
            t.setProxying(true);
            if (eligibleExternalIdpList.size() == 1) {
                String entityId = eligibleExternalIdpList.iterator().next();
                IDPConfig idpConfig = accessor.getExternalIdpConfigForTenant(tenant, entityId);
                t.setExtIDPToUse(idpConfig);
                // turn off choose idp view
                t.setNeedChooseIDPView(false);
            } else {
                log.error("Requested websso proxying, tenant has more than one trusted IDP registered,"
                        + " no IPD specified in IDPList element of the request scoping specification.");
                vr = new ValidationResult(OasisNames.REQUESTER,
                        OasisNames.NO_SUPPORTED_IDP);
            }
        } else {
            vr = discoverIdentityProvidersForSelectionList(eligibleLocalIdpList, eligibleExternalIdpList, t);
        }

        log.info("Authn request proxyCount= " + t.getProxyCount()
                + " set isProxying=" + t.isProxying());

        return vr;
    }


    /**
     * Discover eligible idps that can be selected by user if proxying and choose idp view is enabled.
     * Set up proxy parameters and whether needLoginView is required.
     * Set cookies if idp has been selected.
     * Set validation result in invalid cases.
     */
    private ValidationResult discoverIdentityProvidersForSelectionList(List<String> eligibleLocalIdpList,
            List<String> eligibleExternalIdpList, AuthnRequestState t) {
        log.debug("Discovering eligible identity providers for authentication.");
        Validate.isTrue(t.isChooseIDPViewRequired() == null);

        IdmAccessor accessor = t.getIdmAccessor();
        String tenantIDPCookie = t.getTenantIDPCookie();
        String selectedIDPEntityId = tenantIDPCookie;

        if (tenantIDPCookie == null || tenantIDPCookie.isEmpty()) {
            String tenantIDPHeader = t.getTenantIDPSelectHeader();
            // if no idp header or cookie is available
            if (tenantIDPHeader == null || tenantIDPHeader.isEmpty()) {
                List<String> eligibleIdps = new ArrayList<>();
                // add vsphere.local
                eligibleIdps.add(accessor.getIdpEntityId());
                // add all registered external idps
                eligibleIdps.addAll(eligibleExternalIdpList);
                t.setIDPEntityIdList(eligibleIdps);
                log.debug("No IDP header or cookie is available. Redirect to choose idp view.");
                t.setNeedChooseIDPView(true);
                return null;
            } else {
                log.debug("EntityId from IDP header is " + tenantIDPHeader);
                selectedIDPEntityId = tenantIDPHeader;
                // set idp cookie regardless of validation result
                t.addTenantIDPCookie(tenantIDPHeader, t.getResponse());
            }
        }

        t.setNeedChooseIDPView(false);
        if (eligibleLocalIdpList.contains(selectedIDPEntityId)) {
            log.debug("Tenant IDP cookie is set to local idp. Not proxying.");
            t.setProxying(false);
        } else {
            IDPConfig idpConfigToUse = accessor.getExternalIdpConfigForTenant(accessor.getTenant(), selectedIDPEntityId);
            if (idpConfigToUse == null) {
                log.warn("External IDP with entity id : " + selectedIDPEntityId + " is not registered.");
                return new ValidationResult(
                        OasisNames.REQUESTER,
                        OasisNames.NO_SUPPORTED_IDP);
            } else {
                t.setExtIDPToUse(idpConfigToUse);
                t.setProxying(true);
            }
        }
        return null;
    }

    /**
     * Return a list of eligible external idps. If no eligible ones found, return an empty list.
     *
     * If scoping is set (proxyCount and IdpList), all registered idps that are in the Idplist are eligible.
     * If scoping is not set, all registered idps are eligible, meaning no restriction on the proxy count.
     *
     * @return an empty list if no registered external idp or saml request does not want to proxy
     */
    private List<String> getEligibleExternalIdpList(AuthnRequestState t) {
        Validate.notNull(t);
        IdmAccessor accessor = t.getIdmAccessor();
        Scoping scoping = t.getAuthnRequest().getScoping();
        IDPList idpList = null;
        List<String> validExternalIdpList = new ArrayList<>();

        // Get the IDPList and proxycount
        if (scoping != null) {
            // validate ProxyCount: only use exteral idp if proxy count is set
            // and > 0
            t.setProxyCount(scoping.getProxyCount());
            idpList = scoping.getIDPList();
        }

        // verify against proxy count and idpList if defined
        int proxyCount = t.getProxyCount() == null ? 0 : t.getProxyCount();
        if (proxyCount > 0 && idpList != null && idpList.getIDPEntrys() != null) {
            List<IDPEntry> list = idpList.getIDPEntrys();
            // If the list is provided: we will make sure it
            // a) if is proxying, the registered External IDP should be in the
            // list, if not force local authentication.
            // b) if not proxying && it should contain at least one eligible idp
            // note: SAML 2.0 processing rule does not require this. Since this
            // found IDP is not necessary
            // the one used later, it is unclear why we need this validation
            t.setIdpList(list);
            validExternalIdpList.addAll(findValidExternalIdpListWithinScoping(list, accessor));
            if (validExternalIdpList.isEmpty()) {
                log.warn("No trusted external IDP listed in SAML Request's IDPList. Force local authentication!");
            }
        } else {
            // IDPList is not provided
            // set extIDPToUse to the registered external IDP if
            // isProxying==true
            log.debug("IDPList not provided.  Choose from registered external IDP");
            Collection<IDPConfig> extIdps = accessor.getExternalIdps();
            if (extIdps != null && extIdps.size() > 0) {
                for (IDPConfig idpConfig : extIdps) {
                    validExternalIdpList.add(idpConfig.getEntityID());
                }
            }
        }

        return validExternalIdpList;
    }

    /**
     * This function updates AuthnRequestState.authnTypeSupported attribute by factoring in RequestedAuthnContext in the SAML request.
     *
     * Note about backward compatibility handling. vsphere 6.0u1 or older service providers integrates with older WebSSO client library that
     * sends SAML request with RequestedAuthnContext=={"PasswordProtectedTransport"} when application does not set authentication preference.
     * This function detects that and ignores the setting, as such treat the request as having null RequestedAuthnContext. The drawback
     * is that we can not distinguish an intentional setting of PW-only vs unintentional default value of older service provider. Such that "Use Windows session
     * authentication" checkbox undesirably appears - which is rather benign defect.
     * I think this is acceptable given PW authentication is 1-FA and least secure compare to other types. It is unlikely
     *  that third party SP in the federation would want to restrict authentication to only PW.
     * We could remove this handling code only when the future PSC phase out support of service providers that use WebSSO client lib older than that
     * integrated with vsphere 6.0u2. PR 1623511.
     *
     *
     * @param t
     * @param request
     */
    private void computeSupportedAuthnTypes(AuthnRequestState t,
            AuthnRequest request) {
        log.debug("Computing authentication types to be enabled for this request.");
        Validate.notNull(t, "AuthnRequestState");
        Validate.notNull(request, "AuthnRequest");
        //validate requested authentication context, consolidate with tenant setting to a final support options.
        RequestedAuthnContext authnContext = request.getRequestedAuthnContext();
        if (authnContext != null) {
            AuthnContextComparisonTypeEnumeration comparisonType = authnContext.getComparison();
            if (comparisonType != AuthnContextComparisonTypeEnumeration.EXACT) {
                log.warn("WEBSSO only support EXACT comparison type for RequestedAuthnContext. Ignore RequestedAuthnContext the element.");
            }
            else {
                AuthnTypesSupported allowedTypes = new AuthnTypesSupported(false, false, false,false);  //init with all off
                AuthnTypesSupported tenantAllowedTypes = t.getAuthTypesSupportecd();

                List<AuthnContextClassRef> requestedTypes = authnContext.getAuthnContextClassRefs();

                if (requestedTypes == null || requestedTypes.size() == 0) {
                    //no-op in case RequestedAuthnContext is empty
                    return;
                }

                //Backward compatible handling: for details refer to the function's java doc header.
                if (requestedTypes.size() == 1 && requestedTypes.get(0).getAuthnContextClassRef().equals(AuthnContext.PPT_AUTHN_CTX)) {
                    return;
                }

                for (AuthnContextClassRef requestedType : requestedTypes) {
                    String requestedTypeString  = requestedType.getAuthnContextClassRef();
                    if (requestedTypeString == null)
                        continue;
                    if (requestedTypeString.equals(AuthnContext.PPT_AUTHN_CTX) &&
                            tenantAllowedTypes.supportsPasswordProtectTransport()) {
                        allowedTypes.setPasswordProtectTransport(true);
                    }
                    else if (requestedTypeString.equals(AuthnContext.TLS_CLIENT_AUTHN_CTX) &&
                            tenantAllowedTypes.supportsTlsClientCert()){
                        allowedTypes.setTlsClientCert(true);
                    }
                    else if ((requestedTypeString.equals(AuthnContext.KERBEROS_AUTHN_CTX)
                            || requestedTypeString.equals(OasisNames.INTEGRATED_WINDOWS))
                            && tenantAllowedTypes.supportsWindowsSession()){
                        allowedTypes.setWindowsSession(true);
                    }
                }
                //updates with consolidated supported types

                t.setAuthnTypesSupported(allowedTypes);
            }
        }
        //else leave supportedAuthnType to the initial value which comes from tenant setting

    }

    // check if the provided IDPList contains a trusted external IDP
	// return the list of valid ones
	private List<String> findValidExternalIdpListWithinScoping(List<IDPEntry> requestIdpList, IdmAccessor accessor) {
	    Validate.notNull(requestIdpList);
	    Validate.notNull(accessor);
	    List<String> retVal = new ArrayList<>();
        Collection<IDPConfig> extIdps = accessor.getExternalIdps();

        if (extIdps == null || extIdps.isEmpty()) {
            log.debug("No external IDP registered! ");
        }
        else {
			for (IDPEntry entry : requestIdpList) {
				if (entry!=null) {
					IDPConfig foundConfig = accessor.getExternalIdpConfigForTenant(accessor.getTenant(),
							entry.getProviderID());
					if (foundConfig != null) {
						retVal.add(entry.getProviderID());
					}
				}
			}
        }
        log.debug("check if IDPList contain a trusted external IDP, result: {}", !retVal.isEmpty());

        return retVal;
	}

    //find eligible local IDPs for the tenant
	private List<String> getEligibleLocalIdpList(AuthnRequestState t) {
       Validate.notNull(t);
       IdmAccessor accessor = t.getIdmAccessor();

	   List<String> localIDPs = new ArrayList<String> ( Arrays.asList(
            accessor.getIdpEntityId(),
            accessor.getDefaultIdpEntityId()));
	   List<String> eligibleLocalIDPs = new ArrayList<>();

       IDPList idpList = t.getAuthnRequest().getScoping() == null ? null
               : t.getAuthnRequest().getScoping().getIDPList();

       if (idpList != null && idpList.getIDPEntrys() != null) {
           for (IDPEntry entry : idpList.getIDPEntrys()) {
               if (entry != null
                       && localIDPs.contains(entry.getProviderID())) {
                   eligibleLocalIDPs.add(entry.getProviderID());
               }
           }
           if (eligibleLocalIDPs.isEmpty()) {
               log.debug("samlp:Scoping:IDPList does not contain VMWare local Identity Store.");
           }
       } else {
           eligibleLocalIDPs = localIDPs;
       }

       return eligibleLocalIDPs;
    }

	// verify signing requirement for the request (actual signature, if present, is verified elsewhere)
	private void validateSigning(boolean mustBeSigned, AuthnRequestState requestState) {
		Validate.notNull(requestState);
        if (mustBeSigned && !isRequestSigned(requestState)) {
			throw new IllegalStateException(); // at least one parameter was empty
		}
	}

    private boolean isRequestSigned(AuthnRequestState requestState) {
        return (requestState.getSignatureAlgorithm() != null && requestState
                .getSignature() != null);
    }
}
