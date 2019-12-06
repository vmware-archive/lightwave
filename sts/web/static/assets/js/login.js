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
 * JS util functions for websso
 */

// namespace to prevent polluting the world

// -------------vars --------------------
   var isMac = (navigator.userAgent.indexOf('Mac OS X') !== -1);

   var _VersionStr = null;
   var _ishtml5LocalStorageSupported = null;
   var _xml = null;
   var userName = "";
   var _url = null;

   // Enabling 'enter' on the submit button.
//   $(document).keypress(function(e){
//      if (e.which == 13){
//         submitentry();
//      }
//   });

   // things to do when document is ready
   var docReady = function docReady(){

      // Make sure document is ready before checking if cookies are enabled
      // and displaying the related error.
      if (!areCookiesEnabled()) {
         console.log('Failed to write cookie on document');
      }

      enableLoginButton();
   };

   // Validation that checks if the browser and OS are supported.
   // At the time of writing a minimum of IE10, Firefox 34 or
   // Chrome 39 are required on Windows. A minimum of Firefox 34
   // or Chrome 39 are required on Mac OS X.
   var isBrowserSupportedVC = function isBrowserSupportedVC(){
      var chromeReg = /Mozilla\/.*? \((Windows|Macintosh)(.*?) AppleWebKit\/(\d.*?).*?Chrome\/(.*?) Safari\/(.*)/i;
      var CHROME_VERSION_INDEX = 4;
      var ieReg = /Mozilla\/(.*?) \((compatible|Windows;.*?); (MSIE) ([0-9]*?)\.([0-9]*?);? (.*?)?;? ?(.*?)*\) ?( .*?)?/i;
      var IE_VERSION_INDEX = 4;
      var ie11Reg = /Trident\/.*rv:([0-9]{1,}[\.0-9]{0,})/i;
      var IE11_VERSION_INDEX = 1;
      var firefoxReg = /Mozilla\/(.*?) \(.*?(Windows|Macintosh)(.*?) Gecko\/(\d.*?) ((\w.*)\/(\d[^ ]*))?/i;
      var FF_VERSION_INDEX = 7;
      var usrAgent = navigator.userAgent;
      var result;
      if ((result = chromeReg.exec(usrAgent)) !== null) {
         if (result[CHROME_VERSION_INDEX].split(".")[0] >= 39) {
            return true;
         }
      }
      if ((result = ieReg.exec(usrAgent)) !== null) {
         if (result[IE_VERSION_INDEX] >= 10) {
            return true;
         }
      }
      if ((result = ie11Reg.exec(usrAgent)) !== null) {
         if (result[IE11_VERSION_INDEX] >= 11) {
            return true;
         }
      }
      if ((result = firefoxReg.exec(usrAgent)) !== null) {
         if (result[FF_VERSION_INDEX] >= 34) {
            return true;
         }
      }
      return false;
   };

   //-------------- Cookies!!
   // create a Cookie
   var createCookie =    function createCookie(name, value, days) {
            var expires;
            if (days) {
               var date = new Date();
               date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
               expires = '; expires=' + date.toGMTString();
            } else {
               expires = '';
            }
            document.cookie = name + '=' + value + expires + '; path=/';
        };

   var readCookie =   function readCookie(name) {
           var nameEQ = name + '=';
           var ca = document.cookie.split(';');
           for (var i = 0; i < ca.length; i++) {
               var c = ca[i];
               while (c.charAt(0) == ' ') c = c.substring(1, c.length);
               if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length, c.length);
           }
           return null;
        };

   var eraseCookie =    function eraseCookie(name) {
           createCookie(name, '', -1);
       };

   var areCookiesEnabled =   function areCookiesEnabled() {
            var r = false;
            createCookie('LoginTest', 'HelloWorld', 1);
            if (readCookie('LoginTest') != null) {
               r = true;
               eraseCookie('LoginTest');
            }
            return r;
         };

   var enableLoginButton = function enableLoginButton() {

            var userEle = document.getElementById('username');

            if ( (userEle != null && userEle.value != null && userEle.value.trim().length > 0)) {
                document.getElementById('submit').disabled = false;
            } else {
                document.getElementById('submit').disabled = true;
            }
         };

   // Status = true if you want the fields to be disabled.
   var disableFields = function disableFields(status) {
    var userEle = document.getElementById('username');
    var passwordEle = document.getElementById('password');
    if ( passwordEle != null) {
           passwordEle.disabled = status;
    }
    if (userEle != null){
        userEle.disabled = status;
    }
    document.getElementById('submit').disabled = status;
   };
   // handle the sso response
   var handleResponse = function (evt) {
            var self = this;
            if (self.readyState == 4){
               // process response

               if (self.status == 302) {
                  // redirect back to original url
                  document.location = originalurl;
               } else {
                  //all non second leg scenarios.
                  var response = document.getElementById('response');
                  var progressBar = document.getElementById('progressBar');
                  var serverError = null;

                  if (self.status == 200) {
                     if (protocol === 'openidconnect' && responseMode !== 'form_post') {
                        document.location = self.responseText;
                     } else {
                        var postFormContainer = document.getElementById('postFormContainer');
                        postFormContainer.style.display = 'none';
                        postFormContainer.innerHTML = self.responseText;
                     }
                  } else {
                     // display the result
                     response.style.display = 'block';
                     progressBar.style.display = 'none';
                     var errObj = JSON.parse(self.responseText);
                     serverError = errObj != null ? errObj.error_description : null;
                     response.textContent = serverError != null ? serverError : self.statusText;
                     doLog("Error received from server. Msg : [ " + response.textContent + " ]");
                     disableFields(false);
                  }

                  if (!(protocol === 'openidconnect' && responseMode !== 'form_post')) {
                     // if responsePostForm is present, submit it
                     var responsePostFormElem = document.getElementById(responsePostForm);

                     if (responsePostFormElem != null) {
                        responsePostFormElem.submit();
                     } else {
                        // Re-enable everything since the user will have to attempt
                        // logging in again.
                        progressBar.style.display = 'none';
                        disableFields(false);

                        //give a generic error
                        response.style.display = 'block';
                        if (serverError == null) {
                           response.innerHTML = genericError + self.statusText;
                        }
                        doLog("did the login fail? if using SSPI - ensure the logged in user can login to the SSO service");
                     }
                  }
               }
            }
   };

   var submitentry = function submitentry() {
      // calculate redirect URL
      var originalurl;
      originalurl = document.URL;
      _url = originalurl;

      // get the field values
      var submit = document.getElementById('submit');
      var progressBar = document.getElementById('progressBar');
      var unameElement = document.getElementById('username');
      var username = (unameElement == null)? '' : (unameElement.value == null) ? '' : unameElement.value.trim();
      var password = (document.getElementById('password') == null)? '': document.getElementById('password').value;
      doLog("Login started for user : " + username);
      // Note: it is perfectly fine for the password field to be empty.
      if (username != '') {

         // Display progress bar
         var response = document.getElementById('response');
         response.style.display = 'none';
         progressBar.style.display = 'flex';
         // create a request
         var xml = new XMLHttpRequest();
         // function to call after the request is completed
         xml.onreadystatechange = handleResponse;
         xml.open('POST', _url, true);
         // Disable the fields.
         disableFields(true);
         _xml = null;
         doLog("Using username password to login");
         //if (protocol === 'openidconnect' ){
         params = "grant_type=password"
         params = params + "&username=" + encodeURIComponent(username)
         params = params + "&password=" + encodeURIComponent(password)
         //}

          // disable http caching
          xml.setRequestHeader('Cache-Control', 'no-cache');
          xml.setRequestHeader('Pragma', 'no-cache');
          xml.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
          // send request
          xml.send(params);
      } else {
         doLog("Error : Not ready to login");
      }
   };

   function html5_localstorage_supported () {
      if (_ishtml5LocalStorageSupported != null) {
         return _ishtml5LocalStorageSupported;
      }

      try {
         _ishtml5LocalStorageSupported = 'localStorage' in window && window['localStorage'] !== null;
         return _ishtml5LocalStorageSupported;
      } catch (e) {
         return false;
      }
   }

   function doLog(strLog) {
      console.log(strLog)
   }

   var isEmptyString = function isEmptyString(data) {
	   // checks for null, undefined, '' and ""
       if (!data) {
           return true;
       };
	   return data.length === 0;
   }
