<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ page session="false" %>
<html>
<head>
	<title>SSO POST DEMO</title>
</head>
<body>
<h1>
	Hello<c:if test="${NameIDValue != null}"> ${NameIDValue}</c:if>!  
</h1>

<div style="width:800px;height:100px;overflow:auto">
<P>  SAML Response is ${SAMLResponse}. </P>
</div>
<div style="overflow:auto;">
<P>  Decoded SAML Response: </P>
<PRE>${DecodedSAMLResponse}</PRE>
</div>
<c:if test="${RelayState != null}">
	<P>  Relay State is ${RelayState}. </P>
	<P>  Decoded Relay State is ${DecodedRelayState}. </P>
</c:if>
</body>
</html>
