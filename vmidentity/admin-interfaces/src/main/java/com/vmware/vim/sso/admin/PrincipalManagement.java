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
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.NoPrivilege;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.DuplicateSolutionCertificateException;
import com.vmware.vim.sso.admin.exception.GroupCyclicDependencyException;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;
import com.vmware.vim.sso.admin.exception.PasswordPolicyViolationException;

/**
 * Provides write operations on system domain principals (groups, person and
 * solution users).
 */
public interface PrincipalManagement {

   /**
    * Create new person user account at the system domain.
    * <p>
    * All the following symbols
    * <ul>
    * <li>^ (caret)</li>
    * <li>&lt; (less than)</li>
    * <li>&gt; (greater than)</li>
    * <li>& (and)</li>
    * <li>% (percent)</li>
    * <li>` (back quote)</li>
    * </ul>
    * must not be part of {@code userName} argument or part of {@code firstName}, {@code lastName} or {@code description} fields at {@code userDetails}
    * argument.
    *
    * <p>Additionally the following symbols
    * <ul>
    * <li>@ (at sign, special separator for account name UPN style)</li>
    * <li>\ (back slash, special separator for account name NetBIOS style)</li>
    * </ul>
    * must not be part of {@code userName} argument.
    * @param userName
    *           name of the person user; requires {@code non-null} and not empty
    *           string value
    * @param userDetails
    *           details of the person user; requires {@code non-null} value
    * @param password
    *           password of the person user; requires {@code non-null} and
    *           non-empty array value
    *
    * @return id of the created person user; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is already a system domain user with the specified
    *            name
    * @throws PasswordPolicyViolationException
    *            when the password provided doesn't satisfy the
    *            {@link PasswordPolicy}
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   PrincipalId createLocalPersonUser(String userName,
      PersonDetails userDetails, char[] password)
      throws InvalidPrincipalException, PasswordPolicyViolationException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Create new internal solution user account at the system domain.
    * <p>
    * All the following symbols
    * <ul>
    * <li>^ (caret)</li>
    * <li>&lt; (less than)</li>
    * <li>&gt; (greater than)</li>
    * <li>& (and)</li>
    * <li>% (percent)</li>
    * <li>` (back quote)</li>
    * </ul>
    * must not be part of {@code userName} argument or part of {@code firstName}, {@code lastName} or {@code description} fields at {@code userDetails}
    * argument.
    *
    * <p>Additionally the following symbols
    * <ul>
    * <li>@ (at sign, special separator for account name UPN style)</li>
    * <li>\ (back slash, special separator for account name NetBIOS style)</li>
    * </ul>
    * must not be part of {@code userName} argument.
    * <p>
    * Solution users must have unique identity which means that their
    * certificates must have unique <i>distinguished name</i> ( DN ) and unique
    * <i>public key</i>. Certificates at {@code userDetails} parameter must be
    * generated with a different public/private keypair.
    *
    * @param userName
    *           name of the new solution user; requires {@code non-null} and not
    *           empty string value
    * @param userDetails
    *           details of the new solution user; requires {@code non-null}
    *           value; valid format string for email address should be specified
    *
    * @return id of the created solution user; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is already a principal with the specified name
    * @throws DuplicateSolutionCertificateException
    *            when the provided X.509 certificate's DN is not unique
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   PrincipalId createLocalSolutionUser(String userName,
      SolutionDetails userDetails) throws InvalidPrincipalException,
      DuplicateSolutionCertificateException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Create new solution user account at the system domain.
    * <p>
    * All the following symbols
    * <ul>
    * <li>^ (caret)</li>
    * <li>&lt; (less than)</li>
    * <li>&gt; (greater than)</li>
    * <li>& (and)</li>
    * <li>% (percent)</li>
    * <li>` (back quote)</li>
    * </ul>
    * must not be part of {@code userName} argument or part of {@code firstName}, {@code lastName} or {@code description} fields at {@code userDetails}
    * argument.
    *
    * <p>Additionally the following symbols
    * <ul>
    * <li>@ (at sign, special separator for account name UPN style)</li>
    * <li>\ (back slash, special separator for account name NetBIOS style)</li>
    * </ul>
    * must not be part of {@code userName} argument.
    * <p>
    * Solution users must have unique identity which means that their
    * certificates must have unique <i>distinguished name</i> ( DN ) and unique
    * <i>public key</i>. Certificates at {@code userDetails} parameter must be
    * generated with a different public/private keypair.
    *
    * @param userName
    *           name of the new solution user; requires {@code non-null} and not
    *           empty string value
    * @param userDetails
    *           details of the new solution user; requires {@code non-null}
    *           value; valid format string for email address should be specified
    * @param external
    *           specify whether user is external or internal
    *
    * @return id of the created solution user; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is already a principal with the specified name
    * @throws DuplicateSolutionCertificateException
    *            when the provided X.509 certificate's DN is not unique
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   PrincipalId createLocalSolutionUser(String userName,
      SolutionDetails userDetails, boolean external)
      throws InvalidPrincipalException, DuplicateSolutionCertificateException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Create new group at the system domain.
    *
    * @param groupName
    *           name of the group to create; requires {@code non-null} and
    *           not-empty string value
    * @param groupDetails
    *           details of the group; requires {@code non-null} value
    *
    * @return the id of the created group; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is already a system domain group with the specified
    *            name</li>
    *            <li>when there is already a system domain group with the specified
    *            name, or it is reserved by the SSO system</li>
    *           </ul>
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   PrincipalId createLocalGroup(String groupName, GroupDetails groupDetails)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Delete existing system domain principal account.
    *
    * @param principalName
    *           name of the principal to delete; requires {@code non-null} and
    *           not empty string value
    *
    * @throws InvalidPrincipalException
    *            when there is no system domain principal with the specified
    *            name
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void deleteLocalPrincipal(String principalName)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Remove existing principal from the specified system domain group.
    *
    * @param principalId
    *           id of the principal to remove from the given group; requires
    *           {@code non-null} value
    * @param groupName
    *           name of the group from which to remove given principal; requires
    *           {@code non-null} and non-empty string value value
    * @return the previous state of the principal regarding its membership with
    *         the group which is:
    *         <ul>
    *         <li>{@code false} - when nothing has changed because the principal
    *         has not been a member of the group</li>
    *         <li>
    *         {@code true} - when the principal was a member but has been
    *         successfully detached from the group</li>
    *         </ul>
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when any of the specified principal or system domain group does
    *            not exist </li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   boolean removeFromLocalGroup(PrincipalId principalId, String groupName)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Remove a set of existing principals from the specified system domain
    * group.
    *
    * @param principalIds
    *           id of the principals to remove; requires {@code non-null} value
    * @param groupName
    *           name of the group from which to remove given principals;
    *           requires {@code non-null} and not empty string value
    * @return {@code not-null} value; the previous state of the principal
    *         regarding its membership with the group which is:
    *         <ul>
    *         <li>{@code false} - when nothing has changed because the principal
    *         has not been a member of the group</li>
    *         <li>
    *         {@code true} - when the principal was a member but has been
    *         successfully detached from the group</li>
    *         </ul>
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when any of the specified principal or system domain group does
    *            not exist </li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   boolean[] removeFromLocalGroup(PrincipalId[] principalIds, String groupName)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Add existing person or solution user to the specified system domain group.
    *
    * @param userId
    *           id of the user to add; requires {@code non-null} value
    * @param groupName
    *           id of the group where to add the specified user; requires
    *           {@code non-null} and not-empty string value
    * @return previous state of the user regarding its membership with the group
    *         which is:
    *         <ul>
    *         <li>{@code false} - when nothing has changed because the user has
    *         already been a member of the group, which includes the system
    *         implicit Everyone group</li>
    *         <li>{@code true} - when the user was not a member but has been
    *         successfully attached to the group</li>
    *         </ul>
    *
    * @throws InvalidPrincipalException
    *            when any of the specified principal or system domain group does
    *            not exist
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   boolean addUserToLocalGroup(PrincipalId userId, String groupName)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Add a set of existing person and/or solution users to the specified system
    * domain group
    *
    * @param userIds
    *           IDs of the users which to add; requires {@code non-null} and
    *           not-empty array value
    * @param groupName
    *           id of the group where to add the specified user; requires
    *           {@code non-null} and not empty string value
    * @return {@code not-null} value; the array having equal size and elements
    *         corresponding to the user IDs parameter; each element contains the
    *         previous state of the user regarding its membership with the group
    *         which is:
    *         <ul>
    *         <li>{@code false} - when nothing has changed because the user has
    *         already been a member of the group, which includes the system
    *         implicit Everyone group</li>
    *         <li>{@code true} - when the user was not a member but has been
    *         successfully attached to the group</li>
    *         </ul>
    *
    * @throws InvalidPrincipalException
    *            when any of the specified principal or system domain group does
    *            not exist
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   boolean[] addUsersToLocalGroup(PrincipalId[] userIds, String groupName)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Add existing group to the specified system domain group.
    *
    * @param groupId
    *           id of the 'child' group which to add; requires {@code non-null}
    *           value
    * @param groupName
    *           name of the system domain 'parent' group; requires
    *           {@code non-null} and not-empty string value
    * @return the previous state of the group as a sub-group of the group which
    *         is:
    *         <ul>
    *         <li>{@code false} - when nothing has changed because the group has
    *         already been a sub-group of the group, which includes the system
    *         implicit Everyone group</li>
    *         <li>{@code true} - when the group was not a member but has been
    *         successfully attached to the group</li>
    *         </ul>
    *
    * @throws InvalidPrincipalException
    *            when any of the specified groups do not exist
    * @throws GroupCyclicDependencyException
    *            when any of the groups to add is direct parent ( or parent of
    *            any parent ) group of the specified system domain group
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   boolean addGroupToLocalGroup(PrincipalId groupId, String groupName)
      throws InvalidPrincipalException, GroupCyclicDependencyException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Add a set of existing group to the specified system domain group.
    *
    * @param groupIds
    *           IDs of the 'children' groups to add; requires {@code non-null}
    *           value
    * @param groupName
    *           name of the system domain 'parent' group; requires
    *           {@code non-null} and not-empty string value
    * @return {@code not-null} value; an array having equal size and elements
    *         corresponding to the group IDs parameter; each element contains
    *         the previous state of the group as a sub-group of the group which
    *         is:
    *         <ul>
    *         <li>{@code false} - when nothing has changed because the group has
    *         already been a sub-group of the group, which includes the system
    *         implicit Everyone group</li>
    *         <li>{@code true} - when the group was not a member but has been
    *         successfully attached to the group</li>
    *         </ul>
    *
    * @throws InvalidPrincipalException
    *            when any of the specified groups do not exist
    * @throws GroupCyclicDependencyException
    *            when any of the groups to add is direct parent ( or parent of
    *            any parent ) group of the specified system domain group
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   boolean[] addGroupsToLocalGroup(PrincipalId[] groupIds, String groupName)
      throws InvalidPrincipalException, GroupCyclicDependencyException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Updates the details of the system domain person user with given name. This
    * method updates all the values specified at details argument.
    *
    * @param userName
    *           name of the person user which details to update; requires
    *           {@code non-null} and not-empty string value
    * @param userDetails
    *           the new details to update; requires {@code non-null} value
    *
    * @return id of the affected person user; {@code non-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is no system domain person user with the specified
    *            name
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   PrincipalId updateLocalPersonUserDetails(String userName,
      PersonDetails userDetails) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Updates the details of system domain solution user. This method updates
    * all the values specified with the solution user details.
    *
    * @param userName
    *           name of the solution user which details to update; requires
    *           {@code non-null} and not-empty string value
    * @param userDetails
    *           the new details to update; requires {@code non-null} value;
    *           valid format string for email address should be specified
    *
    * @return id of the affected solution user; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is no system domain solution user with the specified
    *            name
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   PrincipalId updateLocalSolutionUserDetails(String userName,
      SolutionDetails userDetails) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Update the details of system domain group. This method updates all the
    * values specified with the group details.
    *
    * @param groupName
    *           name of the group which details to update; requires
    *           {@code not-null} and not-empty string value
    * @param groupDetails
    *           the new details to update; requires {@code not-null} value
    *
    * @return id of the affected group; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is no system domain solution user with the specified
    *            name
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no system domain group with the specified name</li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.Administrator)
   PrincipalId updateLocalGroupDetails(String groupName,
      GroupDetails groupDetails) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Resets the password of the system domain person user with given name. The
    * previous password will be overridden with the given one.
    *
    * @param userName
    *           name of the person user which password to update; requires
    *           {@code non-null} and not-empty string value
    * @param newPassword
    *           the new password to apply; requires {@code non-null} value
    *
    * @throws InvalidPrincipalException
    *            when there is no system doamin person user with the specified
    *            name
    * @throws PasswordPolicyViolationException
    *            when the password provided doesn't satisfy the
    *            {@link PasswordPolicy}
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void resetLocalPersonUserPassword(String userName, char[] newPassword)
      throws InvalidPrincipalException, PasswordPolicyViolationException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Resets the password of the system domain person user. The person user is
    * required to authorize by current password.
    *
    * @param userName
    *           name of the person user whose password will be reset; requires
    *           {@code non-null} and not-empty string value
    * @param currentPassword
    *           current password of the user; requires {@code non-null} value
    * @param newPassword
    *           the new password to apply; requires {@code non-null} value
    *
    * @throws InvalidPrincipalException
    *            when either no system domain person user with the specified
    *            name exists, or the given password is not correct
    * @throws PasswordPolicyViolationException
    *            when the password provided doesn't satisfy the
    *            {@link PasswordPolicy}
    */
   @NoPrivilege
   void resetLocalPersonUserPassword(String userName, char[] currentPassword,
      char[] newPassword) throws InvalidPrincipalException,
      PasswordPolicyViolationException;

