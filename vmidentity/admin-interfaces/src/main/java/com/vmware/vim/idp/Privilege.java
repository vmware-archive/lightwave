/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp;

import java.lang.annotation.Documented;

/**
 * This annotation indicates the minimal required role that should be possessed
 * by the caller for a successful method invocation
 */
@Documented
public @interface Privilege {

   Role value();
}
