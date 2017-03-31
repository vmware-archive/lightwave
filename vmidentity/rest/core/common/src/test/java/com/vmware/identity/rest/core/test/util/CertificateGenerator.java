/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.rest.core.test.util;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.security.InvalidKeyException;
import java.security.KeyPair;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.Security;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

import org.bouncycastle.asn1.ASN1EncodableVector;
import org.bouncycastle.asn1.ASN1Encoding;
import org.bouncycastle.asn1.ASN1Integer;
import org.bouncycastle.asn1.DERBitString;
import org.bouncycastle.asn1.DERNull;
import org.bouncycastle.asn1.DERSequence;
import org.bouncycastle.asn1.cryptopro.CryptoProObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.asn1.x509.AlgorithmIdentifier;
import org.bouncycastle.asn1.x509.SubjectPublicKeyInfo;
import org.bouncycastle.asn1.x509.TBSCertificate;
import org.bouncycastle.asn1.x509.Time;
import org.bouncycastle.asn1.x509.V1TBSCertificateGenerator;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

/**
 * Certificate generator class based off {@link org.bouncycastle.jce.provider.test.TestUtils}.
 *
 * Uses BouncyCastle to generate certificates for the purpose of testing.
 */
public class CertificateGenerator {

    public static enum AlgorithmName {
        GOST3411_WITH_GOST3410("GOST3411withGOST3410"),
        SHA1_WITH_RSA("SHA1withRSA"),
        SHA256_WITH_RSA("SHA256withRSA");

        private final String name;

        private AlgorithmName(String name) { this.name = name; }

        public String toString() { return this.name; };
    }

    private static Map<AlgorithmName, AlgorithmIdentifier> ALGORITHM_IDS = new HashMap<AlgorithmName, AlgorithmIdentifier>();

    static
    {
        ALGORITHM_IDS.put(AlgorithmName.GOST3411_WITH_GOST3410, new AlgorithmIdentifier(CryptoProObjectIdentifiers.gostR3411_94_with_gostR3410_94));
        ALGORITHM_IDS.put(AlgorithmName.SHA1_WITH_RSA, new AlgorithmIdentifier(PKCSObjectIdentifiers.sha1WithRSAEncryption, DERNull.INSTANCE));
        ALGORITHM_IDS.put(AlgorithmName.SHA256_WITH_RSA, new AlgorithmIdentifier(PKCSObjectIdentifiers.sha256WithRSAEncryption, DERNull.INSTANCE));
    }

    /**
     * Generate a self-signed X.509 certificate
     *
     * @param pair the key pair to use when signing the certificate
     * @param algorithm the signing algorithm to use
     * @param dn the X.509 distinguished name for the certificate
     * @return a self-signed X.509 certificate
     * @throws NoSuchAlgorithmException
     * @throws NoSuchProviderException
     * @throws InvalidKeyException
     * @throws SignatureException
     * @throws IOException
     * @throws CertificateException
     */
    public static X509Certificate generateSelfSignedCertificate(KeyPair pair, AlgorithmName algorithm, String dn)
            throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, SignatureException, IOException, CertificateException
    {
        if (Security.getProvider("BC") == null) {
            Security.addProvider(new BouncyCastleProvider());
        }

        AtomicLong serialNumber = new AtomicLong(System.currentTimeMillis());
        X500Name owner = new X500Name(dn);

        V1TBSCertificateGenerator generator = new V1TBSCertificateGenerator();
        long time = System.currentTimeMillis();

        generator.setSerialNumber(new ASN1Integer(serialNumber.getAndIncrement()));
        generator.setIssuer(owner);
        generator.setSubject(owner);
        generator.setStartDate(new Time(new Date(time - 5000)));
        generator.setEndDate(new Time(new Date(time + 30 * 60 * 1000)));
        generator.setSignature(ALGORITHM_IDS.get(algorithm));
        generator.setSubjectPublicKeyInfo(SubjectPublicKeyInfo.getInstance(pair.getPublic().getEncoded()));

        Signature sig = Signature.getInstance(algorithm.toString(), "BC");

        sig.initSign(pair.getPrivate());

        sig.update(generator.generateTBSCertificate().getEncoded(ASN1Encoding.DER));

        TBSCertificate tbsCert = generator.generateTBSCertificate();

        ASN1EncodableVector v = new ASN1EncodableVector();

        v.add(tbsCert);
        v.add(ALGORITHM_IDS.get(algorithm));
        v.add(new DERBitString(sig.sign()));

        return (X509Certificate) CertificateFactory.getInstance("X.509", "BC")
                .generateCertificate(new ByteArrayInputStream(
                        new DERSequence(v).getEncoded(ASN1Encoding.DER)));
    }

}
