/* ****************************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. VMware Confidential
 * ****************************************************************************************/
 
package com.vmware.identity.websso.client;

import java.util.Collection;

/**
* MetadataSettings for SSO Service Provider components responsible for
* constructing authentication request to the IDP.
 */
public interface MetadataSettings {

 	/**
     *  Store a service provider configuration entry
     * 
     *@param spConfiguration
     * 		service provider configuration entry.
     */
	void addSPConfiguration(SPConfiguration spConfiguration);

 	/**
     *  update service provider configuration entry
     *  @param spConfiguration
 	 * @throws WebssoClientException
     */
	void updateSPConfiguration (SPConfiguration spConfiguration)
			throws WebssoClientException;

	/**
     *  Return all SP configurations
     *  @return collection
     */
	Collection<SPConfiguration> getAllSPConfigurations();

	/**
     *  Return a SP configuration.
     *   @param	alias
     *   		alias of the configuration
     *   @return	service provider configuration entry.
     * 
     */
	SPConfiguration getSPConfiguration (String alias);

	/**
	 * Return SPConfiguration by entityID
	 * @param entityId
	 * @return
	 */
	SPConfiguration getSPConfigurationByEntityID(String entityId);

	/**
     * Remove SP configuration entry identified with its alias.
     */
	SPConfiguration removeSPConfiguration (String alias);

	/**
     *  Store a IDPr configuration entry
     * 
     *@param idp Configuration
     * 		idp configuration entry.
     */
	void addIDPConfiguration (IDPConfiguration idpConfiguration);

	/**
     *  update ipd configuration entry
     *  @param IDPConfiguration
	 * @throws WebssoClientException
     */
	void updateIDPConfiguration (IDPConfiguration idpConfiguration)
			throws WebssoClientException;

	/**
     *  Return all IDP configurations
     *  @return Collection holding all configurations
     */
	Collection<IDPConfiguration> getAllIDPConfigurations();

	/**
     *  Return a IDP configuration given its alias
     *  @return IDP configuration
	 * @throws WebssoClientException
     */
	IDPConfiguration getIDPConfiguration (String alias);

	/**
     *  Return a IDP configuration given its alias
     *  @return IDP configuration
	 * @throws WebssoClientException
     */
	IDPConfiguration getIDPConfigurationByEntityID (String entityID);

	/**
     *  Remove a IDP configuration
     *  @param alias of the configuration
     * @throws WebssoClientException
     */
	IDPConfiguration removeIDPConfiguration (String alias);

	/**
     *  Clear all configurations at once.
     * 
     */
	void clear();


}
