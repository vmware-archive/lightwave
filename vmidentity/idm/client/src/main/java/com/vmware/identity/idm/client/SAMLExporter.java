/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.idm.client;

/** VMware Identity Service
*
* SAML meta data exporter
* Helper class for idm client for exporting tenant configuration
* This wiki lists the supported features of the exporter.
*
* @author:  schai <schai@vmware.com>
*
* @version: 1.0
* @since:   2012-04-12
*
*/

import java.net.Inet6Address;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.UnknownHostException;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.EnumSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.opensaml.xml.util.Base64;
import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServiceEndpoint;

class SAMLExporter {

    //Saml2 metadata spec:
    //   id=saml-metadata-2.0-os
    //   location=http://docs.oasis-open.org/security/saml/v2.0/saml-metadata-2.0-os.pdf

    private final CasIdmClient idmClient;
    private String tenantName;
    private Document doc;

    public SAMLExporter(CasIdmClient client) {
        this.idmClient = client;
        this.tenantName = null;

    }

    /**
     * exportConfiguration() create saml metadata of a specified tenant. a
     * tenant associated to it.
     *
     * @param tenantName
     *            <in> tenant name of the entity.
     * @param exportPrivateData
     *            <in> whether to export complete configuration or not.
     * @throws Exception
     *             ,DOMEception, IDMException
     * @return none Restriction: Root must be EntityDescriptor Support:
     *         Extension Organization IDPSSODescriptor
     */
    public Document exportConfiguration(String tenantName,
            boolean exportPrivateData) throws Exception {
        this.tenantName = tenantName;

        // Return tenant configuration in a file.
        DocumentBuilder docBuilder = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder();
        doc = docBuilder.newDocument();

        Element root = createEntitiesDescriptor();
        doc.appendChild(root);

        // Export IDP
        Element entEle = createIDPEntityDescriptor(exportPrivateData);
        root.appendChild(entEle);

        if (exportPrivateData) {
            // Create one SPSSODescriptor for each relying party.
            Collection<RelyingParty> rpList = idmClient
                    .getRelyingParties(tenantName);
            Iterator<RelyingParty> it = rpList.iterator();

            while (it.hasNext()) {
                RelyingParty rp = it.next();
                Element spEntEle = createSPEntityDescriptor(rp);
                root.appendChild(spEntEle);
            }
        }
        return doc;

    }

    /**
     * exportConfiguration() create saml metadata of a specified tenant. a
     * tenant associated to it.
     *
     * @param tenantName
     *            <in> tenant name of the entity.
     * @param exportPrivateData
     *            <in> whether to export complete configuration or not.
     * @throws Exception
     *             ,DOMEception, IDMException
     * @return none Restriction: Root must be EntityDescriptor Support:
     *         Extension Organization IDPSSODescriptor
     */
    public Document exportSaml2Metadata(String tenantName) throws Exception {
        this.tenantName = tenantName;

        // Return tenant configuration in a file.
        DocumentBuilder docBuilder = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder();
        doc = docBuilder.newDocument();

        // per saml-metadata-2.0-os:
        //    Entities MAY publish their metadata documents at a well known location
        //    by placing the document at the location denoted by its unique identifier,
        //    which MUST be in the form of a URL (rather than a URN).
        //
        //    The XML document provided at the well-known location MUST describe the
        //    metadata only for the entity represented by the unique identifier
        //    (that is, the root element MUST be an <EntityDescriptor> with an entityID
        //     matching the location).
        //     Thus the <EntitiesDescriptor> element MUST NOT be used in documents published
        //     using this mechanism, since a group of entities are not defined by such an identifier.
        Element entityEle =
                doc.createElementNS(SAMLNames.NS_NAME_SAML_METADATA,
                      SAMLNames.ENTDESCRIPTOR);
        entityEle.setAttribute(SAMLNames.NS_NAME_SAML_SAML,
                SAMLNames.NS_VAL_SAML_SAML);
        entityEle.setAttribute(SAMLNames.NS_NAME_SAML_VMWARE_ES,
                SAMLNames.NS_VAL_SAML_VMWARE_ES);

        // TODO: set expiration or valid until will require consumer to have logic to refresh
        // the metadata registration. Do we need to do this?
        //entityEle.setAttribute(SAMLNames.VALIDUNTIL, getExpirationDate(1, 0));

        // Extension
        Element extEle = createExtensions();
        entityEle.appendChild(extEle);

        doc.appendChild(entityEle);

        String id = this.getEntityId(tenantName);

        entityEle.setAttribute(SAMLNames.ENTID, id);

        // IPSSODescriptor
        Element idpSSO = createIDPSSODescriptor(false);
        entityEle.appendChild(idpSSO);

        // Add  SPSSODescriptor for CastleAsSP
        appendSPSSODescriptorForCastleAsSP(entityEle, id);

        return doc;
    }

