/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.wstrust.test.util;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.PasswordDetailsDTO;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.vim.sso.PrincipalId;

import java.io.ByteArrayOutputStream;
import java.security.cert.Certificate;
import java.util.Arrays;
import java.util.List;
import org.apache.commons.codec.binary.Base64;

/**
 * Utility methods provided on top of {@link VmdirClient}
 * interface.
 */
public class PrincipalManagementServiceUtil {

  private final VmdirClient _pm;

  public PrincipalManagementServiceUtil(VmdirClient service) {
    _pm = service;
  }

  public void CreateSolutionUser(String username, String tenant, boolean multiTenant,
                                  Certificate cert) throws Exception {
    CertificateDTO certificateDTO = new CertificateDTO.Builder()
        .withEncoded(convertToBase64PEMString(cert))
        .build();

    SolutionUserDTO solutionUserDTO = new SolutionUserDTO.Builder()
        .withName(username)
        .withDomain(tenant)
        .withCertificate(certificateDTO)
        .withMultiTenant(multiTenant)
        .build();
    _pm.solutionUser().create(tenant, solutionUserDTO);

    // add the solution user to ActAs group and Users group
    List<String> members = Arrays.asList(username + "@" + tenant);
    _pm.group().addMembers(tenant, "ActAsUsers", tenant, members, MemberType.USER);
    _pm.group().addMembers(tenant, "Users", tenant, members, MemberType.USER);
  }

  public void CreatePersonUser(String tenant,
                               String username,
                               UserDetailsDTO userDetailsDTO,
                               String password) throws Exception {
    PasswordDetailsDTO passwordDTO = PasswordDetailsDTO.builder()
        .withPassword(password)
        .build();
    UserDTO userDTO = new UserDTO.Builder()
        .withName(username)
        .withDomain(tenant)
        .withDetails(userDetailsDTO)
        .withPasswordDetails(passwordDTO)
        .build();
    _pm.user().create(tenant, userDTO);
  }

  public void CreateGroup(String tenant, String name) throws Exception {
    GroupDTO groupDTO = new GroupDTO.Builder()
        .withName(name)
        .withDomain(tenant)
        .withDetails(new GroupDetailsDTO("description"))
        .build();
    _pm.group().create(tenant, groupDTO);
  }

  public void AddSolutionUserToGroup(PrincipalId principal, String group) throws Exception {
    List<String> members = Arrays.asList(principal.getName() + "@" + principal.getDomain());
    // TODO: Adding a Solution User to a Group is yet to be implemented
    _pm.group().addMembers(
                    principal.getDomain(),
                    group,
                    principal.getDomain(),
                    members,
                    MemberType.SOLUTIONUSER
                );
  }

  public void AddPersonUserToGroup(PrincipalId principal, String group) throws Exception {
    List<String> members = Arrays.asList(principal.getName() + "@" + principal.getDomain());
    // TODO: Adding a Solution User to a Group is yet to be implemented
    _pm.group().addMembers(
        principal.getDomain(),
        group,
        principal.getDomain(),
        members,
        MemberType.USER
    );
  }

  public void deleteSolutionUserIfExist(String tenant, String name) throws Exception {
    try {
      _pm.solutionUser().delete(tenant, name);
    } catch (NotFoundException e) {
    }
  }

  public void deletePersonUserIfExist(String tenant, String name, String domain) throws Exception {
      try {
        _pm.user().delete(tenant, name, domain);
      } catch (NotFoundException e) {

      }
  }

  public void deleteGroupIfExist(String tenant, String name, String domain) throws Exception {
      try {
        _pm.group().delete(tenant, name, domain);
      } catch (NotFoundException e) {

      }
  }

  private void validateIsLocal(PrincipalId principalId) {

    final String domain = principalId.getDomain();

    if (!StsUtil.SSO_SYSTEM_DOMAIN.equals(domain)) {
      throw new IllegalArgumentException("Local domain is required "
          + domain);
    }
  }

  private static String convertToBase64PEMString(Certificate x509Certificate) throws Exception {
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    byteArrayOutputStream.write("-----BEGIN CERTIFICATE-----".getBytes());
    byteArrayOutputStream.write("\n".getBytes());
    byteArrayOutputStream.write(Base64.encodeBase64(x509Certificate.getEncoded()));
    byteArrayOutputStream.write("-----END CERTIFICATE-----".getBytes());
    byteArrayOutputStream.write("\n".getBytes());
    return byteArrayOutputStream.toString();
  }
}
