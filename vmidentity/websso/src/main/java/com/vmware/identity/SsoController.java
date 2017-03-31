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

package com.vmware.identity;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.Validate;
import org.opensaml.xml.util.Base64;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.AuthnTypesSupported;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.session.SessionManager;

/**
 * Handles requests for the application SSO page.
 */
@Controller
public class SsoController extends BaseSsoController {

    @Autowired
    private MessageSource messageSource;

    @Autowired
    private AuthenticationFilter<AuthnRequestState> kerbAuthenticator;

    @Autowired
    private AuthenticationFilter<AuthnRequestState> passwordAuthenticator;

    @Autowired
    private AuthenticationFilter<AuthnRequestState> rsaamAuthenticator;

    @Autowired
    private AuthenticationFilter<AuthnRequestState> tlsClientAuthenticator;

    @Autowired
    private AuthenticationFilter<AuthnRequestState> cookieAuthenticator;

    @Autowired
    private SessionManager sessionManager;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(SsoController.class);

    public SsoController() {}

    /**
     * Handle SAML AuthnRequest
     */
    @RequestMapping(value = "/websso/SAML2/SSO/{tenant:.*}", method = {RequestMethod.GET, RequestMethod.POST})
    public String sso(Locale locale, @PathVariable(value = "tenant") String tenant,
            Model model, HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        logger.info("Welcome to SP-initiated AuthnRequest handler! "
                + "The client locale is " + locale.toString() + ", tenant is "
                + tenant);
        logger.info("Request URL is " + request.getRequestURL().toString());

        try {
            AuthenticationFilter<AuthnRequestState> authenticator = chooseAuthenticator(request);
            AuthnRequestState requestState = new AuthnRequestState(request,response,
                    sessionManager,tenant);
            processSsoRequest(locale, tenant, request, response, authenticator,requestState,
                    messageSource, sessionManager);

            model.addAttribute("tenant", tenant);
            model.addAttribute("protocol", "websso");
            if (requestState.isChooseIDPViewRequired() != null && requestState.isChooseIDPViewRequired()) {
                setupChooseIDPModel(model,locale, tenant, requestState);
                return "chooseidp";
            } else if (requestState.isLoginViewRequired() != null && requestState.isLoginViewRequired()) {
                setupAuthenticationModel(model,locale, tenant, request, requestState);
                return "unpentry";
            }
        } catch (Exception e) {
            logger.error("Could not handle SAML Authentication request ", e);
            sendError(locale, response, e.getMessage());
        }
        return null;
    }

    /**
     * Reverse proxy is expected to challenge for the client certificate with the request.
     */
    @RequestMapping(value = "/websso/SAML2/SSOCAC/{tenant:.*}", method = {RequestMethod.GET, RequestMethod.POST})
    public String cacSso(Locale locale, @PathVariable(value = "tenant") String tenant,
            Model model, HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        return this.sso( locale, tenant, model, request, response );
    }

    /**
     * Client sending request to this endpoint will be challenged to submit certificate by SSL protocal.
     * Thus a reverse proxy is not expected in between browser and this server.
     */
    @RequestMapping(value = "/websso/SAML2/SmartcardRealm/{tenant:.*}", method = {RequestMethod.GET, RequestMethod.POST})
    public String smartcardRealmSso(Locale locale, @PathVariable(value = "tenant") String tenant,
            Model model, HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        return this.sso( locale, tenant, model, request, response );
    }

    /**
     * Choose auth method to use based on castle auth header contents
     * @param request
     * @return
     */
    private AuthenticationFilter<AuthnRequestState> chooseAuthenticator(
            HttpServletRequest request) {
        // default - try cookie
        AuthenticationFilter<AuthnRequestState> retval = this.getCookieAuthenticator();

        if (request != null) {
            String castleAuthType = request
                    .getParameter(Shared.REQUEST_AUTH_PARAM);

            if (castleAuthType != null) {
                if (castleAuthType.startsWith(Shared.KERB_AUTH_PREFIX)) {
                    retval = this.getKerbAuthenticator();
                    logger.debug("Kerb authenticator chosen");
                } else if (castleAuthType.startsWith(Shared.PASSWORD_AUTH_PREFIX)) {
                    retval = this.getPasswordAuthenticator();
                    logger.debug("Password authenticator chosen");
                } else if (castleAuthType.startsWith(Shared.RSAAM_AUTH_PREFIX)) {
                    retval = this.getRsaAmAuthenticator();
                    logger.debug("Rsa am authenticator chosen");
                } else if (castleAuthType.startsWith(Shared.TLSCLIENT_AUTH_PREFIX)) {
                    retval = this.getTlsClientAuthenticator();
                    logger.debug("TLSClient authenticator chosen");
                }
            }
        }

        return retval;
    }