    public Document exportCastleSPProfile(String tenantName, boolean exportExternalIDPData) throws Exception
    {
        this.tenantName = tenantName;
        // Return tenant configuration in a file.
        DocumentBuilder docBuilder = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder();
        doc = docBuilder.newDocument();

        // Use EntitiesDescriptor as root here.
        // Alternatively, since only one entity (Castle) is exported here, we
        // could switch to use EntityDescriptor as root later.
        Element root = createEntitiesDescriptor();
        doc.appendChild(root);

        Element entEle = doc.createElementNS(null, SAMLNames.ENTDESCRIPTOR);
        root.appendChild(entEle);

        String entityId = this.getEntityId(tenantName);

        entEle.setAttribute(SAMLNames.ENTID, entityId);

        // Add  SPSSODescriptor for CastleAsSP
        appendSPSSODescriptorForCastleAsSP(entEle, entityId);
        // Add IDPSSODescriptor for external trusted IDP
        if (exportExternalIDPData) {
            appendIDPSSODescriptorForExternalIDPs(entEle);
        }

        return doc;
    }

    private void appendSPSSODescriptorForCastleAsSP(Element parent, String entityID) throws Exception
    {
        Element spssodescriptor = doc.createElement(SAMLNames.SPSSODESCRIPTOR);
        spssodescriptor.setAttribute(SAMLNames.AUTHNREQUESTSIGNED,
                Boolean.FALSE.toString());

        //optional
        spssodescriptor.setAttribute(SAMLNames.WANTASSERTIONSSIGNED,
                Boolean.TRUE.toString());//of course!

        spssodescriptor.setAttribute(SAMLNames.PSE, SAMLNames.REQUIREDPROTOCAL);

        List<Certificate> certs = idmClient.getTenantCertificate(tenantName);

        Element keyDescriptor = createSPKeyDescriptor(certs);

        spssodescriptor.appendChild(keyDescriptor);

        //compose SLO services
        spssodescriptor.appendChild(createSingleLogOutForCastleAsSP(
                SAMLNames.HTTP_REDIRECT_BINDING));

        //compose/add NameIDFormats
        createNameIDFormats(spssodescriptor, false);

        String normalizedLoc = convertToIPV6ShortForm(entityID);
        String acsLocStr =
 normalizedLoc.replaceAll(SAMLNames.ENTITY_ID_PLACEHOLDER, SAMLNames.SP_ASSERTIONCONSUMERSERVICE_PLACEHOLDER);

        int index = 0;
        Element ssoEndpoint =
                createIndexedEndPoint(SAMLNames.ASSERTIONCONSUMERSERVICE,
                        SAMLNames.HTTP_POST_BINDING, acsLocStr, null, index++,
                        null);

        spssodescriptor.appendChild(ssoEndpoint);

        parent.appendChild(spssodescriptor);
    }

    /**
     * Compression ipv6 basing on RFC 5952 standard
     *
     * @param entityID in an URL form
     * @throws MalformedURLException
     * @return The compressed URL string representation of the input (in case
     * it is ipv6 address). Same as input if it is not ipv6 address or the host is unknown
     */
    public static String convertToIPV6ShortForm(String entityID) throws MalformedURLException {

        URL inputURL = new URL(entityID);

        //get host address in form of [1:2:4:4:4:5:0:9]
        String hostAddress = inputURL.getHost();
        String retAddress = entityID;

        //Skip IPV4 addresses
        if (! (hostAddress.startsWith("[") && hostAddress.endsWith("]") )) {
            return retAddress;
        }

        try {
            //get raw host address in form of 1:2:4:4:4:5:0:9
            String longAddress = Inet6Address.getByName(hostAddress).getHostAddress();

            String shortHostAddress =  longAddress.replaceAll("((?:(?:^|:)0+\\b){2,}):?(?!\\S*\\b\\1:0+\\b)(\\S*)", "::$2");
            URL retUrl = new URL( inputURL.getProtocol(), shortHostAddress, inputURL.getPort(), inputURL.getFile());
            retAddress = retUrl.toString();

        } catch (UnknownHostException e) {
            //just don't do conversion
        }
        return retAddress;
    }

    private void appendIDPSSODescriptorForExternalIDPs(Element parent) throws Exception
    {
        for (IDPConfig idpConfig : idmClient.getAllExternalIdpConfig(tenantName))
        {
            Element idpssoDescriptor = doc.createElement(SAMLNames.IDPSSODESCRIPTOR);
            idpssoDescriptor.setAttribute(SAMLNames.PSE, SAMLNames.REQUIREDPROTOCAL);
            idpssoDescriptor.setAttribute(SAMLNames.WANTSIGNED, Boolean.FALSE.toString());

            //key descriptor, should be only one chain for signing
            idpssoDescriptor.appendChild(createSPKeyDescriptor(idpConfig
                    .getSigningCertificateChain()));

            //SLO services
            for (ServiceEndpoint slo : idpConfig.getSloServices())
            {
                Element sloElem = doc.createElement(SAMLNames.SINGLELOGOUTSERVICE);
                sloElem.setAttribute(SAMLNames.BINDING, slo.getBinding());
                sloElem.setAttribute(SAMLNames.LOCATION, slo.getEndpoint());
                idpssoDescriptor.appendChild(sloElem);
            }

            //NameIDFormats
            for (String format : idpConfig.getNameIDFormats())
            {
                Element formatElem = doc.createElement(SAMLNames.NAMEIDFORMAT);
                formatElem.appendChild(doc.createTextNode(format));
                idpssoDescriptor.appendChild(formatElem);
            }

            //SSO services
            for (ServiceEndpoint sso : idpConfig.getSsoServices())
            {
                Element ssoElem = doc.createElement(SAMLNames.SINGLESIGNONSERVICE);
                ssoElem.setAttribute(SAMLNames.BINDING, sso.getBinding());
                ssoElem.setAttribute(SAMLNames.LOCATION, sso.getEndpoint());
                idpssoDescriptor.appendChild(ssoElem);
            }
            parent.appendChild(idpssoDescriptor);
        }
    }

