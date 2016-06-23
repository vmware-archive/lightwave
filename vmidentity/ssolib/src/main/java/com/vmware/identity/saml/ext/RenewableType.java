/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.saml.ext;

import javax.xml.namespace.QName;

import org.opensaml.saml2.core.Condition;

/**
 * Renewable condition
 *
 */
public interface RenewableType extends Condition {
    public static final String TYPE_LOCAL_NAME = "RenewableType";
    public static final QName TYPE_NAME = new QName(
       "http://vmware.com/schemas/attr-names/2012/04/Extensions", TYPE_LOCAL_NAME,
       "vmes");
}
