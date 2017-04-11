package com.vmware.identity.idm.server.provider;

import java.util.Collection;
import java.util.HashMap;
import com.vmware.identity.idm.PrincipalId;

public class UserSet extends HashMap<PrincipalId, Collection<String>> {

    /**
     * Set of users with list of attribute values. Currently it used for users
     * and associated AltSecurityIdentities values
     */
    private static final long serialVersionUID = 7086712815054577829L;

}
