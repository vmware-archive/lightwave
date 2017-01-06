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

   // jaked using build number 2137170  for CIP_CLN = 2952709 we need to
   // update manually once CLN changes
   var CSP_BUILD_NUM = '2137170';
   var cipBuildVersion = '6.1.0';
   var _cspId = '';

   var _VersionStr = null;
   var _ishtml5LocalStorageSupported = null;
   var _isLogInitialized = false;
   var _xml = null;
   var userName = "";
   var _url = null;
   var _sspiCtxId = null;
   var _rsaSessionID = null;

   var api = {
      logging: {},
      activeTarget: {},
      session: {},
      sspi: {}
   }
   //snanda : the conn object will be used for all CSD operation
   var conn = new ApiConnection();

   // Enabling 'enter' on the submit button.
   $(document).keypress(function(e){
      if (e.which == 13){
         submitentry();
      }
   });

   // things to do when document is ready
   $(document).ready(function() {
	   // if both logon banner title and content are set, display logon banner on websso
	   if (isLogonBannerEnabled()) {
	       if (!logonBannerCheckboxEnabled) {
	           // hide checkbox and agreementMsg if checkbox is not enabled
	           $('#logonBannerCheckbox').hide();
	           $('#agreementMsg').hide();
	       }
	   } else {
		   $('#logonBannerID').hide();
	   }

      if (tlsclient_auth == "true") {
         $('#smartcardCheckbox').attr('disabled', false);

         //Remove username and pw widgets if both u/p and windows authentication are not available
         if (password_auth == "false" && windows_auth == "false" && rsa_am_auth == "false") {
            //disable username field if hint is not enabled
            if (hint_label.length == 0) {
                var usernameEle = document.getElementById("usernameID");
                usernameEle.parentNode.removeChild(usernameEle);
            }
            var passwordEle = document.getElementById("passwordID");
            passwordEle.parentNode.removeChild(passwordEle);
         }
         //default to use smartcard authn if availble.
         var smartcardEle = document.getElementById('smartcardCheckbox');
         smartcardEle.checked = true;
         enableSmartcard(smartcardEle);
      } else {
         var smartcardIDEle = document.getElementById("smartcardID");
         smartcardIDEle.parentNode.removeChild(smartcardIDEle);
         // Disable login button on page load unless smartcard authn is on
         $('#submit').prop('disabled',true);
      }

      //Remove windows session or smartcard authn checkbox if the corresponding authn type was not turned on
      if (windows_auth == "false") {
         var sspiEle = document.getElementById("sessionID");
         sspiEle.parentNode.removeChild(sspiEle);
      } else {
         //disbable username pw field when password authn is not available
         if (password_auth == "false" && rsa_am_auth == "false" && tlsclient_auth == "false") {
             $('#username').prop('disabled',true);
             $('#password').prop('disabled',true);
         }
      }

      if (rsa_am_auth == "false") {
          var rsaamIDEle = document.getElementById("rsaamID");
          rsaamIDEle.parentNode.removeChild(rsaamIDEle);
      } else {
          $('#rsaamCheckbox').attr('disabled', false);

          //default to select securID authentication if smartcard is not enabled.
          if (tlsclient_auth != "true") {
             var rsaCheckbox = document.getElementById('rsaamCheckbox');
             rsaCheckbox.checked = true;
             enableRsaam(rsaCheckbox);
             displayRsaamMessage(true);
          }
      }
      // Make sure document is ready before checking if cookies are enabled
      // and displaying the related error.
      if (!areCookiesEnabled()) {
         console.log('Failed to write cookie on document');
      }

      //on change of username enable login button
      if (password_auth == "true" || rsa_am_auth == "true") {
          $('#username').on('keyup keypress blur change', enableLoginButton);
      }
      // on check of sspi enable login button
      if (windows_auth == "true") {
          if (!isMac) {
              $('#sspiCheckbox').attr('disabled', false);
          }
          $('#sspiCheckbox').on('change', enableLoginButton);
      }
      //on change of smartcard enable login button
      if (tlsclient_auth == "true") {
          $('#smartcardCheckbox').on('change', enableLoginButton);
      }

      setCSDInstalled();

      //create the actual CSD object. this will also enable/disable
      //sspi depending upon whether the plugin call succeeds or not
      if (!isMac) {
         createCsdInstance();
      }
   });

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

   function handleApiResult(result, err) {
      var msg = result != null ? result : err;
      if (!msg) {
         console.log("Empty result message?");
         return;
      }
      var text = "Object Id: " + msg.requestObjectId + ", Request Id: " + msg.requestId;
      if (err) {
         text += ", Error occurred (" + err.errorCode + "): " + err.message;
      } else {
         text += ", Status (" + result.statusCode + "): " + result.result;
      }
      doLog("handleApiResult : " + text);
   }

   function onAppInit(result, err) {
     if (err) {
        handleApiResult(result, err);
        setCSDInstalled();
        return;
     }
     this._VersionStr = result["version"] + "." + result["build"];
     if (result["version"] != null) {
         cipBuildVersion = result["version"];
     }
     doLog("onAppInit : using CIP Build " + this._VersionStr);
     setCSDInstalled();
   }

   function onGetADUserName(result, err) {
     if (err) {
        handleApiResult(result, err);
        return;
     }
     this.userName = result.result;
     doLog("onGetADUserName : Username is " + this.userName);
     document.getElementById('username').value = this.userName;
     document.getElementById('username').disabled = true;
     document.getElementById('password').disabled = true;
   }

   function createCsdInstance() {
      if (conn.isOpen || conn.isOpenning) {
         return;
      }
      doLog("createCsdInstance : Opening the first connection to WebSocket server");
      conn.open();

      conn.onopen = function (evt) {
         console.log("CSD Plugin : Connection Open");
         api.session = new SessionApi(conn);
         api.config = new ConfigApi(conn);
         api.sspi = new SSPIApi(conn);
         //Initiate the logger. log files are kept
         // @%ProgramData%\VMware\vSphere Web Client\ui\sessions\... login.log
         ActivateLogger();

         api.session.init(
            {appName:"webSSO-NGC"},
            onAppInit
         );
      };

      conn.onerror = function(evt) {
         var message = evt == null ? "None" : evt.data;
         console.log("No Plugin Detected ... Connection error: " + message);
         doLog("No Plugin Detected ... Connection error: " + message);
         setCSDInstalled();
      };

      conn.onclose = function(evt) {
         var message = evt == null ? "None" : evt.data;
         console.log("Connection Closed: " + message);
         doLog("Connection Closed: " + message);
         setCSDInstalled();
      };
   }

   // if CIP is installed, writes to browser localStorage to set var "vmwCIPInstalled" to "true"
   // if CIP is not installed, set var to "false"
   var setCSDInstalled = function setCSDInstalled(){
      if (this._VersionStr != null) {
         $('#footer').html('');
         writeCSDInstalled(true);
      } else {
         if (!isMac) {
             var cspDownloadLink = createCompleteUrl();
             $('#downloadCIPlink').attr('href', cspDownloadLink);
             $('#downloadCIPlinkBox').show();
             writeCSDInstalled(false);
         }
      }
   };

   // get the CSD plugin extension depending on OS type
   var getCsdExtension = function getCsdExtension() {
      var os = navigator.appVersion;
      // Jaked TODO test all OS's return true/false
      if (os.indexOf('Mac') !== -1) {
         return 'mac64.dmg';
      } else if (os.indexOf('Linux')!==-1 || os.indexOf('X11')!==-1) {
         var platform = navigator.platform;
         if (platform.indexOf('x86_64') !== -1) {
            return 'x86_64.bundle';
         } else {
            return 'i386.bundle';
         }
      } else {
         return 'exe';
      }
   };

   // builds a complete url for the CIP plugin
   var createCompleteUrl = function createCompleteUrl() {
      var url = 'http://vsphereclient.vmware.com/vsphereclient/' +
            'VMware-ClientIntegrationPlugin-' +
            cipBuildVersion +
            '.' +
            getCsdExtension();
      return url ;
   };

   var isVCLogin =   function isVCLogin() {
      if (tenant_brandname == null || tenant_brandname == '') {
         return true;
      } else {
         return false;
      }
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
            var sspiEle = document.getElementById('sspiCheckbox');
            var smartcardEle = document.getElementById('smartcardCheckbox');
            if ( (userEle != null && $.trim($('#username').val()).length > 0)  ||
               (sspiEle != null && sspiEle.checked == true) ||
               (smartcardEle != null && smartcardEle.checked==true)) {
                  $('#submit').prop('disabled', false);
            } else {
               $('#submit').prop('disabled', true);
            }
         };

   // Status = true if you want the fields to be disabled.
   var disableFields = function disableFields(status) {
            var userEle = document.getElementById('username');
            var passwordEle = document.getElementById('password');
            if ( userEle != null && passwordEle != null) {
                if (password_auth == "true" || rsa_am_auth == "true") {
                   passwordEle.disabled = status;
                } else {
                   //Not allow to enter user name unless the pw authentication is available
                   passwordEle.disabled = true;
                }
            }
            disableUserNameEle(status);
            document.getElementById('submit').disabled = status;

            var sspiCheckboxEle = document.getElementById('sspiCheckbox');
            if (sspiCheckboxEle != null) {
                sspiCheckboxEle.disabled = status;
            }
            var smartcardEle = document.getElementById('smartcardCheckbox');
            if ( smartcardEle != null) {
                smartcardEle.disabled = status;
                if (smartcardEle.checked && passwordEle != null) {
                    passwordEle.disabled = true;
                }
            }
            var rsaamEle = document.getElementById('rsaamCheckbox');
            if ( rsaamEle != null) {
                rsaamEle.disabled = status;
            }
         };
   var disableUserNameEle = function disableUserNameEle(disable) {
       var userEle = document.getElementById('username');
       var smartcardEle = document.getElementById('smartcardCheckbox');

       if (userEle == null) {
           return;
       }
       if (disable == true) {
           userEle.disabled = true;
       } 
       //enable only if user ele is needed, i.e. when not using smartcard or when hint is enabled. 
       else if (smartcardEle == null || 
           (smartcardEle != null && !smartcardEle.checked ) ||
           (hint_label.length > 0 && smartcardEle != null && smartcardEle.checked) ) {
           userEle.disabled = false;
       }
   }

   var enableSspi = function enableSspi(cb) {
            // reset login guide text
            document.getElementById('response').innerHTML = '';

            var usernameField = document.getElementById('username');
            var passwordField = document.getElementById('password');
            if (cb.checked) {
               doLog("enableSspi : getting the userNamer for this logged on User");
               usernameField.disabled = true;
               passwordField.disabled = true;
               // Get the ad name,

               //uncheck smartcardCheckbox
               var smartcardCheckboxEle = document.getElementById('smartcardCheckbox');
               if (smartcardCheckboxEle != null) {
                   smartcardCheckboxEle.checked = false;
               }

               var rsaamCheckboxEle = document.getElementById('rsaamCheckbox');
               if (rsaamCheckboxEle != null) {
                   if (cb.checked) {
                       rsaamCheckboxEle.checked = false;
                   }
               }

            } else if (password_auth == "true" || rsa_am_auth == "true") {
               usernameField.disabled = false;
               passwordField.disabled = false;
            }
         };

     // return true if only smartcard authenticatin is supported
     var onlySmartcardEnabled = function onlySmartcardEnabled() {
              if (password_auth == "true" || rsa_am_auth == "true" || windows_auth == "true") {
                  return false;
              } else {
                  return true;
              }
           }
     var enableSmartcard = function enableSmartcard(cb) {
         // reset login guide text
         document.getElementById('response').innerHTML = '';

         var usernameField = document.getElementById('username');
         var passwordField = document.getElementById('password');

         if (cb.checked && hint_label.length > 0) {
             document.getElementById('usernameID').getElementsByTagName("span")[0].innerHTML = hint_label;
             usernameField.placeholder = username_hint_placeholder;
         } else {
             document.getElementById('usernameID').getElementsByTagName("span")[0].innerHTML = username_label;
             usernameField.placeholder = username_placeholder;
         }

         //keep it checked if this is the only authentication method.
         if (onlySmartcardEnabled()) {
             cb.checked = true;
         }

         if (usernameField != null && passwordField != null) {
             if (cb.checked) {
                 if (hint_label.length === 0) {
                     usernameField.disabled = true;
                 } else {
                     usernameField.disabled = false;
                 }
                 passwordField.disabled = true;
              } else if (password_auth == "true" || rsa_am_auth == "true") {
                 //usernameField should always be avail if cb is enabled
                 usernameField.disabled = false;
                 passwordField.disabled = false;
              }
         }

         //uncheck sspiCheckbox
         var sspiCheckboxEle = document.getElementById('sspiCheckbox');
         if (sspiCheckboxEle != null) {
             if (cb.checked) {
                 sspiCheckboxEle.checked = false;
             } else {
                 sspiCheckboxEle.disabled = false;
	     }
         }
         var rsaamCheckboxEle = document.getElementById('rsaamCheckbox');
         if (rsaamCheckboxEle != null) {
             if (cb.checked) {
                 rsaamCheckboxEle.checked = false;
             } else {
                 rsaamCheckboxEle.disabled = false;
             }
         }

      };

     var onlyRsaamEnabled = function onlySmartcardEnabled() {
              if (password_auth == "true" || tlsclient_auth == "true" || windows_auth == "true") {
                  return false;
              } else {
                  return true;
              }
           }

     var enableRsaam = function enableRsaam(cb) {
           var usernameField = document.getElementById('username');
           var passwordField = document.getElementById('password');

           //keep it checked if this is the only authentication method.
           if (onlyRsaamEnabled()) {
               cb.checked = true;
           }

           if (usernameField != null && passwordField != null) {
               if (cb.checked) {
                   usernameField.disabled = false;
                   passwordField.disabled = false;
                   document.getElementById("passwordID").getElementsByTagName("span")[0].innerHTML = rsaam_passcode_label;
                } else {
                   //password is the non-user definable lablel. so this is secure.
                   document.getElementById("passwordID").getElementsByTagName("span")[0].innerHTML = password_label;
                }
           }
           displayRsaamMessage(cb.checked? true:false);

           //uncheck or snartcard sspiCheckbox
           var sspiCheckboxEle = document.getElementById('sspiCheckbox');
           if (sspiCheckboxEle != null) {
               if (cb.checked) {
                   sspiCheckboxEle.checked = false;
               }
           }
           var smartcardCheckboxEle = document.getElementById('smartcardCheckbox');
           if (smartcardCheckboxEle != null) {
               if (cb.checked) {
                   smartcardCheckboxEle.checked = false;
               }
           }
        };

   var handleSspiError = function(stage, err) {
      document.getElementById('sspiCheckbox').disabled = false;
      document.getElementById('submit').disabled = false;
      document.getElementById('username').disabled = true;
      document.getElementById('password').disabled = true;
      document.getElementById('progressBar').style.display = 'none';
      document.getElementById('response').style.display = 'block';
      var errorMessage = errorSSPI;
      var logMsg = 'Client Integration Plugin error calling ' + stage;

      if (err && err.localized == "true") {
         errorMessage = err.message;
         logMsg += ": " + errorMessage;
      }
      document.getElementById('response').innerHTML = errorMessage;
      console.log(logMsg);
      doLog(logMsg);
      };

   var readyAcceptingRSANextCode = function readyAcceptingRSANextCode(self) {
           document.getElementById('rsaamCheckbox').disabled = false;
           document.getElementById('submit').disabled = false;
           document.getElementById('username').disabled = false;
           document.getElementById('password').disabled = false
           document.getElementById('progressBar').style.display = 'none';
           document.getElementById('response').style.display = 'block';
           var castleError = self.getResponseHeader('CastleError');
           response.innerHTML = castleError != null ? Base64.decode(castleError) : "Please submit the next passcode";

           console.log('Enter next passcode.');
           doLog("Enter next passcode.");
        };
   // handle the sso response
   var handleResponse = function (evt) {
            var self = this;
            var sspiCheckbox = document.getElementById('sspiCheckbox');
            var rsaamCheckbox = document.getElementById('rsaamCheckbox');
            var sspiLogin = sspiCheckbox != null && sspiCheckbox.checked;
            //var smartcardLogin = smartcardCheckbox != null && smartcardCheckbox.checked;
            var rsaamLogin = rsaamCheckbox != null && rsaamCheckbox.checked;

            if (self.readyState == 4){
               // process response
               var base64ServerToken = null;
               var sspiContextId = null;
               var rsaSessionID = null;

               if (self.status == 401) {
                  // Multiple leg authentication
                  var authHeader = self.getResponseHeader('CastleAuthorization');
                  if (authHeader != null) {
                     authHeaderParts = authHeader.split(' ');
                     if (rsaamLogin) {
                         // RSA AM NextCode mode, first leg will return 401 with rsa sessionID in header.
                         if (authHeaderParts.length == 2 && authHeaderParts[0] == 'RSAAM') {
                             rsaSessionID = authHeaderParts[1];
                         }
                     } else if (sspiLogin) {
                         // SSPI Negotiate is 2-legged, first leg will return 401
                         if (authHeaderParts.length == 3 && authHeaderParts[0] == 'Negotiate') {
                             sspiContextId = authHeaderParts[1];
                             base64ServerToken = authHeaderParts[2];
                         }
                     }
                  }
               }

               if (self.status == 302) {
                  // redirect back to original url
                  document.location = originalurl;
               } else if (rsaSessionID != null) {
                  // next code mode.
                  _rsaSessionID = rsaSessionID;
                  readyAcceptingRSANextCode(this);
               } else if (base64ServerToken != null && sspiContextId != null) {
                  var base64SspiToken = null;
                  try {
                     base64ServerToken = base64ServerToken.replace(/\r\n/g, '');
                     doLog("handleResponse : Proceeding for the second leg of SSPI Negotiation");
                     // snanda:2nd leg of Negotiate SSPI
                     api.sspi.negotiate( {inToken:base64ServerToken} , onNegotiateSSPI);
                  } catch (err) {
                     handleSspiError('negotiateSSPI', null);
                  }
               } else {
                  //all non second leg scenarios.
                  var response = document.getElementById('response');
                  var progressBar = document.getElementById('progressBar');
                  var castleError = null;

                  if (self.status == 200) {
                     if (protocol === 'openidconnect' && responseMode !== 'form_post') {
                        document.location = self.responseText;
                     } else {
                        var postForm = document.getElementById('postForm');
                        postForm.style.display = 'none';
                        postForm.innerHTML = self.responseText;
                     }
                  } else {
                     // display the result
                     response.style.display = 'block';
                     progressBar.style.display = 'none';
                     castleError = self.getResponseHeader('CastleError');
                     response.innerHTML = castleError != null ? Base64.decode(castleError) : self.statusText;
                     doLog("Error received during negotiation. Msg : [ " + response.innerHTML + " ]");
                     disableFields(false);
                  }

                  if (!(protocol === 'openidconnect' && responseMode !== 'form_post')) {
                     // if SamlPostForm is present, submit it
                     var samlPostForm = document.getElementById('SamlPostForm');

                     if (samlPostForm != null) {
                        samlPostForm.submit();
                     } else {
                        // Re-enable everything since the user will have to attempt
                        // logging in again.
                        progressBar.style.display = 'none';
                        disableFields(false);
                        //snanda: also uncheck the SSPIcheckbox since everything is enabled now

                        if (document.getElementById('sspiCheckbox')!= null)
                            document.getElementById('sspiCheckbox').checked = false;
                        //give a generic error
                        response.style.display = 'block';
                        if (castleError == null) {
                           response.innerHTML = error;
                        }
                        doLog("did the login fail? if using SSPI - ensure the logged in user can login to the SSO service");
                     }
                  }
               }
            }
   };

   function displayRsaamMessage(messageOn) {
       var messageEle = document.getElementById('response');
       if (messageOn == true) {
           response.style.display = 'block';
           response.innerHTML = rsaam_reminder;
       } else {
           response.style.display = 'none';
       }
   }

   function generatateSspiCxtId() {
	   return Math.floor(1000000000 + Math.random() * 9000000000);
   }

   function onNegotiateSSPI(result, err) {
      if (result == null || err != null || _sspiCtxId == null) {
         handleSspiError('negotiateSSPI', err);
         return;
      }
      var base64SspiToken = result.result;
      if (base64SspiToken == null) {
         doLog("OnNegotiateSSPI: [Empty Token]");
         handleSspiError('negotiateSSPI', null);
         return;
      }
      base64SspiToken = base64SspiToken.replace(/\r\n/g, '');
      var params = 'CastleAuthorization=' + encodeURIComponent('Negotiate ' + _sspiCtxId + ' ' + base64SspiToken);
      var xml = new XMLHttpRequest();
      xml.onreadystatechange = handleResponse;
      xml.open('POST', document.URL.replace(searchString, replaceString), true);
      // disable http caching
      xml.setRequestHeader('Cache-Control', 'no-cache');
      xml.setRequestHeader('Pragma', 'no-cache');
      xml.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xml.send(params);
      doLog("OnNegotiateSSPI : Logging in now using final SSPI negotiation");
   }

   var submitentry = function submitentry() {
      // calculate redirect URL
      var originalurl;
      originalurl = document.URL;
      _url = originalurl.replace(searchString, replaceString);

      // get the field values
      var submit = document.getElementById('submit');
      var sspiCheckbox = document.getElementById('sspiCheckbox');
      var smartcardCheckbox = document.getElementById('smartcardCheckbox');
      var rsaamCheckbox = document.getElementById('rsaamCheckbox');
      var progressBar = document.getElementById('progressBar');
      var sspiLogin = sspiCheckbox != null && sspiCheckbox.checked;
      var smartcardLogin = smartcardCheckbox != null && smartcardCheckbox.checked;
      var rsaamLogin = rsaamCheckbox != null && rsaamCheckbox.checked;
      var username = (document.getElementById('username') == null)? '': $.trim(document.getElementById('username').value);
      var password = (document.getElementById('password') == null)? '': document.getElementById('password').value;
      doLog("Login started for user : " + username);
      // Note: it is perfectly fine for the password field to be empty.
      if (username != '' || sspiLogin || smartcardLogin) {
        if (isLogonBannerEnabled() && logonBannerCheckboxEnabled && !isBannerChecked()) {
            return;
        }

        if ( smartcardLogin ) {
            _url = _url.replace(sso_endpoint, cac_endpoint);
        }

         // Display progress bar
         progressBar.style.display = 'block';
         var response = document.getElementById('response');
         response.style.display = 'none';
         // create a request
         var xml = new XMLHttpRequest();
         // function to call after the request is completed
         xml.onreadystatechange = handleResponse;
         xml.open('POST', _url, true);
         // Disable the fields.
         disableFields(true);
         _xml = null;
         if (DetectPlugin() == "true" && sspiLogin) {
            doLog("Using CIP Windows SSPI Authentication to login. spn is : [ " + spn + " ]");
            _xml = xml;
            try {
               api.sspi.getADUserName({}, onGetADUserName);
               // 1st leg of Negotiate SSP
               api.sspi.initialize( {providerName: 'Negotiate', target: spn}, onInitializeSSPI);
            } catch (err) {
               handleSspiError('initializeSSPI', null);
            }
            return;
         }
         doLog("Using username password to login");
         unp = username + ':' + password;
         unp = Base64.encode(unp);
         var params = 'CastleAuthorization=';

         //temp solution allowing smartcard authentication test.
         var authType = '';   //default

          if (tlsclient_auth == "true" && document.getElementById('smartcardCheckbox').checked == true) {
             authType = 'TLSClient ' + unp;
          } else if (rsaamLogin) {
             if (_rsaSessionID != null) {
                authType = "RSAAM " + _rsaSessionID+ " "+ unp;
                _rsaSessionID = null;
             } else {
                authType = "RSAAM " + unp;
             }
          } else if (sspiLogin) {
             doLog("Using browser IWA to login.");
             var sspiContextId = generatateSspiCxtId();
             authType = 'Negotiate ' + sspiContextId;
          } else if (password_auth == "true") {
             authType = 'Basic ' + unp;
          }
          params += encodeURIComponent(authType);

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

   function onInitializeSSPI(result, err) {
      var xml = _xml;
      if (err != null || result == null || xml == null) {
         handleSspiError('initializeSSPI', err);
         return;
      }
      var base64SspiToken = result.result;
      if (base64SspiToken == null) {
         doLog("OnInitializeSSPI: [Empty Token]");
         handleSspiError('initializeSSPI', null);
         return;
      }
      base64SspiToken = base64SspiToken.replace(/\r\n/g, '');
      // generate a 10-digit random number
      var sspiContextId = generatateSspiCxtId();
      _sspiCtxId = sspiContextId;
      var params = 'CastleAuthorization=' + encodeURIComponent('Negotiate ' + sspiContextId + ' ' + base64SspiToken);
      // disable http caching
      xml.setRequestHeader('Cache-Control', 'no-cache');
      xml.setRequestHeader('Pragma', 'no-cache');
      xml.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xml.send(params);
   }

   function DetectPlugin() {
      //access localStorage to check if there's a var set
      var isPluginInstalled = getCIPInstalled();
      //no entry yet - assume not installed

      if (isPluginInstalled == null ) {
         return false;
      }
      //entry is there
      return isPluginInstalled;
   }

   function getCIPInstalled() {
      if (html5_localstorage_supported() == false) {
         return false;
      }
      return localStorage.getItem("vmwCIPInstalled");
   }

   function writeCSDInstalled(isInstalled) {
      if (html5_localstorage_supported() == false) {
         return;
      }
      localStorage.setItem("vmwCIPInstalled", isInstalled);
   }


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

   function ActivateLogger () {
      if (api.logTarget) {
         api.logTarget.close();
         api.logTarget = null;
      }
      api.logTarget = new LoggingTargetApi(conn);
      var config = {
         targetName:"login",
         logFileSize: 20000,
         maxLogFiles: 10,
         logTime:"true"
      };
      api.logTarget.setConfig(config, function (result, err) {
         if (err) {
            console.log("Error creating logging target (" + err.errorCode +  "): " + err.message);
            api.logTarget = null;
            return;
         }
         if (api.logTarget != null) {
            console.log("Log Target Activated - (object id = " + api.logTarget.objectId + ")");
            _isLogInitialized = true;
            doLog("Log initialized for websso login");
         }
      });
   }

   function doLog(strLog) {
      if (_isLogInitialized == false) {
         return;
      }
      api.logTarget.log({line:strLog});
   }

   var isBannerChecked = function isBannerChecked() {
       var cb = document.getElementById('logonBannerCheckbox');
       var response = document.getElementById('response');
       response.style.display = 'block';
       var alertMsg = logonBannerAlertMessage + tenant_logonbanner_title;
       if (cb && cb.checked) {
           response.innerHTML = '';
           return true;
	   } else {
		   response.innerHTML = alertMsg;
	       return false;
	   }
   }

   var isEmptyString = function isEmptyString(data) {
	   // checks for null, undefined, '' and ""
       if (!data) {
           return true;
       };
	   return data.length === 0;
   }

   var isLogonBannerEnabled = function isLogonBannerEnabled() {
	   return !isEmptyString(tenant_logonbanner_title) && !isEmptyString(tenant_logonbanner_content)
   }

   function displayLogonBannerDialog() {
       $('#dialogLogonBanner').html('<p class="title">' + tenant_logonbanner_title + '</p>'
               + '<pre class="hyphenate">' + tenant_logonbanner_content + '</pre>');
       $('#dialogLogonBanner').dialog(
              {
                   width: 650,
                   height: 375,
                   modal: true,
                   draggable: false
              }
       );
       $('.ui-dialog-titlebar').html('<span class="close-button"><img src="../../resources/img/close.png" /></span>');
       $('.close-button').click(function() {
           $('#dialogLogonBanner').dialog( "close" );
       });
   }
