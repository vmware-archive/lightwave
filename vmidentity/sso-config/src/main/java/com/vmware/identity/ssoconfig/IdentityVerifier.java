package com.vmware.identity.ssoconfig;

import java.net.URI;

import com.vmware.identity.idm.IIdentityStoreDataEx;

public interface IdentityVerifier {
    public void verify(IIdentityStoreDataEx identitySourceDetails, URI connectionURI) throws Exception;
}
