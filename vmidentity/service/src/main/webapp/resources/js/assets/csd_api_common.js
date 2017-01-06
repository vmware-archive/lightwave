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

//
// Defines the client side Api for the Client Support Web Socket
// Interface.
//

//
// helpers:
//

/*
 * Returns true if obj is of string type.
 */
function isString(obj) {
   // Based on http://stackoverflow.com/questions/4059147/check-if-a-variable-is-a-string
   return obj instanceof String || typeof obj == 'string';
}

/*
 * Definition for Api Message (not used directly)
 */
function ApiMessage() {
   this.isStatic = "false";
   this.objectId = null;
   this.sessionId = null;
   this.requestId = null;
}

/*
 * Fire an handler method if there is one with the event as an argument.
 */
function fire(handler, evt) {
   if (handler) { handler(evt); }
}

// Create an object = Object.create(proto)

/**
 * Clone an object and convert any non-string fields to strings.
 */
function cloneWithStrings(obj) {
   var res = {};
   for (key in obj) {
      var item = obj[key];
      if (item == null) {
         // Just do not pass the null on
      } else if (typeof (item) === 'object') {
         res[key] = cloneWithStrings(item);
      } else if (typeof (item) === 'boolean') {
         res[key] = item ? "true" : "false";
      } else {
         res[key] = item.toString();
      }
   }
   return res;
}

var vmwareUUIDchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" + "1234567890";

function createVMwareUUIDOld() {
   var result = "";
   var n = vmwareUUIDchars.length;

   for (var i = 0; i < 16; i++) {
      if (i > 0 && i%4 == 0) {
         result += "-";
      }
      var rn = Math.floor(Math.random() * n);
      result += vmwareUUIDchars.charAt(rn);
   }
   return result;
}

function createVMwareUUID() {
   if  (!window.crypto || !window.crypto.getRandomValues) {
      return createVMwareUUIDOld();
   }
   var result = "";
   var n = vmwareUUIDchars.length;
   var data = new Uint8Array(16);
   window.crypto.getRandomValues(data);

   for (var i = 0; i < data.length; i++) {
      if (i > 0 && i % 4 == 0) {
         result += "-";
      }
      var rn = data[i] % n;
      result += vmwareUUIDchars.charAt(rn);
   }
   return result;
}

function getFieldWithDefault(args, name, defaultValue) {
   if (args == null) {
      return defaultValue;
   }
   var val = args[name];
   return val ? val : defaultValue;
}

function getIEVersion() {
   var userAgent = navigator.userAgent;
   var patt = new RegExp("((MS)?IE|rv:)[ \t]*([0-9]+)");
   var res = patt.exec(userAgent);
   if (res == null) {
      return null;
   }
   return res[3];
}