    private Element createEntitiesDescriptor() {
        Element entitiesEle =
                doc.createElementNS(SAMLNames.NS_NAME_SAML_METADATA,
                      SAMLNames.ENTITIESDESCRIPTOR);
        entitiesEle.setAttribute(SAMLNames.NS_NAME_SAML_SAML,
                SAMLNames.NS_VAL_SAML_SAML);
        entitiesEle.setAttribute(SAMLNames.NS_NAME_SAML_VMWARE_ES,
                SAMLNames.NS_VAL_SAML_VMWARE_ES);

        entitiesEle.setAttribute(SAMLNames.NAME, tenantName);
        entitiesEle.setAttribute(SAMLNames.VALIDUNTIL, getExpirationDate(1, 0));
        // Extension
        Element extEle = createExtensions();
        entitiesEle.appendChild(extEle);

        return entitiesEle;
    }

    /**
     * Create Extension element of EntityDescriptor in DOM.
     *
     * @param exportPrivateData
     *            whether to export complete configuration or not
     * @return Element Document object that contain the EntityDecriptor.
     * @throws Exception
     *             Note: validUntil current fixed at currentTime+1 day. Date is
     *             in 'UTC' time.
     */
    private Element createIDPEntityDescriptor(boolean exportPrivateData)
            throws Exception {
        Element entEle = doc.createElementNS(null, SAMLNames.ENTDESCRIPTOR);
        String id = idmClient.getEntityID(tenantName);
        String alias = idmClient.getLocalIDPAlias(tenantName);
        if (id == null) {
            // entID is required attribute
            id = tenantName;
        }

        entEle.setAttribute(SAMLNames.ENTID, id);

        // IPSSODescriptor
        Element idpSSO = createIDPSSODescriptor(exportPrivateData);

        entEle.appendChild(idpSSO);

        if (exportPrivateData) {
            // Append Organization Element
            if (alias == null || alias.isEmpty()) {
                alias = id;
            }
            Element orgEle = createOrganization(tenantName, tenantName, alias,
                    SAMLNames.ENGLISH);
            entEle.appendChild(orgEle);
        }
        return entEle;
    }

    /**
     * CrtityDescriptor in DOM.
     *
     * @return Element Document object that contain the EntityDecriptor.
     * @throws Exception
     *             Note: validUntil current fixed at currentTime+1 day. Date is
     *             in 'UTC' time.
     */
    private Element createSPEntityDescriptor(RelyingParty rp) throws Exception {

        Element entEle = doc.createElementNS(null, SAMLNames.ENTDESCRIPTOR);
        entEle.setAttribute(SAMLNames.ENTID, rp.getUrl());

        // IPSSODescriptor
        Element spSSO = createSPSSODescriptor(rp);
        entEle.appendChild(spSSO);

        // Append Organization Element
        Element orgEle = createOrganization(rp.getName(), rp.getName(),
                rp.getUrl(), SAMLNames.ENGLISH);
        entEle.appendChild(orgEle);
        return entEle;
    }

    private Element createSPSSODescriptor(RelyingParty rp) throws Exception {
        Element spssoEle = doc.createElement(SAMLNames.SPSSODESCRIPTOR);
        spssoEle.setAttribute(SAMLNames.AUTHNREQUESTSIGNED,
                new Boolean(rp.isAuthnRequestsSigned()).toString());
        spssoEle.setAttribute(SAMLNames.PSE, SAMLNames.REQUIREDPROTOCAL);

        List<Certificate> certs = Arrays.asList(rp.getCertificate());
        Element keyD = createSPKeyDescriptor(certs);
        if (keyD != null) {
            spssoEle.appendChild(keyD);
        }

        // export single logout services.
        Collection<ServiceEndpoint> sloList = rp.getSingleLogoutServices();
        for (ServiceEndpoint slo : sloList) {
            Element sloEle = createSingleLogoutService(slo);
            if (sloEle != null) {
                spssoEle.appendChild(sloEle);
            }
        }

        // export assertion consumer services.
        Collection<AssertionConsumerService> acsList = rp
                .getAssertionConsumerServices();
        Iterator<AssertionConsumerService> it = acsList.iterator();
        String defaultACS = rp.getDefaultAssertionConsumerService();
        while (it.hasNext()) {
            AssertionConsumerService acs = it.next();
            Element acsEle = createAssertionConsumerService(acs, defaultACS);
            if (acsEle != null) {
                spssoEle.appendChild(acsEle);
            }
        }

        return spssoEle;
    }

