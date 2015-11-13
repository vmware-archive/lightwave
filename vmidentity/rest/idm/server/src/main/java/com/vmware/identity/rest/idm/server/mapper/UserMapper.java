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

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.PasswordDetailsDTO;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;

/**
 * Person object mapper that maps from {@link PersonDTO.class} to {@link PersonUser.class} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public final class UserMapper {

    /**
     * Map object of type {@link PersonUser} to {@link UserDTO}
     *
     * @param personUser IDM Person User
     * @return REST based PersonDTO
     */
    public static UserDTO getUserDTO(PersonUser personUser, boolean withPasswordDetails) {
        PrincipalDTO alias = null;
        if (personUser.getAlias() != null) {
            alias = new PrincipalDTO(personUser.getAlias().getName(), personUser.getAlias().getDomain());
        }

        UserDetailsDTO personDetailDTO = UserDetailsMapper.getUserDetailsDTO(personUser.getDetail());

        PasswordDetailsDTO passwordDetailsDTO = null;
        if (withPasswordDetails) {
            passwordDetailsDTO = PasswordDetailsDTO.builder()
                        .withLastSet(personUser.getDetail().getPwdLastSet())
                        .withLifetime(personUser.getDetail().getPwdLifeTime())
                        .build();
        }

        return UserDTO.builder()
                    .withName(personUser.getId().getName())
                    .withDomain(personUser.getId().getDomain())
                    .withAlias(alias)
                    .withDetails(personDetailDTO)
                    .withDisabled(personUser.isDisabled())
                    .withLocked(personUser.isLocked())
                    .withObjectId(personUser.getObjectId())
                    .withPasswordDetails(passwordDetailsDTO)
                    .build();
    }

    /**
     * Map Set<{@link PersonUser}> to Set<{@link UserDTO}>
     * @param personUsers Set of IDM PersonUser
     * @return Set of REST based PersonDTO
     * @throws DTOMapperException
     */
    public static Set<UserDTO> getUserDTOs(Collection<PersonUser> personUsers, boolean withPasswordDetails) {
        Set<UserDTO> personDTOs = new HashSet<UserDTO>();
        for (PersonUser personUser : personUsers) {
            personDTOs.add(getUserDTO(personUser, withPasswordDetails));
        }
        return personDTOs;
    }

    /**
     * Map {@link UserDTO} to {@link PersonUser}
     *
     * @param REST based person user
     * @return
     *       Fully mapped IDM based {@link PersonUser}
     * @throws DTOMapperException
     */
    public static PersonUser getPersonUser(UserDTO personDTO) throws DTOMapperException {
       PrincipalId principalId = null;
       PrincipalId alias = null;
       PersonDetail personDetail = null;

       principalId = new PrincipalId(personDTO.getName(), personDTO.getDomain());

       try {
           if (personDTO.getAlias() != null) {
               alias = new PrincipalId(personDTO.getAlias().getName(), personDTO.getAlias().getDomain());
           }

           if (personDTO.getDetails() != null) {
               personDetail = new PersonDetail.Builder()
               .description(personDTO.getDetails().getDescription())
               .emailAddress(personDTO.getDetails().getEmail())
               .firstName(personDTO.getDetails().getFirstName())
               .lastName(personDTO.getDetails().getLastName())
               .userPrincipalName(personDTO.getDetails().getUPN())
               .build();
           } else {
               personDetail = new PersonDetail.Builder().build();
           }

           return new PersonUser(principalId, alias, null, personDetail,
                   personDTO.isDisabled() == null ? false : personDTO.isDisabled(),
                   personDTO.isLocked() == null ? false : personDTO.isLocked());
       } catch (Exception e) {
           throw new DTOMapperException("Unable to convert from UserDTO to PersonUser");
       }
    }
}
