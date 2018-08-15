
package org.oasis_open.docs.wsfed.authorization._200706;

import java.util.HashMap;
import java.util.Map;
import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAnyAttribute;
import javax.xml.bind.annotation.XmlAnyElement;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlSchemaType;
import javax.xml.bind.annotation.XmlType;
import javax.xml.namespace.QName;
import org.w3c.dom.Element;

/**
 * <p>
 * Java class for ClaimType complex type.
 *
 * <p>
 * The following schema fragment specifies the expected content contained within
 * this class.
 *
 * <pre>
 * &lt;complexType name="ClaimType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;element name="DisplayName" type="{http://docs.oasis-open.org/wsfed/authorization/200706}DisplayNameType" minOccurs="0"/>
 *         &lt;element name="Description" type="{http://docs.oasis-open.org/wsfed/authorization/200706}DescriptionType" minOccurs="0"/>
 *         &lt;element name="DisplayValue" type="{http://docs.oasis-open.org/wsfed/authorization/200706}DisplayValueType" minOccurs="0"/>
 *         &lt;choice minOccurs="0">
 *           &lt;element name="Value" type="{http://www.w3.org/2001/XMLSchema}string"/>
 *           &lt;element name="StructuredValue" type="{http://docs.oasis-open.org/wsfed/authorization/200706}StructuredValueType"/>
 *           &lt;element name="ConstrainedValue" type="{http://docs.oasis-open.org/wsfed/authorization/200706}ConstrainedValueType"/>
 *           &lt;any processContents='lax' namespace='##other'/>
 *         &lt;/choice>
 *       &lt;/sequence>
 *       &lt;attribute name="Uri" use="required" type="{http://www.w3.org/2001/XMLSchema}anyURI" />
 *       &lt;attribute name="Optional" type="{http://www.w3.org/2001/XMLSchema}boolean" />
 *       &lt;anyAttribute processContents='lax' namespace='##other'/>
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 *
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "ClaimType", propOrder = { "displayName", "description", "displayValue", "value", "structuredValue",
        "constrainedValue", "any" })
public class ClaimType {

    @XmlElement(name = "DisplayName")
    protected DisplayNameType displayName;
    @XmlElement(name = "Description")
    protected DescriptionType description;
    @XmlElement(name = "DisplayValue")
    protected DisplayValueType displayValue;
    @XmlElement(name = "Value")
    protected String value;
    @XmlElement(name = "StructuredValue")
    protected StructuredValueType structuredValue;
    @XmlElement(name = "ConstrainedValue")
    protected ConstrainedValueType constrainedValue;
    @XmlAnyElement(lax = true)
    protected Object any;
    @XmlAttribute(name = "Uri", required = true)
    @XmlSchemaType(name = "anyURI")
    protected String uri;
    @XmlAttribute(name = "Optional")
    protected Boolean optional;
    @XmlAnyAttribute
    private Map<QName, String> otherAttributes = new HashMap<QName, String>();

    /**
     * Gets the value of the displayName property.
     *
     * @return possible object is {@link DisplayNameType }
     *
     */
    public DisplayNameType getDisplayName() {
        return displayName;
    }

    /**
     * Sets the value of the displayName property.
     *
     * @param value
     *            allowed object is {@link DisplayNameType }
     *
     */
    public void setDisplayName(DisplayNameType value) {
        this.displayName = value;
    }

    /**
     * Gets the value of the description property.
     *
     * @return possible object is {@link DescriptionType }
     *
     */
    public DescriptionType getDescription() {
        return description;
    }

    /**
     * Sets the value of the description property.
     *
     * @param value
     *            allowed object is {@link DescriptionType }
     */
    public void setDescription(DescriptionType value) {
        this.description = value;
    }

    /**
     * Gets the value of the displayValue property.
     *
     * @return possible object is {@link DisplayValueType }
     *
     */
    public DisplayValueType getDisplayValue() {
        return displayValue;
    }

    /**
     * Sets the value of the displayValue property.
     *
     * @param value
     *            allowed object is {@link DisplayValueType }
     *
     */
    public void setDisplayValue(DisplayValueType value) {
        this.displayValue = value;
    }

    /**
     * Gets the value of the value property.
     *
     * @return possible object is {@link String }
     *
     */
    public String getValue() {
        return value;
    }

    /**
     * Sets the value of the value property.
     *
     * @param value
     *            allowed object is {@link String }
     *
     */
    public void setValue(String value) {
        this.value = value;
    }

    /**
     * Gets the value of the structuredValue property.
     *
     * @return possible object is {@link StructuredValueType }
     *
     */
    public StructuredValueType getStructuredValue() {
        return structuredValue;
    }

    /**
     * Sets the value of the structuredValue property.
     *
     * @param value
     *            allowed object is {@link StructuredValueType }
     *
     */
    public void setStructuredValue(StructuredValueType value) {
        this.structuredValue = value;
    }

    /**
     * Gets the value of the constrainedValue property.
     *
     * @return possible object is {@link ConstrainedValueType }
     *
     */
    public ConstrainedValueType getConstrainedValue() {
        return constrainedValue;
    }

    /**
     * Sets the value of the constrainedValue property.
     *
     * @param value
     *            allowed object is {@link ConstrainedValueType }
     *
     */
    public void setConstrainedValue(ConstrainedValueType value) {
        this.constrainedValue = value;
    }

    /**
     * Gets the value of the any property.
     *
     * @return possible object is {@link Element } {@link Object }
     *
     */
    public Object getAny() {
        return any;
    }

    /**
     * Sets the value of the any property.
     *
     * @param value
     *            allowed object is {@link Element } {@link Object }
     *
     */
    public void setAny(Object value) {
        this.any = value;
    }

    /**
     * Gets the value of the uri property.
     *
     * @return possible object is {@link String }
     *
     */
    public String getUri() {
        return uri;
    }

    /**
     * Sets the value of the uri property.
     *
     * @param value
     *            allowed object is {@link String }
     *
     */
    public void setUri(String value) {
        this.uri = value;
    }

    /**
     * Gets the value of the optional property.
     *
     * @return possible object is {@link Boolean }
     *
     */
    public Boolean isOptional() {
        return optional;
    }

    /**
     * Sets the value of the optional property.
     *
     * @param value
     *            allowed object is {@link Boolean }
     *
     */
    public void setOptional(Boolean value) {
        this.optional = value;
    }

    /**
     * Gets a map that contains attributes that aren't bound to any typed property
     * on this class.
     *
     * <p>
     * the map is keyed by the name of the attribute and the value is the string
     * value of the attribute.
     *
     * the map returned by this method is live, and you can add new attribute by
     * updating the map directly. Because of this design, there's no setter.
     *
     *
     * @return always non-null
     */
    public Map<QName, String> getOtherAttributes() {
        return otherAttributes;
    }

}
