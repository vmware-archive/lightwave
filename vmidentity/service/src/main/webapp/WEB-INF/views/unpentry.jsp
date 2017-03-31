<%--
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
--%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ page session="false" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">
<html class="base-app-style">
<!--[if lte IE 8]>
   <link rel="stylesheet" type="text/css" href="../../resources/css/loginIE8-7.css" />
 <![endif]-->
 <!--[if (gte IE 9)|!(IE)]><!-->
   <!--<link href="../../resources/css/login.css" rel="stylesheet"> -->
 <!--<![endif]-->
<head>
   <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
   <meta http-equiv="X-UA-Compatible" content="IE=5, IE=8, IE=10" >
   <title>Login</title>

   <script type="text/javascript">
    // copying JSP variables to JS
    var protocol="${protocol}";
    var responseMode="${responseMode}";
    var tenant_brandname="${tenant_brandname}";
    var tenant_logonbanner_title = '${tenant_logonbanner_title}'.trim();
    var tenant_logonbanner_content = '${tenant_logonbanner_content}'.trim();
    var logonBannerCheckboxEnabled = '${enable_logonbanner_checkbox}'.trim() == 'true' ? true : false;
    var logonBannerAlertMessage = '${logonBannerAlertMessage}';
    var searchString ="${searchstring}";
    var replaceString ='${replacestring}';
    var error = '${error}';
    var errorSSPI = '${errorSSPI}'
    var spn = "${spn}"
    var cac_endpoint = "${cac_endpoint}";
    var sso_endpoint = "${sso_endpoint}";

    var tlsclient_auth = '${enable_tlsclient_auth}';
    var password_auth = '${enable_password_auth}';
    var windows_auth = '${enable_windows_auth}';
    var rsa_am_auth = '${enable_rsaam_auth}';
    var rsaam_reminder = '${rsaam_reminder}';
    var rsaam_passcode_label = '${passcode}:';
    var password_label = '${password}:';
    var hint_label = '${username_hint}';
    var username_hint_placeholder = '${username_hint_placeholder}';
    var username_label = '${username}';
    var username_placeholder = '${username_placeholder}';

   </script>

   <script type="text/javascript" src="../../resources/js/assets/csd_api_common.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_connection.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_base.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_factory.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_config.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_logging.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_session.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_sspi.js"></script>
   <script type="text/javascript" src="../../resources/js/assets/csd_api_sso.js"></script>

   <script type="text/javascript" src="../../resources/js/Base64.js"></script>
   <script type="text/javascript" src="../../resources/js/VmrcPluginUtil.js"></script>
   <script type="text/javascript" src="../../resources/js/jquery-2.1.4.min.js"></script>
   <script type="text/javascript" src="../../resources/js/jquery-ui.min.js"></script>
   <script type="text/javascript" src="../../resources/js/websso.js"></script>

   <link rel="icon" type="image/x-icon" href="../../resources/img/favicon.ico" />
   <link rel="SHORTCUT ICON" href="../../resources/img/favicon.ico" />
   <link rel="stylesheet" type="text/css" href="../../resources/css/jquery-ui.min.css">
</head>
<body>

<script type="text/javascript">
    if (!isVCLogin()) {
        document.write('<link rel="stylesheet" type="text/css" href="../../resources/css/login_generic.css">');
    }
    else {
        document.write('<link rel="stylesheet" type="text/css" href="../../resources/css/login.css">');
    }
</script>


<script type="text/javascript">
//createProbes();
//createVmrcPluginObject();
//var _cspId = createCspPluginObject();

if (isVCLogin()) {
    document.write("<img id=\"topSplash\" src=\"../../resources/img/AppBgPattern.png\"/>");

    document.write("<img id=\"brand\" src=\"../../resources/img/vmwareLogoBigger.png\" />");
}
else {
    document.write("<p id=\"tenantBrand\">"+tenant_brandname+"</p>");
}


</script>
<div id="bg-banner"></div>
<div id="loginForm">
   <p id="usernameID" class="loginRow" >
      <span class="loginLabel">${username}:</span>
      <input id="username" class="margeTextInput" type="text" placeholder="${username_placeholder}"/>
    </p>
    <p id="passwordID" class="loginRow" >
      <span class="loginLabel">${password}:</span>
      <input id="password" class="margeTextInput" type="password" />
    </p>
    <p id="sessionID">
       <label id="checkboxLabel"><input id="sspiCheckbox" disabled="true" type="checkbox" onchange='enableSspi(this);'/>${winSession}</label>
    </p>
    <p id="smartcardID">
       <label id="checkboxLabel"><input id="smartcardCheckbox" disabled="false" type="checkbox" onchange='enableSmartcard(this);'/>${smartcard}</label>
    </p>
    <p id="rsaamID">
       <label id="checkboxLabel"><input id="rsaamCheckbox" disabled="false" type="checkbox" onchange='enableRsaam(this);'/>${rsaam}</label>
    </p>
    <p id="loginButtonRow">
       <input id="submit" class="button blue" type="submit" value=${login} onclick="submitentry()"/>
    </p>
    <p id="logonBannerID">
      <input id="logonBannerCheckbox" type="checkbox" onclick="isBannerChecked()"/>
      <span id="agreementMsg">${iAgreeTo}</span>
      <a id="logonBannerTitle" class="hyphenate" href="javascript:void(0);" onClick="displayLogonBannerDialog()">${tenant_logonbanner_title}</a>
    </p>
</div>

<div id="dialogLogonBanner"></div>

<div id="productName">
   <script type="text/javascript">
        if (isVCLogin()) {
           document.write("<img id=\"VCSSO-Title\" src=\"../../resources/img/VCSSO-title.png\" />");
           }
   </script>
   <div id="response" style="display:none"></div>
   <div id="progressBar" style="display:none"><img src="../../resources/img/Marge-anim-progressbar.gif"></div>
</div>

<div id="footer" class="footer">
    <span id="downloadCIPlinkBox" style="display:none">
       <a id="downloadCIPlink" target="_blank">${downloadCIP}</a>
    </span>
</div>
<div id="postForm"></div>

<div class="browser-validation-banner" style="visibility: hidden">
   <span class="validation-message-text">${unsupportedBrowserWarning}</span>
</div>

<script type="text/javascript">
   if (isVCLogin() && !isBrowserSupportedVC()) {
      $(".browser-validation-banner").css("visibility","visible");
   }
</script>
</body>
</html>
