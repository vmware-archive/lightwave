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

/**
 * A helper class to create Web Socket Api instances.
 */
ClientSupportApiFactory = {};
ClientSupportApiFactory._factoryByComponentId = {};

ClientSupportApiFactory.registerApi = function(name, factoryMethod) {
   ClientSupportApiFactory._factoryByComponentId[name] = factoryMethod;
};

ClientSupportApiFactory.createApi = function(name, conn) {
   var fn = ClientSupportApiFactory._factoryByComponentId[name];
   if (fn == null) {
      throw new Error("Api not found: " + name);
   }
   return fn(conn);
};