    /**
     * Handle SSL for NGC. NGC is to host login page from iFrame which would fails silently if the server site is not trusted
     * by browser. This interactive exchange allow user to trust the ssl at the first time browsing this site.
     */
      @RequestMapping(value = "/websso/SAML2/SSOSSL/{tenant:.*}", method = {RequestMethod.GET, RequestMethod.POST})
    public void ssoSSLDummy(Locale locale, Model model,@PathVariable(value = "tenant") String tenant,
            HttpServletRequest request, HttpServletResponse response)
                    throws IOException {
        logger.info("Welcome to ssoSSLDummy handler! "
                + tenant);

        try {
            RelyingParty rp = validateRelyingParty(request,tenant);
            if (rp != null) {
                logger.info("Send ssl probing reponse for: "+rp.getUrl());

                URL rpUrl = new URL(rp.getUrl());
                //For ciswin environments, redirect without trailing slash will be on http
                // and this causes login issues.
                // TODO: if both NGC  and WebSSO are on same machine, it will be nice to communicate
                // directly without going through reverse proxy.
                // see PR 1311527, 1281321
                URL redirectUrl = new URL(rpUrl.getProtocol(), rpUrl.getHost(), rpUrl.getPort(), "/vsphere-client/");
                String redirectUrlStr = redirectUrl.toString() + "?" + ssoSSLDummyQueryString(request);
                response.sendRedirect(redirectUrlStr);
            }
            else {
                logger.error("Not able to respond to the request posted to /SAML2/SSOSSL/"+tenant+
                        ": No replying party was found to match the given query parameter!");
                sendError(locale, response, "Not able to respond to the request posted to /SAML2/SSOSSL/");
            }
        } catch (Exception e) {
            logger.error("Caught exception in ssoSSLDummy", e);
            sendError(locale, response, e.getLocalizedMessage());
        }

    }

      /**
       * Handle SSL for default tenant. NGC is to host login page from iFrame which would fails silently if the server site is not trusted
       * by browser. This interactive exchange allow user to trust the ssl at the first time browsing this site.
       */
        @RequestMapping(value = "/websso/SAML2/SSOSSL", method = {RequestMethod.GET, RequestMethod.POST})
      public void ssoSSLDummyDefault(Locale locale, Model model,
              HttpServletRequest request, HttpServletResponse response)
                      throws IOException {
          ssoSSLDummy(locale,model, Shared.getDefaultTenant(), request, response);
      }

      /**
       * Handle SAML AuthnRequest for default tenant
       */
    @RequestMapping(value = "/websso/SAML2/SSO", method = {RequestMethod.GET, RequestMethod.POST})
    public void ssoDefaultTenant(Locale locale, Model model,
            HttpServletRequest request, HttpServletResponse response)
                    throws IOException {
        logger.info("Welcome to SP-initiated AuthnRequest handler! "
                + "The client locale is " + locale.toString()
                + ", DEFAULT tenant");

        sso(locale, Shared.getDefaultTenant(), model, request, response);
    }

    /**
     * Reverse proxy is expected to challenge for the client certificate with the request of default tenant.
     */
    @RequestMapping(value = "/websso/SAML2/SSOCAC", method = {RequestMethod.GET, RequestMethod.POST})
    public void cacSsoDefaultTenant(Locale locale, Model model,
        HttpServletRequest request, HttpServletResponse response)
        throws IOException {
        ssoDefaultTenant(locale, model, request, response);
    }

    /**
     * Client sending request of default tenant to this endpoint will be challenged to submit certificate by SSL protocal.
     * Thus a reverse proxy is not expected in between browser and this server.
     */
    @RequestMapping(value = "/websso/SAML2/SmartcardRealm", method = {RequestMethod.GET, RequestMethod.POST})
    public void smartcardRealmSsoDefaultTenant(Locale locale, Model model,
        HttpServletRequest request, HttpServletResponse response)
        throws IOException {
        ssoDefaultTenant(locale, model, request, response);
    }

