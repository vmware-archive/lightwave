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

import java.util.Set;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.NoPrivilege;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;
import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Provides read operations for principals (principals means groups and users;
 * users types are person users and solution users). All operations are
 * applicable both on LDAP and SSO local principals.
 */
public interface PrincipalDiscovery {

   // For bulk queries find() API, VC default size limit is 5000, NGC default
   // size limit is 200. So we support the default VC limit here.
   static final int PD_BULK_QUERY_MAX_SIZE = 5000;
   /**
    * The result from searching for person users, solution users and groups
    * using {@link SearchCriteria}
    *
    * @see #find(SearchCriteria, int)
    */
   public interface SearchResult {

      /**
       * Retrieve result of matching solution users
       *
       * @return the solution users found; {@code not-null} value
       * @see #find(SearchCriteria, int)
       */
      Set<SolutionUser> getSolutionUsers();

      /**
       * Retrieve result of matching person users
       *
       * @return person users found; {@code not-null} value
       * @see #find(SearchCriteria, int)
       */
      Set<PersonUser> getPersonUsers();

      /**
       * Retrieve result of matching groups,
       *
       * @return the groups found; {@code not-null} value
       * @see #find(SearchCriteria, int)
       */
      Set<Group> getGroups();
   }

   /**
    * Principals search constraints definition; All string values inside search
    * criteria are case-insensitive; {@code immutable} class
    *
    * @see #find(SearchCriteria, int)
    */
   public final class SearchCriteria {

      /** Search string */
      private final String _searchString;

      /** Domain name ( e.g. VMWARE.COM ) */
      private final String _domain;

      /**
       * Create search criteria
       *
       * @param searchString
       *           refer to {@link #getSearchString()}. requires
       *           {@code not-null} value
       * @param domain
       *           domain name as described at {@link PrincipalId#getDomain()};
       *           {@code not-null} and not-empty string value is required
       */
      public SearchCriteria(String searchString, String domain) {

         ValidateUtil.validateNotNull(searchString, "searchString");
         ValidateUtil.validateNotEmpty(domain, "domain");

         _searchString = searchString;
         _domain = domain;
      }

      /**
       * Retrieve the search string.
       * <p>
       * Search string used to find principals ( users and groups ). Matching
       * principals will be those that have this particular string as a
       * substring either at {@code name} property of their principal ID, or at:
       * <ul>
       * <li>{@code firstName} or {@code lastName} property - for
       * {@link PersonUser}</li>
       * <li>{@code description} property - for {@link Group}</li>
       * </ul>
       *
       * @return the search string; empty string means no restriction;
       *         {@code not-null} value
       *
       * @see PrincipalId#getName() name property
       * @see PersonDetails#getFirstName() firstName property
       * @see PersonDetails#getLastName() lastName property
       * @see GroupDetails#getDescription() description property
       */
      public String getSearchString() {
         return _searchString;
      }

      /**
       * Retrieve the domain name where to search
       *
       * @return the domain name; {@code not-null} and not-empty string value
       */
      public String getDomain() {
         return _domain;
      }

      @Override
      public String toString() {
         return String.format("searchString=%s, domain=%s", getSearchString(),
            getDomain());
      }
   }

