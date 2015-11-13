/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.util.Set;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoDomainSearchPermissionException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * This interface provides methods for group membership check.
 * <p>
 * Case sensitivity of user name to search for depends on the domain. If a
 * domain distinguishes users and groups with names in different case then
 * searches are case sensitive. Otherwise they are case insensitive.
 */
public interface GroupChecker {

   /**
    * Check whether the given user ( person or solution ) is a direct or
    * indirect ( through group to group link ) member of the specified group.
    * Both the user and group should exist.
    *
    * @param userId
    *           user id. cannot be {@code null}
    * @param groupId
    *           group id. cannot be {@code null}
    * @return <code>true</code> when the user is member of the group, or
    *         <code>false</code> otherwise
    *
    * @throws InvalidPrincipalException
    *            when any of the specified user or group IDs does not exist or
    *            does not represent respectively user and group type of object
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws NoDomainSearchPermissionException
    *            when the system does not have search rights in the domain
    */
   @Privilege(Role.RegularUser)
   boolean isMemberOfGroup(PrincipalId userId, PrincipalId groupId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException, NoDomainSearchPermissionException;

   /**
    * Find group IDs, that are subset of specified IDs, for which the specified
    * user is a direct or indirect member ( through group to group link ).
    *
    * @param userId
    *           id of the person or solution user which parent group to return.
    *           cannot be {@code null}
    * @param groupList
    *           the superset of parent group IDs which to return; {@code null}
    *           value to get all parent groups {@link #findAllParentGroups}; IDs
    *           that does not represent existing group will be ignored
    *
    * @return parent group IDs that are subset of the specified IDs or empty set
    *         when none of the given groups in list is a parent group
    *
    * @throws InvalidPrincipalException
    *            when the specified user does not exist or does not represent (
    *            person or solution ) user type of object
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws NoDomainSearchPermissionException
    *            when the system does not have search rights in the domain
    */
   @Privilege(Role.RegularUser)
   Set<PrincipalId> findParentGroups(PrincipalId userId,
      Set<PrincipalId> groupList) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException, NoDomainSearchPermissionException;

   /**
    * Find group IDs for which the specified user is a direct or indirect member
    * (through group to group link).
    *
    * @param userId
    *           id of the person or solution user which parent groups to return.
    *           cannot be {@code null}
    * @return parent group IDs or empty set when given user is not member of any
    *         group.
    *
    * @throws InvalidPrincipalException
    *            when the specified user does not exist or does not represent (
    *            person or solution ) user type of object
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws NoDomainSearchPermissionException
    *            when the system does not have search rights in the domain
    */
   @Privilege(Role.RegularUser)
   Set<PrincipalId> findAllParentGroups(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException, NoDomainSearchPermissionException;
}
