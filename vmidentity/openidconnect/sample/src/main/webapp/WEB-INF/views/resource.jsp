<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@taglib uri="http://www.springframework.org/tags/form" prefix="form"%>
<html>
<head>
    <title>Client UI</title>
</head>

<body>
    <p>Client (RelyingParty)</p>
    <p>client_id: ${clientId}</p>
    <p>${displayText}</p>

    <c:choose>
        <c:when test="${loggedIn}">
            <form:form id="command" method="POST" action="/openidconnect-sample-rp/logout_redirect">
                <input type="submit" STYLE="background-color:#DF7401" value="log out using redirect"/>
            </form:form>
            <form:form id="command" method="POST" action="/openidconnect-sample-rp/logout_form_post">
                <input type="submit" STYLE="background-color:#DF7401" value="log out using form post"/>
            </form:form>
            <form:form id="tokens">
                <input type="hidden" name="id_token" value="${id_token}"/>
                <input type="hidden" name="access_token" value="${access_token}"/>
                <input type="hidden" name="refresh_token" value="${refresh_token}"/>
            </form:form>
        </c:when>
        <c:otherwise>
            <form:form method="POST" action="/openidconnect-sample-rp/login_authz_code_flow_form_response">
                <input type="submit" STYLE="background-color:#088A08" value="log in using authz code flow form_post response mode"/>
            </form:form>
            <form:form method="POST" action="/openidconnect-sample-rp/login_authz_code_flow_query_response">
                <input type="submit" STYLE="background-color:#088A08" value="log in using authz code flow query response mode"/>
            </form:form>
            <form:form method="POST" action="/openidconnect-sample-rp/login_implicit_flow_form_response">
                <input type="submit" STYLE="background-color:#088A08" value="log in using implicit flow form_post response mode"/>
            </form:form>
            <form:form method="POST" action="/openidconnect-sample-rp/login_implicit_flow_fragment_response">
                <input type="submit" STYLE="background-color:#088A08" value="log in using implicit flow fragment response mode"/>
            </form:form>
        </c:otherwise>
    </c:choose>
</body>
</html>