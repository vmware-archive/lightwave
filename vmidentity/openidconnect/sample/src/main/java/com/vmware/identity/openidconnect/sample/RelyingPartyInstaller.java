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

package com.vmware.identity.openidconnect.sample;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.math.BigInteger;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.Security;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.UUID;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import org.apache.http.conn.ssl.DefaultHostnameVerifier;
import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.cert.jcajce.JcaX509v3CertificateBuilder;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.identity.openidconnect.client.AccessToken;
import com.vmware.identity.openidconnect.client.ClientConfig;
import com.vmware.identity.openidconnect.client.ConnectionConfig;
import com.vmware.identity.openidconnect.client.MetadataHelper;
import com.vmware.identity.openidconnect.client.OIDCClient;
import com.vmware.identity.openidconnect.client.OIDCTokens;
import com.vmware.identity.openidconnect.client.TokenSpec;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.openidconnect.common.TokenType;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;

/**
 * @author Jun Sun
 */
class RelyingPartyInstaller {

    private final RelyingPartyConfig relyingPartyConfig;
    private final KeyStore keyStore;

    RelyingPartyInstaller(
            RelyingPartyConfig relyingPartyConfig,
            KeyStore keyStore) {
        this.relyingPartyConfig = relyingPartyConfig;
        this.keyStore = keyStore;
    }

    void install(
            String[] redirectEndpointUrls,
            String[] postLogoutRedirectUrls,
            String logoutUrl) throws Exception {
        String domainControllerFQDN = this.relyingPartyConfig.getOpFQDN();
        int domainControllerPort = Integer.parseInt(this.relyingPartyConfig.getOpListeningPort());
        String tenant = this.relyingPartyConfig.getTenant();

        // retrieve OIDC meta data
        MetadataHelper metadataHelper = new MetadataHelper.Builder(domainControllerFQDN).
                domainControllerPort(domainControllerPort).
                tenant(tenant).
                keyStore(this.keyStore).
                build();

        ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
        RSAPublicKey providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

        // create a non-registered OIDC client and get bearer tokens by admin user name/password
        ConnectionConfig connectionConfig = new ConnectionConfig(providerMetadata, providerPublicKey, this.keyStore);
        ClientConfig clientConfig = new ClientConfig(connectionConfig, null, null, 10 * 60L /* clockToleranceInSeconds */);
        OIDCClient nonRegisteredClient = new OIDCClient(clientConfig);
        TokenSpec tokenSpec = new TokenSpec.Builder().resourceServers(Arrays.asList("rs_admin_server")).build();
        OIDCTokens oidcTokens = nonRegisteredClient.acquireTokensByPassword(
                this.relyingPartyConfig.getAdminUsername(),
                this.relyingPartyConfig.getAdminPassword(),
                tokenSpec);

        // create a private/public key pair, generate a certificate and assign it to a solution user name.
        Security.addProvider(new BouncyCastleProvider());
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA", "BC");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair keypair = keyGen.generateKeyPair();
        String solutionUserName = this.relyingPartyConfig.getClientPrefix() + UUID.randomUUID().toString();
        X509Certificate clientCertificate = generateCertificate(keypair, solutionUserName);

        // create REST idm client
        IdmClient idmClient = createIdmClient(
                oidcTokens.getAccessToken(),
                domainControllerFQDN,
                domainControllerPort);

        VmdirClient vmdirClient = createVMdirClient(
                oidcTokens.getAccessToken(),
                domainControllerFQDN,
                domainControllerPort);

        // create a solution user
        CertificateDTO certificateDTO = new CertificateDTO.Builder()
        .withEncoded(convertToBase64PEMString(clientCertificate))
        .build();
        SolutionUserDTO solutionUserDTO = new SolutionUserDTO.Builder().
                withName(solutionUserName).
                withDomain(tenant).
                withCertificate(certificateDTO).
                build();
        vmdirClient.solutionUser().create(tenant, solutionUserDTO);

        // add the solution user to ActAs group
        List<String> members = Arrays.asList(solutionUserName + "@" + tenant);
        vmdirClient.group().addMembers(tenant, "ActAsUsers", tenant, members, com.vmware.directory.rest.common.data.MemberType.USER);

        // register a OIDC client
        OIDCClientMetadataDTO oidcClientMetadataDTO = new OIDCClientMetadataDTO.Builder().
                withRedirectUris(Arrays.asList(redirectEndpointUrls)).
                withPostLogoutRedirectUris(Arrays.asList(postLogoutRedirectUrls)).
                withLogoutUri(logoutUrl).
                withTokenEndpointAuthMethod("private_key_jwt").
                withCertSubjectDN(clientCertificate.getSubjectDN().getName()).
                withAuthnRequestClientAssertionLifetimeMS(2 * 60 * 1000L).
                build();
        OIDCClientDTO oidcClientDTO = idmClient.oidcClient().register(tenant, oidcClientMetadataDTO);

        // persist data involved installation in files so they can be picked up in case server reboots
        savePublicKey(this.relyingPartyConfig.getOpPublickeyFile(), providerPublicKey);
        savePrivateKey(this.relyingPartyConfig.getRpPrivatekeyFile(), keypair.getPrivate());
        writeObject(this.relyingPartyConfig.getRpCertificateFile(), clientCertificate);
        writeObject(this.relyingPartyConfig.getRpInfoFile(), oidcClientDTO.getClientId());
        writeObject(this.relyingPartyConfig.getRpListeningPortFile(), this.relyingPartyConfig.getRpListeningPort());
    }

