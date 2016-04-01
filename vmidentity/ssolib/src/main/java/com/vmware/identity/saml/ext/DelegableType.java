/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved. 
 * *********************************************************************/
package com.vmware.identity.saml.ext;

import javax.xml.namespace.QName;

import org.opensaml.saml2.core.Condition;

/**
 * Delegable condition type
 *
 */
public interface DelegableType extends Condition {
    public static final String TYPE_LOCAL_NAME = "DelegableType";
    public static final QName TYPE_NAME = new QName(
       "http://vmware.com/schemas/attr-names/2012/04/Extensions", TYPE_LOCAL_NAME,
       "vmes");
}
