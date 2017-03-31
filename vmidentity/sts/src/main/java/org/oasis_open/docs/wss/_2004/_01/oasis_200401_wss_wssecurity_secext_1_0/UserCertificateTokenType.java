
package org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlType;

import org.w3._2000._09.xmldsig_.SignatureValueType;


/**
 * User certificate token type. It includes a certificate, a piece of information to be signed, signature algorithm and signature value.
 * 
 * <p>Java class for UserCertificateTokenType complex type.
 * 
 * <p>The following schema fragment specifies the expected content contained within this class.
 * 
 * <pre>
 * &lt;complexType name="UserCertificateTokenType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;element name="UserCertificate" type="{http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd}BinarySecurityTokenType"/>
 *         &lt;element name="SignatureInfo" type="{http://www.w3.org/2001/XMLSchema}string"/>
 *         &lt;element name="SignatureAlgorithm" type="{http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd}SignatureAlgorithmType"/>
 *         &lt;element name="SignatureValue" type="{http://www.w3.org/2000/09/xmldsig#}SignatureValueType"/>
 *       &lt;/sequence>
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 * 
 * 
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "UserCertificateTokenType", propOrder = {
    "userCertificate",
    "signatureInfo",
    "signatureAlgorithm",
    "signatureValue"
})
public class UserCertificateTokenType {

    @XmlElement(name = "UserCertificate", required = true)
    protected BinarySecurityTokenType userCertificate;
    @XmlElement(name = "SignatureInfo", required = true)
    protected String signatureInfo;
    @XmlElement(name = "SignatureAlgorithm", required = true)
    protected SignatureAlgorithmType signatureAlgorithm;
    @XmlElement(name = "SignatureValue", required = true)
    protected SignatureValueType signatureValue;

    /**
     * Gets the value of the userCertificate property.
     * 
     * @return
     *     possible object is
     *     {@link BinarySecurityTokenType }
     *     
     */
    public BinarySecurityTokenType getUserCertificate() {
        return userCertificate;
    }

    /**
     * Sets the value of the userCertificate property.
     * 
     * @param value
     *     allowed object is
     *     {@link BinarySecurityTokenType }
     *     
     */
    public void setUserCertificate(BinarySecurityTokenType value) {
        this.userCertificate = value;
    }

    /**
     * Gets the value of the signatureInfo property.
     * 
     * @return
     *     possible object is
     *     {@link String }
     *     
     */
    public String getSignatureInfo() {
        return signatureInfo;
    }

    /**
     * Sets the value of the signatureInfo property.
     * 
     * @param value
     *     allowed object is
     *     {@link String }
     *     
     */
    public void setSignatureInfo(String value) {
        this.signatureInfo = value;
    }

    /**
     * Gets the value of the signatureAlgorithm property.
     * 
     * @return
     *     possible object is
     *     {@link SignatureAlgorithmType }
     *     
     */
    public SignatureAlgorithmType getSignatureAlgorithm() {
        return signatureAlgorithm;
    }

    /**
     * Sets the value of the signatureAlgorithm property.
     * 
     * @param value
     *     allowed object is
     *     {@link SignatureAlgorithmType }
     *     
     */
    public void setSignatureAlgorithm(SignatureAlgorithmType value) {
        this.signatureAlgorithm = value;
    }

    /**
     * Gets the value of the signatureValue property.
     * 
     * @return
     *     possible object is
     *     {@link SignatureValueType }
     *     
     */
    public SignatureValueType getSignatureValue() {
        return signatureValue;
    }

    /**
     * Sets the value of the signatureValue property.
     * 
     * @param value
     *     allowed object is
     *     {@link SignatureValueType }
     *     
     */
    public void setSignatureValue(SignatureValueType value) {
        this.signatureValue = value;
    }

}
