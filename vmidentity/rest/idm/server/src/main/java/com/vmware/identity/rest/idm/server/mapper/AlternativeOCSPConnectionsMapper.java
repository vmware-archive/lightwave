package com.vmware.identity.rest.idm.server.mapper;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionsDTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPListDTO;

/**
 * Mapper utility to map objects from { Map<String, AlternativeOCSPList>} to {@link AlternativeOCSPConnectionsDTO} and vice-versa.
 * 
 * @author schai
 * 
 */
public class AlternativeOCSPConnectionsMapper {

    public static AlternativeOCSPConnectionsDTO getAlternativeOCSPConnectionsDTO(Map<String, AlternativeOCSPList> alternativeOCSPConnections) {

        Map<String, AlternativeOCSPListDTO> altOCSPListDTOMap = new HashMap<String, AlternativeOCSPListDTO>();

        Set<String> idSet = alternativeOCSPConnections.keySet();

        for (String siteId : idSet) {
            AlternativeOCSPList altOCSPList = alternativeOCSPConnections.get(siteId);
            AlternativeOCSPListDTO altOCSPListDTO = AlternativeOCSPListMapper.getAlternativeOCSPListDTO(altOCSPList);
            altOCSPListDTOMap.put(siteId, altOCSPListDTO);
        }

        return AlternativeOCSPConnectionsDTO.builder().withOCSPConnections(altOCSPListDTOMap)
                .build();
    }

   public static HashMap<String, AlternativeOCSPList> getAlternativeOCSPConnections(AlternativeOCSPConnectionsDTO alternativeOCSPConnectionsDTO) {

        HashMap<String, AlternativeOCSPList> altOCSPConnections = new HashMap<String, AlternativeOCSPList>();

        Map<String, AlternativeOCSPListDTO> altOCSPConnectionDTOList = alternativeOCSPConnectionsDTO.getAlternativeOCSPConnections();

        if(altOCSPConnectionDTOList != null) {
            Set<String> siteSet = altOCSPConnectionDTOList.keySet();
            for (String siteId : siteSet) {
                AlternativeOCSPListDTO altOCSPListDTO = altOCSPConnectionDTOList.get(siteId);
                AlternativeOCSPList alternativeOCSPList = AlternativeOCSPListMapper.getAlternativeOCSPList(altOCSPListDTO);

                altOCSPConnections.put(siteId, alternativeOCSPList);
            }
        }
        return altOCSPConnections;
    }

}
