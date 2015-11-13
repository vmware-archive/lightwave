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
package com.vmware.vim.sso.client;

import org.w3c.dom.Document;
import org.w3c.dom.Node;

/**
 * Indicates that an object that can be presented as XML
 *
 * <p>
 * The schema of the XML representation is up to implementation.
 */
public interface XmlPresentable {

   /**
    * @return object presented as a valid XML document string, not null
    *
    * <p>
    * The XML document will not have a &lt;?xml declaration.
    */
   public String toXml();

   /**
    * Creates a copy of the XML representation of the object, imported into the
    * given document.
    *
    * <p>
    * This method allows the implementations to restore XML metadata which is
    * lost when a node is imported in other document or serialized to string.
    *
    * @param hostDocument required
    * @return the cloned node, owned by the given document, not null
    */
   public Node importTo(Document hostDocument);
}
