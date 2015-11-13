<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://www.springframework.org/tags/form" prefix="form" %>
<%@ page session="false" %>
<html>
<head>
	<title>Create your own AuthnRequest</title>
</head>
<body>
<h1>
	Create your own AuthnRequest 
</h1>

<form:form method="put" action="" modelAttribute="authnRequestForm">
	<br />id<br /><form:input path="id" size="130"/>
	<br /><br />destination<br /><form:input path="destination" size="130"/>
	<br /><br />issuer<br /><form:input path="issuer" size="130"/>
	<br /><br />providerName<br /><form:input path="providerName" size="130"/>
	<br /><br />protocolBinding<br /><form:input path="protocolBinding" size="130"/>
	<br /><br />assertionConsumerServiceIndex<br /><form:input path="assertionConsumerServiceIndex" size="130"/>
	<br /><br />assertionConsumerServiceUrl<br /><form:input path="assertionConsumerServiceUrl" size="130"/>
	<br /><br />forceAuthn<br /><form:input path="forceAuthn" size="130"/>
	<br /><br />isPassive<br /><form:input path="isPassive" size="130"/>
	<br /><br />nameIdPolicyFormat<br /><form:input path="nameIdPolicyFormat" size="130"/>
	<br /><br />nameIdPolicySPNameQualifier<br /><form:input path="nameIdPolicySPNameQualifier" size="130"/>
	<br /><br />namedIdPolicyAllowCreate<br /><form:input path="namedIdPolicyAllowCreate" size="130"/>
	<br /><br />relayState<br /><form:input path="relayState" size="130"/>
	<br /><br />sigAlg<br /><form:input path="sigAlg" size="130"/>
	<br /><br />signature<br /><form:input path="signature" size="130"/>
	<br /><br /><input type="submit" value="Submit">
</form:form>

</body>
</html>
