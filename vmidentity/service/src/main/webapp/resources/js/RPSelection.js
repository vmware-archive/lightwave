/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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

/*
 * JS functions for Relying Party selection
 */

   var rpDisplayNameArray;
   var rpEntityIdArray;

   $(document).ready(function() {
	   rpDisplayNameArray = rp_display_name_list.substring(1, rp_display_name_list.length - 1).split(",");
	   rpEntityIdArray = rp_entity_id_list.substring(1, rp_entity_id_list.length - 1).split(",");
	   for (i = 0; i < rpDisplayNameArray.length; i++) {
		   $('#chooseRPForm').append('<p>' + rpEntityIdArray[i]);
		   $('#chooseRPForm').append('<input id="submit" class="button blue" type="submit" value="Select" onclick="selectRP(' + i + ')"/></p>');
	   }
   });

   function selectRP(i) {
	   // send a new 'POST' response with the content of the original from external IDP, attaching relying party selection header.
	   var url = document.URL;
	   var xml = new XMLHttpRequest();
	   var param = "SAMLResponse="+encodeURIComponent(saml_response);

       xml.onreadystatechange = handleResponse;
	   xml.open('POST', url, true);
	   xml.setRequestHeader('Cache-Control', 'no-cache');
	   xml.setRequestHeader('Pragma', 'no-cache');
	   xml.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	   xml.setRequestHeader('CastleRPSelection', rpEntityIdArray[i]);
	   xml.send(param);
   };

   // handle the response
   var handleResponse = function (evt) {
        var self = this;
        if (self.readyState == 4){
             if (self.status == 200) {
                 //Submit post response.
                 var chooseRPForm = document.getElementById('chooseRPForm');
                 chooseRPForm.style.display = 'none';
                 chooseRPForm.innerHTML = self.responseText;

                 var samlPostForm = document.getElementById('SamlPostForm');
                 if (samlPostForm != null) {
                    samlPostForm.submit();
                 } else {
                     doLog("did the sign-in to relying party fail?");
                 }
             }
        }
   };
