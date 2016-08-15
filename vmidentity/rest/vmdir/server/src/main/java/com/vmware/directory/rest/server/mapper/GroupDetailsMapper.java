package com.vmware.directory.rest.server.mapper;


import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.identity.idm.GroupDetail;

/**
 * Object mapper mapping from {@link GroupDetailsDTO} to {@link GroupDetail} and vice-versa.
 * This mapper helps in transforming objects which can be serializer/de-serializer friendly.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class GroupDetailsMapper {

    /**
     * Map object from {@link GroupDetailsDTO} to {@link GroupDetail}
     */
    public static GroupDetail getGroupDetails(GroupDetailsDTO groupDetailsDTO) {
        return new GroupDetail(groupDetailsDTO.getDescription());
    }

    /**
     * Map object from {@link GroupDetail} to {@link GroupDetailsDTO}
     */
    public static GroupDetailsDTO getGroupDetailsDTO(GroupDetail groupDetail){
        return GroupDetailsDTO.builder()
                              .withDescription(groupDetail.getDescription())
                              .build();
    }

}