    private Element createSPKeyDescriptor(List<? extends Certificate> certs)
            throws CertificateEncodingException {
        Element keyD = null;

        if (certs.isEmpty()) {
            return null;
        }
        keyD = doc.createElement(SAMLNames.KEYDESCRIPTOR);
        keyD.setAttribute(SAMLNames.NS_NAME_SAML_DS,
                SAMLNames.NS_NAME_SAML_DIGTALSIG);
        keyD.setAttribute(SAMLNames.USE, SAMLNames.SIGNING);

        Element keyInfoEle = doc.createElement(SAMLNames.DS_KEYINFO);
        Element x509DataEle = doc.createElement(SAMLNames.DS_X509DATA);
        for (Certificate cert : certs) {
            Element x509CertEle = createCertificate(cert);
            x509DataEle.appendChild(x509CertEle);
        }
        keyInfoEle.appendChild(x509DataEle);
        keyD.appendChild(keyInfoEle);

        return keyD;
    }

    private Element createAssertionConsumerService(
            AssertionConsumerService service, String defaultService) {
        if (service == null) {
            return null;
        }
        Element acsEle = doc.createElement(SAMLNames.ASSERTIONCONSUMERSERVICE);
        String loc = service.getEndpoint();
        acsEle.setAttribute(SAMLNames.BINDING, service.getBinding());
        acsEle.setAttribute(SAMLNames.LOCATION, loc);
        acsEle.setAttribute(SAMLNames.INDEX,
                Integer.toString(service.getIndex()));

        boolean isDefault = (!defaultService.isEmpty() && loc
                .equals(defaultService)) ? true : false;
        if (isDefault) {
            acsEle.setAttribute(SAMLNames.ISDEFAULT, SAMLNames.TRUE);
        }
        return acsEle;
    }

    private Element createSingleLogoutService(ServiceEndpoint service) {
        if (service == null) {
            return null;
        }
        Element sloEle = doc.createElement(SAMLNames.SINGLELOGOUTSERVICE);
        String loc = service.getEndpoint();
        sloEle.setAttribute(SAMLNames.BINDING, service.getBinding());
        sloEle.setAttribute(SAMLNames.LOCATION, loc);

        return sloEle;
    }

    /**
     * Create Extension element of EntityDescriptor in DOM.
     *
     * @return Element Document object that contain the Organization.
     * @throws DOMException
     */
    private Element createExtensions() throws DOMException {

        Element extEle = doc.createElement(SAMLNames.EXTENSIONS);

        // Create ExportedOn element.
        Element exportedOnEle = doc.createElement(SAMLNames.EXPORTEDON);

        // Create current UTC time.
        TimeZone timeZone = TimeZone.getTimeZone(SAMLNames.UTC);
        Calendar cal = Calendar.getInstance(timeZone);
        Date curTime = cal.getTime();

        // Formated time string
        SimpleDateFormat df = new SimpleDateFormat(SAMLNames.DATE_FORMAT);
        df.setTimeZone(timeZone);

        // link up DOM
        Node textNode = doc.createTextNode(df.format(curTime));
        exportedOnEle.appendChild(textNode);
        extEle.appendChild(exportedOnEle);

        // Create ExportedBy element.
        Element exportedByEle = doc.createElement(SAMLNames.EXPORTEDBY);
        Node valExpByNode = doc.createTextNode(SAMLNames.EXPORTEDBY_VAL);
        exportedByEle.appendChild(valExpByNode);
        extEle.appendChild(exportedByEle);

        return extEle;
    }

    /**
     * Create Organization element of EntityDescriptor in DOM. Note: currently
     * it support only xml:lang = "en". Will add once idm support locale.
     *
     * @return Element Document object that contain the Organization.
     * @throws Exception
     */
    private Element createOrganization(String name, String displName,
            String url, String langStr) throws Exception {

        Element orgEle = doc.createElement(SAMLNames.ORGANIZATION);

        // Create organization name element
        Element nameEle = doc.createElement(SAMLNames.ORGANIZATIONNAME);
        nameEle.setAttribute(SAMLNames.XMLLANG, SAMLNames.ENGLISH);
        nameEle.appendChild(doc.createTextNode(name));
        orgEle.appendChild(nameEle);

        // Create orgnization displayname element
        Element displayNameEle = doc
                .createElement(SAMLNames.ORGANIZATIONDISPLAYNAME);
        displayNameEle.setAttribute(SAMLNames.XMLLANG, SAMLNames.ENGLISH);
        displayNameEle.appendChild(doc.createTextNode(displName));
        orgEle.appendChild(displayNameEle);

        // Create orgnization url element.
        String urlStr = idmClient.getEntityID(tenantName);
        if (urlStr == null) {
            urlStr = tenantName;
        }
        Element urlEle = doc.createElement(SAMLNames.ORGANIZATIONURL);
        urlEle.setAttribute(SAMLNames.XMLLANG, langStr);
        urlEle.appendChild(doc.createTextNode(url));
        orgEle.appendChild(urlEle);

        return orgEle;
    }

