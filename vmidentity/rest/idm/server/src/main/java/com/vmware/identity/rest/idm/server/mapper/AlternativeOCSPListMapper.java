package com.vmware.identity.rest.idm.server.mapper;

import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionDTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionsDTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPListDTO;

/**
 * Mapper utility to map objects from { @AlternativeOCSPList} to {@link AlternativeOCSPListDTO} and vice-versa.
 *
 * @author schai
 * 
 */
public class AlternativeOCSPListMapper {
    public static AlternativeOCSPListDTO getAlternativeOCSPListDTO(AlternativeOCSPList alternativeOCSP) {

        List<AlternativeOCSPConnectionDTO> altOCSPConnectionDTOList = new ArrayList<AlternativeOCSPConnectionDTO>();

        List<AlternativeOCSP> altOCSPList = alternativeOCSP.get_ocspList();

        for (AlternativeOCSP ocsp : altOCSPList) {
            altOCSPConnectionDTOList.add(AlternativeOCSPConnectionMapper.getAlternativeOCSPConnectionDTO(ocsp));
        }
        return AlternativeOCSPListDTO.builder().withSiteId(alternativeOCSP.get_siteID())
                .withOCSPConnectionList(altOCSPConnectionDTOList)
                .build();
    }

    public static AlternativeOCSPList getAlternativeOCSPList(AlternativeOCSPListDTO alternativeOCSPListDTO) {
        List<AlternativeOCSPConnectionDTO> altOCSPConnectionDTOList = alternativeOCSPListDTO.getAltOCSPConnectionList();

        List<AlternativeOCSP> altOCSPList = new ArrayList<AlternativeOCSP>();
        for (AlternativeOCSPConnectionDTO ocspConnectionDTO : altOCSPConnectionDTOList) {
            altOCSPList.add(AlternativeOCSPConnectionMapper.getAlternativeOCSPConnection(ocspConnectionDTO));
        }

        return new AlternativeOCSPList(alternativeOCSPListDTO.getSiteId(), altOCSPList);
    }

}
