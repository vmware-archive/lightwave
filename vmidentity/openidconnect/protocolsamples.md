=======================================================================================
\[Authorization Code Flow\]
=======================================================================================
Authn Request GET /openidconnect/oidc/authorize? response\_type=code&
response\_mode=form\_post& client\_id=*client\_id\_xyz*&
redirect\_uri=https://client.example.com/cb& state=*state\_xyz*&
nonce=*nonce\_xyz*& scope=openid id\_groups at\_groups
rs\_admin\_server& client\_assertion=<client_assertion> Host:
server.example.com

Authn Response HTTP/1.1 200 OK Content-Type: text/html;charset=UTF-8
Cache-Control: no-cache, no-store Pragma: no-cache

    <html>
        <head>
            <script language="JavaScript" type="text/javascript">
                function load(){ document.getElementById('SamlPostForm').submit(); }
            </script>
        </head>
        <body onload="load()">
            <form method="post" id="SamlPostForm" action="https://client.example.com/cb">
                <input type="hidden" name="state" value="_state_xyz_" />
                <input type="hidden" name="code" value="_authz_code_xyz_" />
                <input type="submit" value="Submit" style="position:absolute; left:-9999px; width:1px; height:1px;" />
            </form>
        </body>
    </html>

Token Request POST /openidconnect/token HTTP/1.1 Host:
server.example.com Content-Type: application/x-www-form-urlencoded

    grant_type=authorization_code&
    code=_authz_code_xyz_&
    redirect_uri=https://client.example.com/cb&
    client_assertion_type=urn:ietf:params:oauth:client-assertion-type:jwt-bearer&
    client_assertion=<client_assertion>

Token Response HTTP/1.1 200 OK Content-Type:
application/json;charset=UTF-8 Cache-Control: no-store Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"hotk-pk",
        "expires_in":43200,
        "id_token":"<id_token>",
        "refresh_token":"<refresh_token>"
    }

where

    <id_token>=
    {
        "token_class"="id_token",
        "token_type"="hotk-pk",
        "hotk"=<jwks>,
        "aud"="_client_id_xyz_",
        "groups"=["groupA","groupB"]
        ...
    }

    <access_token>=
    {
        "token_class"="access_token",
        "token_type"="hotk-pk",
        "hotk"=<jwks>,
        "aud"=["_client_id_xyz_","rs_admin_server"],
        "groups"=["groupA","groupB"],
        "admin_server_role"="ConfigurationUser"
        ...
    }

    <refresh_token>=
    {
        "token_class"="refresh_token",
        "token_type"="hotk-pk",
        "hotk"=<jwks>,
        "aud"="_client_id_xyz_",
        ...
    }

=======================================================================================
\[Implicit Flow\]
=======================================================================================
Authn Request GET /openidconnect/oidc/authorize?
response\_type=id\_token& response\_mode=form\_post&
client\_id=*client\_id\_xyz*&
redirect\_uri=https://client.example.com/cb& state=*state\_xyz*&
nonce=*nonce\_xyz*& scope=openid HTTP/1.1 Host: server.example.com

Authn Response HTTP/1.1 200 OK Content-Type: text/html;charset=UTF-8
Cache-Control: no-cache, no-store Pragma: no-cache

    <html>
        <head>
            <script language="JavaScript" type="text/javascript">
                function load(){ document.getElementById('SamlPostForm').submit(); }
            </script>
        </head>
        <body onload="load()">
            <form method="post" id="SamlPostForm" action="https://client.example.com/cb">
                <input type="hidden" name="state" value="_state_xyz_" />
                <input type="hidden" name="id_token" value="<id_token>" />
                <input type="submit" value="Submit" style="position:absolute; left:-9999px; width:1px; height:1px;" />
            </form>
        </body>
    </html>

=======================================================================================
\[Resource Owner Password Credentials Flow\]
=======================================================================================
Token Request POST /openidconnect/token HTTP/1.1 Host:
server.example.com Content-Type: application/x-www-form-urlencoded

    grant_type=password&
    username=_username_xyz_&
    password=_password_xyz_&
    scope=openid offline_access

Token Response HTTP/1.1 200 OK Content-Type:
application/json;charset=UTF-8 Cache-Control: no-store Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"Bearer",
        "expires_in":3600,
        "id_token":"<id_token>",
        "refresh_token":"<refresh_token>"
    }

=======================================================================================
\[Client Credentials Flow\]
=======================================================================================
Token Request POST /openidconnect/token HTTP/1.1 Host:
server.example.com Content-Type: application/x-www-form-urlencoded

    grant_type=client_credentials&
    client_assertion_type=urn:ietf:params:oauth:client-assertion-type:jwt-bearer&
    client_assertion=<jwt>&
    scope=openid

Token Response HTTP/1.1 200 OK Content-Type:
application/json;charset=UTF-8 Cache-Control: no-store Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"hotk-pk",
        "expires_in":43200,
        "id_token":"<id_token>"
    }

=======================================================================================
\[Refresh Token Flow\]
=======================================================================================
Token Request POST /openidconnect/token HTTP/1.1 Host:
server.example.com Content-Type: application/x-www-form-urlencoded

    grant_type=refresh_token&
    refresh_token=<refresh_token>

Token Response HTTP/1.1 200 OK Content-Type:
application/json;charset=UTF-8 Cache-Control: no-store Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"Bearer",
        "expires_in":3600,
        "id_token":"<id_token>"
    }

=======================================================================================
extension grant\_type: Solution User Credentials (token by cert)
=======================================================================================
Token Request POST /openidconnect/token HTTP/1.1 Host:
server.example.com Content-Type: application/x-www-form-urlencoded

    grant_type=urn:vmware:grant_type:solution_user_credentials&
    solution_assertion==<jwt>&
    scope=openid

Token Response HTTP/1.1 200 OK Content-Type:
application/json;charset=UTF-8 Cache-Control: no-store Pragma: no-cache

    {
        "token_type":"hotk-pk",
        "expires_in":43200,
        "access_token":"<access_token>",
        "id_token":"<id_token>",
    }

=======================================================================================
extension grant\_type: GSS Ticket (token by ticket)
=======================================================================================
Token Request POST /openidconnect/token HTTP/1.1 Host:
server.example.com Content-Type: application/x-www-form-urlencoded

    grant_type=urn:vmware:grant_type:gss_ticket&
    gss_ticket=base64(gss_ticket_bytes)&
    context_id=_context_id_xyz_&
    scope=openid offline_access

Token Response HTTP/1.1 200 OK Content-Type:
application/json;charset=UTF-8 Cache-Control: no-store Pragma: no-cache

    {
        "token_type":"Bearer",
        "expires_in":3600,
        "access_token":"<access_token>",
        "id_token":"<id_token>",
        "refresh_token":"<refresh_token>"
    }

=======================================================================================
\[Logout\]
=======================================================================================
Logout Request GET /logout? id\_token\_hint=<id_token>&
post\_logout\_redirect\_uri=https://client.example.com/cb&
state=*state\_xyz*& client\_assertion=<client_assertion> Host:
server.example.com Cookie:oidc\_session\_id-<tenant>=<session_id>

Logout Response HTTP/1.1 302 Found Location:
https://client.example.com/cb? state=*state\_xyz*
Set-Cookie:oidc\_session\_id-<tenant>=""
=======================================================================================
