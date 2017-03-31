package com.vmware.identity.rest.idm.server.mapper;

import java.net.MalformedURLException;
import java.net.URL;
import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionDTO;


/**
 * Mapper class for AlternativeOCSPConnectionDTO
 *
 * @author schai
 *
 */
public class AlternativeOCSPConnectionMapper {

    public static AlternativeOCSPConnectionDTO getAlternativeOCSPConnectionDTO(AlternativeOCSP alternativeOCSP) {
        String ocspUrlStr = null;

        if (alternativeOCSP.get_responderURL() != null) {
            ocspUrlStr = alternativeOCSP.get_responderURL().toExternalForm();
        }

        return AlternativeOCSPConnectionDTO.builder().withURL(ocspUrlStr)
                .withSigningCert(alternativeOCSP.get_responderSigningCert())
                .build();
    }

    public static AlternativeOCSP getAlternativeOCSPConnection(AlternativeOCSPConnectionDTO alternativeOCSPConnectionDTO) {
        URL url;
        try {
            url = new URL(alternativeOCSPConnectionDTO.getOcspResponderUrl());

        } catch (MalformedURLException e) {
            throw new DTOMapperException("Failed to convert AlternativeOCSPConnectionDTO to AlternativeOCSP", e);
        }

        return new AlternativeOCSP(url, alternativeOCSPConnectionDTO.getSigningCert());
    }

}