package com.vmware.identity.rest.idm.server.test.util;

import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.URL;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;

/**
 * Helper class to populate testing objects
 * 
 * @author schai
 * 
 */
public class AlternativeOCSPConnectionsUtil {

    static public String TEST_SITE_ID_1 = "psc1";
    static public String TEST_OCSP_SITE1_URL_1 = "http://test_ocsp_1/responder1";
    static public String TEST_OCSP_SITE1_URL_2 = "http://test_ocsp_1/responder2";

    static public String TEST_SITE_ID_2 = "psc2";
    static public String TEST_OCSP_SITE2_URL_1 = "http://test_ocsp_2/responder1";
    static public String TEST_OCSP_SITE2_URL_2 = "http://test_ocsp_2/responder1";

    static public X509Certificate TEST_OCSP_CERT;

    static {
        try {
            TEST_OCSP_CERT = CertificateUtil.getTestCertificate();
        } catch (Exception e) {
            fail("Not able to read test certificate resource." + e.getMessage());
        }
    }

    /**
     * @return HashMap of one site and one alternative OCSP connection.
     * @throws CertificateException
     * @throws IOException
     */
    public static HashMap<String, AlternativeOCSPList> getTestOCSPConnections() throws CertificateException, IOException {

        HashMap<String, AlternativeOCSPList> ocspConnections = new HashMap<String, AlternativeOCSPList>();

        X509Certificate cert = CertificateUtil.getTestCertificate();

        List<AlternativeOCSP> altOCSPs = new ArrayList<AlternativeOCSP>();
        altOCSPs.add(new AlternativeOCSP(new URL(TEST_OCSP_SITE1_URL_1), cert));

        AlternativeOCSPList altOCSPList = new AlternativeOCSPList(TEST_SITE_ID_1, altOCSPs);

        ocspConnections.put(TEST_SITE_ID_1, altOCSPList);

        return ocspConnections;
    }

}