    /**
     * Handle request sent with a wrong binding
     */
    @RequestMapping(value = "/websso/SAML2/SSO/{tenant:.*}")
    public void ssoBindingError(Locale locale,
            @PathVariable(value = "tenant") String tenant, HttpServletResponse response)
                    throws IOException {
        logger.info("SSO binding error! The client locale is "
                + locale.toString() + ", tenant is " + tenant);

        ssoDefaultTenantBindingError(locale, response);
    }

    /**
     * Handle default tenant request sent with a wrong binding
     */
    @RequestMapping(value = "/websso/SAML2/SSO")
    public void ssoDefaultTenantBindingError(Locale locale,
            HttpServletResponse response) throws IOException {
        logger.info("SSO binding error! The client locale is "
                + locale.toString() + ", DEFAULT tenant");
        sendError(locale, response, "Binding");
    }

    /**
     * Handle SAML AuthnRequest, UNP entry form
     */
    @RequestMapping(value = "/websso/SAML2/SSO/{tenant:.*}", method = RequestMethod.GET, params = Shared.PASSWORD_ENTRY)
    public String ssoPasswordEntry(Locale locale,
            @PathVariable(value = "tenant") String tenant, Model model,
            HttpServletRequest request, HttpServletResponse response)
                    throws IOException {
        logger.info("Welcome to SP-initiated AuthnRequest handler, PASSWORD entry form! "
                + "The client locale is "
                + locale.toString()
                + ", tenant is "
                + tenant);

        try {
            // fix for PR 964366, check the cookie first
            if (Shared.hasSessionCookie(request, this.getSessionManager(), tenant)) {
                sso(locale, tenant, model, request, response);
                return null;
            }
            model.addAttribute("tenant", tenant);
            model.addAttribute("protocol", "websso");
            setupAuthenticationModel(model,locale, tenant, request, null);
        } catch (Exception e) {
            logger.error("Found exception while populating model object ", e);
            sendError(locale, response, e.getLocalizedMessage());
            return null;
        }

        return "unpentry";
     }

    private void setupChooseIDPModel(Model model, Locale locale, String tenant, AuthnRequestState requestState) {
        model.addAttribute("tenant", tenant);
        model.addAttribute("protocol", "websso");
        model.addAttribute("tenant_brandname", StringEscapeUtils.escapeJavaScript(getBrandName(tenant)));
        List<String> entityIdList = requestState.getIDPSelectionEntityIdList();
        model.addAttribute("idp_entity_id_list",  StringEscapeUtils.escapeJavaScript(entityIdList.toString()));
        model.addAttribute("idp_display_name_list", StringEscapeUtils.escapeJavaScript(requestState.getIDPSelectionDisplayNameList(entityIdList).toString()) );
        model.addAttribute("title",
                messageSource.getMessage("ChooseIDP.Title", null, locale));
    }

