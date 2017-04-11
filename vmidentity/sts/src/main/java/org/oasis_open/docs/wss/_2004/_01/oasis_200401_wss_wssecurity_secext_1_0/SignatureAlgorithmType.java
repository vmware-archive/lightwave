
package org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0;

import javax.xml.bind.annotation.XmlEnum;
import javax.xml.bind.annotation.XmlEnumValue;
import javax.xml.bind.annotation.XmlType;


/**
 * <p>Java class for SignatureAlgorithmType.
 * 
 * <p>The following schema fragment specifies the expected content contained within this class.
 * <p>
 * <pre>
 * &lt;simpleType name="SignatureAlgorithmType">
 *   &lt;restriction base="{http://www.w3.org/2001/XMLSchema}string">
 *     &lt;enumeration value="SHA256withRSA"/>
 *   &lt;/restriction>
 * &lt;/simpleType>
 * </pre>
 * 
 */
@XmlType(name = "SignatureAlgorithmType")
@XmlEnum
public enum SignatureAlgorithmType {

    @XmlEnumValue("SHA256withRSA")
    SHA_256_WITH_RSA("SHA256withRSA");
    private final String value;

    SignatureAlgorithmType(String v) {
        value = v;
    }

    public String value() {
        return value;
    }

    public static SignatureAlgorithmType fromValue(String v) {
        for (SignatureAlgorithmType c: SignatureAlgorithmType.values()) {
            if (c.value.equals(v)) {
                return c;
            }
        }
        throw new IllegalArgumentException(v);
    }

}
