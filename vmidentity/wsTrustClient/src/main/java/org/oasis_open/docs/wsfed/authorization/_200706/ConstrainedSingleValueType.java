
package org.oasis_open.docs.wsfed.authorization._200706;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlType;


/**
 * <p>Java class for ConstrainedSingleValueType complex type.
 *
 * <p>The following schema fragment specifies the expected content contained within this class.
 *
 * <pre>
 * &lt;complexType name="ConstrainedSingleValueType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;choice minOccurs="0">
 *         &lt;element name="Value" type="{http://www.w3.org/2001/XMLSchema}string"/>
 *         &lt;element name="StructuredValue" type="{http://docs.oasis-open.org/wsfed/authorization/200706}StructuredValueType"/>
 *       &lt;/choice>
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 *
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "ConstrainedSingleValueType", propOrder = {
    "value",
    "structuredValue"
})
public class ConstrainedSingleValueType {

    @XmlElement(name = "Value")
    protected String value;
    @XmlElement(name = "StructuredValue")
    protected StructuredValueType structuredValue;

    /**
     * Gets the value of the value property.
     *
     * @return
     *     possible object is
     *     {@link String }
     *
     */
    public String getValue() {
        return value;
    }

    /**
     * Sets the value of the value property.
     *
     * @param value
     *     allowed object is
     *     {@link String }
     *
     */
    public void setValue(String value) {
        this.value = value;
    }

    /**
     * Gets the value of the structuredValue property.
     *
     * @return
     *     possible object is
     *     {@link StructuredValueType }
     *
     */
    public StructuredValueType getStructuredValue() {
        return structuredValue;
    }

    /**
     * Sets the value of the structuredValue property.
     *
     * @param value
     *     allowed object is
     *     {@link StructuredValueType }
     *
     */
    public void setStructuredValue(StructuredValueType value) {
        this.structuredValue = value;
    }

}
