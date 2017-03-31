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

function VmrcPluginInstance(flashPlayer) {

   /**
    * For internal js use.
    */

    var isIE = /MSIE (\d+\.\d+);/.test(navigator.userAgent) || /Trident\/(\d+\.\d+);.*rv:(\d+\.\d+)/.test(navigator.userAgent);

   getPlugin = function(pluginId) {
      return document.getElementById(pluginId);
   }

   //---------------------------------------------------------------------------
   // VMRC functions

   getConnectionState = function(pluginId) {
      return getPlugin(pluginId).getConnectionState();
   }

   getScreenWidth = function(pluginId) {
      return getPlugin(pluginId).screenWidth;
   }

   getScreenHeight = function(pluginId) {
      return getPlugin(pluginId).screenHeight;
   }

   sendCAD = function(pluginId) {
      return getPlugin(pluginId).sendCAD();
   }

   setFullscreen = function(pluginId, fullscreen) {
      return getPlugin(pluginId).setFullscreen(fullscreen);
   }

   startup = function(pluginId, modes, msgMode, advancedConfig) {
     return getPlugin(pluginId).startup(modes, msgMode, advancedConfig);
   }

   shutdown = function(pluginId) {
      return getPlugin(pluginId).shutdown();
   }

   connect = function(pluginId, host, thumbprint, allowSSLErrors, ticket, usr, pwd, vmId, dataCenter, vmPath) {
      try {
         getPlugin(pluginId).connect(
         host, thumbprint, allowSSLErrors, ticket, usr, pwd, vmId, dataCenter, vmPath);
      } catch (err) {
         return false;
      }
      return true;
   }

   disconnect = function(pluginId) {
      try {
         getPlugin(pluginId).disconnect();
      } catch (err) {
         return false;
      }
      return true;
   }

   getPhysicalClientDevices = function(pluginId, typeMask) {
      var devs = getPlugin(pluginId).getPhysicalClientDevices(typeMask);
      if (isIE) {
         devs = (new VBArray(devs)).toArray();
      }
      return devs;
   }

   getPhysicalClientDeviceDetails = function(pluginId, key) {
      var details = getPlugin(pluginId).getPhysicalClientDeviceDetails(unescape(key));
      if (isIE) {
         var keys = [
                     'key',
                     'type',
                     'state',
                     'connectedByMe',
                     'name',
                     'path',
                     'usbFamilies',
                     'usbSharable',
                     'usbSpeeds'
                 ];
         var details_arr = new Object();
         for (var i in keys) {
            details_arr[keys[i]] = details[keys[i]];
         }
         return details_arr;
      }
      return details;
   }

   connectDevice = function(pluginId, key, clientPath, fileBacking) {
      try {
         getPlugin(pluginId).connectDevice(unescape(key), unescape(clientPath), fileBacking);
      } catch (err) {
         return false;
      }
      return true;
   }

   disconnectDevice = function(pluginId, key) {
      try {
         getPlugin(pluginId).disconnectDevice(unescape(key));
      } catch (err) {
         return false;
      }
      return true;
   }

   setVisible = function(pluginId, visible) {
      getPlugin(pluginId).style.visibility = visible ? "visible" : "hidden";
   }

   setLocation = function(pluginId, locX, locY) {
      var plugin = getPlugin(pluginId);
      plugin.style.position = "absolute";
      plugin.style.left = locX + "px";
      plugin.style.top = locY + "px";
   }

   setSize = function(pluginId, width, height) {
      var plugin = getPlugin(pluginId);
      plugin.width = width;
      plugin.height = height;
   }

   //---------------------------------------------------------------------------
   // Initialization

   function initialize() {
      if (typeof(flashPlayer) == "string") {
         flashPlayer = document.getElementById(flashPlayer);
      }
   }

   // TODO dgunchev: listDevices
   // TODO dgunchev: connectDevice
   // TODO dgunchev: isDeviceConnected
   // TODO dgunchev: disconnectDevice
   // TODO dgunchev: attach an error handler to the vmrc
   //                and pass any errors to the Flex code.

   initialize();
   return true;
}
