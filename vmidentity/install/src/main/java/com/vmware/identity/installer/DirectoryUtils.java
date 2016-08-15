package com.vmware.identity.installer;

/**
 * This class is used to clean up the LDU structure and move the contents
 * within it under Services Dn.
 *
 * This will first verify if the current install has old STructure , if yes
 * then it will find move the signers under LDUGuid, and copy all Cert Chains
 * under Services Dn.
 *
 * This will cover all scenarios like - fresh install, standalone and federated SSO.
 */
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.interop.directory.Directory;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;

public class DirectoryUtils {

    private String rootDn;
    private String tenantName;
    private String lduGuid;
    private ILdapConnectionEx connection;

    private final String CONTAINER_TRUSTED_CERTIFICATE_CHAINS = "TrustedCertificateChains";
    private final String TRUSTED_CERT_CHAIN_OBJECT_CLASS = "vmwSTSTenantTrustedCertificateChain";
    private final String TRUSTED_CRED_OBJECT_CLASS = "vmwSTSTenantCredential";
    private final String CERT_CHAIN_PREFIX = "TrustedCertChain";
    private final String CRED_PREFIX = "TenantCredential";

    public DirectoryUtils(ILdapConnectionEx connection, String rootDn,
            String tenantName) {
        this.connection = connection;
        this.rootDn = rootDn;
        this.tenantName = tenantName;
        this.lduGuid = Directory.GetLocalLduGuid();
    }

    /**
     * Checks if the current directory has LDU structure.
     *
     * @return
     */
    public boolean hasOldLduStructure() {

        boolean isLduStructure = false;

        String searchFilter = String.format("(objectclass=%s)",
                TRUSTED_CRED_OBJECT_CLASS);
        ILdapMessage message = lookupObject(getLduDn(),
                LdapScope.SCOPE_ONE_LEVEL, searchFilter);
        if (message != null) {
            ILdapEntry[] entries = message.getEntries();
            if (entries != null && entries.length > 0) {
                isLduStructure = true;
            }
        }
        return isLduStructure;
    }

    /**
     * This will do the actual upgrade process by moving tenant credentials and
     * copying all tenant certificate chains to new location under services.
     *
     * Anything going wrong here would fail the upgrade.
     */
    public void upgradeNode() throws Exception {

        try {
            moveSigner();
            copyTrustedCertChain();
        } catch (Exception ex) {
            System.out.println("Error occured during ldu cleanup.");
            throw ex;
        }

        try {
            if (!hasSignerAtOldLocation()) {
                deleteTrustedCertChains();
            }
        } catch (Exception ex) {
            System.out
                    .println("Upgrade preserves legacy tenant credential information, "
                            + "however, cleaning up unused trusted certification chain "
                            + "is incomplete. " + ex);
        }
    }

    /**
     * This will copy the signers at new loctaion if needed and then delete from
     * the old location.
     */
    private void moveSigner() {
        if (!hasSignerAtNewLocation()) {
            // copy signer at new location
            copySignerToNewLocation();
        }
        // delete the signer
        deleteSignerFromOldLocation();
    }

    /**
     * This method will check if the system Tenant already has any signer under
     * CN
     * =<SystemTenant>,CN=Tenants,CN=IdentityManager,CN=Services,dc=vsphere,dc=
     * local.
     */
    private boolean hasSignerAtNewLocation() {

        boolean hasSigner = false;

        if (tenantName != null) {
            ILdapMessage message = null;
            try {
                String searchFilter = String.format("(objectclass=%s)",
                        TRUSTED_CRED_OBJECT_CLASS);
                message = lookupObject(getTenantsDn(tenantName),
                        LdapScope.SCOPE_ONE_LEVEL, searchFilter);
                if (message != null) {
                    ILdapEntry[] entries = message.getEntries();
                    if (entries != null && entries.length > 0) {
                        hasSigner = true;
                    }
                }
            } finally {
                closeMessage(message);
            }
        }
        return hasSigner;
    }

