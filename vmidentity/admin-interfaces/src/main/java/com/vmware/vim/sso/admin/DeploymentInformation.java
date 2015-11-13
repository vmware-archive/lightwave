/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.io.InputStream;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.InternalError;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Provides information about the deployment type of this SSO Server's instance
 * and to the configuration parameters necessary to install additional servers
 * in a High-availability cluster or in a multi-site deployment.
 */
public interface DeploymentInformation {

   /**
    * Returns whether this SSO Server instance supports multi-site (a.k.a.
    * linked-mode) deployment.
    * <p>
    * The multi-site support (or the lack of) is configured during the
    * installation of the server and cannot be changed afterwards.
    *
    * @return True if this instance is part of replicated deployment and false
    *         otherwise.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    *
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @throws InternalError
    *            Indicates unexpected, internal error in the SSO Server.
    *
    */
   @Privilege(Role.Administrator)
   boolean isMultiSiteDeployment() throws NotAuthenticatedException,
      NoPermissionException, InternalError;

   /**
    * Returns the configuration parameters necessary to install a "backup"
    * (a.k.a. "secondary") SSO Server node in the same High-Availability
    * cluster.
    *
    * @return A binary stream containing the configuration data. The package is
    *         encrypted with the Server's master password. It is the caller's
    *         responsibility to close the returned stream.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    *
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @throws InternalError
    *            Indicates unexpected, internal error in the SSO Server.
    *
    * @deprecated Since SSO 2.0 this method is irrelevant and will always throw
    *             {@link UnsupportedOperationException}.
    */
   @Privilege(Role.Administrator)
   @Deprecated
   InputStream retrieveHaBackupConfigurationPackage()
      throws NotAuthenticatedException, NoPermissionException, InternalError,
      UnsupportedOperationException;

   /**
    * Returns the configuration parameters necessary to install another replica
    * in the same multi-site deployment.
    * <p>
    * Note that this data is only available if during the installation this
    * instance was configured with multi-site support.
    *
    * @return A binary stream containing the configuration data. The package is
    *         in form of a zip archive comprised of compressed entries. There is
    *         always an entry with name "instance.pkg". It is encrypted with the
    *         "multi-site password" which is picked up during the installation
    *         of the first instance. Note that this has noting in relation to
    *         password-protected archives! Other entries may or may not be
    *         present in the archive. The caller is responsible to close the
    *         returned stream.
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
    *            Thrown to indicate this instance does not support replication
    *            (i.e. is not part of a replicated deployment). @see
    *            {@linkplain #isDeploymentMultiSite()}.
    *
    * @throws InternalError
    *            Indicates unexpected, internal error in the SSO Server.
    *
    * @deprecated Since SSO 2.0 this method is irrelevant and will always throw
    *             {@link UnsupportedOperationException}.
    */
   @Privilege(Role.Administrator)
   @Deprecated
   InputStream retrieveReplicaConfigurationPackage()
      throws NotAuthenticatedException, NoPermissionException, InternalError,
      UnsupportedOperationException;
}
