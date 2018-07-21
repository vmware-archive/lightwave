<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://www.springframework.org/tags/form" prefix="form" %>
<%@ page session="false" %>
<html>
<head>
    <title>Export Metadata to IDP</title>
</head>
<body>
<h1>
    Export Metadata for ${tenant}
</h1>

<form:form method="put" action="" modelAttribute="exportForm">
	<br />Message<br /><form:input path="message" disabled="true" size="130"/>
    <br />Identity Provider FQDN <br /><form:input path="identityProviderFQDN" size="130"/>
    <br /><br />metadata<br /><form:textarea path="metadata" rows="40" cols="80" />
    <br /><br /><input type="submit" value="Submit">
</form:form>

</body>
</html>
