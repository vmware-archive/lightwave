<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ page session="false" %>
<html>
<head>
	<title>Logout Response</title>
</head>
<body>


<h1>
	Press Submit button below to send logout success response  
</h1>
<div style="width:800px;height:100px;overflow:auto">
<P>  SAML Request is ${SAMLRequest}. </P>
</div>
<div style="overflow:auto;">
<P>  Decoded SAML Request: </P>
<PRE>${DecodedSAMLRequest}</PRE>
</div>
<c:if test="${RelayState != null}">
    <P>  Relay State is ${RelayState}. </P>
    <P>  Decoded Relay State is ${DecodedRelayState}. </P>
</c:if>
<br /><br /><input id="submit"  type="submit" value="Submit" onclick="document.location = '${redirecturl}'"/>
</body>
</html>