    /**
     * Create IDPSSOElement in DOM.
     *
     * @param exportPrivateData
     *            whether to export complete configuration or not
     *
     * @return Element Document object that contain the IDPSSOElement.
     * @throws Exception
     */
    private Element createIDPSSODescriptor(boolean exportPrivateData)
            throws Exception {
        Element idpssoD = doc.createElement(SAMLNames.IDPSSODESCRIPTOR);
        idpssoD.setAttribute(SAMLNames.PSE, SAMLNames.REQUIREDPROTOCAL); // required
        idpssoD.setAttribute(SAMLNames.WANTSIGNED, SAMLNames.FALSE);

        if (exportPrivateData) {
            // extension attributes
            setExtensionAttributes(idpssoD);

            // Extension element of RoleDescriptor
            Element extEle = createExtensionEle();
            if (extEle != null) {
                idpssoD.appendChild(extEle);
            }
        }

        // Optional Key Descriptor (part of roledescriptor)
        Element keyD = createKeyDescriptor(exportPrivateData);
        if (keyD != null) {
            idpssoD.appendChild(keyD);
        }

        // Single logout service

        Element sloEle = createSingleLogOutForCastleAsIDP(SAMLNames.HTTP_REDIRECT_BINDING);
        idpssoD.appendChild(sloEle);
        sloEle = createSingleLogOutForCastleAsIDP(SAMLNames.SOAP_BINDING);
        idpssoD.appendChild(sloEle);

        createNameIDFormats(idpssoD, true);

        // SingleSignOnService elemement
        String ssoLocationStr = convertToIPV6ShortForm(idmClient.getEntityID(tenantName));
        if (ssoLocationStr == null) {
            ssoLocationStr = tenantName;
        }
        ssoLocationStr = ssoLocationStr.replaceAll("/Metadata/", "/SSO/");
        Element ssos = createEndPoint(SAMLNames.SSOS,
                SAMLNames.HTTP_REDIRECT_BINDING, ssoLocationStr, null);
        idpssoD.appendChild(ssos);

        // Attribute elements
        createAttributesEle(idpssoD);

        return idpssoD;
    }

    /**
     * write IDPSSODecriptor extension element. This include all idm id stores
     * information.
     *
     * @return extension element, or null if nothing created.
     * @throws Exception
     */
    private Element createExtensionEle() throws Exception {
        Element extEle = null;

        // Place a local store element in extension element.
        Collection<IIdentityStoreData> localStores = idmClient.getProviders(
                tenantName, EnumSet.of(DomainType.LOCAL_OS_DOMAIN));
        if (!localStores.isEmpty()) {

            IIdentityStoreData store = localStores.iterator().next();
            extEle = doc.createElement(SAMLNames.EXTENSIONS);
            Element localStoreEle = doc.createElement(SAMLNames.LOCALOSIDSTORE);
            localStoreEle.setAttribute(SAMLNames.IDNAME, store.getName());
            extEle.appendChild(localStoreEle);
        }

        // Place Active Directory Store elements in extension element.
        Collection<IIdentityStoreData> ADStores = idmClient.getProviders(
                tenantName, EnumSet.of(DomainType.EXTERNAL_DOMAIN));
        if (!ADStores.isEmpty()) {
            for (IIdentityStoreData store : ADStores) {
                IIdentityStoreDataEx storeDataExt = store
                        .getExtendedIdentityStoreData();
                if (storeDataExt.getProviderType() != IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY) {
                    continue;
                }
                // Now we are sure there is at least one valid AD store.
                // create Extension if not done yet.
                if (extEle == null) {
                    extEle = doc.createElement(SAMLNames.EXTENSIONS);
                }

                Element adStoreEle = doc
                        .createElement(SAMLNames.ACTIVEDIRECTORYIDSTORE);
                adStoreEle.setAttribute(SAMLNames.IDNAME, store.getName());
                adStoreEle.setAttribute(SAMLNames.IDUSERNAME,
                        storeDataExt.getUserName());
                adStoreEle.setAttribute(SAMLNames.IDSPN,
                        storeDataExt.getServicePrincipalName());
                if (storeDataExt.getFriendlyName() != null) {
                    adStoreEle.setAttribute(SAMLNames.IDFRIENDLYNAME,
                            storeDataExt.getFriendlyName());
                }
                adStoreEle.setAttribute(SAMLNames.IDUSERPWD,
                        storeDataExt.getPassword());
                adStoreEle.setAttribute(SAMLNames.IDSEARCHBASEDN,
                        storeDataExt.getUserBaseDn());
                adStoreEle.setAttribute(SAMLNames.IDSEARCHTIMEOUTSCNDS, Integer
                        .toString(storeDataExt.getSearchTimeoutSeconds()));

                createKDCs(adStoreEle, store);
                createAttributeMaps(adStoreEle, store);
                extEle.appendChild(adStoreEle);
            }
        }
        return extEle;
    }

    /**
     * write key distribution center host as KDC elements in ADStore.
     *
     * @param ActiveDirectoryStore
     *            element.
     * @return void.
     */
    private void createKDCs(Element adStore, IIdentityStoreData store) {
        Collection<String> kdcHosts = store.getExtendedIdentityStoreData()
                .getConnectionStrings();

        if (kdcHosts.isEmpty()) {
            return;
        }
        Iterator<String> it = kdcHosts.iterator();
        while (it.hasNext()) {
            String kdcHost = it.next();
            Element kdcEle = doc.createElement(SAMLNames.KDC);
            kdcEle.appendChild(doc.createTextNode(kdcHost));
            adStore.appendChild(kdcEle);
        }

    }

