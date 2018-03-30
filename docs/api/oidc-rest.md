# OpenID Connect REST API in Lightwave #

## Roles ##
* User (Resource Owner)
* User agent (browser)
* Client (aka Relying Party / RP) e.g: lightwaveui
* Authz Server (aka OpenID Provider / OP)
* Resource Server (server that wants to implement authz for its API's) e.g: admin server

## JWT tokens ##
OAuth 2.0 defines an access_token and a refresh_token but does not specify their format. We are using JWT as the format.
OpenID Connect additionaly defines an id_token and specifies its format as a JWT.
Lightwave OIDC issues: id_tokens, access_tokens, and refresh_tokens. All will be signed using the tenant's private key.

### id_token ###
Represents the authentication of an end user at the Authz Server. The Client will validate this token and read its claims to sign in the end user.

Example id_token JSON body:
```
{
  "sub": "administrator@lw-testdom.com",
  "iss": "https://photon-63mciz57.lw-testdom.com/openidconnect/LW-TESTDOM.COM",
  "groups": [
    "LW-TESTDOM.COM\\Users",
    "LW-TESTDOM.COM\\Administrators",
    "LW-TESTDOM.COM\\CAAdmins",
    "LW-TESTDOM.COM\\Everyone"
  ],
  "token_class": "id_token",
  "token_type": "Bearer",
  "given_name": "Administrator",
  "aud": "administrator@lw-testdom.com",
  "scope": "at_groups rs_admin_server openid offline_access id_groups",
  "exp": 1501698283,
  "iat": 1501694683,
  "family_name": "LW-TESTDOM.COM",
  "jti": "rbdevuzSlBfwgSvQJdgtviOeVUmidOHcDZw79iy6ouQ",
  "tenant": "LW-TESTDOM.COM"
}
```

### access_token ###
The Client will use this in its protected resource requests against the Resource Server.
The Resource Server, on receiving the access_token from the Client, will validate the token and make an API authz decision based on the user's group membership in the groups claim.

Example access_token JSON body:
```
{
  "sub": "administrator@lw-testdom.com",
  "aud": [
    "administrator@lw-testdom.com",
    "rs_admin_server"
  ],
  "scope": "at_groups rs_admin_server openid offline_access id_groups",
  "iss": "https://photon-63mciz57.lw-testdom.com/openidconnect/LW-TESTDOM.COM",
  "groups": [
    "LW-TESTDOM.COM\\Users",
    "LW-TESTDOM.COM\\Administrators",
    "LW-TESTDOM.COM\\CAAdmins",
    "LW-TESTDOM.COM\\Everyone"
  ],
  "token_class": "access_token",
  "token_type": "Bearer",
  "exp": 1501698283,
  "iat": 1501694683,
  "jti": "0lXE9y17uMevd0gBaIhRZ1ZqoYb7P64EuY7Bh7qyPXY",
  "tenant": "LW-TESTDOM.COM",
  "admin_server_role": "Administrator"
}
```

## Supported Flows ##

### Authz Code Flow ###
Sequence diagram:

![picture alt](images/oidc_authzflow.png)


Sample request/response:
```
Authn Request
    GET /authorize?
        response_type=code&
        client_id=_client_id_xyz_&
        redirect_uri=https://client.example.com/cb&
        state=_state_xyz_&
        nonce=_nonce_xyz_&
        scope=openid offline_access HTTP/1.1
    Host: server.example.com

Authn Response
    HTTP/1.1 302 Found
    Location: https://client.example.com/cb?
        code=_authz_code_xyz_&
        state=_state_xyz_
```

```
Token Request
    POST /token HTTP/1.1
    Host: server.example.com
    Content-Type: application/x-www-form-urlencoded

    grant_type=authorization_code&
    code=_authz_code_xyz_&
    redirect_uri=https://client.example.com/cb&
    client_assertion_type=urn:ietf:params:oauth:client-assertion-type:jwt-bearer&
    client_assertion=<private_key_jwt>

Token Response
    HTTP/1.1 200 OK
    Content-Type: application/json;charset=UTF-8
    Cache-Control: no-store
    Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"Bearer",
        "expires_in":3600,
        "id_token":"<id_token>",
        "refresh_token":"_refresh_token_xyz_"
    }
```

### Implicit Flow ###
Sequence diagram:

![picture alt](images/oidc_implicitflow.png)


Sample request/response:
```
Authn Request
    GET /authorize?
        response_type=id_token&
        client_id=_client_id_xyz_&
        redirect_uri=https://client.example.com/cb&
        state=_state_xyz_&
        nonce=_nonce_xyz_&
        scope=openid HTTP/1.1
    Host: server.example.com

Authn Response
    HTTP/1.1 302 Found
    Location: http://example.com/cb#
        state=_state_xyz_&
        id_token=<id_token>
```

## Password Flow ##
Sequence diagram:

![picture alt](images/oidc_passwordflow.png)


Sample request/response:
```
Token Request
    POST /token HTTP/1.1
    Host: server.example.com
    Authorization: Basic czZCaGRSa3F0MzpnWDFmQmF0M2JW
    Content-Type: application/x-www-form-urlencoded

    grant_type=password&
    username=_username_xyz_&
    password=_password_xyz_&
    scope=openid offline_access

Token Response
    HTTP/1.1 200 OK
    Content-Type: application/json;charset=UTF-8
    Cache-Control: no-store
    Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"Bearer",
        "expires_in":3600,
        "id_token":"<id_token>",
        "refresh_token":"_refresh_token_xyz_"
    }
```

### Refresh Token Flow ###
Sequence diagram:

![picture alt](images/oidc_refreshflow.png)


Sample request/response:
```
Token Request
    POST /token HTTP/1.1
    Host: server.example.com
    Authorization: Basic czZCaGRSa3F0MzpnWDFmQmF0M2JW
    Content-Type: application/x-www-form-urlencoded

    grant_type=refresh_token&
    refresh_token=_refresh_token_xyz_&
    client_id=_client_id_xyz_

Token Response
    HTTP/1.1 200 OK
    Content-Type: application/json;charset=UTF-8
    Cache-Control: no-store
    Pragma: no-cache

    {
        "access_token":"<access_token>",
        "token_type":"Bearer",
        "expires_in":3600,
        "id_token":"<id_token>"
    }
```

## Metadata endpoint ##

[Spec](http://openid.net/specs/openid-connect-discovery-1_0.html)

OIDC Metadata endpoint publishes a JSON format document that includes server metadata such as server endpoints and server supported capability configurations. The client can use this data to help construct requests used in OIDC workflows.

Metadata endpoint format:
```
https://[OIDC Server URL]/{tenant}/.well-known/openid-configuration
```

Request to the metadata endpoint should use HTTP GET. A sample request is listed below:
```
GET [OIDC Server URL]/{tenant}/.well-known/openid-configuration HTTP/1.1
Host: example.com
```

A successful response will return HTTP status code 200 (OK), and a JSON object using the application/json content type. The details for claim definitions can be found in the spec above. A sample response is listed below:
```
HTTP/1.1 200 OK
Content-Type: application/json
{
   "response_types_supported":[
      "code",
      "id_token",
      "token id_token"
   ],
   "jwks_uri":"https:\/\/photon-63mciz57.lw-testdom.com\/openidconnect\/jwks\/LW-TESTDOM.COM",
   "end_session_endpoint":"https:\/\/photon-63mciz57.lw-testdom.com\/openidconnect\/logout\/LW-TESTDOM.COM",
   "subject_types_supported":[
      "public"
   ],
   "id_token_signing_alg_values_supported":[
      "RS256"
   ],
   "issuer":"https:\/\/photon-63mciz57.lw-testdom.com\/openidconnect\/LW-TESTDOM.COM",
   "authorization_endpoint":"https:\/\/photon-63mciz57.lw-testdom.com\/openidconnect\/oidc\/authorize\/LW-TESTDOM.COM",
   "token_endpoint":"https:\/\/photon-63mciz57.lw-testdom.com\/openidconnect\/token\/LW-TESTDOM.COM"
}
```

An error response will return HTTP status code 404 (Not Found) or 500 (Internal Server Error).

## JWKS endpoint ##
OIDC server also provides a JWK set endpoint to publish OIDC server public RSA keys in the format of JSON Web Key (JWK) set. [JWK spec](https://tools.ietf.org/html/draft-ietf-jose-json-web-key-41)

Relying parties and resource servers need to retrieve this public key so they can verify token signature signed by the OIDC server.

JWK set endpoint format:
```
https://[OIDC Server URL]/jwks/{tenant}
```
Request to the JWK set endpoint should use HTTP verb: GET. A sample request is listed below:
```
GET [OIDC Server URL]/jwks HTTP/1.1
Host: example.com
```
A successful response will return HTTP status code 200 (OK), and a JSON object using the application/json content type. A sample response is listed below:
```
HTTP/1.1 200 OK
Content-Type: application/json
{
 "keys":
 [
  {
   "e": "AQAB",
   "n": "......",
   ...... 
  }
 ]
}
```

An error response will return HTTP status code 404 (Not Found) or 500 (Internal Server Error).

## Holder Of Key Tokens ##
Solution Users or Clients that have a backing Solution User can request HOK tokens. The token request will include a proof of possession of the private key in the form of a solution_user_assertion or client_assertion which are JWT's signed using the private key.
OIDC server will use the Solution User's registered certificate to validate the signature of the JWT.
The issued access_token will contain an "hotk" claim that is a JWK Set containing the Solution User's public key.
The Client will then send signed requests to Resource Servers that support HoK access_tokens.
The Resource Server will validate the signature of the request using the public key in the access_token's "hotk" claim.

## Act As Tokens ##
ActAs tokens are HOK tokens whose subject is a Person User. A token request that includes PersonUser and SolutionUser credentials at the same time will result in ActAs tokens.
There are many forms  of Person User credentials, for example username/password or authorization code.
Solution User credentials will be in the form of solution_user_assertion or client_assertion which are both JWT's signed using the Solution User's private key.
The most common form of acquiring ActAs tokens is the Authorization Code Flow. The authorization code represents Person User credentials (the user who logged) and the token request is required to have a client_assertion which represents a registered OIDC client with a backing Solution User.

## Login ##
When a user is redirected to the Authz Server, if the Authz Server does not receive a session cookie, it will serve a login form.
The user can then login using username/password (or RSA SecurID, Kerberos, Smart Card, if enabled).

### Single Sign On ###
SSO is achieved by the use of cookies.
When a user logs in to the Authz Server (after being redirected to it by a Client), a session cookie is returned " oidc_session_id-<tenant>=<session_id>".
When the user attempts to login to another Client, the other Client will redirect the user to the same Authz Server, the browser will then include the Authz Server session cookie which will allow the Authz Server to login the same user without prompting for credentials.

## Logout ##
Implemented according to [OpenID Connect Session Management 1.0 "RP-Initiated Logout"](http://openid.net/specs/openid-connect-session-1_0.html)

Sequence diagram:

![picture alt](images/oidc_logoutflow.png)


Sample request/response:
```
Logout Request
    GET /logout?
        id_token_hint=<id_token>&
        post_logout_redirect_uri=https://client.example.com/cb&
        state=_state_xyz_
    Host: server.example.com
    Cookie:oidc_session_id-vsphere.local=<session_id>

Logout Response
    HTTP/1.1 302 Found
    Location: https://client.example.com/cb?
        state=_state_xyz_
    Set-Cookie:oidc_session_id-vsphere.local=""
```

### Single Log Out ###
Implemented according to [OpenID Connect HTTP-Based Logout 1.0](http://openid.net/specs/openid-connect-frontchannel-1_0.html)

When Authz Server receives a logout request, it returns an html page with iframe tags whose source is the logout_uri of all the Clients the user has logged into.
The browser will make HTTP GET requests and include session cookies which will allow the Client to logout the user.

## Specs ##

### Endpoints ###

Supported Endpoints    | Usage
-----------------------|------------------------
authorization_endpoint | interactive user login
token_endpoint         | obtain id_token, access_token, refresh_token
userinfo_endpoint      | obtain user profile information
jwks_uri               | obtain public key corresponding to private signing key per tenant
discovery              | OIDC Discovery
end_session_endpoint   | Logout

### Grant Types ###

Supported Grant Types    | Usage
-------------------------|------------------------
authorization_code       | authz code flow
implicit                 | implicit flow
password                 | password flow
refresh_token            | refresh token flow
certificate              | extension grant type
gss ticket               | extension grant type
securid                  | extension grant type
person user certificate  | extension grant type
client_credentials       | client credentials flow

### Supported Client Authentication methods ###
* private_key_jwt

### Supported JWT signature algorithms ###
* RS256

### Returned Error Codes ###
OAuth2.0 Errors:
* invalid_request
* unauthorized_client
* invalid_client
* invalid_scope
* unsupported_response_type
* unsuppported_grant_type
* invalid_grant
* access_denied
* server_error


