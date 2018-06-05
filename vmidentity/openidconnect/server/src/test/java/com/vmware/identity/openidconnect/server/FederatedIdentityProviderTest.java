package com.vmware.identity.openidconnect.server;

import static com.vmware.identity.openidconnect.server.FederatedIdentityProvider.*;

import java.util.ArrayList;
import java.util.List;

import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;

public class FederatedIdentityProviderTest {

    private static final String externalUserId = "testExternalUser@gmail.com";
    private static final String internalUserId = "testInternalUser";
    private static final String internalUserDomain = "vmware.com";
    private static final String internalUserUPN = "testInternalUser@vmware.com";
    private static List<String> invalidUsernames;
    static {
        invalidUsernames = new ArrayList<>();
        String usernamePrefix = "abc.123.ABC";
        String usernameSuffix = "def-456-DEF";
        for (char c : invalidCharsForUsername) {
            invalidUsernames.add(usernamePrefix + c + usernameSuffix);
        }
    }

    @Test
    public void testGetPrincipalIdForExternalUseraccount() {
        PrincipalId id = getPrincipalId(externalUserId, null);
        Assert.assertEquals(externalUserId, id.getUPN());
    }

    @Test
    public void testCheckInvalidCharForUsername() {
        for (String username : invalidUsernames) {
            Exception ex = null;
            try {
                checkInvalidCharForUsername(username);
            } catch (Exception e) {
                ex = e;
            }
            Assert.assertTrue(ex != null && ex instanceof IllegalArgumentException);
        }
    }

    @Test
    public void testGetPrincipalForInternalUseraccount() {
        PrincipalId principalId = getPrincipalId(internalUserId, internalUserDomain);
        Assert.assertEquals(internalUserUPN, principalId.getUPN());
    }
}
