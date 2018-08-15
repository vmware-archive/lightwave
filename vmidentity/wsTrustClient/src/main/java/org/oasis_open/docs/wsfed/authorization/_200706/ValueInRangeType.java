
package org.oasis_open.docs.wsfed.authorization._200706;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlType;


/**
 * <p>Java class for ValueInRangeType complex type.
 *
 * <p>The following schema fragment specifies the expected content contained within this class.
 *
 * <pre>
 * &lt;complexType name="ValueInRangeType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;element name="ValueUpperBound" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedSingleValueType"/>
 *         &lt;element name="ValueLowerBound" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedSingleValueType"/>
 *       &lt;/sequence>
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 *
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "ValueInRangeType", propOrder = {
    "valueUpperBound",
    "valueLowerBound"
})
public class ValueInRangeType {

    @XmlElement(name = "ValueUpperBound", required = true)
    protected ConstrainedSingleValueType valueUpperBound;
    @XmlElement(name = "ValueLowerBound", required = true)
    protected ConstrainedSingleValueType valueLowerBound;

    /**
     * Gets the value of the valueUpperBound property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public ConstrainedSingleValueType getValueUpperBound() {
        return valueUpperBound;
    }

    /**
     * Sets the value of the valueUpperBound property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public void setValueUpperBound(ConstrainedSingleValueType value) {
        this.valueUpperBound = value;
    }

    /**
     * Gets the value of the valueLowerBound property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public ConstrainedSingleValueType getValueLowerBound() {
        return valueLowerBound;
    }

    /**
     * Sets the value of the valueLowerBound property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public void setValueLowerBound(ConstrainedSingleValueType value) {
        this.valueLowerBound = value;
    }

}
