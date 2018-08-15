package org.oasis_open.docs.wsfed.authorization._200706;

import javax.xml.bind.JAXBElement;
import javax.xml.bind.annotation.XmlElementDecl;
import javax.xml.bind.annotation.XmlRegistry;
import javax.xml.namespace.QName;

/**
 * This object contains factory methods for each Java content interface and Java
 * element interface generated in the
 * org.oasis_open.docs.wsfed.authorization._200706
 * package.
 * <p>
 * An ObjectFactory allows you to programmatically construct new instances of the
 * Java representation for XML content. The Java representation of XML content
 * can consist of schema derived interfaces and classes representing the binding
 * of schema type definitions, element declarations and model groups. Factory
 * methods for each of these are provided in this class.
 * 
 */
@XmlRegistry
public class ObjectFactory {

    private final static QName _ClaimType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "ClaimType");
    private final static QName _ConstrainedManyValueType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "ConstrainedManyValueType");
    private final static QName _ConstrainedSingleValueType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "ConstrainedSingleValueType");
    private final static QName _ConstrainedValueType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "ConstrainedValueType");
    private final static QName _DescriptionType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "DescriptionType");
    private final static QName _DisplayNameType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "DisplayNameType");
    private final static QName _DisplayValueType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "DisplayValueType");
    private final static QName _StructuredValueType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "StructuredValueType");
    private final static QName _ValueInRangeType_QName = new QName(
            "http://docs.oasis-open.org/wsfed/authorization/200706",
            "ValueInRangeType");

    /**
     * Create a new ObjectFactory that can be used to create new instances of
     * schema derived classes for package: org.oasis_open.docs.wss._2004._01.
     * oasis_200401_wss_wssecurity_utility_1_0
     * 
     */
    public ObjectFactory() {
    }

    /**
     * Create an instance of {@link ClaimType }
     * 
     */
    public ClaimType createClaimType() {
        return new ClaimType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link ClaimType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "ClaimType")
    public JAXBElement<ClaimType> createClaimType(ClaimType value) {
        return new JAXBElement<ClaimType>(_ClaimType_QName,
                ClaimType.class, null, value);
    }

    /**
     * Create an instance of {@link ConstrainedManyValueType }
     * 
     */
    public ConstrainedManyValueType createConstrainedManyValueType() {
        return new ConstrainedManyValueType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link ConstrainedManyValueType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "ConstrainedManyValueType")
    public JAXBElement<ConstrainedManyValueType> createConstrainedManyValueType(ConstrainedManyValueType value) {
        return new JAXBElement<ConstrainedManyValueType>(_ConstrainedManyValueType_QName,
                ConstrainedManyValueType.class, null, value);
    }

    /**
     * Create an instance of {@link ConstrainedSingleValueType }
     * 
     */
    public ConstrainedSingleValueType createConstrainedSingleValueType() {
        return new ConstrainedSingleValueType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link ConstrainedSingleValueType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "ConstrainedSingleValueType")
    public JAXBElement<ConstrainedSingleValueType> createConstrainedSingleValueType(ConstrainedSingleValueType value) {
        return new JAXBElement<ConstrainedSingleValueType>(_ConstrainedSingleValueType_QName,
                ConstrainedSingleValueType.class, null, value);
    }

    /**
     * Create an instance of {@link ConstrainedValueType }
     * 
     */
    public ConstrainedValueType createConstrainedValueType() {
        return new ConstrainedValueType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link ConstrainedValueType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "ConstrainedValueType")
    public JAXBElement<ConstrainedValueType> createConstrainedValueType(ConstrainedValueType value) {
        return new JAXBElement<ConstrainedValueType>(_ConstrainedValueType_QName,
                ConstrainedValueType.class, null, value);
    }

    /**
     * Create an instance of {@link DescriptionType }
     * 
     */
    public DescriptionType createDescriptionType() {
        return new DescriptionType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link DescriptionType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "DescriptionType")
    public JAXBElement<DescriptionType> createDescriptionType(DescriptionType value) {
        return new JAXBElement<DescriptionType>(_DescriptionType_QName,
                DescriptionType.class, null, value);
    }

    /**
     * Create an instance of {@link DisplayNameType }
     * 
     */
    public DisplayNameType createDisplayNameType() {
        return new DisplayNameType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link DisplayNameType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "DisplayNameType")
    public JAXBElement<DisplayNameType> createDisplayNameType(DisplayNameType value) {
        return new JAXBElement<DisplayNameType>(_DisplayNameType_QName,
                DisplayNameType.class, null, value);
    }

    /**
     * Create an instance of {@link DisplayValueType }
     * 
     */
    public DisplayValueType createDisplayValueType() {
        return new DisplayValueType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link DisplayValueType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "DisplayValueType")
    public JAXBElement<DisplayValueType> createDisplayValueType(DisplayValueType value) {
        return new JAXBElement<DisplayValueType>(_DisplayValueType_QName,
                DisplayValueType.class, null, value);
    }

    /**
     * Create an instance of {@link StructuredValueType }
     * 
     */
    public StructuredValueType createStructuredValueType() {
        return new StructuredValueType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link StructuredValueType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "StructuredValueType")
    public JAXBElement<StructuredValueType> createDStructuredValueType(StructuredValueType value) {
        return new JAXBElement<StructuredValueType>(_StructuredValueType_QName,
                StructuredValueType.class, null, value);
    }

    /**
     * Create an instance of {@link ValueInRangeType }
     * 
     */
    public ValueInRangeType createValueInRangeType() {
        return new ValueInRangeType();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}
     * {@link ValueInRangeType }{@code >}
     * 
     */
    @XmlElementDecl(namespace = "http://docs.oasis-open.org/wsfed/authorization/200706", name = "ValueInRangeType")
    public JAXBElement<ValueInRangeType> createValueInRangeType(ValueInRangeType value) {
        return new JAXBElement<ValueInRangeType>(_ValueInRangeType_QName,
                ValueInRangeType.class, null, value);
    }
}

