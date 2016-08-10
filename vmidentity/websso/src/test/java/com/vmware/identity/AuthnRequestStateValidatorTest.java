/*
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
 */

package com.vmware.identity;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.opensaml.saml2.core.AuthnRequest;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.AuthnRequestStateValidator;

public class AuthnRequestStateValidatorTest {

	private static IdmAccessor accessor;
	private static AuthnRequest authnRequest;
	private static AuthnRequestState authnRequestState;
	private static String entityId_1 = "TestEntityId_1";
	private static String entityId_2 = "TestEntityId_2";
	private static IDPConfig idpConfig_1;
	private static IDPConfig idpConfig_2;
	private static String tenant = "TestTenant";

	@Before
	public void setUp() throws Exception {
		authnRequest = createMock(AuthnRequest.class);

		accessor = createMock(IdmAccessor.class);
		expect(accessor.getTenant()).andReturn(tenant).anyTimes();
		expect(accessor.getIdpEntityId()).andReturn(entityId_1).anyTimes();
		expect(accessor.getDefaultIdpEntityId()).andReturn(entityId_2).anyTimes();

		authnRequestState = createMock(AuthnRequestState.class);
		expect(authnRequestState.getIdmAccessor()).andReturn(accessor).anyTimes();
		expect(authnRequestState.getAuthnRequest()).andReturn(authnRequest).anyTimes();

		String extIdpEntityId_1 = "TestExternalIdpEntityId_1";
		String extIdpEntityId_2 = "TestExternalIdpEntityId_2";
		idpConfig_1 = new IDPConfig(extIdpEntityId_1);
		idpConfig_2 = new IDPConfig(extIdpEntityId_2);
	}

	/**
	 * Test when no eligible external idps, do local authentication.
	 * @throws Exception
	 */
	@Test
	public void TestValidateScopingLocalAuthN() throws Exception {
		expect(accessor.getExternalIdps()).andReturn(null).once();
		expect(authnRequestState.isIDPSelectionEnabled(tenant)).andReturn(true).once();
		expect(authnRequestState.getProxyCount()).andReturn(null).times(2);
		expect(authnRequestState.isProxying()).andReturn(false);
		expect(authnRequest.getScoping()).andReturn(null).times(2);

		// make sure expected set methods are called
		authnRequestState.setProxying(false);
		expectLastCall().once();
		authnRequestState.setNeedChooseIDPView(false);
		expectLastCall().once();

		replay(authnRequestState);
		replay(accessor);
		replay(authnRequest);

		AuthnRequestStateValidator validator = new AuthnRequestStateValidator();
		ValidationResult vr = validator.validateScoping(authnRequestState);
		Assert.assertNull(vr);

		verify(authnRequestState);
		verify(accessor);
		verify(authnRequest);
	}

	/**
	 * Test when there is one registered external idp and idp selection is not enabled.
	 *
	 * @throws Exception
	 */
	@Test
	public void TestValidateScopingExternalIdpAuthN() throws Exception {
		Collection<IDPConfig> extIdps = new ArrayList<>();
		extIdps.add(idpConfig_1);

		expect(accessor.getExternalIdps()).andReturn(extIdps);
		expect(accessor.getExternalIdpConfigForTenant(tenant, idpConfig_1.getEntityID())).andReturn(idpConfig_1);
		expect(authnRequestState.isIDPSelectionEnabled(tenant)).andReturn(false);
		expect(authnRequestState.getProxyCount()).andReturn(null).times(2);
		expect(authnRequestState.isProxying()).andReturn(true);
		expect(authnRequest.getScoping()).andReturn(null).times(2);

		// make sure expected set methods are called
		authnRequestState.setExtIDPToUse(idpConfig_1);
		expectLastCall().once();
		authnRequestState.setProxying(true);
		expectLastCall().once();
		authnRequestState.setNeedChooseIDPView(false);
		expectLastCall().once();

		replay(authnRequestState);
		replay(accessor);
		replay(authnRequest);

		AuthnRequestStateValidator validator = new AuthnRequestStateValidator();
		ValidationResult vr = validator.validateScoping(authnRequestState);
		Assert.assertNull(vr);

		verify(authnRequestState);
		verify(accessor);
		verify(authnRequest);
	}

