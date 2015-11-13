/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.io.IOException;
import java.io.InputStream;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.InternalError;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * This service groups replication operations which are relevant only to
 * multi-site deployments. The service is not usable in other SSO deployments.
 *
 * Customer-driven replication is handled through {@link #exportFullState()} and
 * {@link #importFullState(InputStream)} methods of this service. This
 * replication:
 * <ul>
 * <li>is planned, initiated and operated by the customer;</li>
 * <li>relies on customer to transport the exported data to the other peers;</li>
 * <li>replicates always the full state;</li>
 * <li>does not support conflict resolution - just replace the data;</li>
 * <li>cannot be used as a backup/restore tool.</li>
 * </ul>
 *
 * @deprecated Since SSO 2.0 this interface has been deprecated.
 */
@Deprecated
public interface Replication {

   /**
    * Performs export of the full replication state.
    *
    * After that, the exported data can be imported to other peers. Note that
    * the data are not encrypted!
    *
    * @return not null stream of replicated data in binary form
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    *
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @throws UnsupportedOperationException
    *            when the current deployment is not multi-site
    *
    * @throws InternalError
    *            Indicates unexpected internal error in the SSO Server.
    *
    * @deprecated Since SSO 2.0 {@link Replication} interface is deprecated and
    *             this method will always throw
    *             {@link UnsupportedOperationException}.
    */
   @Deprecated
   @Privilege(Role.Administrator)
   InputStream exportFullState() throws NotAuthenticatedException,
      NoPermissionException, UnsupportedOperationException, InternalError;

   /**
    * Performs import of the full replication state.
    *
    * After successful completion of this request, the current SSO service has
    * the same state as the node where export has been done.
    *
    * @param fullState
    *           replicated data in binary form, mandatory
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    *
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @throws IOException
    *            when an IO error has occurred reading the given state
    *
    * @throws UnsupportedOperationException
    *            when the current deployment is not multi-site, when the origin
    *            of the replication state and the current peer match or if there
    *            is ongoing import from same peer.
    *
    * @throws IllegalArgumentException
    *            when some or all of the data chunks are missing or pertaining
    *            to more than one peer
    *
    * @throws InternalError
    *            Indicates unexpected internal error in the SSO Server.
    *
    * @deprecated Since SSO 2.0 {@link Replication} interface is deprecated and
    *             this method will always throw
    *             {@link UnsupportedOperationException}.
    */
   @Deprecated
   @Privilege(Role.Administrator)
   void importFullState(InputStream fullState)
      throws NotAuthenticatedException, NoPermissionException, IOException,
      UnsupportedOperationException, IllegalArgumentException, InternalError;
}
