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

//--------------------------------------------------------------------------------------
// Session Api

/**
 * Session Api class defintion.
 */
function SessionApi(conn) {
   ApiBase.call(this);
   this.__init__(conn, "session");
}

SessionApi.prototype = new ApiBase();
SessionApi.prototype.constructor = SessionApi;

ClientSupportApiFactory.registerApi("session", function(conn) {
   return new SessionApi(conn);
});

/**
 * Set the name of the client app.
 * @param appName Name of the client app.
 * @param clientVersion Version of the client app.
 * @param clientBuild Build of the client app.
 * @param clientKeepAlive Set to true to support keep alive. (True by default)
 */
SessionApi.prototype.init = function(args, callback) {
   if (args.clientKeepAlive === undefined) {
      args.clientKeepAlive = true;
   }
   var me = this;
   this.simpleApiCall("init", args, function(res, err) {
      if (res) {
         if (args.clientKeepAlive) {
            me.__startKeepAlive__();
         }
      }
      if (callback) {
         callback(res, err);
      }
   });
};

/**
 * Set the name of the client app.
 * @param appName Name of the client app.
 * @return
 *     obj.port The port of the protocol server
 */
SessionApi.prototype.addSimpleApiCall("queryProtocolServer");

/**
 * Ping to keep server alive.
 */
SessionApi.prototype.ping = function (args, callback) {
   this.simpleApiCall("ping", args, callback);
};

/**
 * Remove this session.
 */
SessionApi.prototype.remove = function (args, callback) {
   this.__stopKeepAlive__();
   this.simpleApiCall("remove", args, callback);
};

////////////////////////////////////////////////////////
// Internal - Keep alive methods

/**
 * Stop out keep alive.
 */
SessionApi.prototype.__stopKeepAlive__ = function () {
   if (this._keepAliveTimer) {
      clearInterval(this._keepAliveTimer);
      this._keepAliveTimer = null;
   }
}

/**
 * Push out keep alive.  (Only if it is enabled)
 */
SessionApi.prototype.__delayKeepAlive__ = function () {
   if (!this._keepAliveTimer) {
      // Keep alive not enabled, so do nothing.
      return;
   }
   this.__startKeepAlive__();
}

/**
 * Start keep alive timer or push it out.
 * Ping every 30 seconds.
 */
SessionApi.prototype.__startKeepAlive__ = function () {
   this.__stopKeepAlive__();
   this._keepAliveTimer = setInterval(this.ping.bind(this), 30000);
};
