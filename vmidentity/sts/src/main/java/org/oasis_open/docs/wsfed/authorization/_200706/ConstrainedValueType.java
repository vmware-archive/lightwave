
package org.oasis_open.docs.wsfed.authorization._200706;

import java.util.ArrayList;
import java.util.List;
import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAnyElement;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlType;
import org.w3c.dom.Element;


/**
 * <p>Java class for ConstrainedValueType complex type.
 *
 * <p>The following schema fragment specifies the expected content contained within this class.
 *
 * <pre>
 * &lt;complexType name="ConstrainedValueType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;choice>
 *           &lt;element name="ValueLessThan" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedSingleValueType"/>
 *           &lt;element name="ValueLessThanOrEqual" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedSingleValueType"/>
 *           &lt;element name="ValueGreaterThan" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedSingleValueType"/>
 *           &lt;element name="ValueGreaterThanOrEqual" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedSingleValueType"/>
 *           &lt;element name="ValueInRangen" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ValueInRangeType"/>
 *           &lt;element name="ValueOneOf" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedManyValueType"/>
 *         &lt;/choice>
 *         &lt;any processContents='lax' namespace='##other' maxOccurs="unbounded"/>
 *       &lt;/sequence>
 *       &lt;attribute name="AssertConstraint" type="{http://www.w3.org/2001/XMLSchema}boolean" />
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 *
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "ConstrainedValueType", propOrder = {
    "valueLessThan",
    "valueLessThanOrEqual",
    "valueGreaterThan",
    "valueGreaterThanOrEqual",
    "valueInRangen",
    "valueOneOf",
    "any"
})
public class ConstrainedValueType {

    @XmlElement(name = "ValueLessThan")
    protected ConstrainedSingleValueType valueLessThan;
    @XmlElement(name = "ValueLessThanOrEqual")
    protected ConstrainedSingleValueType valueLessThanOrEqual;
    @XmlElement(name = "ValueGreaterThan")
    protected ConstrainedSingleValueType valueGreaterThan;
    @XmlElement(name = "ValueGreaterThanOrEqual")
    protected ConstrainedSingleValueType valueGreaterThanOrEqual;
    @XmlElement(name = "ValueInRangen")
    protected ValueInRangeType valueInRangen;
    @XmlElement(name = "ValueOneOf")
    protected ConstrainedManyValueType valueOneOf;
    @XmlAnyElement(lax = true)
    protected List<Object> any;
    @XmlAttribute(name = "AssertConstraint")
    protected Boolean assertConstraint;

    /**
     * Gets the value of the valueLessThan property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public ConstrainedSingleValueType getValueLessThan() {
        return valueLessThan;
    }

    /**
     * Sets the value of the valueLessThan property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public void setValueLessThan(ConstrainedSingleValueType value) {
        this.valueLessThan = value;
    }

    /**
     * Gets the value of the valueLessThanOrEqual property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public ConstrainedSingleValueType getValueLessThanOrEqual() {
        return valueLessThanOrEqual;
    }

    /**
     * Sets the value of the valueLessThanOrEqual property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public void setValueLessThanOrEqual(ConstrainedSingleValueType value) {
        this.valueLessThanOrEqual = value;
    }

    /**
     * Gets the value of the valueGreaterThan property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public ConstrainedSingleValueType getValueGreaterThan() {
        return valueGreaterThan;
    }

    /**
     * Sets the value of the valueGreaterThan property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public void setValueGreaterThan(ConstrainedSingleValueType value) {
        this.valueGreaterThan = value;
    }

    /**
     * Gets the value of the valueGreaterThanOrEqual property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public ConstrainedSingleValueType getValueGreaterThanOrEqual() {
        return valueGreaterThanOrEqual;
    }

    /**
     * Sets the value of the valueGreaterThanOrEqual property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedSingleValueType }
     *
     */
    public void setValueGreaterThanOrEqual(ConstrainedSingleValueType value) {
        this.valueGreaterThanOrEqual = value;
    }

    /**
     * Gets the value of the valueInRangen property.
     *
     * @return
     *     possible object is
     *     {@link ValueInRangeType }
     *
     */
    public ValueInRangeType getValueInRangen() {
        return valueInRangen;
    }

    /**
     * Sets the value of the valueInRangen property.
     *
     * @param value
     *     allowed object is
     *     {@link ValueInRangeType }
     *
     */
    public void setValueInRangen(ValueInRangeType value) {
        this.valueInRangen = value;
    }

    /**
     * Gets the value of the valueOneOf property.
     *
     * @return
     *     possible object is
     *     {@link ConstrainedManyValueType }
     *
     */
    public ConstrainedManyValueType getValueOneOf() {
        return valueOneOf;
    }

    /**
     * Sets the value of the valueOneOf property.
     *
     * @param value
     *     allowed object is
     *     {@link ConstrainedManyValueType }
     *
     */
    public void setValueOneOf(ConstrainedManyValueType value) {
        this.valueOneOf = value;
    }

    /**
     * Gets the value of the any property.
     *
     * <p>
     * This accessor method returns a reference to the live list,
     * not a snapshot. Therefore any modification you make to the
     * returned list will be present inside the JAXB object.
     * This is why there is not a <CODE>set</CODE> method for the any property.
     *
     * <p>
     * For example, to add a new item, do as follows:
     * <pre>
     *    getAny().add(newItem);
     * </pre>
     *
     * <p>
     * Objects of the following type(s) are allowed in the list
     * {@link Element }
     * {@link Object }
     *
     */
    public List<Object> getAny() {
        if (any == null) {
            any = new ArrayList<Object>();
        }
        return this.any;
    }

    /**
     * Gets the value of the assertConstraint property.
     *
     * @return
     *     possible object is
     *     {@link Boolean }
     *
     */
    public Boolean isAssertConstraint() {
        return assertConstraint;
    }

    /**
     * Sets the value of the assertConstraint property.
     *
     * @param value
     *     allowed object is
     *     {@link Boolean }
     *
     */
    public void setAssertConstraint(Boolean value) {
        this.assertConstraint = value;
    }

}
