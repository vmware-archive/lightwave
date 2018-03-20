package com.vmware.identity.rest.idm.client.test.integration.util;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.vmware.identity.rest.idm.data.FederatedIdpDTO;
import com.vmware.identity.rest.idm.data.FederatedOidcConfigDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;

public class FederatedIdpGenerator {

    private static final String TEST_IDP_NAME = "federated.test.integration";
    private static final String IDP_PROTOCOL_OAUTH_2_0 = "urn:oasis:names:tc:OAUTH:2.0:protocol";

    public static FederatedIdpDTO generateFederatedIdpDTO() {
        return new FederatedIdpDTO.Builder()
                .withEntityID(TEST_IDP_NAME)
                .withJitEnabled(true)
                .withMultiTenant(true)
                .withProtocol(IDP_PROTOCOL_OAUTH_2_0)
                .withUpnSuffix("test.dom")
                .withOidcConfig(generateOidcConfig())
                .withRoleGroupMappings(getRoleGroupMappings())
                .build();
    }

    private static FederatedOidcConfigDTO generateOidcConfig() {
        return new FederatedOidcConfigDTO.Builder()
                .withAuthorizeEndpoint("https://testhost/authorize")
                .withClientId("test_client_id")
                .withClientSecret("test_sercret")
                .withIssuerType("test_issuer_type")
                .withJwksEndpoint("https://testhost/jwks")
                .withLogoutEndpoint("https://testhost/logout")
                .withMetadataEndpoint("https://testhost/metadata")
                .withPostLogoutEndpoint("https://testhost/login")
                .withRedirectURL("https://testhost/service")
                .withTokenEndpoint("htttps://testhost/token")
                .build();
    }

    private static List<TokenClaimGroupDTO> getRoleGroupMappings() {
        List<TokenClaimGroupDTO> claimGroups = new ArrayList<TokenClaimGroupDTO>();

        TokenClaimGroupDTO group = new TokenClaimGroupDTO.Builder()
            .withClaimName("perm")
            .withClaimValue("org_member")
            .withGroups(Arrays.asList(new String[] { "Users" }))
            .build();

        claimGroups.add(group);
        return claimGroups;
    }
}
