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
package com.vmware.identity.saml.ext.impl;

import org.opensaml.xml.schema.impl.XSStringImpl;
import org.opensaml.xml.util.DatatypeHelper;

public class XSNonTrimmingString extends XSStringImpl
{
    protected XSNonTrimmingString(String namespaceURI, String elementLocalName, String namespacePrefix)
    {
        super(namespaceURI, elementLocalName, namespacePrefix);
    }

    /**
     * A helper function for derived classes. This 'normalizes' newString and then if it is different from oldString
     * invalidates the DOM. It returns the normalized value so subclasses just have to go. this.foo =
     * prepareForAssignment(this.foo, foo);
     *
     * @param oldValue - the current value
     * @param newValue - the new value
     *
     * @return the value that should be assigned
     */
    protected String prepareForAssignment(String oldValue, String newValue)
    {
        if ( ( newValue != null ) && (newValue.length() == 0) )
        {
            newValue = null;
        }

        if (!DatatypeHelper.safeEquals(oldValue, newValue))
        {
            releaseThisandParentDOM();
        }
        return newValue;
    }
}
