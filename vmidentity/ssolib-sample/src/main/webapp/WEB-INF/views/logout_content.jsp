<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ page session="false" %>
<html>
<head>
    <title>Logout content page</title>
</head>
<body>
<h1>
    ${user}: You are logged out.
</h1>

<p>Some content here. Press <a href="${logon}">this link to re-login</a></p>

</body>
</html>
