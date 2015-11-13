/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.idm;

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/7/11
 * Time: 5:57 PM
 * To change this template use File | Settings | File Templates.
 */

import java.io.Serializable;
import java.security.cert.Certificate;
import java.util.Collection;

public class RelyingParty implements Serializable
{
	private static final long serialVersionUID = 3852289172990602788L;

	private String                     name;
	private String                     url;
    private Collection<SignatureAlgorithm>       signatureAlgorithms;
    private Collection<AssertionConsumerService> assertionConsumerServices;
    private Collection<AttributeConsumerService> attributeConsumerServices;
    private Collection<ServiceEndpoint> singleLogoutServices;
    private Certificate                certificate;
    private String                     defaultAssertionConsumerService;
    private String                     defaultAttributeConsumerService;
    private boolean                    authnRequestsSigned;

    public RelyingParty(String name)
    {
        ValidateUtil.validateNotEmpty(name, "name");
        this.name = name;
    }

    public String getName()
    {
        return this.name;
    }

    public Collection<SignatureAlgorithm> getSignatureAlgorithms()
    {
        return signatureAlgorithms;
    }

    public void
    setSignatureAlgorithms(
    	Collection<SignatureAlgorithm> signatureAlgorithms
    	)
    {
        this.signatureAlgorithms = signatureAlgorithms;
    }

    public Collection<AssertionConsumerService> getAssertionConsumerServices()
    {
        return assertionConsumerServices;
    }

    public
    void
    setAssertionConsumerServices(
    	Collection<AssertionConsumerService> assertionConsumerServices
    	)
    {
        this.assertionConsumerServices = assertionConsumerServices;
    }

    public Collection<AttributeConsumerService> getAttributeConsumerServices()
    {
        return attributeConsumerServices;
    }

    public
    void
    setAttributeConsumerServices(
        Collection<AttributeConsumerService> attributeConsumerServices
        )
    {
        this.attributeConsumerServices = attributeConsumerServices;
    }

    public Collection<ServiceEndpoint> getSingleLogoutServices()
    {
        return singleLogoutServices;
    }

    public
    void
    setSingleLogoutServices(
        Collection<ServiceEndpoint> singleLogoutServices
    	)
    {
        this.singleLogoutServices = singleLogoutServices;
    }

    public Certificate getCertificate()
    {
    	return certificate;
    }

    public void setCertificate(Certificate certificate)
    {
    	this.certificate = certificate;
    }

	public String getUrl()
	{
		return url;
	}

	public void setUrl(String url)
	{
        ValidateUtil.validateNotEmpty(url, "url");
		this.url = url;
	}

	/**
	 * @return the defaultAssertionConsumerService
	 */
	public String getDefaultAssertionConsumerService()
	{
		return defaultAssertionConsumerService;
	}

	/**
	 * @param defaultAssertionConsumerService the defaultAssertionConsumerService to set
	 */
	public void setDefaultAssertionConsumerService(
			String defaultAssertionConsumerService)
	{
		this.defaultAssertionConsumerService = defaultAssertionConsumerService;
	}

	/**
	 * @return the defaultAttributeConsumerService
	 */
	public String getDefaultAttributeConsumerService()
	{
		return defaultAttributeConsumerService;
	}

	/**
	 * @param defaultAttributeConsumerService the defaultAttributeConsumerService to set
	 */
	public void setDefaultAttributeConsumerService(
			String defaultAttributeConsumerService)
	{
		this.defaultAttributeConsumerService = defaultAttributeConsumerService;
	}

	/**
	 * @return the authnRequestsSigned
	 */
	public boolean isAuthnRequestsSigned()
	{
		return authnRequestsSigned;
	}

	/**
	 * @param authnRequestsSigned the authnRequestsSigned to set
	 */
	public void setAuthnRequestsSigned(boolean authnRequestsSigned)
	{
		this.authnRequestsSigned = authnRequestsSigned;
	}
}
