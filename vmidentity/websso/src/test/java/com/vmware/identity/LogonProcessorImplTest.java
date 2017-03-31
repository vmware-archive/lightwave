/**
 *
 */
package com.vmware.identity;

import static org.junit.Assert.*;

import java.io.IOException;
import java.io.StringWriter;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Date;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.xml.io.MarshallingException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
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

    private static final int tenantId = 1000;

    static LogonProcessorImpl processor;
    private static IDiagnosticsLogger log;
    private static AuthenticationFilter<AuthnRequestState> filter;
    private static SessionManagerImpl sessionManager;
    private static String tenant;
    private static String relayStateParameter;



    @Test
    public void testRegisterRequestState() throws IOException, MarshallingException, NoSuchMethodException, SecurityException, IllegalAccessException, IllegalArgumentException, InvocationTargetException {

        String incomingReqId = "200";
        String outRequestId = "42"; // the answer to life the universe and everything
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest(outRequestId, tenantId);
        StringBuffer sbRequestUrl = new StringBuffer();
        sbRequestUrl.append(authnRequest.getDestination());

        HttpServletResponse response = SharedUtils.buildMockResponseSuccessObject(new StringWriter(), Shared.HTML_CONTENT_TYPE, false, null);

         // encode
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, null, null, null, sbRequestUrl, null, null, tenantId);

        AuthnRequestState requestState = new AuthnRequestState(request, response,sessionManager,tenant );

        processor.registerRequestState(incomingReqId, outRequestId, requestState);

        Method method = LogonProcessorImpl.class.getDeclaredMethod("findOriginalRequstState", String.class);
        method.setAccessible(true);
        AuthnRequestState result = (AuthnRequestState)method.invoke(processor, request);

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
        Integer incomingReqId = 1;
        Integer outRequestId = 10000; // the answer to life the universe and everything
        Integer threshHoldSize = LogonProcessorImpl.THRESHHOLD_SIZE_FOR_MAP_CHECK;
        Integer numberOfEntriesToBeAdded = threshHoldSize + 10;

        Date dateOld = new Date(System.currentTimeMillis()-24*60*60*1000);

        HttpServletRequest request = null;
        while (incomingReqId <= numberOfEntriesToBeAdded) {

            AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest(outRequestId.toString(), tenantId);
            StringBuffer sbRequestUrl = new StringBuffer();
            sbRequestUrl.append(authnRequest.getDestination());

            HttpServletResponse response = SharedUtils.buildMockResponseSuccessObject(new StringWriter(), Shared.HTML_CONTENT_TYPE, false, null);

             // encode
            request = SharedUtils.buildMockRequestObject(
                    authnRequest, null, null, null, sbRequestUrl, null, null, tenantId);

            AuthnRequestState requestState = new AuthnRequestState(request, response,sessionManager,tenant );
            requestState.setStartTime(dateOld);
            processor.registerRequestState(incomingReqId.toString(), outRequestId.toString(), requestState);
            outRequestId++;
            incomingReqId++;
        }


        //check map size == nubmerOfEntriesRemain. all other entries were removed in the last add operation
        Integer nubmerOfEntriesRemain = numberOfEntriesToBeAdded = threshHoldSize;
        Assert.assertTrue(nubmerOfEntriesRemain == inReqIDMap.size());
        Assert.assertTrue(nubmerOfEntriesRemain == stateMap.size());

        Method method = LogonProcessorImpl.class.getDeclaredMethod("findOriginalRequstState", String.class);
        method.setAccessible(true);
        AuthnRequestState result = (AuthnRequestState)method.invoke(processor, request);

        //we should be able to find the last added request state.
        Integer inReqIdofLastItem = numberOfEntriesToBeAdded;
        Assert.assertEquals(inReqIdofLastItem, result.getAuthnRequest().getID());

    }
    @BeforeClass
    public static void setUp() throws Exception {
        log = DiagnosticsLoggerFactory.getLogger(AuthnRequestStateTest.class);
        filter = new AuthnRequestStateCookieWrapper(
                new TestAuthnRequestStateAuthenticationFilter());
        sessionManager = new SessionManagerImpl();
        Shared.bootstrap();
        SharedUtils.bootstrap(false); // use real data
        tenant = ServerConfig.getTenant(0);
        relayStateParameter = Shared.encodeString(TestConstants.RELAY_STATE);
        processor = new LogonProcessorImpl();
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }
}
