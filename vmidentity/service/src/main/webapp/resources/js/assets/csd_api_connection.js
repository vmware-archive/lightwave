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
 
// global
var VMW_CSD_DEFAULT_WSS_PORT = 8093;
var VMW_CSD_CONNECT_TRIES_SECONDS = 30; // 30 Seconds
var VMW_CSD_VERSION = '2016';

/**
 * The connection object for a Web Socket Apis.
 */
function ApiConnection() {
   // Instance
   this._sessionId = null;
   this._nextRequestId = 1;

   // _apiLookup[componentId][objectId]
   this._apiLookup = {};
   this._apiByRequestId = {}; // If the object Id is not known then this map will be used
   this._socket = null;
   this.isOpenning = false;
   this.isOpen = false;
   this._connectFailCount = 0;
   this.defaultCallback = null;
   this.createApiInstance = true;
   this.debug = false; // Set to true to have debug lines print to the console.
   this._connectingSockets = [];

   /**
    * Establish a connection.
    * @param args.appName (default 'ui')
    * @param args.sessionId (default random uuid)
    * @param args.port (default VMW_CSD_DEFAULT_WSS_PORT)
    * @param args.retrySeconds (default VMW_CSD_CONNECT_TRIES_SECONDS)
    * @param args.csdVersion (default VMW_CSD_VERSION)
    *        '2016' - 6.5, '2015' - 6.0, '2014' - 5.5
    */
   this.open = function(args) {
      this.__debug_log__("Open Args: ", JSON.stringify(args));
      this.appName = getFieldWithDefault(args, 'appName', 'ui');
      this.__debug_log__("Open App Name: ", this.appName);
      this._sessionId = getFieldWithDefault(args, 'sessionId', null);
      this._port = getFieldWithDefault(args, 'port', VMW_CSD_DEFAULT_WSS_PORT);
      this._maxTrials = getFieldWithDefault(args, 'retrySeconds',
            VMW_CSD_CONNECT_TRIES_SECONDS);
      this._csdVersion = getFieldWithDefault(args, 'csdVersion', VMW_CSD_VERSION);
      if (this._csdVersion.indexOf('@') == 0) {
         // This was launcher by the test folder, and the version was not replaced.
         // Use test defined value,
         if (VMW_TEST_CSD_VERSION) {
            this._csdVersion = VMW_TEST_CSD_VERSION;
         }
      }
      if (null == this._sessionId) {
         this._sessionId = createVMwareUUID();
      }
      this._connectFailCount = 0;
      this.__openImpl__();
   };

   this.__debug_log__ = function() {
      if (!this.debug) {
         return;
      }
      var line = "CIP";
      if (this._port) {
         line += "[" + this._port + "]";
      }
      var argArray = [].slice.apply(arguments);
      line += ": " + argArray.join(" ");
      console.log(line);
   }

   this.__openImpl__ = function() {
      this.isOpenning = true;
      this._hLoginTimer = setTimeout(this.__login_timer__.bind(this), 1000);
      this.__tryOpen__("wss", this._port);
   };

   this.__clearLogin_Timer__ = function() {
      if (this._hLoginTimer) {
         clearTimeout(this._hLoginTimer);
         this._hLoginTimer = null;
      }
   }

   /**
    * Internal method to establish a localhost web socket connection.
    * The method can be either ws or wss.
    */
   this.__tryOpen__ = function(method, port) {
      var me = this;
      try {
         var socket = new WebSocket(method + "://vmware-localhost:" + port + "/" +
               "?src=client&sessionId=" + this._sessionId +
               "&appName=" + this.appName +
               "&version=" + this._csdVersion
               );

         this._connectingSockets.push(socket);
         socket.onmessage = function(evt) { me.__onmessage__(evt) };
         socket.onopen = function(evt) {
            me.__clearLogin_Timer__();
            me._socket = socket;
            me.__on_lookup_connected__(evt);
         };
         onfail = function(evt) {
            me.__close_socket__(socket);
         }
         socket.onclose = onfail;
         socket.onerror = onfail;
      } catch (err) {
         this.__debug_log__("Connection Error: " + err.message);
      }
   };

   /**
    * Handles the connected event.
    */
   this.__on_lookup_connected__ = function(evt) {
      if (this.isOpen) {
         this.__debug_log__("Extra Lookup Connected, ignoring for: ", this.appName);
         // This was called already no need to prcoess again.
         return;
      }
      this.__debug_log__("Lookup Connected: ", this.appName);
      this.isOpen = true;
      this.isOpenning = false;
      var me = this;
      var socket = this._socket;
      this._socket.onopen = null;
      this._socket.onclose = function(evt) {
         fire(me.onclose, evt);
      };
      this._socket.onerror = function(evt) {
         if (!me.isOpen) {
            // Already failed
            return;
         }
         me.__debug_log__("Conn Failed: ", evt.data);
         fire(me.onerror, evt);
         me.close();
      };

      this._connectingSockets.forEach(function(connectingSocket) {
         if (connectingSocket == socket) {
            return;
         }
         connectingSocket.close();
      });
      this._connectingSockets = [];

      this.__debug_log__("CSD Connected");
      this._sessionApi = this.getOrCreateApi("session");

      // Might need to do something different for H5, but this will
      // work for flex which does not show different pages.
      window.addEventListener('unload', this.__on_unload__.bind(this));

      this.__callStartProtocolServer__();

      // Give the protocol server, some time to start.
      setTimeout(function() {
            me.onopen(evt);
            }, 500);
   };

   this.__callStartProtocolServer__ = function() {
      var pUrl = 'vmware-csd://csd?sessionId=' + this._sessionId +
            '&appName=' + this.appName +
            '&version=' + this._csdVersion;

      this.__startProtocolServer__(pUrl);
   };

   this.__startProtocolServer__ = function(url) {
      if (getIEVersion() != null) {
         var frm = document.getElementById('csdframe');
         if (frm == null) {
            frm = document.createElement("iframe");
            frm.id = 'csdframe';
            frm.name = 'csdframe';
            frm.src = 'about:blank';
            frm.style.display = 'none';
            document.body.appendChild(frm);
         }

         frm.src = url;
      } else {
         var win = (window.parent)? window.parent : window;
         win.location.assign(url);
      }
   };

   this.__on_unload__ = function(evt) {
      this.__debug_log__("OnUnload");
      // End the protocol server
      this._sessionApi.remove();
      // The browser is closed.
      this.close();
   };

   /**
    * Handles handles connection timer.
    */
   this.__login_timer__ = function(evt) {
      if (!this.isOpenning) {
         return;
      }
      if (this.isOpen) {
         return;
      }
      if (this._socket == null) {
         // Not connected.
         this._connectFailCount++;
         if (this._connectFailCount < this._maxTrials) {
            this.__debug_log__("Retry connect " + this._connectFailCount + "...");
            // try again - this can take a long time, since the user needs to accept the
            // protocol handler warning dialogs or the protocol handler is not installed.
            // TODO mvdb: Add code to detect if the protocol handler is installed.
            this.__openImpl__();
         } else {
            fire(this.onerror.bind(this), {data:"Connection timed out."});
         }
      }
   };

   /**
    * Override in local instance to handle connection open event.
    */
   this.onopen = function(evt) { };

   /**
    * Override in local instance to handle connection close event.
    */
   this.onclose = function (evt) { };

   /**
    * Override to handle a connection error.
    */
   this.onerror = function(evt) {
      // Log error rather then showing an alert.
      this.__debug_log__("Connection Error: " + evt.data);
   };

   /**
    * Override in local instance to handle unknown Api Error.
    */
   this.onApiError = function(msg) {
      alert("Api Error: " + msg);
   };

   /**
    * Internal method to verify and send web socket message.
    */
   this.__send__ = function (msg, api) {
      if (!this._socket) {
         this.__debug_log__("send failed socket is null");
         return;
      }
      if (!msg.method) {
         this.onApiError("A method is required.");
         return;
      }
      if (!msg.requestId) {
         this.onApiError("A requestId is required.");
         return;
      }
      if (!msg.sessionId && this._sessionId) {
         msg.sessionId = this._sessionId;
      }
      if (msg.method == "free") {
         this.getApi(msg, "delete");
         delete this._apiByRequestId[msg.requestId];
      } else {
         this.__registerApi__(msg, api);
      }
      var jsonMsg = JSON.stringify(msg);
      this.__debug_log__("send: ", jsonMsg);
      this._socket.send(jsonMsg);

      if (this._sessionApi) {
         this._sessionApi.__delayKeepAlive__();
      }
   };

   this.__close_socket__ = function(socket) {
      if (socket == null) {
         return;
      }
      socket.onmessage = null;
      socket.onopen = null;
      socket.onclose = null;
      socket.onerror = null;
   };

   /**
    * Closes the web socket.
    */
   this.close = function() {
      this.__debug_log__("Close Called");
      if (this._socket != null) {
         if (this.isOpen) {
            this._socket.close();
         }
         this.__close_socket__(this._socket);
         this._socket = null;
      }
      this.isOpen = false;
      this.isOpenning = false;
      this._apiByObjectId = {};
      this._apiByRequestId = {};
   };

   /**
    * Get the next request id.
    */
   this.getNextRequestId = function() {
      return (this._nextRequestId++).toString();
   };

   /**
    * @param op Operation - create or delete or none
    */
   this.getApi = function(msg, op) {
      if (msg == null) {
         return null;
      }
      var objectId = msg.requestObjectId || msg.objectId;
      if (objectId == null) {
         throw new Error("objectId is required");
      }
      var objects = this.__getApiObjects__(msg);
      var api = objects[objectId];
      if (api == null && op == 'create') {
         var componentId = msg.requestComponentId || msg.componentId;
         api = ClientSupportApiFactory.createApi(componentId, this);
         this.__registerApi__(msg, api);
      }
      if (api != null && op == 'delete') {
         delete objects[msg.objectId];
      }
      return api;
   };

   /**
    * @param msg Message used to get componentId.
    */
   this.__getApiObjects__ = function(msg) {
      if (msg == null) {
         return null;
      }
      var componentId = msg.requestComponentId || msg.componentId;
      if (componentId == null) {
         throw new Error("componentId is required");
      }
      var objects = this._apiLookup[componentId];
      if (objects == null) {
         this._apiLookup[componentId] = objects = {};
      }
      return objects;
   };

   this.__registerApi__ = function(msg, api) {
      if (msg == null || api == null) {
         return;
      }
      var objectId = msg.requestObjectId || msg.objectId;
      if (objectId == null) {
         this._apiByRequestId[msg.requestId] = api;
         return;
      }
      var objects = this.__getApiObjects__(msg);
      objects[objectId] = api;
      api.objectId = msg.objectId;
   };

   this.getOrCreateApi = function(componentId, objectId) {
      if (null == objectId) {
         objectId = componentId;
      }
      return this.getApi(
            {componentId:componentId, objectId:objectId},
            'create');
   };

   /**
    * Handles a message from the server.
    * Finds the source api object and dispatches the message to it.
    */
   this.__onmessage__ = function (evt) {
      this.__debug_log__("onmessage: ", evt.data);
      var msg = JSON.parse(evt.data);
      var errorMessage = "";
      if (!this._sessionId) {
         this._sessionId = msg.sessionId;
      }
      if (this._sessionId != msg.sessionId) {
         this.onApiError("Session id for the api connection '" + this._sessionId +
               "' does not match the response '" + msg.sessionId + "'.");
         return;
      }
      if (!msg.type) {
         this.onApiError("The response had no type.");
         return;
      }
      if (!msg.requestId) {
         this.onApiError("There was no requestId in the response.");
         return;
      }
      if (msg.type != "error") {
         if (!msg.requestObjectId) {
            this.onApiError("There was no requestObjectId in the response.");
            return;
         }
      }
      var api = null;
      if (msg.requestId) {
         api = this._apiByRequestId[msg.requestId];
         if (api != null) {
            delete this._apiByRequestId[msg.requestId];
         }
      }
      if (api == null) {
         api = this.getApi(msg, "none");
      }

      if (api != null) {
         api.__onmessage__(msg);
      }
   };

   this.console = window.console;

   if (!this.console) {
      this.console = {log: function() {} };
   }
};
