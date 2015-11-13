/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * Insert your comment for AttributeValuePair here
 */
public class AttributeValuePair implements Serializable
{
   private static final long serialVersionUID = 1L;

   private Attribute attrDefinition;
   private List<String>  values;

   public AttributeValuePair()
   {
	   values = new ArrayList<String>();
   }

   /**
    * @return the attrDefinition
    */
   public Attribute getAttrDefinition()
   {
      return attrDefinition;
   }

   /**
    * @param attrDefinition the attrDefinition to set
    */
   public void setAttrDefinition(Attribute attrDefinition)
   {
      this.attrDefinition = attrDefinition;
   }

   /**
    * @return the value
    */
   public List<String> getValues()
   {
      return values;
   }

   /**
    * @return true if the values list is empty
    */
   public boolean isEmpty()
   {
      return values.isEmpty();
   }
}
