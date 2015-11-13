<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ page session="false" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">
<html class="base-app-style">
<head>
   <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
   <meta http-equiv="X-UA-Compatible" content="IE=5, IE=8, IE=10" >
   <title>Choose Identity Provider</title>

   <script type="text/javascript">
   // copying JSP variables to JS
   var protocol = '${protocol}';
   var tenant = '${tenant}';
   var tenant_brandname = '${tenant_brandname}';
   var idp_entity_id_list = '${idp_entity_id_list}';
   var idp_display_name_list = '${idp_display_name_list}';
   </script>

   <script type="text/javascript" src="../../resources/js/jquery-2.1.4.min.js"></script>
   <script type="text/javascript" src="../../resources/js/IDPSelection.js"></script>

</head>
<body>
<script type="text/javascript">
    // regex to check for internet explorer 11 and below
    var isInternetExplorer = /MSIE (\d+\.\d+);/.test(navigator.userAgent) || /Trident\/(\d+\.\d+);.*rv:(\d+\.\d+)/.test(navigator.userAgent);

    if (!isVCLogin()) {
        document.write('<link rel="stylesheet" type="text/css" href="../../resources/css/chooseidp_generic.css">');
    }
    else {
        document.write('<link rel="stylesheet" type="text/css" href="../../resources/css/chooseidp.css">');
    }
</script>

<script type="text/javascript">
if (isVCLogin()) {
    document.write("<img id=\"topSplash\" src=\"../../resources/img/AppBgPattern.png\"/>");

    document.write("<img id=\"brand\" src=\"../../resources/img/vmwareLogoBigger.png\" />");
}
else {
    document.write("<p id=\"tenantBrand\">" + tenant_brandname + "</p>");
}
</script>

<div id="pageTitle">${title}</div>
<div id="response" style="display:none"></div>

<div id="chooseIDPForm"></div>

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