    /**
     * Copy the tenant credentials/signers at new location under Services.
     */
    private void copySignerToNewLocation() {

        // retrieve signers at old location
        String tenantLduDn = getLduDn();
        ILdapMessage credsObjects = null;
        try {
            credsObjects = getTenantCredentials(tenantLduDn);

            int maxIndex = getCurrMaxIndex(TRUSTED_CRED_OBJECT_CLASS, null) + 1;
            String cn = getTenantCredentialCn(maxIndex);

            ILdapEntry lastSigner = getLastSigner(credsObjects);

            List<LdapMod> modValues = createAtts(lastSigner);
            LdapValue[] values = { new LdapValue(cn) };

            LdapMod mod = new LdapMod(LdapModOperation.ADD, "cn", values);
            modValues.add(mod);

            String baseDn = String.format(
                    "CN=%s,CN=Tenants,CN=IdentityManager,CN=Services,%s",
                    tenantName, this.rootDn);
            String newDn = "CN=" + cn + "," + baseDn;

            // set signers at new location
            connection.addObject(newDn, modValues.toArray(new LdapMod[] {}));
        } finally {
            closeMessage(credsObjects);
        }
    }

    /**
     * This will delete the signer objects from old loctaion.
     */
    private void deleteSignerFromOldLocation() {

        String tenantLduDn = getLduDn();
        ILdapMessage credsObjects = null;
        try {
            credsObjects = getTenantCredentials(tenantLduDn);
            delete(credsObjects);
        } finally {
            closeMessage(credsObjects);
        }
    }

    /**
     * This will check if the LDU Structure has any more signers available under
     * LDus.
     */
    public boolean hasSignerAtOldLocation() {

        boolean hasSigner = false;

        String baseDn = getRootLdusDn();

        // retrieve TenantCredentials if any in LDUs
        ILdapMessage message = null;
        try {
            message = getTenantCredentials(baseDn);

            if (message != null) {

                ILdapEntry[] entries = message.getEntries();
                if (entries != null && entries.length != 0) {

                    hasSigner = true;
                }
            }
        } finally {
            closeMessage(message);
        }
        return hasSigner;
    }

    /**
     * This will remove the LDU Structure if no more signers are available under
     * LDus. This is done to handle the federated SSO.
     *
     * Once all the signers are moved which means that all nodes are upgraded,
     * we can remove the Ldu structure overall. Anything which fails here is
     * just a log message without failing the upgrade process.
     */
    private void deleteTrustedCertChains() {

        String baseDn = getRootLdusDn();

        // Remove trusted cert chains
        ILdapMessage certChainsMsg = null;
        ILdapMessage certContainerMsg = null;
        try {
            certChainsMsg = getTenantCertChains(baseDn);
            delete(certChainsMsg);

            // Now Remove trusted cert chains container
            certContainerMsg = getTenantCertChainsContainer(baseDn);
            delete(certContainerMsg);
        } finally {
            closeMessage(certChainsMsg);
            closeMessage(certContainerMsg);
        }
    }

    private void closeMessage(ILdapMessage message) {
        if (message != null) {
            message.close();
        }
    }

    private void copyTrustedCertChain() throws Exception {

        // Get trusted cert chains at old locations.
        ILdapMessage certChainsOld = null;
        try {
            certChainsOld = getTenantCertChains(getRootLdusDn());

            // Get trusted cert chains at new locations.
            List<ArrayList<String>> certChainsNew = getCertificates(getTenantsDn(tenantName));

            // If No Cert at New Location, add Cert Chain Container.
            if (certChainsNew == null || certChainsNew.isEmpty()) {
                addTenantCertChainsContainer();
            }

            // for each cert at old location, check if its is not present at new
            // location then add.
            addTrustedCertChainsIfAbsent(certChainsOld, certChainsNew);
        } finally {
            closeMessage(certChainsOld);
        }
    }