    /**
     * write attribute map elements in ADStore.
     *
     * @param ActiveDirectoryStore
     *            element.
     * @return void.
     */
    private void createAttributeMaps(Element adStore, IIdentityStoreData store) {
        Map<String, String> attrPairs = store.getExtendedIdentityStoreData()
                .getAttributeMap();

        for (Map.Entry<String, String> attrPair : attrPairs.entrySet()) {

            Element attrMapEle = doc.createElement(SAMLNames.ATTRIBUTEMAP);
            attrMapEle.setAttribute(SAMLNames.IDNAME, attrPair.getKey());
            attrMapEle.setAttribute(SAMLNames.IDVALUE, attrPair.getValue());
            adStore.appendChild(attrMapEle);
        }

    }

    /**
     * write IDPSSODecriptor extension.
     *
     * @param IDPSSODecriptor
     *            element.
     * @return void.
     * @throws Exception
     *             ,DOMException
     */
    private void setExtensionAttributes(Element idpSSO) throws DOMException,
            Exception {
        // isDefault
        String deftTenant = idmClient.getDefaultTenant();

        if (deftTenant != null && deftTenant.equals(tenantName)) {

            idpSSO.setAttribute(SAMLNames.VMES_ISDEFAULT, SAMLNames.TRUE);
        }
        // clockTolerance
        try {
            long val = idmClient.getClockTolerance(tenantName);
            String result = Long.toString(val);
            idpSSO.setAttribute(SAMLNames.CLOCKTOLERANCE, result);
        } catch (IDMException e) {
        }
        // delegation count
        try {
            int intval = idmClient.getDelegationCount(tenantName);
            String result = Integer.toString(intval);
            idpSSO.setAttribute(SAMLNames.DELEGATIONCNT, result);
        } catch (IDMException e) {
        }

        // renew count
        try {
            int intval = idmClient.getRenewCount(tenantName);
            String result = Integer.toString(intval);
            idpSSO.setAttribute(SAMLNames.RENEWCNT, result);
        } catch (IDMException e) {

        }
        // maximum bearer token life time.
        try {
            long val = idmClient.getMaximumBearerTokenLifetime(tenantName);
            String result = Long.toString(val);
            idpSSO.setAttribute(SAMLNames.MAXBEARERTKNLIFETIME, result);
        } catch (IDMException e) {

        }

        // maximum HOK token life time
        try {
            long val = idmClient.getMaximumHoKTokenLifetime(tenantName);
            String result = Long.toString(val);
            idpSSO.setAttribute(SAMLNames.MAXHOKTKNLIFETIME, result);
        } catch (IDMException e) {

        }

        // issure name
        String result = idmClient.getTenant(tenantName)._issuerName;
        if (result != null) {
            idpSSO.setAttribute(SAMLNames.ISSUERNAME, result);
        }

    }

    /**
     * Write tenant idp supported attributes (for principle) as <Attribute>
     * element of a given parent element.
     *
     * @param parent
     *            element.
     * @return void.
     * @throws Exception
     */
    private void createAttributesEle(Element parent) throws Exception {
        Collection<Attribute> attributes = idmClient
                .getAttributeDefinitions(tenantName);

        for (Attribute attr : attributes) {
            Element attrEle = doc.createElement(SAMLNames.ATTRIBUTE);

            attrEle.setAttribute(SAMLNames.NAME, attr.getName());
            attrEle.setAttribute(SAMLNames.NAMEFORMAT, attr.getNameFormat());
            attrEle.setAttribute(SAMLNames.FRIENDLYNAME, attr.getFriendlyName());
            parent.appendChild(attrEle);
        }
    }

    /**
     * Return SingleLogOutService Element for Castle as IDP.
     *
     * @param binding
     *            binding string
     * @return Element. Always create this element.
     * @throws Exception
     */
    private Element createSingleLogOutForCastleAsIDP(String binding) throws Exception {
        String entityID = idmClient.getEntityID(tenantName);
        String ssoLoc = convertToIPV6ShortForm(entityID);
        if (ssoLoc == null) {
            // entID is required attribute
            ssoLoc = tenantName;
        }
        String sloLoc = ssoLoc.replaceAll("/Metadata/", "/SLO/");
        Element slosEle = createEndPoint(SAMLNames.SLOS, binding, sloLoc, null);
        return slosEle;
    }

    /**
     * Return SingleLogOutService Element for Castle as SP.
     *
     * @param binding
     *            binding string
     * @return Element. Always create this element.
     * @throws Exception
     */
    private Element createSingleLogOutForCastleAsSP(String binding) throws Exception {
        String entityID = idmClient.getEntityID(tenantName);
        String ssoLoc = convertToIPV6ShortForm(entityID);
        if (ssoLoc == null) {
            // entID is required attribute
            ssoLoc = tenantName;
        }
        String sloLoc = ssoLoc.replaceAll(SAMLNames.ENTITY_ID_PLACEHOLDER,
                SAMLNames.SP_SINGLELOGOUTSERVICE_PLACEHOLDER);
        Element slosEle = createEndPoint(SAMLNames.SLOS, binding, sloLoc, null);
        return slosEle;
    }