   /**
    * Unlocks an existing user account. The result depends on whether the
    * account was locked at the time of the call.
    *
    * @param userId
    *           id of the user account to unlock; requires {@code non-null}
    *           value
    *
    * @return {@code true}, if the account was unlocked, false - when unlock was
    *         not needed because the account was not locked
    *
    * @throws InvalidPrincipalException
    *            when there is no person or solution user with the given id
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean unlockUserAccount(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Enables an existing user account. The return value indicates whether there
    * were any changes on 'enabled/disabled' state that has happened at this
    * method invocation.
    *
    * @param userId
    *           id of the user account to enable; requires {@code non-null}
    *           value
    * @return {@code false}, when nothing has changed, {@code true} - when the
    *         state has changed from 'disabled' to 'enabled'
    *
    * @throws InvalidPrincipalException
    *            when there is no person or solution user with the given id
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean enableUserAccount(final PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Disables an existing user account. The return value indicates whether
    * there were any changes on 'enabled/disabled' state that has happened at
    * this method invocation.
    *
    * @param userId
    *           id of the user account to disable; requires {@code non-null}
    *           value
    * @return {@code false}, when nothing has changed, {@code true} - when the
    *         state has changed from 'enabled' to 'disabled'
    *
    * @throws InvalidPrincipalException
    *            when there is no person or solution user with the given id
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean disableUserAccount(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Retrieve the number of days left until password expiration for the
    * specified person user account.
    * <p>
    * The returned value is:
    * <ul>
    * <li>a positive integer value - for number of days (rounded up) after which
    * password will expire</li>
    * <li>zero value - when password has already expired</li>
    * <li>-1 (minus one) value - when password policy is disabled or set to
    * never expire</li>
    * </ul>
    *
    * @param userId
    *           id of the person user which days until password expiration to
    *           get; requires {@code non-null} value
    * @return number of days until password expiration; returned value is
    *         greater or equal to -1
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user is not a person user
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   int getDaysRemainingUntilPasswordExpiration(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Register external IDP user in the system provider.
    *
    * @param principalId
    *           Id representing externalIDP user on this SSO system, cannot be
    *           null;
    * @return true if user user's registration is successfully created. false if
    *         user was already registered previously
    * @throws NotAuthenticatedException
    * @throws NoPermissionException
    */
   @Privilege(Role.Administrator)
   boolean registerExternalUser(PrincipalId principalId)
         throws NotAuthenticatedException, NoPermissionException;

   /**
    * Remove the registration of external IDP user from the system provider.
    *
    * @param principalId
    *           id of the globally unique external user; cannot be null;
    *
    * @return true if the principal ID was registered and its registration is
    *         removed successfully.
    * @throws NotAuthenticatedException
    * @throws NoPermissionException
    * @throws InvalidPrincipalException
    *            when the userId is not registered in the system provider.
    */
   @Privilege(Role.Administrator)
   boolean removeExternalUser(PrincipalId principalId)
         throws NotAuthenticatedException, NoPermissionException,
         InvalidPrincipalException;
}