    /**
     * This method will add the cert chains at the new location if its not
     * already present by comparing their fingerprints.
     *
     * @param certChainsOld
     * @param certChainsNew
     * @throws CertificateEncodingException
     * @throws NoSuchAlgorithmException
     */
    private void addTrustedCertChainsIfAbsent(ILdapMessage certChainsOld,
            List<ArrayList<String>> certChainsNew)
            throws CertificateEncodingException, NoSuchAlgorithmException {

        if (certChainsOld != null) {
            ILdapEntry[] entries = certChainsOld.getEntries();

            if (entries != null && entries.length > 0) {
                int certsIndex = getCurrMaxIndex(
                        TRUSTED_CERT_CHAIN_OBJECT_CLASS,
                        CONTAINER_TRUSTED_CERTIFICATE_CHAINS) + 1;

                String dn = String
                        .format("CN=%s,CN=%s,CN=Tenants,CN=IdentityManager,CN=Services,%s",
                                CONTAINER_TRUSTED_CERTIFICATE_CHAINS,
                                tenantName, this.rootDn);

                Map<String, LdapMod[]> ldapObjects = new HashMap<>();

                for (ILdapEntry lduCert : entries) {

                    ArrayList<String> certFingerprints = new ArrayList<>();
                    certFingerprints.addAll(CertificatesUtil
                            .getCertFingerPrints(lduCert));

                    // If certificate not present at new location, then add it.

                    if (! containsCertChain(certChainsNew, certFingerprints)) {

                        List<LdapMod> objAttributes = createAtts(lduCert);

                        String cn = getTrustedCertChainCn(certsIndex);
                        LdapValue[] values = { new LdapValue(cn) };

                        LdapMod mod = new LdapMod(LdapModOperation.ADD, "cn",
                                values);
                        objAttributes.add(mod);

                        String objNewDn = "CN=" + cn + "," + dn;

                        ldapObjects.put(objNewDn,
                                objAttributes.toArray(new LdapMod[] {}));
                        certsIndex++;
                    }
                }

                if (ldapObjects != null && ldapObjects.size() > 0) {
                    addObjects(ldapObjects);
                }
            }
        }
    }

    /**
     * Compares the given certchain fingerprint with the collection of cert chains.
     *
     * @param certChainsNew
     * @param certFingerprints
     * @return true if cert chain is already present at new location else false.
     */
    private boolean containsCertChain(List<ArrayList<String>> certChainsNew,
            ArrayList<String> certFingerprints) {
        boolean hasCertChain = false;

        for (ArrayList<String> chain : certChainsNew) {
            if (chain.size() == certFingerprints.size()
                    && chain.containsAll(certFingerprints)) {
                hasCertChain = true;
                break;
            }
        }
        return hasCertChain;
    }

    private String getLduDn() {
        return String.format("CN=%s,CN=Ldus,CN=ComponentManager,%s",
                this.lduGuid, this.rootDn);
    }

    private String getRootLdusDn() {
        return String.format("CN=Ldus,CN=ComponentManager,%s", this.rootDn);
    }

    private String getTenantsDn(String tenant) {
        return String.format(
                "CN=%s,CN=Tenants,CN=IdentityManager,CN=Services,%s", tenant,
                this.rootDn);
    }

    /**
     * Get tenant Credentials at baseDn
     *
     * @param baseDn
     * @return
     */
    private ILdapMessage getTenantCredentials(String baseDn) {

        String searchFilter = String.format("(objectclass=%s)",
                TRUSTED_CRED_OBJECT_CLASS);
        ILdapMessage message = lookupObject(baseDn, LdapScope.SCOPE_SUBTREE,
                searchFilter);
        return message;
    }

    /**
     * Get tenant Credentials at baseDn
     *
     * @param baseDn
     * @return
     */
    public boolean hasTenantCredentials(String tenantName) {

        boolean hasCreds = false;
        ILdapMessage message = null;

        try {
            message = getTenantCredentials(getTenantsDn(tenantName));

            if (message != null) {
                ILdapEntry[] entries = message.getEntries();

                if (entries != null && entries.length > 0) {
                    hasCreds = true;
                }
            }
        } finally {
           closeMessage(message);
        }
        return hasCreds;
    }