    /**
     * Return KeyDescriptor.
     *
     * @param exportPrivateData
     *            whether to export private key or not
     * @param void
     * @return Element Document object that contain the KeyDescriptor. null if
     *         none created.
     * @throws Exception
     */
    private Element createKeyDescriptor(boolean exportPrivateData)
            throws Exception {

        Element keyValueEle = null;
        if (exportPrivateData) {
            keyValueEle = createKeyValue();
        }
        Element x509DataEle = createX509Data();

        if (x509DataEle == null && keyValueEle == null) {
            return null;
        } else {
            Element keyD = doc.createElement(SAMLNames.KEYDESCRIPTOR);
            keyD.setAttribute(SAMLNames.NS_NAME_SAML_DS,
                    SAMLNames.NS_NAME_SAML_DIGTALSIG);
            keyD.setAttribute(SAMLNames.USE, SAMLNames.SIGNING);

            Element keyInfoEle = doc.createElement(SAMLNames.DS_KEYINFO);

            // Append kayvalue and X509Data elements to keyInfo
            if (keyValueEle != null && exportPrivateData) {
                keyInfoEle.appendChild(keyValueEle);
            }
            if (x509DataEle != null) {
                keyInfoEle.appendChild(x509DataEle);
            }
            keyD.appendChild(keyInfoEle);
            return keyD;
        }
    }

    /**
     * Return a ds:x509Data Element in DOM.
     *
     * @param void
     * @return Element null if none created, Document object that contain the
     *         element of 'ds:KeyValueType'.
     * @throws Exception
     */
    private Element createX509Data() throws Exception {
        Element x509DataEle = null;

        List<Certificate> certs = idmClient.getTenantCertificate(tenantName);
        if (!certs.isEmpty()) {
            x509DataEle = doc.createElement(SAMLNames.DS_X509DATA);

            for (Certificate cert : certs) {
                Element x509CertificateEle = createCertificate(cert);
                x509DataEle.appendChild(x509CertificateEle);
            }
        }
        return x509DataEle;
    }

    /**
     * Return a ds:X509Certificate Element
     *
     * @param Certificate
     * @return Element Document object that contain the element
     * @throws Exception
     *             if failed for any reason.
     */
    private Element createCertificate(Certificate cert)
            throws CertificateEncodingException {
        if (cert == null || !cert.getType().equals(SAMLNames.X509)) {
            throw new IllegalArgumentException("Certl is null or not a X509 type");
        }

        Element x509CertificateEle = doc
                .createElement(SAMLNames.DS_X509CERTIFICATE);
        X509Certificate x509Cert = (X509Certificate) cert;
        String base64Str = Base64.encodeBytes(x509Cert.getEncoded());
        Node certText = doc.createTextNode(base64Str);

        x509CertificateEle.appendChild(certText);
        return x509CertificateEle;
    }

    /**
     * Return a ds:KeyValue Element in DOM. KeyValue | -----RsaKeyValue
     *
     * @param void
     * @return Element Document object that contain the element of
     *         'ds:KeyValueType'. Caller should check for null object return.
     * @throws Exception
     */
    private Element createKeyValue() throws Exception {
        Element keyValueEle = null;

        PrivateKey key = idmClient.getTenantPrivateKey(tenantName);
        if (key != null) {
            RSAPrivateKey rsaKey = (RSAPrivateKey) key;
            Element rsaKeyEle = createRSAKeyValue(rsaKey);
            if (rsaKeyEle != null) {
                keyValueEle = doc.createElement(SAMLNames.DS_KEYVALUE);
                keyValueEle.appendChild(rsaKeyEle);
            }
        }
        return keyValueEle;
    }

    /**
     * Return a ds:RSAKeyValue Element in DOM. KeyValue | -----RsaKeyValue
     *
     * @param RSAPrivateKey
     * @return Element Document object that contain the element of
     *         'ds:RSAKeyValueType'. Caller should check for null object return.
     * @throws Exception
     */
    private Element createRSAKeyValue(RSAPrivateKey rsaKey) {
        if (rsaKey == null) {
            return null;
        }

        String modulusStr = Base64.encodeBytes(rsaKey.getModulus()
                .toByteArray());
        String exponentStr = Base64.encodeBytes(rsaKey.getPrivateExponent()
                .toByteArray());

        if (modulusStr == null || exponentStr == null) {
            return null;
        }

        Element rsaEle = doc.createElement(SAMLNames.DS_RSAKEYVALUE);

        Element modulusEle = doc.createElement(SAMLNames.DS_MODULUS);
        Node modText = doc.createTextNode(modulusStr);
        modulusEle.appendChild(modText);

        Element exponentEle = doc.createElement(SAMLNames.DS_EXPONENT);
        Node expText = doc.createTextNode(exponentStr);
        exponentEle.appendChild(expText);

        rsaEle.appendChild(modulusEle);
        rsaEle.appendChild(exponentEle);
        return rsaEle;
    }