    private void setupAuthenticationModel(Model model, Locale locale, String tenant, HttpServletRequest request, AuthnRequestState requestState) {

        DefaultIdmAccessorFactory idmFactory = new DefaultIdmAccessorFactory();
        IdmAccessor idmAccessor = idmFactory.getIdmAccessor();

        model.addAttribute("spn", StringEscapeUtils.escapeJavaScript(getServerSPN()));
        model.addAttribute("tenant_brandname", StringEscapeUtils.escapeJavaScript(getBrandName(tenant)));
        model.addAttribute("username",
                messageSource.getMessage("LoginForm.UserName", null, locale));
        model.addAttribute("username_placeholder",
                messageSource.getMessage("LoginForm.UserName.Placeholder", null, locale));
        model.addAttribute("password",
                messageSource.getMessage("LoginForm.Password", null, locale));
        model.addAttribute("passcode",
                messageSource.getMessage("LoginForm.Passcode", null, locale));
        model.addAttribute("submit",
                messageSource.getMessage("LoginForm.Submit", null, locale));
        model.addAttribute("error",
                StringEscapeUtils.escapeJavaScript(messageSource.getMessage("LoginForm.Error", null, locale)));
        model.addAttribute("login",
                messageSource.getMessage("LoginForm.Login", null, locale));
        model.addAttribute("help",
                messageSource.getMessage("LoginForm.Help", null, locale));
        model.addAttribute("winSession",
                messageSource.getMessage("LoginForm.WinSession", null, locale));
        model.addAttribute("downloadCIP",
                messageSource.getMessage("LoginForm.DownloadCIP", null, locale));
        model.addAttribute("unsupportedBrowserWarning",
                messageSource.getMessage("LoginForm.UnsupportedBrowserWarning", null, locale));
        model.addAttribute("searchstring", Shared.PASSWORD_ENTRY);
        model.addAttribute("replacestring", Shared.PASSWORD_SUPPLIED);
        model.addAttribute("iAgreeTo",
                messageSource.getMessage("LoginForm.IAgreeTo", null, locale));
        model.addAttribute("logonBannerAlertMessage",
                StringEscapeUtils.escapeJavaScript(messageSource.getMessage("LoginForm.LogonBannerAlertMessage", null, locale)));

        setLogonBannerModelAttributes(tenant, model, idmAccessor);

        AuthnTypesSupported supportedAuthnTypes;
        if (requestState != null) {
            supportedAuthnTypes = requestState.getAuthTypesSupportecd();
        } else {
            supportedAuthnTypes = new AuthnTypesSupported(true,true, false, false);
        }
        if (supportedAuthnTypes.supportsTlsClientCert()) {
            boolean enableHint = idmAccessor.getAuthnPolicy(tenant).getClientCertPolicy().getEnableHint();
            if (enableHint) {
                model.addAttribute("username_hint",
                        messageSource.getMessage("LoginForm.UserNameHint", null, locale));
                model.addAttribute("username_hint_placeholder",
                        messageSource.getMessage("LoginForm.UserNameHint.Placeholder", null, locale));
            }
        }
        model.addAttribute("enable_password_auth", supportedAuthnTypes.supportsPasswordProtectTransport());
        model.addAttribute("enable_windows_auth",supportedAuthnTypes.supportsWindowsSession());
        model.addAttribute("enable_tlsclient_auth",supportedAuthnTypes.supportsTlsClientCert());
        model.addAttribute("enable_rsaam_auth",supportedAuthnTypes.supportsRsaSecureID());

        model.addAttribute("smartcard",
                        messageSource.getMessage("LoginForm.Smartcard", null, locale));
        model.addAttribute("rsaam", messageSource.getMessage("LoginForm.RsaSecurID", null, locale));

        String cacEndpoint = request.getAuthType() == SecurityRequestWrapper.VMWARE_CLIENT_CERT_AUTH ? Shared.ssoCACEndpoint : Shared.ssoSmartcardRealmEndpoint;
        model.addAttribute("cac_endpoint", cacEndpoint);

        model.addAttribute("sso_endpoint", Shared.ssoEndpoint);

        if (supportedAuthnTypes.supportsRsaSecureID()) {
            model.addAttribute("rsaam_reminder",
                    StringEscapeUtils.escapeJavaScript(getRsaSecurIDLoginGuide(tenant)));
        }
    }

    /**
     * Handle SAML AuthnRequest for default tenant, UNP entry form
     */
    @RequestMapping(value = "/websso/SAML2/SSO", method = RequestMethod.GET, params = Shared.PASSWORD_ENTRY)
    public String ssoDefaultTenantPasswordEntry(Locale locale, Model model,
            HttpServletRequest request, HttpServletResponse response)
                    throws IOException {
        logger.info("Welcome to SP-initiated AuthnRequest handler, PASSWORD entry form! "
                + "The client locale is "
                + locale.toString()
                + ", DEFAULT tenant");

        return ssoPasswordEntry(locale, Shared.getDefaultTenant(), model, request, response);
    }

    /**
     * Handle SSL for landing page.
     */
    @RequestMapping(value = "/websso/", method = RequestMethod.GET)
    public String webssoRootPageEntry(Locale locale, Model model,
            HttpServletRequest request, HttpServletResponse response)
                    throws IOException {
        logger.info("Welcome to webssoRootPage handler! ");
        return "index";
    }

    public MessageSource getMessageSource() {
        return messageSource;
    }