    /**
     * Get tenant Certificate Chains Container at baseDn
     *
     * @param baseDn
     * @return
     */
    private ILdapMessage getTenantCertChainsContainer(String baseDn) {

        String searchFilter = String.format("(cn=%s)",
                CONTAINER_TRUSTED_CERTIFICATE_CHAINS);
        ILdapMessage message = lookupObject(baseDn, LdapScope.SCOPE_SUBTREE,
                searchFilter);
        return message;

    }

    /**
     * Get tenant Certificate Chains at baseDn
     *
     * @param baseDn
     * @return
     */
    public ILdapMessage getTenantCertChains(String baseDn) {

        String searchFilter = String.format("(&(objectclass=%s)(cn=%s-*))",
                TRUSTED_CERT_CHAIN_OBJECT_CLASS, CERT_CHAIN_PREFIX);
        ILdapMessage message = lookupObject(baseDn, LdapScope.SCOPE_SUBTREE,
                searchFilter);
        return message;
    }

    /**
     * This would get the last(Latest) signer from the Ldu to be moved under
     * Services.
     *
     * @param message
     * @return
     */
    private ILdapEntry getLastSigner(ILdapMessage message) {

        ILdapEntry lastSigner = null;

        if (message != null) {

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0) {

                int maxIndex = -1;

                // get last signer checking its CN
                for (ILdapEntry entry : entries) {

                    String objectCn = entry.getAttributeValues("cn")[0]
                            .getString();
                    Character c = objectCn.charAt(objectCn.length() - 1);
                    int index = Character.digit(c, 10);
                    if ((index > maxIndex)) {
                        lastSigner = entry;
                        maxIndex = index;
                    }
                }
            }
        }
        return lastSigner;
    }

    /**
     * Copy trusted certificate chains container under services
     *
     * @param message
     */
    private void addTenantCertChainsContainer() {

        ILdapMessage message = null;
        try {
            message = getTenantCertChainsContainer(getLduDn());

            String objNewDn = String.format(
                    "CN=%s,CN=Tenants,CN=IdentityManager,CN=Services,%s",
                    tenantName, this.rootDn);

            Map<String, LdapMod[]> containerObjects = getLdapObjects(message,
                    objNewDn);
            if (containerObjects != null && containerObjects.size() > 0) {
                addObjects(containerObjects);
            }
        } finally {
            closeMessage(message);
        }
    }

    private String getTrustedCertChainCn(int index) {
        return String.format("%s-%d", CERT_CHAIN_PREFIX, index);
    }

    private String getTenantCredentialCn(int index) {
        return String.format("%s-%d", CRED_PREFIX, index);
    }

    /**
     * Deletes the ldapObjects in the message.
     *
     * @param message
     */
    private void delete(ILdapMessage message) {

        if (message != null) {

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length != 0) {

                for (ILdapEntry entry : entries) {
                    connection.deleteObject(entry.getDN());
                }
            }

        }
    }

    /**
     * Looks up the LDAP objects with given search filter.
     *
     * @param baseDn
     * @param scope
     * @param searchFilter
     * @return ILdapMessage
     */
    private ILdapMessage lookupObject(String baseDn, LdapScope scope,
            String searchFilter) {
        ValidateUtil.validateNotNull(this.connection, "connection");
        ValidateUtil.validateNotEmpty(baseDn, "baseDn");

        ILdapMessage message = connection.search(baseDn, scope, searchFilter,
                new String[] {}, false);

        return message;
    }

    /**
     * Returns the map of [NewDN and Attributes values] for the objects to be
     * added.
     *
     * @param message
     * @param objNewDn
     * @return
     */
    private Map<String, LdapMod[]> getLdapObjects(ILdapMessage message,
            String objNewDn) {
        Map<String, LdapMod[]> ldapObjects = new HashMap<>();

        if (message != null) {
            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0) {

                for (ILdapEntry entry : entries) {

                    // for each tenant credential, get all attributes and
                    // their values to create new object.
                    String[] attrs = entry.getAttributeNames();
                    List<LdapMod> objAttributes = new ArrayList<>();

                    for (String attr : attrs) {
                        if (!attr.equalsIgnoreCase("nTSecurityDescriptor"))
                        {
                            LdapValue[] values = entry.getAttributeValues(attr);

                            LdapMod mod = new LdapMod(LdapModOperation.ADD, attr,
                                                        values);
                            objAttributes.add(mod);
                        }
                    }

                    String objectCn = entry.getAttributeValues("cn")[0]
                            .getString();

                    if (objectCn != null) {

                        String objDn = "cn=" + objectCn + "," + objNewDn;
                        ldapObjects.put(objDn,
                                objAttributes.toArray(new LdapMod[] {}));
                    }
                }
            }
        }
        return ldapObjects;
    }

    /**
     * This would formulate all attributes for object to be added.
     *
     * @param entry
     * @return
     */
    private List<LdapMod> createAtts(ILdapEntry entry) {

        String[] attrs = entry.getAttributeNames();
        List<LdapMod> objAttributes = new ArrayList<>();

        for (String attr : attrs) {
            // Ignore ntSecurityDescriptor is readonly on the vmdir and should be ignored
            if (!attr.equalsIgnoreCase("cn")  && !attr.equalsIgnoreCase("nTSecurityDescriptor")) {
                LdapValue[] values = entry.getAttributeValues(attr);
                LdapMod mod = new LdapMod(LdapModOperation.ADD, attr, values);
                objAttributes.add(mod);
            }
        }
        return objAttributes;
    }

    /**
     * Adds up the LDAP objects at the new DN.
     *
     * @param objects
     */
    private void addObjects(Map<String, LdapMod[]> objects) {

        if (objects != null && objects.size() > 0) {

            for (Map.Entry<String, LdapMod[]> entry : objects.entrySet()) {
                connection.addObject(entry.getKey(), entry.getValue());
            }
        }
    }

    private int getCurrMaxIndex(String filter, String subDn) {

        int currMaxCredIndex = 0;
        String baseDn = getTenantsDn(tenantName);

        if (subDn != null) {
            baseDn = "CN=" + subDn + "," + baseDn;
        }

        String searchFilter = String.format("(objectclass=%s)", filter);
        ILdapMessage message = null;
        try {
            message = lookupObject(baseDn, LdapScope.SCOPE_ONE_LEVEL,
                    searchFilter);
            if (message != null) {

                ILdapEntry[] entries = message.getEntries();
                if (entries != null && entries.length > 0) {
                    for (ILdapEntry entry : entries) {

                        String objectDn = entry.getDN();
                        if (objectDn != null) {
                            String indexStr = objectDn.substring(
                                    objectDn.indexOf('-') + 1,
                                    objectDn.indexOf(','));
                            int currIndex = Integer.parseInt(indexStr);
                            if (currMaxCredIndex < currIndex) {
                                currMaxCredIndex = currIndex;
                            }
                        }
                    }
                }
            }
        } finally {
            closeMessage(message);
        }
        return currMaxCredIndex;
    }

    private List<ArrayList<String>> getCertificates(String baseDn)
            throws Exception {

        List<ArrayList<String>> certificatesList = new ArrayList<ArrayList<String>>();

        ILdapMessage certChains = null;
        try {
            certChains = getTenantCertChains(baseDn);

            if (certChains != null) {
                ILdapEntry[] entries = certChains.getEntries();

                if (entries != null && entries.length > 0) {
                    // for all certificates get their fingerprints.
                    for (ILdapEntry entry : entries) {
                        certificatesList.add(CertificatesUtil
                                .getCertFingerPrints(entry));
                    }
                }
            }
        } finally {
            closeMessage(certChains);
        }
        return certificatesList;
    }
}
