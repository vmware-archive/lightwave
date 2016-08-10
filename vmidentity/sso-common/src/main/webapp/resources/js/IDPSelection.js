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

/*
 * JS util functions for idp discovery
 */

   var idpDisplayNameArray;
   var idpEntityIdArray;

   $(document).ready(function() {
	   idpDisplayNameArray = idp_display_name_list.substring(1, idp_display_name_list.length - 1).split(",");
	   idpEntityIdArray = idp_entity_id_list.substring(1, idp_entity_id_list.length - 1).split(",");
	   for (i = 0; i < idpDisplayNameArray.length; i++) {
		   $('#chooseIDPForm').append('<p>' + idpDisplayNameArray[i]);
		   $('#chooseIDPForm').append('<input id="submit" class="button blue" type="submit" value="Select" onclick="selectIDP(' + i + ')"/></p>');
	   }
   });

   function selectIDP(i) {
	   // calculate redirect URL
	   var url = document.URL;
	   var xml = new XMLHttpRequest();
	   // function to call after the request is completed
       xml.onreadystatechange = handleResponse;
	   xml.open('GET', url, true);
	   xml.setRequestHeader('Cache-Control', 'no-cache');
	   xml.setRequestHeader('Pragma', 'no-cache');
	   xml.setRequestHeader('CastleIDPSelection', idpEntityIdArray[i]);
	   xml.send(null);
   };

   // handle the response
   var handleResponse = function (evt) {
        var self = this;
        if (self.readyState == 4){
            var redirect_url = self.getResponseHeader('CastleIDPRedirect');
            if (redirect_url != null) {
                // redirect to the external idp url
                document.location = redirect_url;
             } else if (self.status == 200) {
                // redirect to the response url for local idp
                document.location = self.responseURL;
             }
        }
   };