<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ page session="false" %>
<html>
<head>
    <title>Content page</title>
</head>
<body>
<h1>
    Welcome ${user}
</h1>

<p>Some content here. Press <a href="${logout}">this link to logout</a></p>

</body>
</html>
