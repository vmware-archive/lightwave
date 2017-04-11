package com.vmware.identity.rest.idm.server.test.mapper;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.security.cert.CertificateException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionDTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionsDTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPListDTO;
import com.vmware.identity.rest.idm.server.mapper.AlternativeOCSPConnectionsMapper;
import com.vmware.identity.rest.idm.server.test.util.AlternativeOCSPConnectionsUtil;

/**
 * Unit tests for AlternativeOCSPConnectionsMapper
 *
 * @author schai
 *
 */
public class AlternativeOCSPConnectionsMapperTest {


    @Test
    public void testGetAlternativeOCSPConnectionsDTO() throws CertificateException, IOException {
        // Read test ocsp responder connections.
        HashMap<String, AlternativeOCSPList> ocspConnections = AlternativeOCSPConnectionsUtil.getTestOCSPConnections();

        // Map above to DTO object
        AlternativeOCSPConnectionsDTO altOCSPConnectionsDTO = AlternativeOCSPConnectionsMapper.getAlternativeOCSPConnectionsDTO(ocspConnections);

        // Validate DTO object
        Map<String, AlternativeOCSPListDTO> altOCSPListDTOMap = altOCSPConnectionsDTO.getAlternativeOCSPConnections();
        assertNotNull(altOCSPListDTOMap);
        assertEquals(1, altOCSPListDTOMap.size());

        AlternativeOCSPListDTO ocspListDTO = altOCSPListDTOMap.get(AlternativeOCSPConnectionsUtil.TEST_SITE_ID_1);
        assertNotNull(ocspListDTO);
        assertEquals(1, ocspListDTO.getAltOCSPConnectionList().size());

        AlternativeOCSPConnectionDTO altOCSPConnectionDTO = ocspListDTO.getAltOCSPConnectionList().get(0);
        assertNotNull(altOCSPConnectionDTO);
        assertEquals(altOCSPConnectionDTO.getOcspResponderUrl(), AlternativeOCSPConnectionsUtil.TEST_OCSP_SITE1_URL_1);
        assertEquals(altOCSPConnectionDTO.getSigningCert(), AlternativeOCSPConnectionsUtil.TEST_OCSP_CERT);
    }

    @Test
    public void testGetAlternativeOCSPListMap() throws CertificateException, IOException {
        // Read test ocsp responder connections.
        HashMap<String, AlternativeOCSPList> ocspConnections = AlternativeOCSPConnectionsUtil.getTestOCSPConnections();

        // Map above to DTO object
        AlternativeOCSPConnectionsDTO altOCSPConnectionsDTO = AlternativeOCSPConnectionsMapper.getAlternativeOCSPConnectionsDTO(ocspConnections);

        // Map back to ocsp responder connections.

        HashMap<String, AlternativeOCSPList> ocspConnectionsMappedFromDTO = AlternativeOCSPConnectionsMapper.getAlternativeOCSPConnections(altOCSPConnectionsDTO);

        // Validate ocsp responder connections mapped from DTO
        AlternativeOCSPList altOCSPList = ocspConnectionsMappedFromDTO.get(AlternativeOCSPConnectionsUtil.TEST_SITE_ID_1);
        assertNotNull(altOCSPList);
        assertEquals(AlternativeOCSPConnectionsUtil.TEST_SITE_ID_1, altOCSPList.get_siteID());

        List<AlternativeOCSP> ocspList = altOCSPList.get_ocspList();
        assertNotNull(ocspList);
        assertEquals(1, ocspList.size());

        AlternativeOCSP altOCSP = ocspList.get(0);
        assertNotNull(altOCSP);
        assertEquals(altOCSP.get_responderURL().toExternalForm(), AlternativeOCSPConnectionsUtil.TEST_OCSP_SITE1_URL_1);
        assertEquals(altOCSP.get_responderSigningCert(), AlternativeOCSPConnectionsUtil.TEST_OCSP_CERT);
    }

}