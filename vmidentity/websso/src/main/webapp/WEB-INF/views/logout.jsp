<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://www.springframework.org/tags/form" prefix="form" %>
<%@ page session="false" %>
<html>
<head>
    <title>Create your own LogoutRequest</title>
</head>
<body>
<h1>
    Create your own LogoutRequest 
</h1>

<form:form method="put" action="" modelAttribute="logoutRequestForm">
    <br />id<br /><form:input path="id" size="130"/>
    <br /><br />destination<br /><form:input path="destination" size="130"/>
    <br /><br />issuer<br /><form:input path="issuer" size="130"/>
    <br /><br />nameIdFormat<br /><form:input path="nameIdFormat" size="130"/>
    <br /><br />nameId<br /><form:input path="nameId" size="130"/>
    <br /><br />sessionIndex<br /><form:input path="sessionIndex" size="130"/>
    <br /><br />relayState<br /><form:input path="relayState" size="130"/>
    <br /><br />sigAlg<br /><form:input path="sigAlg" size="130"/>
    <br /><br />signature<br /><form:input path="signature" size="130"/>
    <br /><br /><input type="submit" value="Submit">
</form:form>

</body>
</html>
