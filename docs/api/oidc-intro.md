# OpenID Connect in Lightwave #

Lightwave supports OpenID Connect 1.0, which is a simple identity layer on top of the OAuth 2.0 protocol. It allows Clients to verify the identity of the End-User based on the authentication performed by an Authorization Server, as well as to obtain basic profile information about the End-User in an interoperable and REST-like manner.


Lightwave OIDC serves two main purposes:
1. Security Token Service (STS) in the sense that it issues security tokens in the form of JWT tokens (similar to WS-TRUST which issues SAML 2.0 Assertions)
2. Single sign-on (SSO) / single logout (SLO) service for web-based applications (similar to SAML 2.0 Authentication Request and Single Logout protocols aka websso)