    /**
     * Create end point element in DOM.
     *
     * @param name
     *            tag name of the endpoint ele. required.
     * @attrLocation service url. required
     * @attrResponseLocation attribute response URL. optional: can be null
     * @return EndPoint type element.
     * @throws DOMException
     */
    private Element createEndPoint(String name, String attrBinding,
            String attrLocation, String attrResponseLocation
    ) throws DOMException {
        if (name == null ) {
            throw new IllegalArgumentException("Null name");
        }
        if (attrBinding == null ) {
            throw new IllegalArgumentException("Null binding");
        }
        if (attrLocation == null) {
            throw new IllegalArgumentException("Null location");
        }
        Element endPt = doc.createElement(name);
        endPt.setAttribute(SAMLNames.BINDING, attrBinding);
        endPt.setAttribute(SAMLNames.LOCATION, attrLocation);
        if (attrResponseLocation != null) {
            endPt.setAttribute(SAMLNames.RESPLOC, attrResponseLocation);
        }
        return endPt;
    }

    /**
     * Create element of IndexedEndPoint type
     *
     * @param name
     *            required, cannot be null
     * @param attrBinding
     *            required, cannot be null
     * @param attrLocation
     *            required, cannot be
     * @param attrResponseLocation
     *            optional
     * @param index
     *            required, cannot be null
     * @param isDefault
     *            optional
     * @return IndexedEndPoint type element
     * @throws DOMException
     */
    private Element createIndexedEndPoint(String name, String attrBinding,
            String attrLocation, String attrResponseLocation, int index,
            Boolean isDefault) throws DOMException
    {
//        Element indexedEndPt =
//                createEndPoint(name, attrBinding, attrLocation,
//                        attrResponseLocation);
//
        if (name == null ) {
            throw new IllegalArgumentException("Null name");
        }
        if (attrBinding == null ) {
            throw new IllegalArgumentException("Null binding");
        }
        if (attrLocation == null) {
            throw new IllegalArgumentException("Null location");
        }
        Element indexedEndPt = doc.createElement(name);
        indexedEndPt.setAttribute(SAMLNames.BINDING, attrBinding);
        indexedEndPt.setAttribute(SAMLNames.LOCATION, attrLocation);
        if (attrResponseLocation != null) {
            indexedEndPt.setAttribute(SAMLNames.RESPLOC, attrResponseLocation);
        }

        indexedEndPt.setAttribute(SAMLNames.INDEX, String.valueOf(index));

        if (isDefault != null)
        {
            indexedEndPt
                    .setAttribute(SAMLNames.ISDEFAULT, isDefault.toString());
        }
        return indexedEndPt;
    }

    /**
     * Create NameIDFormat elements as child element of given parent element.
     * This method creates two type of name id format: email address and
     * persistent.
     *
     * @param parentEle
     *            Parent element in Dom
     * @return void
     * @throw Exception
     */
    private void createNameIDFormats(Element parentEle, boolean asIdp) throws Exception {
        Element nameIdEle = doc.createElement(SAMLNames.NAMEIDFORMAT);
        nameIdEle.appendChild(doc
                .createTextNode(SAMLNames.IDFORMAT_VAL_EMAILADD.toString()));
        parentEle.appendChild(nameIdEle);

        //if ( asIdp != true )
        //{
            nameIdEle = doc.createElement(SAMLNames.NAMEIDFORMAT);
            nameIdEle.appendChild(doc
                    .createTextNode(SAMLNames.IDFORMAT_VAL_PERSIST.toString()));
            parentEle.appendChild(nameIdEle);
        //}
        nameIdEle = doc.createElement(SAMLNames.NAMEIDFORMAT);
        nameIdEle.appendChild(doc
                .createTextNode(SAMLNames.IDFORMAT_VAL_UPN.toString()));
        parentEle.appendChild(nameIdEle);
    }

    /**
     * Return expiration date in 'UTC' (GMT)
     *
     * @param daysFromCurrent
     *            days from current date.
     * @param hoursFromCurrent
     *            hours from current date.
     * @return String date string
     * @throws none
     */
    private String getExpirationDate(int daysFromCurrent, int hoursFromCurrrent) {
        // Set up cal in UTC
        TimeZone timeZone = TimeZone.getTimeZone(SAMLNames.UTC);
        Calendar cal = Calendar.getInstance(timeZone);

        // and add time allowed
        int extraDays = daysFromCurrent + hoursFromCurrrent / 24;
        int extraHours = hoursFromCurrrent % 24;
        cal.add(Calendar.DAY_OF_MONTH, extraDays);
        cal.add(Calendar.HOUR_OF_DAY, extraHours);

        // Get UTC time formated string.
        Date expDate = cal.getTime();
        SimpleDateFormat df = new SimpleDateFormat(SAMLNames.DATE_FORMAT);
        df.setTimeZone(timeZone);
        String expDateStr = df.format(expDate);

        return expDateStr;
    }

    private String getEntityId(String tenantName) throws Exception
    {
        String entityId = idmClient.getEntityID(tenantName);
        if ( (entityId == null) || (entityId.isEmpty()) )
        {
            throw new IDMException(
                // entID is required attribute
                String.format("Unable to obtain a valid EntityID for tenant [%s]", tenantName) );
        }
        return entityId;
    }
}
