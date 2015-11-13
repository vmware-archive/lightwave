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

    VMRC_PROBE_ID = "__PROBE_VMRC_INSTANCE";
    CSP_PROBE_ID = "__PROBE_CSP_INSTANCE";

    var isIE = /MSIE (\d+\.\d+);/.test(navigator.userAgent) || /Trident\/(\d+\.\d+);.*rv:(\d+\.\d+)/.test(navigator.userAgent);


   /**
    * Create a "probe" instance of VMRC
    */
   createProbeVmrc = function(classid, mimetype) {
      var probe = document.getElementById(VMRC_PROBE_ID);

      if (probe != null) {
         // object already exists
         return probe;
      } else if (classid != null) {
         // passing a classid means we're dealing with IE
         return createPluginObject("classid", classid, VMRC_PROBE_ID);
      } else if (mimetype != null && isFirefoxPluginInstalled(mimetype)) {
         // passing a mime type means we're dealing with FF
         return createPluginObject("type", mimetype, VMRC_PROBE_ID);
      }
      // no probe object could be created
      return null;
   };


   /**
    * Destroys the "probe" instance of VMRC
    */
   destroyProbeVmrc = function() {
     removePluginObject(VMRC_PROBE_ID);
     return true;
   };

   /**
    * Get the version of the vmrc plugin.
    */
   function getVmrcVersion() {
      var probe =  document.getElementById(VMRC_PROBE_ID);
      if (probe == null) {
         return null;
      }
      return probe.getVersion();
   }

   getVmrcConstant = function(constantType, constantName) {
      var probe = document.getElementById(VMRC_PROBE_ID);
      if (probe == null) {
         return "";
      }
      if (isIE) {
         return probe[constantType](constantName);
      } else {
        return probe[constantType][constantName];
      }
   }

   isInProtectedMode = function() {
      var probe = document.getElementById(VMRC_PROBE_ID);
      if (probe != null && isIE) {
         return probe.isInProtectedMode();
      }
      return false;
   }

   /**
    * Create a "probe" instance of CSP
    */
   createProbeCsp = function(mimetype) {
      var probe = document.getElementById(CSP_PROBE_ID);

      if (probe != null) {
         // object already exists
         return probe;
      } else if (mimetype != null) {
         if (isIE) {
            return createPluginObject("type", mimetype, CSP_PROBE_ID);
         } else if (isFirefoxPluginInstalled(mimetype)) {
            return createPluginObject("type", mimetype, CSP_PROBE_ID);
         }
      }
      // no probe object could be created
      return null;
   };

   /**
    * Destroys the "probe" instance of CSP
    */
   destroyProbeCsp = function() {
     removePluginObject(CSP_PROBE_ID);
     return true;
   };

   /**
    * Get the version of the client support plugin.
    */
   getCspVersion = function() {
      var probe =  document.getElementById(CSP_PROBE_ID);
      if (probe == null) {
         return null;
      }
      return probe.version;
   };

   /**
    * Get the version of OVF Tool from the client support plugin. We need to check if
    * OVF Tool is present before we can check for its version.
    */
   getCspOvfToolVersion = function() {
      var probe =  document.getElementById(CSP_PROBE_ID);
      if (probe == null) {
         return null;
      }
      var ovfToolInstalled = probe.isOvfToolInstalled();
      if (!ovfToolInstalled) {
         return null;
      }
      var ovfToolVersion = probe.getOvfToolVersion();
      if (ovfToolVersion == "") {
         return null;
      }
      return ovfToolVersion;
   };

   // ----------------------------------------------------------------------------------

    /**
     * For internal js use.
     */
    createUniqueId = function(baseId) {
       var counter = 0;
       while(document.getElementById(baseId + counter)) {
          counter++;
       }
       return baseId + counter;
    };

    createVmrcIEPluginObject = function(classid) {
       var id = createUniqueId('vmrcInstance');
       var obj = createPluginObject("classid", classid, id);
       return obj;
    };

    createVmrcFirefoxPluginObject = function(mimeType) {
       if (!isFirefoxPluginInstalled(mimeType)) {
          return null;
       }
       var id = createUniqueId('vmrcInstance');
       var obj = createPluginObject("type", mimeType, id);
       return obj;
    };

    createCspIEPluginObject = function(mimeType) {
        var obj = createFBPluginObject(mimeType);
        return obj;
     };

     createCspFirefoxPluginObject = function(mimeType) {
        if (!isFirefoxPluginInstalled(mimeType)) {
           return null;
        }
        var obj = createFBPluginObject(mimeType);
        return obj;
     };

    // Create Firebreath based plugin instances
    createFBPluginObject = function(mimeType) {
        var containerId = createUniqueId('cspInstanceContainer');
        var instanceId = createUniqueId('cspInstance');

        var divElm = document.createElement("div");
        divElm.setAttribute("id", containerId);
        document.body.appendChild(divElm);

        var content = '<object id="' + instanceId + '" type="';
        content += mimeType + '" style="width: 1px; height: 1px" height="1" width="1">';
        document.getElementById(containerId).innerHTML = content;
        return instanceId;
    };

   createPluginObject = function(prop, value, id, visible) {
      var obj = document.createElement("OBJECT");
      obj.setAttribute("id", id);
      obj.setAttribute(prop, value);
      if (!visible || visible == undefined) {
         obj.setAttribute("style", "width: 1px; height: 1px");
         obj.setAttribute("height", "1");
         obj.setAttribute("width", "1");
      } else {
         obj.setAttribute("style", "width: 100%; height: 100%;");
      }
      document.body.appendChild(obj);
      return id;
   };

   isFirefoxPluginInstalled = function(mimetype) {
      for (var i = 0; i < navigator.mimeTypes.length; i++) {
         // A typical MIME/type looks like this:
         //
         //    "application/x-vmware-vmrc;version=2.5.0.252575"
         // or
         //    "application/x-vmware-remote-console-2012"
         //
         // split() will return the full string of no "version" is found,
         // thus it works in both cases.

         var mt = navigator.mimeTypes[i].type.split(";version")[0]
         if (mt == mimetype) {
            return true;
         }
      }
      return false;
   };

    removePluginObject = function(pluginId) {
       var plugin = document.getElementById(pluginId);
       if (plugin != null) {
          var parent = plugin.parentNode;
          parent.removeChild(plugin);
       }
    };

   isArray = function(obj) {
      return obj.constructor == Array;
   };
