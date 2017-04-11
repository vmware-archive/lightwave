package com.vmware.identity.installer;

import java.io.ByteArrayInputStream;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.idm.CertificateUtil;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.LdapValue;

public class CertificatesUtil {

    private static final String USER_CERTIFICATE = "userCertificate";

    public static X509Certificate getCertificateValue(LdapValue[] value) {
        X509Certificate cert = null;

        if (value != null && value.length == 1 && value[0] != null) {
            cert = getCert(value[0]);
        }
        return cert;
    }

    private static X509Certificate getCert(LdapValue value) {
        X509Certificate cert = null;

        if (value != null) {
            byte[] certBytes = value.getValue();

            if (certBytes != null) {
                try {
                    ByteArrayInputStream inpstream = new ByteArrayInputStream(
                            certBytes);
                    CertificateFactory cf = CertificateFactory
                            .getInstance("X.509");
                    cert = (X509Certificate) cf.generateCertificate(inpstream);
                } catch (CertificateException e) {
                    throw new RuntimeException("Failed to generate certificate");
                }
            }
        }

        return cert;
    }

    public static ArrayList<X509Certificate> getCertificateValues(LdapValue[] value)
    {
        ArrayList<X509Certificate> certs = new ArrayList<X509Certificate>();

        if ( value != null  && value.length >= 1)
        {
            for (int i = 0; i < value.length; i++)
            {
                X509Certificate cert = getCert(value[i]);

                if (cert!=null)
                {
                    certs.add(cert);
                }
            }
        }
        return certs;
    }

    public static ArrayList<String> getCertFingerPrints(ILdapEntry entry)
            throws CertificateEncodingException, NoSuchAlgorithmException {

        ArrayList<String> certFingerprints = new ArrayList<>();

        LdapValue[] value = entry.getAttributeValues(USER_CERTIFICATE);
        List<X509Certificate> certs = CertificatesUtil
                .getCertificateValues(value);

        certFingerprints.addAll(createFingerPrints(certs));

        return certFingerprints;
    }

    private static ArrayList<String> createFingerPrints(List<X509Certificate> certs)
            throws CertificateEncodingException, NoSuchAlgorithmException {

        ArrayList<String> certFingerprints = new ArrayList<>();

        if (certs != null && !certs.isEmpty()) {

            for (X509Certificate each : certs) {
                certFingerprints.add(CertificateUtil.generateFingerprint(each));

            }
        }
        return certFingerprints;
    }

}
