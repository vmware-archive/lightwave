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

/**
 * ApiBase class constructor
 */
function ApiBase() {
   this.__init__ = function (conn, componentId) {
      if (conn == null) {
         throw new Error("Error a connection is required.");
      }
      this._conn = conn;
      this.componentId = componentId;
      this.objectId = null;
      this._callbackByRequestId = {};
      this.defaultCallback = conn.defaultCallback;
      this.createInstance = conn.createApiInstance;
      this.closeOnResponse = false;
   }
}

ApiBase.prototype = {};
ApiBase.prototype.constructor = ApiBase;

/**
 * Sends a message to a client.
 */
ApiBase.prototype.send = function (msg, callback) {
   if (!msg.requestId) {
      msg.requestId = this._conn.getNextRequestId();
   }
   if (this.componentId) {
      msg.componentId = this.componentId;
   }
   if (this.objectId) {
      msg.objectId = this.objectId;
   }
   if (this.createInstance) {
      msg.createInstance = "true";
   }
   if (callback) {
      this._callbackByRequestId[msg.requestId] = callback;
   }
   this._conn.__send__(msg, this);
};

/**
 * Handles a message from the server and dispatches the message to the callback method.
 */
ApiBase.prototype.__onmessage__ = function (msg) {
   var callback = this._callbackByRequestId[msg.requestId];
   var isFinal = msg.isFinal == "true";
   if (isFinal) {
      delete this._callbackByRequestId[msg.requestId];
   }
   if (msg.requestObjectId && !this.objectId) {
      this.objectId = msg.requestObjectId;
   }
   var err = null;
   if (msg.type == "error") {
      err = msg;
      msg = null;
   }
   if (callback) {
      callback(msg, err);
   }
   if (this.defaultCallback) {
      this.defaultCallback(msg, err);
   }
   if (this.closeOnResponse && isFinal) {
      this.close();
   }
};

/**
 * Close the Api object and release its resources.
 */
ApiBase.prototype.close = function () {
   if (this._closed) {
      return;
   }
   this._closed = true;
   this._callbackByRequestId = {};
   var msg = { method: "free" }
   this.send(msg, null);
};

/**
 * Invokes the named api call using the the provided args object.
 * The result is provided via the callback(msg, err);
 * The msg object contains the following fields:
 *    statusCode, result.
 *    Also optionally resulting component and objectId.
 * The err object contains the following fields:
 *    errorCode, message.
 * Note: The Message and Error's also have the following fields,
 *    but they are used mostly by the framework.
 *    sessionId, requestId, and requestObjectId.
 */
ApiBase.prototype.simpleApiCall = function(name, args, callback) {
   var msg = cloneWithStrings(args);
   msg.method = name;
   this.send(msg, callback);
}

/**
 * Adds an Simple Api call to the prototype.
 * @see simpleApiCall
 */
ApiBase.prototype.addSimpleApiCall = function(name) {
   this[name] = function (args, callback) {
      this.simpleApiCall(name, args, callback);
   };
}
