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
package com.vmware.identity.rest.idm.server.mapper;

import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;

/**
 * Object mapper mapping from {@link UserDetailsDTO} to {@link PersonDetail} and vice-versa.
 * This mapper helps in transforming objects which can be serializer/de-serializer friendly.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public final class UserDetailsMapper {

    /**
     * Map object from {@link UserDetailsDTO} to {@link PersonDetail}
     *
     * @param userDetails
     * @return
     * @throws DTOMapperException
     */
    public static PersonDetail getPersonDetail(UserDetailsDTO userDetails) throws DTOMapperException {
        PersonDetail personDetail;
        try {
           personDetail =  new PersonDetail.Builder()
                                  .firstName(userDetails.getFirstName())
                                  .lastName(userDetails.getLastName())
                                  .emailAddress(userDetails.getEmail())
                                  .description(userDetails.getDescription())
                                  .userPrincipalName(userDetails.getUPN())
                                  .build();
        } catch (Exception e) {
            throw new DTOMapperException("Unable to convert PersonDetailDTO to PersonDetail", e);
        }
        return personDetail;
    }


    /**
     * Map object from {@link PersonDetail} to {@link UserDetailsDTO}
     *
     * @param personDetail
     * @return
     * @throws DTOMapperException
     */
    public static UserDetailsDTO getUserDetailsDTO(PersonDetail personDetail) {
        return new UserDetailsDTO(personDetail.getEmailAddress(),
                                   personDetail.getUserPrincipalName(),
                                   personDetail.getFirstName(),
                                   personDetail.getLastName(),
                                   personDetail.getDescription());
    }
}
