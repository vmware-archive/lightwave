/**
 *
 */
package com.vmware.identity;

import static com.vmware.identity.SharedUtils.bootstrap;
import static com.vmware.identity.SharedUtils.buildMockRequestObject;
import static com.vmware.identity.SharedUtils.buildMockResponseSuccessObject;
import static com.vmware.identity.SharedUtils.createSamlAuthnRequest;
import static com.vmware.identity.SharedUtils.createSamlAuthnResponse;
import static com.vmware.identity.SharedUtils.getMockIdmAccessorFactory;

import java.io.StringWriter;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Date;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Response;

import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.proxyservice.LogonProcessorImpl;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.TestAuthnRequestStateAuthenticationFilter;
import com.vmware.identity.samlservice.impl.AuthnRequestStateCookieWrapper;
import com.vmware.identity.session.impl.SessionManagerImpl;


/**
 * @author root
 *
 */
public class LogonProcessorImplTest {

    private static final int tenantId = 0;

    static LogonProcessorImpl processor;
    private static SessionManagerImpl sessionManager;
    private static String tenant;
    private static AuthenticationFilter<AuthnRequestState> filter;

    @BeforeClass
    public static void setUp() throws Exception {
        sessionManager = new SessionManagerImpl();
        Shared.bootstrap();
        bootstrap();
        tenant = ServerConfig.getTenant(0);
        processor = new LogonProcessorImpl();
        filter = new AuthnRequestStateCookieWrapper(
                new TestAuthnRequestStateAuthenticationFilter(tenantId));
    }

    @Test
    public void testRegisterRequestState() throws Exception {
        String incomingReqId = "200";
        String outRequestId = "42";
        AuthnRequest authnRequest = createSamlAuthnRequest(outRequestId, tenantId);
        StringBuffer sbRequestUrl = new StringBuffer();
        sbRequestUrl.append(authnRequest.getDestination());

        HttpServletRequest incomingRequest = buildMockRequestObject(authnRequest, null, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);
        HttpServletResponse response = buildMockResponseSuccessObject(new StringWriter(), Shared.HTML_CONTENT_TYPE,
                false, null);

        AuthnRequestState requestState = new AuthnRequestState(incomingRequest, response, sessionManager, tenant,
                getMockIdmAccessorFactory(tenantId, 0));
        requestState.parseRequestForTenant(tenant, filter);
        processor.registerRequestState(incomingReqId, outRequestId, requestState);

        Response authnResponse = createSamlAuthnResponse(outRequestId, tenantId, 0);
        HttpServletRequest outgoingRequest = buildMockRequestObject(authnResponse, null, null, null, sbRequestUrl, null,
                null, tenantId);
        Method method = LogonProcessorImpl.class.getDeclaredMethod("findOriginalRequestState", HttpServletRequest.class);
        method.setAccessible(true);
        AuthnRequestState result = (AuthnRequestState) method.invoke(processor, outgoingRequest);

        Assert.assertEquals(requestState.getAuthnRequest().getID(), result.getAuthnRequest().getID());
    }

    @SuppressWarnings("unchecked")
    @Test
    public void testStateMapCleanup() throws Exception {
        //clean maps in logonprocessimpl
        Field outgoingReqToIncomingReqMap = LogonProcessorImpl.class.getDeclaredField("outgoingReqToIncomingReqMap");
        outgoingReqToIncomingReqMap.setAccessible(true);
        Field authnReqStateMap = LogonProcessorImpl.class.getDeclaredField("authnReqStateMap");
        authnReqStateMap.setAccessible(true);
        Map<String, String> inReqIDMap = (Map<String, String>) outgoingReqToIncomingReqMap.get(processor);
        Map<String, AuthnRequestState> stateMap = (Map<String, AuthnRequestState>) authnReqStateMap.get(processor);
        stateMap.clear();
        inReqIDMap.clear();

        //add entries
        int diff = 1000;
        Integer incomingReqId = 1;
        Integer outRequestId = incomingReqId + diff;
        Integer threshHoldSize = LogonProcessorImpl.THRESHHOLD_SIZE_FOR_MAP_CHECK;
        Integer numberOfEntriesToBeAdded = threshHoldSize + 10;
        Date dateOld = new Date(System.currentTimeMillis()-24*60*60*1000);

        HttpServletRequest incomingRequest = null;
        StringBuffer sbRequestUrl = null;
        AuthnRequestState requestState = null;
        while (incomingReqId <= numberOfEntriesToBeAdded) {
            AuthnRequest authnRequest = createSamlAuthnRequest(outRequestId.toString(), tenantId);
            sbRequestUrl = new StringBuffer();
            sbRequestUrl.append(authnRequest.getDestination());

            incomingRequest = buildMockRequestObject(authnRequest, null, null, null, sbRequestUrl,
                    TestConstants.AUTHORIZATION, null, tenantId);
            HttpServletResponse response = buildMockResponseSuccessObject(new StringWriter(), Shared.HTML_CONTENT_TYPE,
                    false, null);

            requestState = new AuthnRequestState(incomingRequest, response, sessionManager, tenant,
                    getMockIdmAccessorFactory(tenantId, 0));
            requestState.setStartTime(dateOld);
            requestState.parseRequestForTenant(tenant, filter);
            processor.registerRequestState(incomingReqId.toString(), outRequestId.toString(), requestState);

            outRequestId++;
            incomingReqId++;
        }

        //check map size == nubmerOfEntriesRemain. all other entries were removed in the last add operation
        Integer nubmerOfEntriesRemain = numberOfEntriesToBeAdded - threshHoldSize;
        Assert.assertTrue(nubmerOfEntriesRemain == inReqIDMap.size());
        Assert.assertTrue(nubmerOfEntriesRemain == stateMap.size());

        Integer inReqIdofLastItem = numberOfEntriesToBeAdded;
        Integer outReqIdofLastItem = inReqIdofLastItem + diff;
        Response authnResponse = createSamlAuthnResponse(outReqIdofLastItem.toString(), tenantId, 0);
        HttpServletRequest outgoingRequest = buildMockRequestObject(authnResponse, null, null, null, sbRequestUrl, null,
                null, tenantId);
        Method method = LogonProcessorImpl.class.getDeclaredMethod("findOriginalRequestState", HttpServletRequest.class);
        method.setAccessible(true);
        AuthnRequestState result = (AuthnRequestState)method.invoke(processor, outgoingRequest);

        //we should be able to find the last added request state.
        Assert.assertEquals(requestState.getAuthnRequest().getID(), result.getAuthnRequest().getID());
    }
}
