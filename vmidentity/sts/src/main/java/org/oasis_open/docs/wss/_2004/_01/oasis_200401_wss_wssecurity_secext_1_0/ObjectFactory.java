
package org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0;

import javax.xml.bind.JAXBElement;
import javax.xml.bind.annotation.XmlElementDecl;
import javax.xml.bind.annotation.XmlRegistry;
import javax.xml.namespace.QName;


/**
 * This object contains factory methods for each 
 * Java content interface and Java element interface 
 * generated in the org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0 package. 
 * <p>An ObjectFactory allows you to programatically 
 * construct new instances of the Java representation 
 * for XML content. The Java representation of XML 
 * content can consist of schema derived interfaces 
 * and classes representing the binding of schema 
 * type definitions, element declarations and model 
 * groups.  Factory methods for each of these are 
 * provided in this class.
 * 
 */
@XmlRegistry
public class ObjectFactory {

    private final static QName _Password_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "Password");
    private final static QName _Reference_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "Reference");
    private final static QName _Security_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "Security");
    private final static QName _SecurityTokenReference_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "SecurityTokenReference");
    private final static QName _BinarySecurityToken_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "BinarySecurityToken");
    private final static QName _KeyIdentifier_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "KeyIdentifier");
    private final static QName _Passcode_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "Passcode");
    private final static QName _UserCertificateToken_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "UserCertificateToken");
    private final static QName _UsernameToken_QNAME = new QName("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", "UsernameToken");

    /**
     * Create a new ObjectFactory that can be used to create new instances of schema derived classes for package: org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0
     * 
     */
    public ObjectFactory() {
    }

    /**
     * Create an instance of {@link SecurityHeaderType }
     * 
     */
    public SecurityHeaderType createSecurityHeaderType() {
        return new SecurityHeaderType();
    }

    /**
     * Create an instance of {@link UserCertificateTokenType }
     * 
     */
    public UserCertificateTokenType createUserCertificateTokenType() {
        return new UserCertificateTokenType();
    }

    /**
     * Create an instance of {@link PasswordString }
     * 
     */
    public PasswordString createPasswordString() {
        return new PasswordString();
    }

    /**
     * Create an instance of {@link SecurityTokenReferenceType }
     * 
     */
    public SecurityTokenReferenceType createSecurityTokenReferenceType() {
        return new SecurityTokenReferenceType();
    }

    /**
     * Create an instance of {@link UsernameTokenType }
     * 
     */
    public UsernameTokenType createUsernameTokenType() {
        return new UsernameTokenType();
    }

    /**
     * Create an instance of {@link BinarySecurityTokenType }
     * 
     */
    public BinarySecurityTokenType createBinarySecurityTokenType() {
        return new BinarySecurityTokenType();
    }

    /**
     * Create an instance of {@link ReferenceType }
     * 
     */
    public ReferenceType createReferenceType() {
        return new ReferenceType();
    }

    /**
     * Create an instance of {@link PasscodeString }
     * 
     */
    public PasscodeString createPasscodeString() {
        return new PasscodeString();
    }

    /**
     * Create an instance of {@link KeyIdentifierType }
     * 
     */
    public KeyIdentifierType createKeyIdentifierType() {
        return new KeyIdentifierType();
    }

    /**
     * Create an instance of {@link EncodedString }
     * 
     */
    public EncodedString createEncodedString() {
        return new EncodedString();
    }

    /**
     * Create an instance of {@link AttributedString }
     * 
     */
    public AttributedString createAttributedString() {
        return new AttributedString();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link PasswordString }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "Password")
    public JAXBElement<PasswordString> createPassword(PasswordString value) {
        return new JAXBElement<PasswordString>(_Password_QNAME, PasswordString.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link ReferenceType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "Reference")
    public JAXBElement<ReferenceType> createReference(ReferenceType value) {
        return new JAXBElement<ReferenceType>(_Reference_QNAME, ReferenceType.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link SecurityHeaderType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "Security")
    public JAXBElement<SecurityHeaderType> createSecurity(SecurityHeaderType value) {
        return new JAXBElement<SecurityHeaderType>(_Security_QNAME, SecurityHeaderType.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link SecurityTokenReferenceType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "SecurityTokenReference")
    public JAXBElement<SecurityTokenReferenceType> createSecurityTokenReference(SecurityTokenReferenceType value) {
        return new JAXBElement<SecurityTokenReferenceType>(_SecurityTokenReference_QNAME, SecurityTokenReferenceType.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link BinarySecurityTokenType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "BinarySecurityToken")
    public JAXBElement<BinarySecurityTokenType> createBinarySecurityToken(BinarySecurityTokenType value) {
        return new JAXBElement<BinarySecurityTokenType>(_BinarySecurityToken_QNAME, BinarySecurityTokenType.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link KeyIdentifierType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "KeyIdentifier")
    public JAXBElement<KeyIdentifierType> createKeyIdentifier(KeyIdentifierType value) {
        return new JAXBElement<KeyIdentifierType>(_KeyIdentifier_QNAME, KeyIdentifierType.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link PasscodeString }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "Passcode")
    public JAXBElement<PasscodeString> createPasscode(PasscodeString value) {
        return new JAXBElement<PasscodeString>(_Passcode_QNAME, PasscodeString.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link UserCertificateTokenType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "UserCertificateToken")
    public JAXBElement<UserCertificateTokenType> createUserCertificateToken(UserCertificateTokenType value) {
        return new JAXBElement<UserCertificateTokenType>(_UserCertificateToken_QNAME, UserCertificateTokenType.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link UsernameTokenType }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", name = "UsernameToken")
    public JAXBElement<UsernameTokenType> createUsernameToken(UsernameTokenType value) {
        return new JAXBElement<UsernameTokenType>(_UsernameToken_QNAME, UsernameTokenType.class, null, value);
    }

}