    public void setMessageSource(MessageSource ms) {
        messageSource = ms;
    }

    public AuthenticationFilter<AuthnRequestState> getKerbAuthenticator() {
        return kerbAuthenticator;
    }

    public void setKerbAuthenticator(
            AuthenticationFilter<AuthnRequestState> kerbAuthenticator) {
        this.kerbAuthenticator = kerbAuthenticator;
    }

    /**
     * @return the passwordAuthenticator
     */
    public AuthenticationFilter<AuthnRequestState> getPasswordAuthenticator() {
        return passwordAuthenticator;
    }

    /**
     * @param passwordAuthenticator
     *            the passwordAuthenticator to set
     */
    public void setPasswordAuthenticator(
            AuthenticationFilter<AuthnRequestState> passwordAuthenticator) {
        this.passwordAuthenticator = passwordAuthenticator;
    }

    /**
     * @return the rsaamAuthenticator
     */
    public AuthenticationFilter<AuthnRequestState> getRsaAmAuthenticator() {
        return rsaamAuthenticator;
    }

    /**
     * @param rsaamAuthenticator
     *            the rsaamAuthenticator to set
     */
    public void setRsaAmAuthenticator(
            AuthenticationFilter<AuthnRequestState> rsaamAuthenticator) {
        this.rsaamAuthenticator = rsaamAuthenticator;
    }

    /**
     * @return the sessionManager
     */
    public SessionManager getSessionManager() {
        return sessionManager;
    }

    /**
     * @param sessionManager
     *            the sessionManager to set
     */
    public void setSessionManager(SessionManager sessionManager) {
        this.sessionManager = sessionManager;
    }

    /**
     * @return the cookieAuthenticator
     */
    public AuthenticationFilter<AuthnRequestState> getCookieAuthenticator() {
        return cookieAuthenticator;
    }

    /**
     * @param cookieAuthenticator the cookieAuthenticator to set
     */
    public void setCookieAuthenticator(AuthenticationFilter<AuthnRequestState> cookieAuthenticator) {
        this.cookieAuthenticator = cookieAuthenticator;
    }

    /**
     * Allows view jsp to query brandname of the tenant
     *
     * @param tenantName
     *            the cookieAuthenticator to set
     * @return tenant's brandname as ModelAttribute
     */
    // @ModelAttribute("tenant_brandname")
    public String getBrandName(String tenantName) {
         //initiate idmAccessor
        if (tenantName == null || tenantName.isEmpty()) {
            return null;
        }
        DefaultIdmAccessorFactory idmFactory = new DefaultIdmAccessorFactory();
        Validate.notNull(idmFactory, "idmFactory");
        IdmAccessor idmAccessor = idmFactory.getIdmAccessor();

        idmAccessor.setTenant(tenantName);
        String brand = idmAccessor.getBrandName();
        logger.info("Accessing Tenant " + tenantName
                + ", brand name string " + brand);
        return brand;
    }

    /**
     * Allows view jsp to query logon banner attributes of the tenant
     *
     * @param tenant
     *            the cookieAuthenticator to set

     */
    // @ModelAttribute("tenant_logonbanner_title")
    // @ModelAttribute("tenant_logonbanner_content")
    // @ModelAttribute("enable_logonbanner_checkbox")
    private void setLogonBannerModelAttributes(String tenant, Model model, IdmAccessor idmAccessor) {
        if (tenant == null || tenant.isEmpty()) {
            return;
        }

        idmAccessor.setTenant(tenant);
        String logonBannerTitle = idmAccessor.getLogonBannerTitle();
        String logonBannerContent = idmAccessor.getLogonBannerContent();

        // escape html and javascript
        logonBannerTitle = StringEscapeUtils.escapeJavaScript(StringEscapeUtils.escapeHtml(logonBannerTitle));
        logonBannerContent = StringEscapeUtils.escapeJavaScript(StringEscapeUtils.escapeHtml(logonBannerContent));

        logger.trace("Accessing Tenant " + tenant + ", logon banner title " + logonBannerTitle);
        logger.trace("Accessing Tenant " + tenant + ", logon banner content" + System.lineSeparator() + logonBannerContent);
        model.addAttribute("tenant_logonbanner_title", logonBannerTitle);
        model.addAttribute("tenant_logonbanner_content", logonBannerContent);
        model.addAttribute("enable_logonbanner_checkbox", idmAccessor.getLogonBannerCheckboxFlag());
    }

