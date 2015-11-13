
package org.oasis_open.docs.ws_sx.ws_trust._200512;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlType;


/**
 * 
 *         The <wst:RequestSecurityTokenResponseCollection> element (RSTRC) MUST be used to return a security token or 
 *         response to a security token request on the final response.
 *       
 * 
 * <p>Java class for RequestSecurityTokenResponseCollectionType complex type.
 * 
 * <p>The following schema fragment specifies the expected content contained within this class.
 * 
 * <pre>
 * &lt;complexType name="RequestSecurityTokenResponseCollectionType">
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;element ref="{http://docs.oasis-open.org/ws-sx/ws-trust/200512}RequestSecurityTokenResponse"/>
 *       &lt;/sequence>
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 * 
 * 
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "RequestSecurityTokenResponseCollectionType", propOrder = {
    "requestSecurityTokenResponse"
})
public class RequestSecurityTokenResponseCollectionType {

    @XmlElement(name = "RequestSecurityTokenResponse", required = true)
    protected RequestSecurityTokenResponseType requestSecurityTokenResponse;

    /**
     * Gets the value of the requestSecurityTokenResponse property.
     * 
     * @return
     *     possible object is
     *     {@link RequestSecurityTokenResponseType }
     *     
     */
    public RequestSecurityTokenResponseType getRequestSecurityTokenResponse() {
        return requestSecurityTokenResponse;
    }

    /**
     * Sets the value of the requestSecurityTokenResponse property.
     * 
     * @param value
     *     allowed object is
     *     {@link RequestSecurityTokenResponseType }
     *     
     */
    public void setRequestSecurityTokenResponse(RequestSecurityTokenResponseType value) {
        this.requestSecurityTokenResponse = value;
    }

}
