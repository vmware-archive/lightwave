<%--
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
--%>

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

   <script type="text/javascript" src="../../resources/js/Base64.js"></script>
   <script type="text/javascript" src="../../resources/js/jquery-2.1.4.min.js"></script>
   <script type="text/javascript" src="../../resources/js/IDPSelection.js"></script>

</head>
<body>
<script type="text/javascript">
    document.write('<link rel="stylesheet" type="text/css" href="../../resources/css/chooseidp.css">');
    document.write("<img id=\"topSplash\" src=\"../../resources/img/AppBgPattern.png\"/>");
    document.write("<img id=\"brand\" src=\"../../resources/img/vmwareLogoBigger.png\" />");
</script>

<div id="pageTitle">${title}</div>
<div id="response" style="display:none"></div>
<div id="chooseIDPForm"></div>
</body>
</html>