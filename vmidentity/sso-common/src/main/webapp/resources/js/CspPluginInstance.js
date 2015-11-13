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
    * For internal js use.
    */
   var isIE = /MSIE (\d+\.\d+);/.test(navigator.userAgent) || /Trident\/(\d+\.\d+);.*rv:(\d+\.\d+)/.test(navigator.userAgent);

   getPlugin = function(pluginId) {
      return document.getElementById(pluginId);
   }

   //---------------------------------------------------------------------------
   // Util functions

   function js2ax(jsArray) {
      var dict = new ActiveXObject("Scripting.Dictionary");
      for (var i = 0; i < jsArray.length; i++) {
         dict.add(i, jsArray[i]);
      }
      return dict.Items();
   }

   //---------------------------------------------------------------------------
   // Client authentication functions

   initializeSSPI = function(pluginId, providerName, target) {
      var plugin = getPlugin(pluginId);
      return plugin.sspi.initialize(providerName, target);
   }

   negotiateSSPI = function(pluginId, serverToken) {
      var plugin = getPlugin(pluginId);
      return plugin.sspi.negotiate(serverToken);
   }

   getADUserName = function(pluginId) {
      var plugin = getPlugin(pluginId);
      return plugin.sspi.getADUserName();
   }

   //---------------------------------------------------------------------------
   // File transfer functions

   selectFile = function(pluginId, initDir, filter, filterDesc, fileName) {
      //Set a delay so the UI is not blocked
     var timeout = setTimeout(function() {
        var plugin = getPlugin(pluginId);
        var selectedFile = plugin.selectFile(initDir, filter, filterDesc, fileName);
        onFilePathSelected(selectedFile);
     }, 1000);
     return true;
   }

   selectFolder = function(pluginId, foldersOnly) {
     var timeout = setTimeout(function() {
         var plugin = getPlugin(pluginId);
         var selectedFolder = plugin.selectFolder(foldersOnly);
         onFilePathSelected(selectedFolder);
      }, 1000);
      return true;
   }

   uploadFile = function(pluginId, hostname, username, ticket, thumbprint, port, source, destination) {
      var plugin = getPlugin(pluginId);
      return getPlugin(pluginId).uploadFile(hostname, username, ticket, thumbprint, port, source, destination);
   }

   downloadFile = function(pluginId, hostname, username, ticket, thumbprint, port, source, destination) {
      var plugin = getPlugin(pluginId);
      return getPlugin(pluginId).downloadFile(hostname, username, ticket, thumbprint, port, source, destination);
    }

   cancelFileTransfer = function(pluginId, processId) {
      var plugin = getPlugin(pluginId);
      return plugin.cancelFileTransfer(processId);
   }

   //---------------------------------------------------------------------------
   // File transfer callback functions

   function onFilePathSelected(filePath) {
      return flashPlayer.onFilePathSelected(filePath);
   }

   //---------------------------------------------------------------------------
   // OVF functions - see wiki OvfTool/Plugin

   isOvfToolInstalled = function(pluginId) {
      return getPlugin(pluginId).isOvfToolInstalled();
   };

   getOvfToolVersion = function(pluginId) {
      return getPlugin(pluginId).getOvfToolVersion();
   };

   getOvfRunningSessionCount = function(pluginId) {
	  return getPlugin(pluginId).getOvfRunningSessionCount();
   };

   getOvfLastStatus = function(pluginId) {
      return getPlugin(pluginId).getOvfLastStatus();
   };

   getOvfTicket = function(pluginId) {
      return getPlugin(pluginId).getOvfTicket();
   };

   setOvfSource = function(pluginId, ticket, source) {
      return getPlugin(pluginId).setOvfSource(ticket, source);
   };

   setOvfTarget = function(pluginId, ticket, target) {
      return getPlugin(pluginId).setOvfTarget(ticket, target);
   };

   ovfProbe = function(pluginId, ticket, options) {
      var opt = options;
      if (isIE) {
         opt = js2ax(opt);
      }
      return getPlugin(pluginId).ovfProbe(ticket, opt);
   };

   ovfVerify = function(pluginId, ticket, options) {
      var opt = options;
      if (isIE) {
         opt = js2ax(opt);
      }
      return getPlugin(pluginId).ovfVerify(ticket, opt);
   };

   ovfExecute = function(pluginId, ticket, options) {
      var opt = options;
      if (isIE) {
         opt = js2ax(opt);
      }
      return getPlugin(pluginId).ovfExecute(ticket, opt);
   };

   ovfPassword = function(pluginId, ticket, type, username, password) {
      return getPlugin(pluginId).ovfPassword(ticket, type, username, password);
   };

   ovfCancel = function(pluginId, ticket) {
      return getPlugin(pluginId).ovfCancel(ticket);
   };

   ovfRemoveTicket = function(pluginId, ticket) {
      return getPlugin(pluginId).ovfRemoveTicket(ticket);
   };
