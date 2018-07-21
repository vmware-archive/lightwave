<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://www.springframework.org/tags/form" prefix="form" %>
<%@ page session="false" %>
<html>
<head>
    <title>Import IDP Metadata</title>
</head>
<body>
<h1>
    Import IDP Metadata for ${tenant}
</h1>

<form:form method="put" action="" modelAttribute="importForm">
    <br />message<br /><form:input path="message" disabled="true" size="130"/>
    <br /><br />metadata<br /><form:textarea path="metadata" rows="40" cols="80" />
    <br /><br /><input type="submit" value="Submit">
</form:form>

</body>
</html>
