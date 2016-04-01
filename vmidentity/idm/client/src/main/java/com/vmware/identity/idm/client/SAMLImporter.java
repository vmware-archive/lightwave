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

import java.io.ByteArrayInputStream;
import java.math.BigInteger;
import java.net.URI;
import java.net.URISyntaxException;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.RSAPrivateKeySpec;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.datatype.DatatypeConfigurationException;
import javax.xml.datatype.DatatypeConstants;
import javax.xml.datatype.DatatypeFactory;
import javax.xml.datatype.XMLGregorianCalendar;

import org.opensaml.xml.util.Base64;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.AttributeConfig;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.ExternalIDPCertChainInvalidTrustedPathException;
import com.vmware.identity.idm.ExternalIDPExtraneousCertsInCertChainException;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.ValidateUtil;

/**
 * VMware Identity Service
 *
 * SAML meta data importer
 * Helper class for idm client for importing tenant configuration
 * This wiki lists the supported features of the importer.
 *
 * @author:  Scott Chai <schai@vmware.com>
 *
 * @version: 1.0
 * @since:   2012-04-10
 * Note: -maybe I should use getAttributeByNS instead of getAttribute
 */

class SAMLImporter {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SAMLImporter.class);

    private final CasIdmClient idmClient;
    private String tenantName;
    public SAMLImporter(CasIdmClient client
            ) {
        this.idmClient = client;
        this.tenantName = null;
    }

    /**
     * importConfiguration() parses all interested information in saml document
     * and configure a existing tenant associated to it.
     * Note: this function was originally design for both importing SP config and IDP config backup.
     * Now since the later was never needed and used. this function now will only used for SP configuration. Meaning
     * it will ignore IDP metadata if presented in the doc.
     *
     * @param tenantName
     *            <in> tenant name of the entity.
     * @param doc
     *            <in> DOM object represent the saml file.
     * @throws Exception
     *             ,IDMException,ParseException
     * @throws NoSuchTenantException
     *             if the tenant does not exist.
     * @return none Note: standard output of warning when date format parsing
     *         failed. In that case the method returns false.
     *
     */

    public void importConfiguration(
            String tenantName,
            Document doc
            ) throws Exception, NoSuchTenantException,IDMException, ParseException {
        this.tenantName = tenantName;

        Tenant backup_tenant = this.idmClient.getTenant(tenantName);

        try {
            NodeList entNodes = doc.getElementsByTagNameNS(
                    SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.ENTDESCRIPTOR);
            for (int ind=0; ind < entNodes.getLength();ind++) {
                Element entityEle = (Element) entNodes.item(ind);
                if (isExpired(entityEle)) {
                    throw new IDMException("Document has expired!");
                }
                //An entity descriptor could represent either sp or idp, not both.
                //We are looking for SPSSODESCRIPTOR
                NodeList idpList = entityEle.getElementsByTagNameNS(
                        SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.SPSSODESCRIPTOR);
                if (idpList.getLength() > 0) {
                    importSPEntity(entityEle);
                }
            }
        }
        catch (Exception e) {
            //recover tenant settings for non-system.
            //for system tenant, we do nothing since we can't delete it. This is not optimal solution obviously.
            if (!idmClient.getSystemTenant().equalsIgnoreCase(tenantName)) {
                //clean up partial importing.
                Tenant tenant = null;
                try
                {
                    tenant = idmClient.getTenant(tenantName);
                }
                catch(NoSuchTenantException ex)
                {
                    //we are good
                }

                if (backup_tenant != null)
                {
                    this.idmClient.setTenant(backup_tenant);
                }
            }
            throw e;
        }
    }

    /**
     * Import external IDP configuration per the XML doc.
     *
     * @param tenantName
     * @param doc
     * @return entityID of the imported configuration
     * @throws Exception
     * @throws IDMException
     *             when none or more than one EntityDescriptor elements are
     *             found in the doc, or when protocol validation fails for the
     *             internal <IDPSSODescriptor> element.
     */
    public String importExternalIDPConfig(String tenantName, Document doc)
            throws ExternalIDPCertChainInvalidTrustedPathException,
            ExternalIDPExtraneousCertsInCertChainException, Exception,
            IDMException    {
        List<Element> entityDescriptors = getEntityDescriptors(doc);
        int entityDescLen = entityDescriptors.size();
        if (0 == entityDescLen)
        {
            throw new IDMException(String.format("No %s are found",
                    SAMLNames.ENTDESCRIPTOR));
        }
        if (entityDescLen > 1)
        {
            throw new IDMException(
                    String.format(
                            "Only can import one IDP at a time, but multiple %s elements are found",
                            SAMLNames.ENTDESCRIPTOR));
        }

        Element entityDescriptor = entityDescriptors.get(0);
        String entityID = entityDescriptor.getAttribute(SAMLNames.ENTID);

        if (isExpired(entityDescriptor))
        {
            throw new IDMException("Document has expired!");
        }
        try
        {
            // for tenant external IDP import, we are only looking for one <IDPSSODescriptor>
            NodeList idpSSODescriptors =
                    entityDescriptor.getElementsByTagNameNS(
                            SAMLNames.NS_NAME_SAML_METADATA,
                            SAMLNames.IDPSSODESCRIPTOR);
            int idpSSODescLen = idpSSODescriptors.getLength();
            if (0 == idpSSODescLen)
            {
                throw new IDMException(String.format("No %s are found",
                        SAMLNames.IDPSSODESCRIPTOR));
            }
            if (idpSSODescLen > 1)
            {
                throw new IDMException(
                        String.format(
                                "Only can import one IDP at a time, but multiple %s element are found",
                                SAMLNames.IDPSSODESCRIPTOR));
            }
            Element idpSSODescriptor = (Element) idpSSODescriptors.item(0);
            validateRequiredProtocol(idpSSODescriptor);

            List<Certificate> certs = getCertificates(idpSSODescriptor);

            Collection<ServiceEndpoint> sloServices =
                    getSingleLogoutServices(idpSSODescriptor);

            Collection<ServiceEndpoint> ssoServices =
                    getSingleSignOnServices(idpSSODescriptor);

            Collection<URI> nameIDFormats = getNameIDFormats(idpSSODescriptor);

            IDPConfig object = new IDPConfig(entityID);

            // use organization name as alias
            String entityAlias = entityDescriptor.getAttribute(SAMLNames.ORGANIZATIONNAME);
            if (entityAlias != null && !entityAlias.isEmpty()) {
                object.setAlias(entityAlias);
            }

            // converting of XML type to POJO type here
            List<String> nameIDFormatStrs = new ArrayList<String>();
            for (URI uri : nameIDFormats)
            {
                nameIDFormatStrs.add(uri.toString());
            }
            // certificate is generated from X.509 factory, cast it back now
            List<X509Certificate> x509Certs = new ArrayList<X509Certificate>();
            for (Certificate cert : certs) {
                x509Certs.add((X509Certificate)cert);
            }

            ValidateUtil.validateNotEmpty(nameIDFormatStrs, "[idpConfig.nameIDFormatStrs]");
            ValidateUtil.validateNotEmpty(x509Certs, "[idpConfig.signingCertificates]");
            ValidateUtil.validateNotEmpty(ssoServices, "[idpConfig.ssoServices]");

            object.setNameIDFormats(nameIDFormatStrs);
            object.setSigningCertificateChain(x509Certs);
            object.setSloServices(sloServices);
            object.setSsoServices(ssoServices);

            //setup subjectformat mappings. This is default setting that allows
            //mapping non-upn subjectIdFormat to a lotus attribute.
            AttributeConfig[] subjectFormatMappings = new AttributeConfig[1];

            subjectFormatMappings[0] = new AttributeConfig(SAMLNames.IDFORMAT_VAL_EMAILADD.toString(), "mail");

            if (subjectFormatMappings != null) {
                object.setSubjectFormatMappings(subjectFormatMappings);
            }
            idmClient.setExternalIdpConfig(tenantName, object);
        } catch (Exception e) {
            if (e instanceof ExternalIDPCertChainInvalidTrustedPathException
                    || e instanceof ExternalIDPExtraneousCertsInCertChainException) {
                throw e;
            } else {
                try {
                    //cleanup partial import
                    idmClient.removeExternalIdpConfig(tenantName,
                            entityDescriptor.getAttribute(SAMLNames.ENTID));
                } catch (NoSuchExternalIdpConfigException nseice) {
                    //the import did not go very far. Nothing to cleanup.
                }
                throw new IDMException(String.format(
                        "Couldn't import external IDP Config for tenant: %s ",
                        tenantName), e);
            }
        }
        return entityID;
    }

    /**
     * parse KeyDescriptor structure of the element passed in, to get list of
     * signing certificates
     *
     * @param roleDescElem
     *            element of type <RoleDescriptor> or its sub-types
     * @return list of signing certificates
     * @throws IDMException
     *             when multiple signing KeyDescriptors are found
     * @throws CertificateException
     *             on processing error or parsing error for the X509 data
     */
    private List<Certificate> getCertificates(Element roleDescElem) throws IDMException, CertificateException
    {
        //multiple KeyDescriptors can be inside RoleDescriptor, but for import purpose only one is for signing
        NodeList keyDescriptorNodes =
                roleDescElem.getElementsByTagNameNS(
                        SAMLNames.NS_NAME_SAML_METADATA,
                        SAMLNames.KEYDESCRIPTOR);
        Element signingKeyDescriptor = null;
        for (int idx = 0; idx < keyDescriptorNodes.getLength(); idx++)
        {
            Element currentElem = (Element)keyDescriptorNodes.item(idx);
            String useAttr = currentElem.getAttribute(SAMLNames.USE);
            //if "use" attribute is missing, we are assuming the same cert is for both signing and encryption.
            if (useAttr == null || useAttr.equalsIgnoreCase(SAMLNames.SIGNING) )
            {
                if (null != signingKeyDescriptor)
                {
                    throw new IDMException("Multiple KeyDescriptors used for signing are found, not allowed");
                }
                else
                {
                    signingKeyDescriptor = currentElem;
                    //keep checking for multiple signing keys
                }
            }
        }

        NodeList keyInfoElements = signingKeyDescriptor.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_DIGTALSIG, SAMLNames.KEYINFO);
        Element keyInfoElem = (Element) keyInfoElements.item(0);

        return parseCertificates(keyInfoElem);
    }

    /**
     * From the document, get the list of EntityDescriptor
     *
     * @param doc
     *            the xml doc
     * @return list of elements of <EntityDescriptor>
     */
    private List<Element> getEntityDescriptors(Document doc) throws IDMException
    {
        List<Element> entityDescElems = new ArrayList<Element>();

        //grep all <EntityDescriptor> nodes
        NodeList entityDescriptorNodes =
                doc.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_METADATA,
                        SAMLNames.ENTDESCRIPTOR);

        for (int idx = 0; idx < entityDescriptorNodes.getLength(); idx++)
        {
            Node currentNode = entityDescriptorNodes.item(idx);

            if (currentNode.getNodeType() == Node.ELEMENT_NODE)
            {
                Element currentElem = (Element)currentNode;
                entityDescElems.add(currentElem);
            }
        }

        return entityDescElems;
    }
    /**
     * Validate organization name consistency with the provide entity name
     * @param Element   must be Entity element.
     * @throws IDMException:
     * @return  void
     */
    private
    void
    checkOrganizationName(
            Element entityEle, String id
            ) throws IDMException
            {

        //Check tenant name consistency with Organization name.
        NodeList nodes = entityEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.ORGANIZATION);
        if (nodes.getLength() > 0) {
            Element orgEle = (Element) nodes.item(0);
            String orgName = orgEle.getElementsByTagNameNS(
                    SAMLNames.NS_NAME_SAML_METADATA,"OrganizationName").item(0).getTextContent();
            if (!orgName.equals(id) ) {
                throw new IDMException("Oranganization name is inconsistant with tenant name");
            }
        }
            }

    /**
     * Compare current date with expiration date provided.
     * The time format:
     *  SAML 2.0 core request time be expressed in UTC form, with no time zone component.
     *
     * @param Element   must be Entity element.
     * Note: standard output of warning when date format parsing failed. In that case
     *  the method returns false.
     * @throws ParseException:
     * @return  true/false
     * @throws DatatypeConfigurationException
     *
     */

    public
    boolean
    isExpired(Element entityEle
            ) throws ParseException, DatatypeConfigurationException
    {
        if (entityEle == null) {
            throw new IllegalArgumentException("Null entity element");
        }

        String expDateStr = entityEle.getAttribute(SAMLNames.VALIDUNTIL);
        if (expDateStr.isEmpty()) {
            return false;
        }

        DatatypeFactory dateFactory = DatatypeFactory.newInstance();
        XMLGregorianCalendar expCal = dateFactory.newXMLGregorianCalendar(expDateStr);
        if (expCal.getTimezone() != DatatypeConstants.FIELD_UNDEFINED && expCal.getTimezone() != 0) {
            throw new IllegalArgumentException(expDateStr + "SAML spec does not allow time zoon offset.");
        }
        GregorianCalendar gregorianCalendar = new GregorianCalendar();
        XMLGregorianCalendar now = dateFactory.newXMLGregorianCalendar(gregorianCalendar);

        if (expCal == null) {
            //failed to parse the time string
            throw new ParseException("Unable to read "+expDateStr+" into XMLGregorianCalendar.",0);
        }
        if (now.normalize().compare(expCal) == DatatypeConstants.GREATER) {
            return true;
        } else {
            return false;
        }
    }
    /**
     * To import configurable IDP settings to idm.
     * The following information will be imported: signing key and certificates
     * Meta data ignored: SSO/SLO bindings and location, IDP Attributes,etc
     * @param EntityElement that contain single IDP
     * @throws Exception:
     * @return  void
     * Note: This function parse for IDP only. Any other role information will be ignored.
     */
    private
    void
    importIDPEntity(
            Element entity) throws Exception
            {
        //validate organizatin name
        checkOrganizationName(entity,this.tenantName);

        //Set entity id and issuerName
        String id = entity.getAttribute(SAMLNames.ENTID);
        this.idmClient.setEntityID(this.tenantName, id);
        Tenant tenant = this.idmClient.getTenant(this.tenantName);
        tenant._issuerName = id;
        this.idmClient.setTenant(tenant);

        NodeList idpList = entity.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.IDPSSODESCRIPTOR);
        if (idpList.getLength() == 0) {
            throw new IDMException("SAML medadata error: file " +
                    "does not have a idp or sp descriptor!");
        }

        Element idpSSOEle = (Element)idpList.item(0);
        importIDPSSODescriptor(idpSSOEle);

            }

    private
    void
    importIDPSSODescriptor(
            Element idpSSOEle) throws IDMException, Exception
            {

        //We should parse for multiple keys. This is optional.
        NodeList keyDescriptorList = idpSSOEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,
                SAMLNames.KEYDESCRIPTOR);
        if (keyDescriptorList.getLength() == 0) {
            throw new IDMException("SAML metadata error: no signing key for IDP");
        }

        Element keyDescriptorEle = (Element) keyDescriptorList.item(0);
        //"use" attribute should be "signing" ignore other types.
        if (!keyDescriptorEle.getAttribute(SAMLNames.USE).equals(SAMLNames.SIGNING)) {
            throw new IDMException("SAML metadata error: no signing key for IDP");
        }
        //Get Keyinfo. no need to check availability since it is schema-inforced.
        NodeList keyInfoList = keyDescriptorEle.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_DIGTALSIG,
                SAMLNames.KEYINFO);
        Element keyInfoEle = (Element) keyInfoList.item(0);
        PrivateKey tenantPrivateKey = parseKey(keyInfoEle);
        //check

        Collection<Certificate> tenantCertificates = parseCertificates(keyInfoEle);
        this.idmClient.setTenantCredentials(this.tenantName, tenantCertificates, tenantPrivateKey);

        importIDPExtensionAttributes(idpSSOEle);
        importIDPExtensionsElement(idpSSOEle);
            }

    private
    void
    importIDPExtensionAttributes(Element idpSSOEle) throws Exception
    {

        //isDefault
        if (idpSSOEle.hasAttribute(SAMLNames.VMES_ISDEFAULT)) {
            String isDefault = idpSSOEle.getAttribute(
                    SAMLNames.VMES_ISDEFAULT);
            if (isDefault.equals(SAMLNames.TRUE)) {
                this.idmClient.setDefaultTenant(this.tenantName);
            }
        }

        //clockTolerance
        if (idpSSOEle.hasAttribute(SAMLNames.CLOCKTOLERANCE)) {
            String clockToleranceStr = idpSSOEle.getAttribute(
                    SAMLNames.CLOCKTOLERANCE);
            Long tol = Long.valueOf(clockToleranceStr);
            this.idmClient.setClockTolerance(this.tenantName, tol);
        }

        //delegation count
        if (idpSSOEle.hasAttribute( SAMLNames.DELEGATIONCNT)) {
            String resultStr = idpSSOEle.getAttribute(SAMLNames.DELEGATIONCNT);
            int delegationCount = Integer.valueOf(resultStr);
            this.idmClient.setDelegationCount(this.tenantName, delegationCount);
        }

        //renew count
        if (idpSSOEle.hasAttribute(SAMLNames.RENEWCNT)) {
            String resultStr = idpSSOEle.getAttribute( SAMLNames.RENEWCNT);
            int renewCount = Integer.valueOf(resultStr);
            this.idmClient.setRenewCount(this.tenantName, renewCount);
        }

        // maximum bearer token life time
        if (idpSSOEle.hasAttribute(SAMLNames.MAXBEARERTKNLIFETIME)) {
            String resultStr = idpSSOEle.getAttribute(SAMLNames.MAXBEARERTKNLIFETIME);
            Long lifeTime = Long.valueOf(resultStr);
            this.idmClient.setMaximumBearerTokenLifetime(this.tenantName, lifeTime);
        }

        // maximum HOK token life time
        if (idpSSOEle.hasAttribute(SAMLNames.MAXHOKTKNLIFETIME)) {
            String resultStr = idpSSOEle.getAttribute(
                    SAMLNames.MAXHOKTKNLIFETIME);
            Long lifeTime = Long.valueOf(resultStr);
            this.idmClient.setMaximumHoKTokenLifetime(this.tenantName, lifeTime);
        }

        //issuer name
        if (idpSSOEle.hasAttribute(SAMLNames.ISSUERNAME)) {
            Tenant tenant = this.idmClient.getTenant(this.tenantName);
            tenant._issuerName = idpSSOEle.getAttribute(
                    SAMLNames.ISSUERNAME);
            this.idmClient.setTenant(tenant);
        }

    }
    /**
     *
     * @param IDPSSODecriptor element.
     * @throws Exception:
     * @return  void
     * Note:void
     */
    private
    void
    importIDPExtensionsElement(Element idpSSOEle) throws Exception
    {
        if (idpSSOEle == null) {
            throw new IDMException("SAML medadata error: file " +
                    "does not have a idp or sp descriptor!");
        }
        NodeList nodes = idpSSOEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,
                SAMLNames.EXTENSIONS);
        //local store
        if (nodes.getLength() == 0) {
            return;
        }
        Element extEle = (Element) nodes.item(0);

        importLocalIDStore(extEle);
        importADStores(extEle);

    }
    /**
     *  import local os store element in extensions element.
     * @param Extensions element.
     * @throws Exception:
     * @return  void
     */
    private
    void
    importLocalIDStore(Element extEle) throws Exception
    {
        if (extEle == null) {
            return;
        }
        NodeList nodes = extEle.getElementsByTagName(SAMLNames.LOCALOSIDSTORE);
        if (nodes.getLength() == 0) {
            return;
        }

        Element localStoreEle = (Element) nodes.item(0);
        //add store if the tenant is default tenant.

        if (this.tenantName.equalsIgnoreCase(this.idmClient.getDefaultTenant())) {
            if (!localStoreEle.hasAttribute(SAMLNames.IDNAME)) {
                throw new IDMException("SAML medadata error: !"
                        +SAMLNames.LOCALOSIDSTORE
                        +" expect name attribute.");
            }
            String localOsDomainName = localStoreEle.getAttribute(SAMLNames.IDNAME);
            IIdentityStoreData localOsProvider =
                    IdentityStoreData.CreateLocalOSIdentityStoreData(localOsDomainName);
            Collection<IIdentityStoreData> existLocalOsDomainName = idmClient
                    .getProviders(this.tenantName,
                            EnumSet.of(DomainType.LOCAL_OS_DOMAIN));
            if (existLocalOsDomainName != null
                    && !existLocalOsDomainName.isEmpty()) {
                this.idmClient.deleteProvider(this.tenantName,
                        existLocalOsDomainName.iterator().next().getName());
            }
            this.idmClient.addProvider(this.tenantName, localOsProvider);
        }
    }

    /**
     *  import active directory store element in extensions element.
     * @param Extensions element.
     * @throws Exception:
     * @return  void
     */
    private
    void
    importADStores(Element extEle) throws Exception
    {

        if (extEle == null) {
            return;
        }
        //get active directory store element in extensions element.
        NodeList nodes = extEle.getElementsByTagName(SAMLNames.ACTIVEDIRECTORYIDSTORE);
        if (nodes.getLength() == 0) {
            return;
        }

        Element adEle = (Element) nodes.item(0);

        //parse ad store attributes
        if (!adEle.hasAttribute(SAMLNames.IDNAME) ||
                !adEle.hasAttribute(SAMLNames.IDUSERNAME) ||
                !adEle.hasAttribute(SAMLNames.IDSPN) ||
                !adEle.hasAttribute(SAMLNames.IDUSERPWD) ||
                !adEle.hasAttribute(SAMLNames.IDSEARCHBASEDN) ||
                !adEle.hasAttribute(SAMLNames.IDSEARCHTIMEOUTSCNDS)) {
            throw new IDMException("SAML medadata error: required " +
                    "ActiveDirectoryIdentityStore attribute is missing!");
        }

        String providerName = adEle.getAttribute(SAMLNames.IDNAME);
        String adFriendlyName =(adEle.hasAttribute(SAMLNames.IDFRIENDLYNAME))?
                adEle.getAttribute(SAMLNames.IDFRIENDLYNAME):null;
                String adUserName = adEle.getAttribute(SAMLNames.IDUSERNAME);
                String adUseMachineAccount = adEle.getAttribute(SAMLNames.IDUSEMACHACCT);
                String adSPN = adEle.getAttribute(SAMLNames.IDSPN);
                String adPwd = adEle.getAttribute(SAMLNames.IDUSERPWD);
                String searchBaseDn = adEle.getAttribute(SAMLNames.IDSEARCHBASEDN);
                String timeOutSecStr = adEle.getAttribute(SAMLNames.IDSEARCHTIMEOUTSCNDS);
                int timeOutSec = (timeOutSecStr == null)?
                        0:Integer.valueOf(timeOutSecStr);

                //parse KDC
                ArrayList<String> kdcList = new ArrayList<String>();
                NodeList kdcNodes = adEle.getElementsByTagName(SAMLNames.KDC);
                if (nodes.getLength() > 0) {
                    for (int ind=0; ind < kdcNodes.getLength();ind++) {
                        String kdc = ((Element)kdcNodes.item(ind)).getTextContent();
                        if (kdc != null) {
                            kdcList.add(kdc);
                        }
                    }
                }
                else {
                    throw new IDMException("SAML medadata error: missing KDC for " +
                            "ActiveDirectoryIdentityStore");
                }
                //parse Attribute map element
                Map<String, String> attrMap = new HashMap<String, String>();
                NodeList attrMapNodes = adEle.getElementsByTagName(SAMLNames.ATTRIBUTEMAP);
                if (nodes.getLength() > 0) {
                    for (int ind=0; ind < attrMapNodes.getLength();ind++) {
                        Element attrMapEle = (Element)attrMapNodes.item(ind);
                        if (!attrMapEle.hasAttribute(SAMLNames.IDNAME) ||
                                !attrMapEle.hasAttribute(SAMLNames.IDVALUE)) {
                            throw new IDMException("SAML medadata error: illegal AD attr map!");
                        }
                        attrMap.put(attrMapEle.getAttribute(SAMLNames.IDNAME),
                                attrMapEle.getAttribute(SAMLNames.IDVALUE));
                    }
                }
                else {
                    throw new IDMException("SAML medadata error: missing AttributMap " +
                            "for ActiveDirectoryIdentityStore");
                }

                //update store. Note: addProvider would fail if providerName is also used as aliase name
                IdentityStoreData adStore = IdentityStoreData.CreateExternalIdentityStoreData(
                        providerName,
                        null,
                        IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
                        AuthenticationType.USE_KERBEROS,
                        adFriendlyName,
                        timeOutSec,
                        adUserName,
                        adUseMachineAccount != null ? Boolean.getBoolean(adUseMachineAccount) : false,
                                adSPN,
                                adPwd,
                                searchBaseDn,
                                searchBaseDn,
                                kdcList,
                                attrMap,
                                null
                        );

                //  delete duplicated store
                this.idmClient.deleteProvider(this.tenantName, adStore.getName());
                this.idmClient.addProvider(this.tenantName, adStore);

    }

    /**
     * To import configurable Service provider settings to idm.
     * The following information will be imported: signing key and certificates
     * Meta data ignored: SSO/SLO bindings and location, IDP Attributes,etc
     * @param SPSSODescriptor element.
     * @throws Exception:
     * @return  void
     */
    private
    void
    importSPEntity(
            Element entity
            ) throws Exception
    {
        if (entity == null) {
            throw new IllegalArgumentException("Null Service Provider entity");
        }

        NodeList spList = entity.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.SPSSODESCRIPTOR);
        if (spList.getLength() == 0) {
            throw new IDMException("SAML medadata error: EntityDescriptor " +
                    "does not have an idp or sp data!");
        }
        Element spssoEle = (Element) spList.item(0);

        //Set entity id
        String rpID = entity.getAttribute(SAMLNames.ENTID);
        RelyingParty rp = null;

        try {
            //validate organization name
            rp = parseSPOrganization(entity);
            //Create RP with entityID, if organization element was not provided.
            if (rp == null) {
                rp = new RelyingParty(rpID);
                rp.setUrl(rpID);
            }
            boolean authnReqSigned = spssoEle.getAttribute(
                    SAMLNames.AUTHNREQUESTSIGNED).equals(SAMLNames.TRUE)?
                            true:false;
            rp.setAuthnRequestsSigned(authnReqSigned);

            validateRequiredProtocol(spssoEle);
            parseSPKeyDescriptor(spssoEle, rp);
            parseAssertionConsumerService(spssoEle, rp);
            parseSingleLogoutService(spssoEle, rp);
        } catch (Exception e) {
            throw new IDMException("Service provider metadata parsing error. ", e);
        }

        try {
            //delete existing relying party that matches the url;
            RelyingParty oldRP = this.idmClient.getRelyingPartyByUrl(this.tenantName, rp.getUrl());
            if (oldRP != null) {
                this.idmClient.deleteRelyingParty(this.tenantName, oldRP.getName());
            }
            oldRP = this.idmClient.getRelyingParty(this.tenantName, rp.getName());
            if (oldRP != null) {
                this.idmClient.deleteRelyingParty(this.tenantName, oldRP.getName());
            }
        } catch (Exception e) {
            throw new IDMException("Failed to remove existing service provider registration. ", e);
        }

        try {
            //adding rp has to be after completion of creating the object.
            this.idmClient.addRelyingParty(this.tenantName, rp);
        } catch (Exception e) {
            RelyingParty createdRP = this.idmClient.getRelyingPartyByUrl(this.tenantName, rp.getUrl());
            if (createdRP != null) {
                this.idmClient.deleteRelyingParty(this.tenantName, createdRP.getName());
            }
            throw e;
        }

    }

    /**
     * Create a new idm relying party by parsing "Organization" elment.
     * @param EntityDescriptptor that suppose to contains SPSSODescriptor .
     * @throws Exception:
     * @return  RelyingParty, or null if no organization element was found.
     */
    private
    RelyingParty
    parseSPOrganization(Element entityEle
            ) throws Exception
            {
        RelyingParty rp = null;
        //Organization element is optional.
        NodeList nodes = entityEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.ORGANIZATION);
        if (nodes.getLength() > 0) {
            Element orgEle = (Element) nodes.item(0);
            String orgName = orgEle.getElementsByTagNameNS(
                    SAMLNames.NS_NAME_SAML_METADATA,"OrganizationName")
                    .item(0).getTextContent();
            String orgURL = orgEle.getElementsByTagNameNS(
                    SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.ORGANIZATIONURL)
                    .item(0).getTextContent();
            rp = new RelyingParty(orgName);
            rp.setUrl(orgURL);
        }
        return rp;
            }


    /**
     * @throws IDMException
     * Import assertion consumer service. Required for at least one instance
     * @param SPSSODescriptor element.
     * @throws Exception:
     * @return  void
     *
     */
    private
    void
    parseAssertionConsumerService(
            Element spssoEle,
            RelyingParty rp
            ) throws IDMException
            {
        NodeList serviceList = spssoEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,
                SAMLNames.ASSERTIONCONSUMERSERVICE);
        int nNode = serviceList.getLength();
        if (nNode == 0) {
            throw new IDMException("SAML metadata error: No AssertionConsumerService");
        }

        List<AssertionConsumerService> services =
                new ArrayList<AssertionConsumerService>();
        for (int ind=0; ind < nNode; ind++) {
            Element ele = (Element) serviceList.item(ind);
            String binding = ele.getAttribute(SAMLNames.BINDING);
            if (!binding.equals(SAMLNames.HTTP_POST_BINDING)) {
                continue;
            }
            String loc = ele.getAttribute(SAMLNames.LOCATION);
            int  index = Integer.valueOf(ele.getAttribute(SAMLNames.INDEX));

            //create new object and set the three required attributes.
            AssertionConsumerService e = new AssertionConsumerService(loc);
            e.setBinding(binding);
            e.setEndpoint(loc);
            e.setIndex(index);

            //check if it is default assertion consumer service..
            String isDefault = ele.getAttribute(SAMLNames.ISDEFAULT);
            if (!isDefault.isEmpty() && isDefault.equals(SAMLNames.TRUE)) {
                rp.setDefaultAssertionConsumerService(loc);
            }
            services.add(e);
        }
        if (services.isEmpty()) {
            throw new IDMException("No supported assertion consumer service was provided!");
        }
        rp.setAssertionConsumerServices(services);
        return;
            }

    /**
     * @throws IDMException
     * Import single logout service. Requires both POST and SOAP bindings.
     * @param SPSSODescriptor element.
     * @throws Exception:
     * @return  void
     *
     */
    private
    void
    parseSingleLogoutService(
            Element spssoEle,
            RelyingParty rp
            ) throws IDMException
            {
        rp.setSingleLogoutServices(getSingleLogoutServices(spssoEle));
        return;
            }

    /**
     * extract SingleLogoutSerive objects from the descriptor element passed in.
     * Currently only the two required binding types http-redirect are supported SLO binding.
     *
     * @param ssoDescElem
     *            element of SSODecriptor or its sub-type
     * @return List of SingleLogoutService objects.
     * @throws IDMException
     *             when there are no SingleSignoutService specified, or the
     *             required SOAP-binding and HTTP-Redirect-binding are not found
     */
    private Collection<ServiceEndpoint> getSingleLogoutServices(Element ssoDescElem) throws IDMException
    {
        List<ServiceEndpoint> services = new ArrayList<ServiceEndpoint>();

        NodeList serviceList = ssoDescElem.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,
                SAMLNames.SINGLELOGOUTSERVICE);
        int nNode = serviceList.getLength();

        // SLO service is optional and the number of node can be 0.
        if (nNode != 0) {
            boolean redirectService = false;

            for (int ind=0; ind < nNode; ind++) {
                Element ele = (Element) serviceList.item(ind);
                String binding = ele.getAttribute(SAMLNames.BINDING);
                if (binding.equals(SAMLNames.HTTP_REDIRECT_BINDING)) {
                    redirectService = true;
                } else {
                    continue;
                }
                String loc = ele.getAttribute(SAMLNames.LOCATION);
                String rspLoc = null;
                if (ele.hasAttribute(SAMLNames.RESPONSE_LOCATION)) {
                    rspLoc = ele.getAttribute(SAMLNames.RESPONSE_LOCATION);
                }

                //create new object and set the three required attributes.
                ServiceEndpoint e = new ServiceEndpoint(loc+binding, loc, rspLoc, binding);

                services.add(e);
            }
            if (!redirectService) {
                logger.warn("Expect HTTP-Redirect binding Single Logout Service!");
            }
        }
        return services;
    }

    /**
     * extract SingleSignOnSerive objects from the descriptor element passed in
     *
     * @param idpSSODescElem
     *            element of IDPSSODescriptor element.
     * @return List of SingleSignOnService objects
     * @throws IDMException
     *             when no such nodes are found
     */
    private Collection<ServiceEndpoint> getSingleSignOnServices(
            Element idpSSODescElem) throws IDMException
            {
        NodeList ssoServiceNodes =
                idpSSODescElem.getElementsByTagNameNS(
                        SAMLNames.NS_NAME_SAML_METADATA,
                        SAMLNames.SINGLESIGNONSERVICE);
        if (0 == ssoServiceNodes.getLength())
        {
            throw new IDMException(
                    "SAML metadata error: at least one SingleSignOnService element is required");
        }
        List<ServiceEndpoint> ssoServices =
                new ArrayList<ServiceEndpoint>();
        for (int idx = 0; idx < ssoServiceNodes.getLength(); idx++)
        {
            Element current = (Element) ssoServiceNodes.item(idx);
            String binding = current.getAttribute(SAMLNames.BINDING);
            String loc = current.getAttribute(SAMLNames.LOCATION);
            //compose the name from binding and loc
            ServiceEndpoint ssoService =
                    new ServiceEndpoint(loc + binding, loc, binding);
            ssoServices.add(ssoService);
        }
        return ssoServices;
            }

    /**
     * extract NameIDFormat URI objects from the descriptor element passed in
     *
     * @param ssoDescElem
     *            element of SSODescriptor or its sub-type
     * @return list of URIs for the nameIDFormats
     * @throws IDMException
     *             when no NameIDFormats are found, or when invalid URI syntax
     *             detected, or the nameIDFormat is not supported by the SSO
     *             System
     */
    private Collection<URI> getNameIDFormats(Element ssoDescElem)
            throws IDMException
            {
        NodeList nameIDFormatNodes =
                ssoDescElem
                .getElementsByTagNameNS(
                        SAMLNames.NS_NAME_SAML_METADATA,
                        SAMLNames.NAMEIDFORMAT);
        if (0 == nameIDFormatNodes.getLength())
        {
            throw new IDMException(
                    "SAML metadata error: at least one NameIDFormat element is required");
        }

        Collection<URI> result = new ArrayList<URI>();

        for (int idx = 0; idx < nameIDFormatNodes.getLength(); idx++)
        {
            try
            {
                String uriStr = nameIDFormatNodes.item(idx).getTextContent();
                URI current = new URI(sanitize(uriStr));
                result.add(current);
            } catch (URISyntaxException use)
            {
                throw new IDMException(String.format("Invalid URI Syntax [%s]",
                        nameIDFormatNodes.item(idx).getTextContent()));
            }
        }
        return result;
            }

    /**
     *
     * @param KeyDescriptor element.
     * @throws Exception:
     * @return  void
     * @throws CertificateException
     * @throws IDMException
     *
     */

    private
    void
    parseSPKeyDescriptor(
            Element spssoEle,
            RelyingParty rp
            ) throws CertificateException, IDMException
            {
        //One KeyDescriptor for one key chain.
        NodeList keyDescriptorList = spssoEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA,SAMLNames.KEYDESCRIPTOR);
        if (keyDescriptorList.getLength() == 0) {
            return;
        }
        Element keyDescriptorEle = (Element) keyDescriptorList.item(0);

        //"use" attribute should be "signing", if defined.
        String useStr = keyDescriptorEle.getAttribute(SAMLNames.USE);
        if (!useStr.isEmpty() && !useStr.equals(SAMLNames.SIGNING)) {
            return;
        }

        //Get Keyinfo. no need to check availability since it is schema-inforced.
        NodeList keyInfoList = keyDescriptorEle.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_DIGTALSIG,
                SAMLNames.KEYINFO);
        Element keyInfoEle = (Element) keyInfoList.item(0);

        Collection<Certificate> spCertificates = parseCertificates(keyInfoEle);
        if (spCertificates == null) {
            throw new IDMException("SAML metadata error: no valid certificate found!");
        }
        //Question - could it be certificate chain, as that for idp?
        rp.setCertificate(spCertificates.iterator().next());

            }

    /**
     * make sure a required protocol is contained in protocalSupportEnumeration attribute.
     * @param roleDescriptorElem element under examination -- could be any sub-type
     * @throws IDMException when attribute does not match required protocol for IDP
     */
    private
    void
    validateRequiredProtocol(
            Element roleDescriptorElem) throws IDMException
            {
        String protocols = roleDescriptorElem.getAttribute(SAMLNames.PSE);

        if (!protocols.contains(SAMLNames.REQUIREDPROTOCAL)) {
            throw new IDMException(
                    String.format(
                            "SAML meta data error. Element's attribute %s=[%s]"
                                    + "does not have required protocal declaration [%s]!",
                                    SAMLNames.PSE,
                                    protocols,
                                    SAMLNames.REQUIREDPROTOCAL));
        }
            }

    private
    PrivateKey
    parseKey(
            Element keyInfo
            ) throws NoSuchAlgorithmException, InvalidKeySpecException {

        PrivateKey key= null;
        NodeList modulusList = keyInfo.getElementsByTagName(
                SAMLNames.DS_MODULUS);
        if (modulusList.getLength() == 0) {
            return null;
        }
        NodeList exponentList = keyInfo.getElementsByTagName(
                SAMLNames.DS_EXPONENT);
        if (exponentList.getLength() == 0) {
            return null;
        }
        //get the elements
        Element exponentEle = (Element) exponentList.item(0);
        Element modulusEle = (Element) modulusList.item(0);

        //decode exponent and modulus
        BigInteger expInt = new BigInteger(Base64.decode(
                exponentEle.getTextContent()));
        BigInteger modInt = new BigInteger(Base64.decode(
                modulusEle.getTextContent()));

        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        RSAPrivateKeySpec rsaKeySpec = new RSAPrivateKeySpec(modInt,expInt);

        key = keyFactory.generatePrivate(rsaKeySpec);
        return key;
    }

    private
    List<Certificate>
    parseCertificates(
            Element keyInfoEle
            ) throws CertificateException {

        //expecting one certificate chain at least.
        NodeList nodes = keyInfoEle.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_DIGTALSIG, SAMLNames.X509DATA);
        if (nodes.getLength() == 0) {
            return null;
        }
        //The schema allows multiple X509Data element, and we just pick the first
        //one, in which we expect one or more certificates in the chain
        NodeList certList = ((Element) nodes.item(0)).getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_DIGTALSIG,
                SAMLNames.X509CERTIFICATE);
        if (certList.getLength() == 0) {
            return null;
        }

        List<Certificate> certificates = new ArrayList<Certificate>();
        for (int i = 0; i < certList.getLength();i++ ) {

            Element x509Ele = (Element)certList.item(i);

            if (x509Ele != null) {

                byte [] certDecoded = Base64.decode(
                        x509Ele.getTextContent());
                CertificateFactory cf = CertificateFactory.getInstance("X.509");
                X509Certificate c = (X509Certificate) cf.generateCertificate(
                        new ByteArrayInputStream(certDecoded));
                certificates.add(c);
            }
        }

        if (certificates.size() == 0) {
            return null;
        }
        return certificates;
    }

    /**
     * sanitize the string by removing CR and leading/tailing white-spaces
     * @param input
     * @return
     */
    private String sanitize(String input)
    {
        return input.replaceAll("\\r|\\n", "").trim();
    }

}
