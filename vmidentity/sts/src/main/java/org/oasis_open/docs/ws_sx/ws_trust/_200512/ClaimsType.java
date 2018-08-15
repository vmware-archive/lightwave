
package org.oasis_open.docs.ws_sx.ws_trust._200512;

import java.util.ArrayList;
import java.util.List;
import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlSchemaType;
import javax.xml.bind.annotation.XmlType;
import org.oasis_open.docs.wsfed.authorization._200706.ClaimType;


/**
 * <p>Java class for ClaimsType complex type.
 *
 * <p>The following schema fragment specifies the expected content contained within this class.
 *
 * <pre>
 * &lt;complexType name="ClaimsType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;element ref="{http://docs.oasis-open.org/wsfed/authorization/200706}ClaimType" maxOccurs="unbounded"/>
 *       &lt;/sequence>
 *       &lt;attribute name="Dialect" type="{http://www.w3.org/2001/XMLSchema}anyURI" />
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 *
 *
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "ClaimsType", propOrder = {
    "claimType"
})
public class ClaimsType {

    @XmlElement(name = "ClaimType", namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", required = true)
    protected List<ClaimType> claimType;
    @XmlAttribute(name = "Dialect")
    @XmlSchemaType(name = "anyURI")
    protected String dialect;

    /**
     * Gets the value of the claimType property.
     *
     * <p>
     * This accessor method returns a reference to the live list,
     * not a snapshot. Therefore any modification you make to the
     * returned list will be present inside the JAXB object.
     * This is why there is not a <CODE>set</CODE> method for the claimType property.
     *
     * <p>
     * For example, to add a new item, do as follows:
     * <pre>
     *    getClaimType().add(newItem);
     * </pre>
     *
     *
     * <p>
     * Objects of the following type(s) are allowed in the list
     * {@link ClaimType }
     *
     *
     */
    public List<ClaimType> getClaimType() {
        if (claimType == null) {
            claimType = new ArrayList<ClaimType>();
        }
        return this.claimType;
    }

    /**
     * Gets the value of the dialect property.
     *
     * @return
     *     possible object is
     *     {@link String }
     *
     */
    public String getDialect() {
        return dialect;
    }

    /**
     * Sets the value of the dialect property.
     *
     * @param value
     *     allowed object is
     *     {@link String }
     *
     */
    public void setDialect(String value) {
        this.dialect = value;
    }

}
