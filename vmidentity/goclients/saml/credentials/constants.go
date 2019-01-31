package credentials

const (
	soapEnvTemplate = `
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" >%s%s
</SOAP-ENV:Envelope>`

	soapSecurityHeaderTemplate = `
<SOAP-ENV:Header xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" >
    <wsse:Security
        xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd"
        xmlns:wsa="http://www.w3.org/2005/08/addressing"
        xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"
        xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706"
        xmlns:wst="http://docs.oasis-open.org/ws-sx/ws-trust/200512">%s%s%s
    </wsse:Security>
</SOAP-ENV:Header>`

	soapBodyTemplate = `
<SOAP-ENV:Body xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" %s><wst:RequestSecurityToken
        xmlns:wst="http://docs.oasis-open.org/ws-sx/ws-trust/200512">
        <wst:TokenType>urn:oasis:names:tc:SAML:2.0:assertion</wst:TokenType>
        <wst:RequestType>http://docs.oasis-open.org/ws-sx/ws-trust/200512/Issue</wst:RequestType>
        <wst:KeyType>%s</wst:KeyType>%s
        <wst:Lifetime>
            <wsu:Created>%s</wsu:Created>
            <wsu:Expires>%s</wsu:Expires>
        </wst:Lifetime>
        <wst:Renewing Allow="false" OK="false"/>
        <wst:Delegatable>false</wst:Delegatable>
        <wst:Participants>
            <wst:Primary>
                <wsa:EndpointReference xmlns:wsa="http://www.w3.org/2005/08/addressing">
                    <wsa:Address>https://signin.aws.amazon.com/saml</wsa:Address>
                </wsa:EndpointReference>
            </wst:Primary>
        </wst:Participants>
        <wst:Claims Dialect="http://schemas.xmlsoap.org/ws/2005/05/fedclaims">
            <auth:ClaimType xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706" Uri="https://aws.amazon.com/SAML/Attributes/Role"></auth:ClaimType>
            <auth:ClaimType xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706" Uri="https://aws.amazon.com/SAML/Attributes/RoleSessionName"></auth:ClaimType>
            <auth:ClaimType xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706" Uri="http://rsa.com/schemas/attr-names/2009/01/GroupIdentity"></auth:ClaimType>
            <auth:ClaimType xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706" Uri="http://vmware.com/schemas/attr-names/2011/07/isSolution"></auth:ClaimType>
            <auth:ClaimType xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706" Uri="http://schemas.xmlsoap.org/claims/UPN"></auth:ClaimType>
        </wst:Claims>%s
    </wst:RequestSecurityToken></SOAP-ENV:Body>`

	wsuTimestampTemplate = `
<wsu:Timestamp %s>
    <wsu:Created>%s</wsu:Created>
    <wsu:Expires>%s</wsu:Expires>
</wsu:Timestamp>`

	usernameTokenRequestTemplate = `
<wsse:UsernameToken
    xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
    <wsse:Username>%s</wsse:Username>
    <wsse:Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText"><![CDATA[%s]]></wsse:Password>
</wsse:UsernameToken>`

	binarySecurityTokenRequestTemplate = `
<wsse:BinarySecurityToken EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary" ValueType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3" %s>
	%s
</wsse:BinarySecurityToken>`

	signedInfoTemplate = `
<ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" />
<ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha512" />
<ds:Reference URI="#%s">
	<ds:Transforms>
		<ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" >
			<InclusiveNamespaces xmlns="http://www.w3.org/2001/10/xml-exc-c14n#" PrefixList="SOAP-ENV"/>
		</ds:Transform>
	</ds:Transforms>
	<ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha512" />
	<ds:DigestValue></ds:DigestValue>
</ds:Reference>
<ds:Reference URI="#%s">
	<ds:Transforms>
		<ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" />
	</ds:Transforms>
	<ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha512" />
	<ds:DigestValue></ds:DigestValue>
</ds:Reference>`

	signatureTemplate = `
<ds:Signature
    xmlns:ds="http://www.w3.org/2000/09/xmldsig#" Id="%s">
    <ds:SignedInfo>%s</ds:SignedInfo>
    <ds:SignatureValue></ds:SignatureValue>
    <ds:KeyInfo>
        <wsse:SecurityTokenReference>
            <wsse:Reference URI="#%s" ValueType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3"/>
        </wsse:SecurityTokenReference>
    </ds:KeyInfo>
</ds:Signature>`

	samlResponseTemplate = `
<samlp:Response
    xmlns:samlp="urn:oasis:names:tc:SAML:2.0:protocol"
    xmlns:saml="urn:oasis:names:tc:SAML:2.0:assertion">
    <saml:Issuer>%s</saml:Issuer>
    <samlp:Status>
        <samlp:StatusCode Value="urn:oasis:names:tc:SAML:2.0:status:Success"/>
    </samlp:Status>
	%s
</samlp:Response>`

	beginSamlAssertionTag = `<saml2:Assertion`
	endSamlAssertionTag   = `</saml2:Assertion>`
	useKeyTemplate        = `<wst:UseKey Sig="%s" />`
	wsuIdTemplate         = `wsu:Id="%s"`
	keyTypeBearer         = `http://docs.oasis-open.org/ws-sx/ws-trust/200512/Bearer`
	keyTypePublicKey      = `http://docs.oasis-open.org/ws-sx/ws-trust/200512/PublicKey`
	sigAlg                = `<wst:SignatureAlgorithm>http://www.w3.org/2001/04/xmldsig-more#rsa-sha256</wst:SignatureAlgorithm>`
)