    /**
     * return tenant setting of rsa login guide.
     * @param tenantName
     * @return guide string. if not defined.
     */
    private String getRsaSecurIDLoginGuide(String tenantName) {
       if (tenantName == null || tenantName.isEmpty()) {
           return null;
       }
       DefaultIdmAccessorFactory idmFactory = new DefaultIdmAccessorFactory();
       Validate.notNull(idmFactory, "idmFactory");
       IdmAccessor idmAccessor = idmFactory.getIdmAccessor();

       idmAccessor.setTenant(tenantName);
       RSAAgentConfig config = idmAccessor.getAuthnPolicy(tenantName).get_rsaAgentConfig();
       String rsaLoginGuide = null;
       if (null != config) {
           rsaLoginGuide = config.get_loginGuide();
       }
       return rsaLoginGuide;
    }

    private String getServerSPN() {
        DefaultIdmAccessorFactory idmFactory = new DefaultIdmAccessorFactory();
        IdmAccessor idmAccessor = idmFactory.getIdmAccessor();
        String spn = idmAccessor.getServerSPN();
        logger.info("Server SPN is " + spn);
        return spn;
    }

    private void sendError(Locale locale, HttpServletResponse response, String subStatus) throws IOException {
        // use validation result code to return error to client
        ValidationResult vr = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, "BadRequest", subStatus);
        String message = vr.getMessage(messageSource, locale);
        response.sendError(vr.getResponseCode(), Shared.encodeString(message));
        logger.info("Responded with ERROR " + vr.getResponseCode() + ", message " + message);
    }

    /**
     * This method validates the relying party.
     * @param request
     * @param tenantName
     * @return
     */
    private RelyingParty validateRelyingParty(HttpServletRequest request,String tenantName ) {
        Validate.notNull(request, "request");
        Validate.notEmpty(tenantName,"tenantName");
        DefaultIdmAccessorFactory idmFactory = new DefaultIdmAccessorFactory();
        Validate.notNull(idmFactory, "idmFactory");
        IdmAccessor idmAccessor = idmFactory.getIdmAccessor();
        idmAccessor.setTenant(tenantName);

        String encodedEntityId = request.getParameter(Shared.RELYINGPARTY_ENTITYID);
        if (encodedEntityId == null || encodedEntityId.isEmpty()) {
            logger.error("No Relying Party's entity ID found. Ignore the request!");
            return null;
        }
        String rpEntityId = new String(Base64.decode(encodedEntityId));
        RelyingParty rp = idmAccessor.getRelyingPartyByUrl(rpEntityId);

        if (rp != null) {
            return rp;
        }
        else {
            logger.error("Unknown relying party: "+rpEntityId);
            return null;
        }
    }

    /**
     * reconstruct original query string minus RelyingPartyEntityId plus csp
     * @param request
     * @return
     * @throws MalformedURLException
     */
    static String ssoSSLDummyQueryString(HttpServletRequest request) throws MalformedURLException {
        StringBuilder result = new StringBuilder();

        Map<String, String[]> parameterMap = request.getParameterMap();
        boolean appendAmpersand = false;
        for (Entry<String, String[]> entry : parameterMap.entrySet()) {
            String key = entry.getKey();
            if (Shared.RELYINGPARTY_ENTITYID.equals(key)) {
                continue;
            }
            String[] values = entry.getValue();
            for (String value : values) {
                if (appendAmpersand) {
                    result.append("&");
                }
                if (value == null || value.length() == 0) {
                    result.append(String.format("%s", key));
                } else {
                    result.append(String.format("%s=%s", key, value));
                }
                appendAmpersand = true;
            }
        }

        if (appendAmpersand) {
            result.append("&");
        }
        result.append("csp");

        return result.toString();
    }

    public AuthenticationFilter<AuthnRequestState> getTlsClientAuthenticator() {
        return tlsClientAuthenticator;
    }

    public void setTlsClientAuthenticator(AuthenticationFilter<AuthnRequestState> tlsClientAuthenticator) {
        this.tlsClientAuthenticator = tlsClientAuthenticator;
    }
}
