/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. VMware Confidential
 * ************************************************************************/
package com.vmware.identity.websso.client;

/**
 * The structure holds Assertion Consumer Service data. It has a number of properties with getters and setters.
 * 
 * 
 * Here is an example of configuring it through the Spring XML.
 * 
 * <pre>
 *       <bean id="myAssertionConsumerServiceBean1" class="com.vmware.identity.websso.client.AssertionConsumerService">
 *           <property name="location">
 *               <value>http://...</value>
 *           </property>
 *           <property name="isDefault">
 *                 <value>true</value>
 *             </property>
 *             <property name="binding">
 *                 <value>urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST</value>
 *             </property>
 *             <property name="index">
 *                 <value>0</value>
 *             </property>
 *         </bean>
 * </pre>
 * 
 */
public class AssertionConsumerService {

    private String location;

    private boolean isDefault;

    private String binding;

    private int index;

    /**
     * Construct AssertionConsumerService object. This object correspond to SAML metedata protocal
     * AssertionConsumerIndexType
     * 
     * @param location
     *            Required. URL value.
     * @param binding
     *            Required. uri of the binding type per SAML2.0 metadata protocal.
     * @param index
     *            Required. index addtribute.
     * @param isDefault
     *            Optional. attribute.
     */
    public AssertionConsumerService(String location, boolean isDefault, String binding, int index) {
        this.location = location;
        this.isDefault = isDefault;
        this.binding = binding;
        this.index = index;
    }

    public void setLocation(String location) {
        this.location = location;
    }

    public String getLocation() {
        return location;
    }

    public void setIsDefault(boolean isDefault) {
        this.isDefault = isDefault;
    }

    public boolean isDefault() {
        return isDefault;
    }

    public void setBinding(String binding) {
        this.binding = binding;
    }

    public String getBinding() {
        return binding;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public int getIndex() {
        return index;
    }

}