    static Object readObject(String file) throws IOException, ClassNotFoundException {
        FileInputStream fis = null;
        ObjectInputStream ois = null;
        try {
            fis = new FileInputStream(file);
            ois = new ObjectInputStream(fis);
            Object object = ois.readObject();
            return object;
        } finally {
            if (ois != null) {
                ois.close();
            }
        }
    }

    private void writeObject(String file, Object object) throws IOException {
        FileOutputStream fos = null;
        ObjectOutputStream oos = null;
        try {
            fos = new FileOutputStream(file.toString());
            oos = new ObjectOutputStream(fos);
            oos.writeObject(object);
        } finally {
            if (oos != null) {
                oos.close();
            }
        }
    }

    private void savePublicKey(String file, PublicKey publicKey) throws IOException {
        // Store Public Key.
        X509EncodedKeySpec x509EncodedKeySpec = new X509EncodedKeySpec(publicKey.getEncoded());
        FileOutputStream fos = new FileOutputStream(file);
        fos.write(x509EncodedKeySpec.getEncoded());
        fos.close();
    }

    static PublicKey loadPublicKey(String file, String algorithm) throws IOException, NoSuchAlgorithmException, InvalidKeySpecException {
        // Read Public Key.
        File filePublicKey = new File(file);
        FileInputStream fis = new FileInputStream(file);
        byte[] encodedPublicKey = new byte[(int) filePublicKey.length()];
        fis.read(encodedPublicKey);
        fis.close();

        // Generate Public Key.
        KeyFactory keyFactory = KeyFactory.getInstance(algorithm);
        X509EncodedKeySpec publicKeySpec = new X509EncodedKeySpec(encodedPublicKey);
        PublicKey publicKey = keyFactory.generatePublic(publicKeySpec);

        return publicKey;
    }

    private void savePrivateKey(String file, PrivateKey privateKey) throws IOException {
        // Store Private Key.
        PKCS8EncodedKeySpec pkcs8EncodedKeySpec = new PKCS8EncodedKeySpec(privateKey.getEncoded());
        FileOutputStream fos = new FileOutputStream(file);
        fos.write(pkcs8EncodedKeySpec.getEncoded());
        fos.close();
    }

    static PrivateKey loadPrivateKey(String file, String algorithm) throws IOException, NoSuchAlgorithmException, InvalidKeySpecException {
        // Read Private Key.
        File filePrivateKey = new File(file);
        FileInputStream fis = new FileInputStream(file);
        byte[] encodedPrivateKey = new byte[(int) filePrivateKey.length()];
        fis.read(encodedPrivateKey);
        fis.close();

        // Generate KeyPair.
        KeyFactory keyFactory = KeyFactory.getInstance(algorithm);
        PKCS8EncodedKeySpec privateKeySpec = new PKCS8EncodedKeySpec(encodedPrivateKey);
        PrivateKey privateKey = keyFactory.generatePrivate(privateKeySpec);

        return privateKey;
    }

    private X509Certificate generateCertificate(KeyPair keyPair, String dn) throws Exception {
        ContentSigner sigGen = new JcaContentSignerBuilder("SHA1withRSA").build(keyPair.getPrivate());

        Date startDate = new Date(System.currentTimeMillis() - 24 * 60 * 60 * 1000);
        Date endDate = new Date(System.currentTimeMillis() + 365 * 24 * 60 * 60 * 1000);

        X509v3CertificateBuilder v3CertGen =
                new JcaX509v3CertificateBuilder(new X500Name("CN=" + dn),
                        new BigInteger(64, new SecureRandom()),
                        startDate,
                        endDate,
                        new X500Name("CN=" + dn),
                        keyPair.getPublic());

        X509CertificateHolder certHolder = v3CertGen.build(sigGen);
        X509Certificate x509Certificate = new JcaX509CertificateConverter().getCertificate(certHolder);
        return x509Certificate;
    }

    private IdmClient createIdmClient(
            AccessToken accessToken,
            String domainControllerFQDN,
            int domainControllerPort)
            throws Exception {
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(this.keyStore);
        SSLContext sslContext = SSLContext.getInstance("SSL");
        sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
        IdmClient idmClient = new IdmClient(domainControllerFQDN, domainControllerPort, new DefaultHostnameVerifier(), sslContext);
        com.vmware.identity.rest.core.client.AccessToken restAccessToken =
                new com.vmware.identity.rest.core.client.AccessToken(accessToken.getValue(),
                        com.vmware.identity.rest.core.client.AccessToken.Type.JWT);
        idmClient.setToken(restAccessToken);
        return idmClient;
    }

    private VmdirClient createVMdirClient(
            AccessToken accessToken,
            String domainControllerFQDN,
            int domainControllerPort)
            throws Exception {
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(this.keyStore);
        SSLContext sslContext = SSLContext.getInstance("SSL");
        sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
        VmdirClient vmdirClient = new VmdirClient(domainControllerFQDN, domainControllerPort, new DefaultHostnameVerifier(), sslContext);
        com.vmware.identity.rest.core.client.AccessToken restAccessToken =
                new com.vmware.identity.rest.core.client.AccessToken(accessToken.getValue(),
                        com.vmware.identity.rest.core.client.AccessToken.Type.JWT);
        vmdirClient.setToken(restAccessToken);
        return vmdirClient;
    }

    private String convertToBase64PEMString(X509Certificate x509Certificate) throws Exception {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        byteArrayOutputStream.write("-----BEGIN CERTIFICATE-----".getBytes());
        byteArrayOutputStream.write("\n".getBytes());
        byteArrayOutputStream.write(Base64Utils.encodeToBytes(x509Certificate.getEncoded()));
        byteArrayOutputStream.write("-----END CERTIFICATE-----".getBytes());
        byteArrayOutputStream.write("\n".getBytes());
        return byteArrayOutputStream.toString();
    }
}