	/**
	 * Test when idp selection is enabled, cookie/header are not available
	 * and there are more than one registered, choose idp view is provided.
	 *
	 * @throws Exception
	 */
	@Test
	public void TestValidateScopingNeedChooseIdpView() throws Exception {
		Collection<IDPConfig> extIdps = new ArrayList<>();
		extIdps.add(idpConfig_1);
		extIdps.add(idpConfig_2);

		expect(accessor.getExternalIdps()).andReturn(extIdps);
		expect(authnRequestState.isIDPSelectionEnabled(tenant)).andReturn(true);
		expect(authnRequestState.getProxyCount()).andReturn(null).times(2);
		expect(authnRequestState.isProxying()).andReturn(true);
		expect(authnRequestState.isChooseIDPViewRequired()).andReturn(null);
		expect(authnRequestState.getTenantIDPCookie()).andReturn(null);
		expect(authnRequestState.getTenantIDPSelectHeader()).andReturn(null);
		expect(authnRequest.getScoping()).andReturn(null).times(2);

		// make sure expected set methods are called
		authnRequestState.setNeedChooseIDPView(true);
		expectLastCall().once();
		List<String> expectedIdpList = new ArrayList<>();
		expectedIdpList.add(entityId_1);
		expectedIdpList.add(idpConfig_1.getEntityID());
		expectedIdpList.add(idpConfig_2.getEntityID());
		authnRequestState.setIDPEntityIdList(expectedIdpList);
		expectLastCall().once();

		replay(authnRequestState);
		replay(accessor);
		replay(authnRequest);

		AuthnRequestStateValidator validator = new AuthnRequestStateValidator();
		ValidationResult vr = validator.validateScoping(authnRequestState);
		Assert.assertNull(vr);

		verify(authnRequestState);
		verify(accessor);
		verify(authnRequest);
	}

	/**
	 * Test the case idp selection is enabled and cookie is available.
	 *
	 * @throws Exception
	 */
	@Test
	public void TestValidateScopingTenantIDPCookie() throws Exception {
		Collection<IDPConfig> extIdps = new ArrayList<>();
		extIdps.add(idpConfig_1);
		extIdps.add(idpConfig_2);

		expect(accessor.getExternalIdps()).andReturn(extIdps);
		expect(accessor.getExternalIdpConfigForTenant(tenant, idpConfig_1.getEntityID())).andReturn(idpConfig_1);
		expect(authnRequestState.isIDPSelectionEnabled(tenant)).andReturn(true);
		expect(authnRequestState.getProxyCount()).andReturn(null).times(2);
		expect(authnRequestState.isProxying()).andReturn(true);
		expect(authnRequestState.isChooseIDPViewRequired()).andReturn(null);
		expect(authnRequestState.getTenantIDPCookie()).andReturn(idpConfig_1.getEntityID());
		expect(authnRequest.getScoping()).andReturn(null).times(2);

		// make sure expected set methods are called
		authnRequestState.setProxying(true);
		expectLastCall().once();
		authnRequestState.setNeedChooseIDPView(false);
		expectLastCall().once();
		authnRequestState.setExtIDPToUse(idpConfig_1);
		expectLastCall().once();

		replay(authnRequestState);
		replay(accessor);
		replay(authnRequest);

		AuthnRequestStateValidator validator = new AuthnRequestStateValidator();
		ValidationResult vr = validator.validateScoping(authnRequestState);
		Assert.assertNull(vr);

		verify(authnRequestState);
		verify(accessor);
		verify(authnRequest);
	}

	/**
	 * Test the case idp selection is enabled, cookie is not available but header is available.
	 *
	 * @throws Exception
	 */
	@Test
	public void TestValidateScopingTenantIDPHeader() throws Exception {
		Collection<IDPConfig> extIdps = new ArrayList<>();
		extIdps.add(idpConfig_1);
		extIdps.add(idpConfig_2);

		expect(accessor.getExternalIdps()).andReturn(extIdps);
		expect(accessor.getExternalIdpConfigForTenant(tenant, idpConfig_1.getEntityID())).andReturn(idpConfig_1);
		expect(authnRequestState.isIDPSelectionEnabled(tenant)).andReturn(true);
		expect(authnRequestState.getProxyCount()).andReturn(null).times(2);
		expect(authnRequestState.isProxying()).andReturn(true);
		expect(authnRequestState.isChooseIDPViewRequired()).andReturn(null);
		expect(authnRequestState.getTenantIDPCookie()).andReturn(null);
		expect(authnRequestState.getTenantIDPSelectHeader()).andReturn(idpConfig_1.getEntityID());
		expect(authnRequestState.getResponse()).andReturn(null);
		expect(authnRequest.getScoping()).andReturn(null).times(2);

		// make sure expected set methods are called
		authnRequestState.setProxying(true);
		expectLastCall().once();
		authnRequestState.setNeedChooseIDPView(false);
		expectLastCall().once();
		authnRequestState.setExtIDPToUse(idpConfig_1);
		expectLastCall().once();
		authnRequestState.addTenantIDPCookie(idpConfig_1.getEntityID(), null);
		expectLastCall().once();

		replay(authnRequestState);
		replay(accessor);
		replay(authnRequest);

		AuthnRequestStateValidator validator = new AuthnRequestStateValidator();
		ValidationResult vr = validator.validateScoping(authnRequestState);
		Assert.assertNull(vr);

		verify(authnRequestState);
		verify(accessor);
		verify(authnRequest);
	}
}
