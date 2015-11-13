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

   /**
   /**
    * Establish a connection.
    * @param args.appName (default 'ui')
    * @param args.sessionId (default random uuid)
    * @param args.port (default VMW_CSD_DEFAULT_WSS_PORT)
    * @param args.retrySeconds (default VMW_CSD_CONNECT_TRIES_SECONDS)
    */
   this.open = function(args) {
      this.__debug_log__("Open Args: ", args);
      this.appName = getFieldWithDefault(args, 'appName', 'ui');
      this.__debug_log__("Open App Name: ", this.appName);
      this._sessionId = getFieldWithDefault(args, 'sessionId', null);
      this._port = getFieldWithDefault(args, 'port', VMW_CSD_DEFAULT_WSS_PORT);
      this._maxTrials = getFieldWithDefault(args, 'retrySeconds',
            VMW_CSD_CONNECT_TRIES_SECONDS);
      if (null == this._sessionId) {
         this._sessionId = createVMwareUUID();
      }
      this.__callStartProtocolServer__();
      this.__on_fail_handler__ = this.__on_lookup_fail__;
      this.__on_connected_handler__ = this.__on_lookup_connected__;
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
      line += ": " + Array.join(arguments, " ");
      console.log(line);
   }

   this.__open_protocol__ = function(port) {
      this._port = port;
      this.__on_fail_handler__ = this.__on_connect_fail__;
      this.__on_connected_handler__ = this.__on_connected__;
      this._maxTrials = 1; // Should not fail
      this._connectFailCount = 0;
      this.__openImpl__();
   };

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
         var socket = new WebSocket(method + "://vmware-localhost:" + port + "/");
         socket.onmessage = function(evt) { me.__onmessage__(evt) };
         socket.onopen = function(evt) {
            me.__clearLogin_Timer__();
            me._socket = socket;
            me.__on_connected_handler__(evt);
         };
         onfail = function(evt) {
            socket.onopen = null;
            socket.onclose = null;
            socket.onerror = null;
            me.__on_fail_handler__(evt);
         }
         socket.onclose = onfail;
         socket.onerror = onfail;

      } catch (err) {
         this.__on_fail_handler__({data:err.message});
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
      this._socket.onclose = null;
      this._socket.onerror = function(evt) {
         me.__close_socket__(socket);
         if (me._socket == socket) {
            me._socket = null;
         }
	 fire(me.onerror, evt);
      };

      // Give the protocol server, some time to start.
      setTimeout(me.__init_session__.bind(this), 500);
   };

   this.__init_session__ = function() {
      var sessionApi = this.getOrCreateApi("session");
      // Passing csdService as a hack, since appName is required
      // The lookup server does not use a keep alive, so passing false.
      this.__debug_log__("init lookup server session: ", this.appName);
      sessionApi.init({appName:this.appName, clientKeepAlive:false},
            this.__on_protocol_session_init.bind(this));
   };

   this.__on_protocol_session_init = function(result, err) {
      if (err) {
         this.onApiError("Service init error code: " + err.statusCode);
         return;
      }
      this.close();
      this.__debug_log__("Init callback: port - ", result.port);
      this.__open_protocol__(result.port);
   };

   this.__callStartProtocolServer__ = function() {
      var pUrl = 'vmware-csd://csd?sessionId=' + this._sessionId;
      pUrl += '&appName=' + this.appName;

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

   /**
    * Handles the connected event.
    */
   this.__on_connected__ = function(con_evt) {
      this.__debug_log__("CSD Connected");
      this.isOpen = true;
      this.isOpenning = false;
      var me = this;
      this._socket.onopen = null;
      this._socket.onclose = function(evt) {
         me.close();
         fire(me.onclose, evt);
      };
      this._socket.onerror = function(evt) { fire(me.onerror, evt); };
      this._sessionApi = this.getOrCreateApi("session");
      this.__debug_log__("Fire onopen");
      fire(me.onopen, con_evt);
      // Might need to do something different for H5, but this will
      // work for flex which does not show different pages.
      window.addEventListener('unload', this.__on_unload__.bind(this));
   };

   this.__on_unload__ = function(evt) {
      this.__debug_log__("OnUnload");
      // End the protocol server
      this._sessionApi.remove();
      // The browser is closed.
      this.close();
   };

   /**
    * Handles the connection failure event.
    */
   this.__on_connect_fail__ = function(evt) {
      this.__debug_log__("Conn Failed: ", evt.data);
      if (!this.isOpenning) {
         // Already failed
         return;
      }
      this.close();
      fire(this.onclose, evt);
   };

   /**
    * Handles the lookup sever connection failure event.
    */
   this.__on_lookup_fail__ = function(evt) {
      // The timer will do the retry logic to avoid hitting the sever to often.
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
         this.__debug_log__("Close, and try again...");
         this.close();
         // Not connected.
         this._connectFailCount++;
         if (this._connectFailCount < this._maxTrials) {
            // at the 3rd retrial - send a onfail event
            // so the download url is shown
            if (this._connectFailCount == 3) {
               fire(this.onfail, evt);
            }
            // try again - this can take a long time, since the user needs to accept the
            // protocol handler warning dialogs or the protocol handler is not installed.
            // TODO mvdb: Add code to detect if the protocol handler is installed.
            this.__openImpl__();
         } else {
            fire(this.onclose, evt);
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
   this.onerror = function(evt) { };

   /* used to detect if a connection fails */
   this.onfail = function(evt) { };

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
};