   /**
    * Lookup for user or group
    *
    * Normalized principal ID is a domain name based principalID (as opposed to
    * alias name based one) with principal and domain names in their canonical
    * form. Principal name canonical form is the exact name which has been
    * provided on user/group creation in the respective domain. Domain name
    * canonical form is the exact name which has been provided on domain
    * registration in SSO.
    * <p>
    * Search is case insensitive with respect to domain name and domain-specific
    * sensitive with respect to user/group name. Alternative domain names cannot
    * be used.
    * <p>
    * Note that this method will succeed even for domains in which the SSO
    * Server does not have search rights.
    *
    * @param id
    *           id of user or group, required
    * @param isGroup
    *           whether this is a group
    * @return normalized ID of the user or group if found, otherwise
    *         {@code null}
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   PrincipalId lookup(PrincipalId id, boolean isGroup)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Search for a user based on account name.
    * <ul>
    * The following formats are supported:
    * <li><b>&lt;user_name&gt;</b> - short form; search at default domain if
    * such is configured</li>
    * <li><b>&lt;user_name&gt;@[domain | alias]</b> - UPN-style</li>
    * <li><b>&lt;[domain | alias]&gt;\&lt;user_name&gt;</b> - NetBIOS-style</li>
    * </ul>
    * Domain and alias are case-insensitive; user_name depends on the identity
    * source implementation.
    *
    * @param userName
    *           user name at the specified format; {@code not-null} and
    *           {@code not-empty} string value is required
    * @return search result with users only; {@code not-null} value
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   SearchResult findUser(String userName) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Search for a group based on account name.
    * <ul>
    * The following formats are supported:
    * <li><b>&lt;group_name&gt;</b> - short form; search at default domain if
    * such is configured</li>
    * <li><b>&lt;[domain | alias]&gt;\&lt;group_name&gt;</b> - NetBIOS-style</li>
    * <li><b>&lt;group_name&gt;@[domain | alias]</b> - UPN-style</li>
    * </ul>
    * Domain and alias are case-insensitive; group_name depends on the identity
    * source implementation.
    *
    * @param groupName
    *           group name at the specified format; {@code not-null} and
    *           {@code not-empty} string value is required
    * @return group found or {@code null} when there is no match
    *
    * <p>Note: when the given group name is one of the implicit group names reserved
    * by the SSO system, {@code null} is returned.
    * @see PrincipalDiscovery #getImplicitGroupNames()
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Group findGroup(String groupName) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Find person user by id
    *
    * @param userId
    *           id of a person user; requires {@code not-null} value
    * @return person user found or {@code null} when there is no match
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws InvalidPrincipalException
    *            indicates an incorrect <i>userId</i>
    */
   @Privilege(Role.RegularUser)
   PersonUser findPersonUser(PrincipalId userId)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find solution user by name
    *
    * @param userName
    *           the name of solution user; requires {@code not-null} value
    * @return solution user found or {@code null} when there is no match
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   SolutionUser findSolutionUser(String userName)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find solution by its certificate DN
    *
    * @param certDN
    *           solution's certificate exact distinguished name; requires
    *           {@code not-null} and not-empty string value
    * @return solution user found or {@code null} when there is no match
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   SolutionUser findSolutionUserByCertDN(String certDN)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find group by id
    *
    * @param groupId
    *           id of the group; requires {@code not-null} value
    * @return group found or {@code null} when there is no match
    * <p>Note: when the given group Id matches one of the implicit group names reserved
    * by the SSO system, {@code null} is returned.
    * @see PrincipalDiscovery #getImplicitGroupNames()
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws InvalidPrincipalException
    *            when the provided group does not exist or is invalid.
    */
   @Privilege(Role.RegularUser)
   Group findGroup(PrincipalId groupId) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Find person users that satisfy the specified search criteria
    *
    * @param criteria
    *           search criteria; requires {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return The
    *           requested limit will be capped at
    *           {@link PrincipalDiscovery#PD_BULK_QUERY_MAX_SIZE}.
    * @return person users found up to the capped limit; empty set when there is
    *         no match; {@code not-null} value
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<PersonUser> findPersonUsers(SearchCriteria criteria, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find person users based upon their account name.
    *
    * A users account is chosen for the search results if the search string is
    * part of the account name, i.e. for AD, samAccountName is used.
    *
    * Regular expressions are not allowed in the search string.
    *
    * @param criteria
    *          search criteria; requires {@code not-null} value
    * @param limit
    *          a positive integer for the maximum number of items to return
    * @return person users found; empty set when there is no match;
    *         {@code not-null} value
    *
    * @throws NotAuthenticatedException
    *          when there is no authenticated SSO user associated with this
    *          method call
    * @throws NoPermissionException
    *          when the required privilege for calling this method is not held
    *          by the caller
    */
   @Privilege(Role.RegularUser)
   Set<PersonUser> findPersonUsersByName(SearchCriteria criteria, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find solution users that satisfy the specified search criteria
    *
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return.
    *           The requested limit will be capped at
    *           {@link PrincipalDiscovery#PD_BULK_QUERY_MAX_SIZE}.
    * @return solution users found up to the capped limit; empty set when there
    *         is no match; {@code not-null} value
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<SolutionUser> findSolutionUsers(String searchString, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find groups that satisfy the specified search criteria
    *
    * @param criteria
    *           search criteria; requires {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return.
    *           The requested limit will be capped at
    *           {@link PrincipalDiscovery#PD_BULK_QUERY_MAX_SIZE}.
    * @return groups found found up to the capped limit; empty set when there is
    *         no match; {@code not-null} value
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<Group> findGroups(SearchCriteria criteria, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find security groups based upon their name.
    *
    * Regular expressions are not allowed in the search string.
    *
    * @param criteria
    *          search criteria; requires {@code not-null} value
    * @param limit
    *          a positive integer for the maximum number of items to return
    * @return groups found; empty set when there is no match; {@code not-null}
    *         value
    *
    * @throws NotAuthenticatedException
    *          when there is no authenticated SSO user associated with this
    *          method call
    * @throws NoPermissionException
    *          when the required privilege for calling this method is not held
    *          by the caller
    */
   @Privilege(Role.RegularUser)
   Set<Group> findGroupsByName(SearchCriteria criteria, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find users and groups that satisfy the specified search criteria
    *
    * @param criteria
    *           search criteria; requires {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return.
    *           The requested limit will be capped at
    *           {@link PrincipalDiscovery#PD_BULK_QUERY_MAX_SIZE}.
    * @return users and groups found up to the capped limit; {@code not-null}
    *         value.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   SearchResult find(SearchCriteria criteria, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Finds regular users, security groups and service principals based upon
    * their name.
    *
    * Regular expressions are not allowed in the search string.
    *
    * @param criteria
    *          search criteria; requires {@code not-null} value
    * @param limit
    *          a positive integer for the maximum number of items to return
    * @return set of users, groups, and security principals found;
    *         {@code not-null} value
    *
    * @throws NotAuthenticatedException
    *          when there is no authenticated SSO user associated with this
    *          method call
    * @throws NoPermissionException
    *          when the required privilege for calling this method is not held
    *          by the caller
    */
   @Privilege(Role.RegularUser)
   SearchResult findByName(SearchCriteria criteria, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find person users that are direct members of the specified group.
    *
    * Utilizes 'contains'-style queries.
    *
    * @param groupId
    *           id of the group where to search; requires {@code not-null} value
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return
    * @return person users found; empty set when there is no match;
    *         {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no group with the specified id </li>
    *            <li>when the given group Id matches one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    *
    */
   @Privilege(Role.RegularUser)
   Set<PersonUser> findPersonUsersInGroup(PrincipalId groupId,
      String searchString, int limit) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Find person users that are direct members of the specified group.
    *
    * Utilizes 'starts with'-style queries.
    *
    * @param groupId
    *           id of the group where to search; requires {@code not-null} value
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return
    * @return person users found; empty set when there is no match;
    *         {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no group with the specified id </li>
    *            <li>when the given group Id matches one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    *
    */
   @Privilege(Role.RegularUser)
   Set<PersonUser> findPersonUsersByNameInGroup(PrincipalId groupId,
       String searchString, int limit) throws InvalidPrincipalException,
           NotAuthenticatedException, NoPermissionException;

   /**
    * Find solution users that are direct members of the specified local group
    *
    * @param groupName
    *           name of the local group where to search; requires
    *           {@code not-null} and not-empty string value
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return
    * @return solution users found; empty set when there is no match;
    *         {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no group with the specified id </li>
    *            <li>when the given group Id matches one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws IllegalArgumentException
    *            when the limit argument value is incorrect
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.RegularUser)
   Set<SolutionUser> findSolutionUsersInGroup(String groupName,
      String searchString, int limit) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Find groups that are direct members of the specified group.
    *
    * Utilizes 'contains'-style queries.
    *
    * @param groupId
    *           id of the group where to search; requires {@code not-null} value
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return
    * @return groups found; empty set when there is no match; this value is
    *         {@code not-null}
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no group with the specified id </li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.RegularUser)
   Set<Group> findGroupsInGroup(PrincipalId groupId, String searchString,
      int limit) throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Find groups that are direct members of the specified group.
    *
    * Utilizes 'starts with'-style queries.
    *
    * @param groupId
    *           id of the group where to search; requires {@code not-null} value
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return
    * @return groups found; empty set when there is no match; this value is
    *         {@code not-null}
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no group with the specified id </li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.RegularUser)
   Set<Group> findGroupsByNameInGroup(PrincipalId groupId, String searchString,
      int limit) throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Return the parent groups which have the specified principal as a direct
    * member
    *
    * @param principalId
    *           id of the principal for which the immediate parent groups should
    *           be returned; requires {@code not-null} value
    * @return parent groups found; empty set when the given principal is not
    *         member of any group; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is no group with the specified id </li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.RegularUser)
   Set<Group> findDirectParentGroups(PrincipalId principalId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Return the set of groups for which the specified user is a direct or
    * indirect member (through group to group link). In this case group id
    * cannot be specified as argument value.
    *
    * @param userId
    *           id of the user for which the direct and indirect parent groups
    *           should be returned; requires {@code not-null} value
    * @return parent groups found; empty set when given user is not member of
    *         any group; {@code not-null} value
    *
    * @throws InvalidPrincipalException
    *           <ul>
    *            <li>when there is neither person, nor solution user with the specified id</li>
    *            <li>when the given group name is one of the implicit group
    *            names reserved by the SSO system</li>
    *           </ul>
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @see PrincipalDiscovery #getImplicitGroupNames()
    */
   @Privilege(Role.RegularUser)
   Set<Group> findNestedParentGroups(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Find all users that have been locked, corresponding to the search string. <br />
    * Note that users from all identity sources, the SSO System identity source
    * and all configured external identity sources, will be listed.
    *
    * @param searchString
    *           refer to {@link SearchCriteria#getSearchString()}; requires
    *           {@code not-null} value
    * @param limit
    *           a positive integer for the maximum number of items to return The
    *           requested limit will be capped at
    *           {@link PrincipalDiscovery#PD_BULK_QUERY_MAX_SIZE}.
    * @return all locked users found up to the capped limit, or empty set if
    *         none were found
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @see LockoutPolicy for more information on the policy for locking users
    */
   @Privilege(Role.RegularUser)
   Set<PersonUser> findLockedUsers(String searchString, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find person users that are disabled
    *
    * @param searchString
    *           refer to {@link SearchCriteria#_searchString}
    * @param limit
    *           a positive integer for the maximum number of items to return The
    *           requested limit will be capped at
    *           {@link PrincipalDiscovery#PD_BULK_QUERY_MAX_SIZE}.
    * @return all disabled person users found up to the capped limit or
    *         empty if none were found
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<PersonUser> findDisabledPersonUsers(String searchString, int limit)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find solution users that are disabled
    *
    * @param searchString
    *           refer to {@link SearchCriteria#_searchString}
    * @return all disabled solution users or empty if none were found
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<SolutionUser> findDisabledSolutionUsers(String searchString)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Find person user represented by external IDP user's registered id
    *
    * @param userId
    *           id of person user. The id is created by registration of the external
    *           IDP user on the primary system.
    * @return person user found or {@code null} when there is no match.
    * @throws NotAuthenticatedException
    * @throws NoPermissionException
    */
   @Privilege(Role.RegularUser)
   PersonUser findRegisteredExternalIDPUser(PrincipalId userId)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Retrieve the list of group names reserved by SSO system. For example,
    * Everyone group, to which all authenticated users belong.
    *
    * @return A fixed set of group names reserved by the SSO system.
    *         <p>
    *         Consider these group name as being reserved by the SSO system and
    *         might not actually exist. Therefore, they cannot be created, and
    *         operations using any of them as input could return an error.
    */
   @NoPrivilege
   Set<String> getImplicitGroupNames();
}
