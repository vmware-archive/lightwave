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
package com.vmware.identity.rest.idm.server.resources;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.DuplicateTenantException;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPasswordPolicyException;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;
import com.vmware.identity.rest.idm.data.BrandPolicyDTO;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;
import com.vmware.identity.rest.idm.data.PasswordPolicyDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.ProviderPolicyDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.TokenPolicyDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.data.attributes.SearchType;
import com.vmware.identity.rest.idm.data.attributes.TenantConfigType;
import com.vmware.identity.rest.idm.server.PathParameters;
import com.vmware.identity.rest.idm.server.mapper.AuthenticationPolicyMapper;
import com.vmware.identity.rest.idm.server.mapper.CertificateMapper;
import com.vmware.identity.rest.idm.server.mapper.GroupMapper;
import com.vmware.identity.rest.idm.server.mapper.LockoutPolicyMapper;
import com.vmware.identity.rest.idm.server.mapper.PasswordPolicyMapper;
import com.vmware.identity.rest.idm.server.mapper.ProviderPolicyMapper;
import com.vmware.identity.rest.idm.server.mapper.SolutionUserMapper;
import com.vmware.identity.rest.idm.server.mapper.TenantMapper;
import com.vmware.identity.rest.idm.server.mapper.UserMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Tenant resource. Serves information specifically about tenants.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Path("/tenant")
public class TenantResource extends BaseResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(TenantResource.class);

    public TenantResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Creates new tenant
     *
     * @param tenantName Name of tenant to create
     * @param longTenantName (Optional - long name of tenant )
     * @return
     * <code> 200 </code> If tenant was created successfully.
     * <code> 500 </code> Otherwise
     * @throws {@link BadRequestException} On bad requests (invalid input)
     * @throws {@link InternalServerErrorException} Otherwise
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public TenantDTO create(TenantDTO tenantDTO) {

        Validate.notNull(tenantDTO.getCredentials(), sm.getString("valid.not.null", "tenant credentials"));
        Validate.notEmpty(tenantDTO.getCredentials().getCertificates(), sm.getString("valid.not.empty", "trusted certificates"));
        Validate.notNull(tenantDTO.getCredentials().getPrivateKey(), sm.getString("valid.not.null", "signing certificate"));
        try {
            // Create tenant
            Tenant tenantToCreate = TenantMapper.getTenant(tenantDTO);
            PrincipalId adminId = PrincipalUtil.fromName(tenantDTO.getUsername());
            getIDMClient().addTenant(tenantToCreate, adminId.getName(), tenantDTO.getPassword().toCharArray());

            // Set tenant credentials (signing + trusted certs) to above created tenant
            try {
                List<CertificateDTO> signatureCerts = tenantDTO.getCredentials().getCertificates();
                PrivateKeyDTO tenantPrivateKey = tenantDTO.getCredentials().getPrivateKey();
                getIDMClient().setTenantCredentials(tenantDTO.getName(), CertificateMapper.getCertificates(signatureCerts), tenantPrivateKey.getPrivateKey());
            } catch (Exception e) {
                // Exception occurred while setting credentials - delete the tenant we created so it
                // doesn't wind up in a bizarre state
                try {
                    getIDMClient().deleteTenant(tenantDTO.getName());
                } catch (Exception e1) {
                    log.error("Failed to delete tenant '{}' after failed creation due to a server side error", tenantDTO.getName(), e1);
                    throw new InternalServerErrorException(sm.getString("ec.500"), e1);
                }

                throw e;
            }

            return TenantMapper.getTenantDTO(getIDMClient().getTenant(tenantDTO.getName()));

        } catch (DuplicateTenantException | DTOMapperException | InvalidArgumentException e) {
            log.warn("Failed to create tenant '{}' due to a client side error", tenantDTO.getName(), e);
            throw new BadRequestException(sm.getString("res.ten.create.failed", tenantDTO.getName()), e);
        } catch (Exception e) {
            log.error("Failed to create tenant '{}' due to a server side error", tenantDTO.getName(), e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get details of tenant
     *
     * @param tenantName Name of tenant to retrieve details
     * @return Details of Tenant. @see {@link Tenant}
     */
    @GET @Path(PathParameters.TENANT_NAME_VAR)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public TenantDTO get(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        try {
            return TenantMapper.getTenantDTO(getIDMClient().getTenant(tenantName));
        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve tenant '{}'", tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.error("Failed to retrieve tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException(sm.getString("res.ten.get.failed", tenantName), e);
        } catch (Exception e) {
            log.error("Failed to retrieve tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Search principals(users, groups, solution users) on tenant
     *
     * @param query search string used to find users. Matching users will be those that have this
     *        particular string as a substring in their user principal name. If name is empty or
     *        null, expect to return all users associated with tenant.
     * @param limit Maximum number of users to be retrieved. No limit is set on negative value.
     * @param domain name of domain to search principals on tenant
     * @param searchBy semantics used to search members in tenant @see SearchType
     */
    @GET @Path(PathParameters.TENANT_NAME_VAR + "/" + PathParameters.SEARCH)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public SearchResultDTO searchMembers(@PathParam(PathParameters.TENANT_NAME) String tenantName,
            @DefaultValue("all") @QueryParam("type") String memberType,
            @QueryParam("domain") String domain,
            @DefaultValue("200") @QueryParam("limit") int limit,
            @DefaultValue("NAME") @QueryParam("searchBy") String searchBy,
            @DefaultValue("") @QueryParam("query") String query) {

        validateMemberType(memberType.toUpperCase());
        validateSearchBy(searchBy.toUpperCase());

        MemberType requestedMemberType = MemberType.valueOf(memberType.toUpperCase());
        SearchType requestedSearchType = SearchType.valueOf(searchBy.toUpperCase());

        Set<UserDTO> users = null;
        Set<GroupDTO> groups = null;
        Set<SolutionUserDTO> solutionUsers = null;

        try {
            SearchCriteria criteria = new SearchCriteria(query, domain);
            if (requestedMemberType == MemberType.ALL) {
                Map<MemberType, Integer> memberToLimit = computeSearchLimits(limit, MemberType.ALL);
                users = searchPersonUsers(tenantName, criteria, memberToLimit.get(MemberType.USER));
                groups = searchGroups(tenantName, criteria, memberToLimit.get(MemberType.GROUP));
                solutionUsers = searchSolutionUsers(tenantName, criteria, memberToLimit.get(MemberType.SOLUTIONUSER), requestedSearchType);
            } else {
                switch (requestedMemberType) {
                case GROUP:
                    groups = searchGroups(tenantName, criteria, limit);
                    break;
                case SOLUTIONUSER:
                    solutionUsers = searchSolutionUsers(tenantName, criteria, limit, requestedSearchType);
                    break;
                case USER:
                    users = searchPersonUsers(tenantName, criteria, limit);
                    break;
                }
            }
            return SearchResultDTO.builder()
                    .withUsers(users)
                    .withGroups(groups)
                    .withSolutionUsers(solutionUsers)
                    .build();

        } catch (NoSuchTenantException e) {
            log.debug("Failed to search members on tenant '{}'", tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.error("Failed to search members on tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException("");
        } catch (Exception e) {
            log.error("Failed to search members on tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Delete a tenant
     *
     * @param tenantName Name of tenant to be deleted
     * @return
     * <code> HTTP 200 OK </code> If deletion is successful <br/>
     * <code> HTTP 500 {@link InternalServerErrorException} </code> Otherwise
     */
    @DELETE @Path(PathParameters.TENANT_NAME_VAR)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public void delete(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        try {
            getIDMClient().deleteTenant(tenantName);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to delete tenant '{}'", tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.error("Failed to delete tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException(sm.getString("res.ten.delete.failed", tenantName), e);
        } catch (Exception e) {
            log.error("Failed to delete tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    @GET @Path(PathParameters.TENANT_NAME_VAR + "/config")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public TenantConfigurationDTO getConfig(@PathParam(PathParameters.TENANT_NAME) String tenantName,
            @QueryParam("type") final List<String> configTypes) {

        Set<TenantConfigType> requestedConfigs = new HashSet<TenantConfigType>();
        for (String configType : configTypes) {
            validateConfigType(configType);
            requestedConfigs.add(TenantConfigType.valueOf(configType.toUpperCase()));
        }

        // Default to retrieve complete tenant configuration
        if (requestedConfigs.size() == 0) {
            requestedConfigs.add(TenantConfigType.ALL);
        }

        LockoutPolicyDTO lockoutPolicy = null;
        PasswordPolicyDTO passwordPolicy = null;
        TokenPolicyDTO tokenPolicy = null;
        ProviderPolicyDTO providerPolicy = null;
        BrandPolicyDTO brandPolicy = null;
        AuthenticationPolicyDTO authenticationPolicy = null;

        try {
            if (requestedConfigs.contains(TenantConfigType.ALL)) {
                lockoutPolicy = getLockoutPolicy(tenantName);
                passwordPolicy = getPasswordPolicy(tenantName);
                tokenPolicy = getTokenPolicy(tenantName);
                providerPolicy = getProviderPolicy(tenantName);
                brandPolicy = getBrandPolicy(tenantName);
                authenticationPolicy = getAuthenticationPolicy(tenantName);
            } else {
                for (TenantConfigType type : requestedConfigs) {
                    switch (type) {
                    case LOCKOUT:
                        lockoutPolicy = getLockoutPolicy(tenantName);
                        break;

                    case PASSWORD:
                        passwordPolicy = getPasswordPolicy(tenantName);
                        break;

                    case TOKEN:
                        tokenPolicy = getTokenPolicy(tenantName);
                        break;

                    case PROVIDER:
                        providerPolicy = getProviderPolicy(tenantName);
                        break;

                    case BRAND:
                        brandPolicy = getBrandPolicy(tenantName);
                        break;

                    case AUTHENTICATION:
                        authenticationPolicy = getAuthenticationPolicy(tenantName);
                        break;
                    }
                }
            }

            return TenantConfigurationDTO.builder()
                    .withLockoutPolicy(lockoutPolicy)
                    .withPasswordPolicy(passwordPolicy)
                    .withTokenPolicy(tokenPolicy)
                    .withProviderPolicy(providerPolicy)
                    .withBrandPolicy(brandPolicy)
                    .withAuthenticationPolicy(authenticationPolicy)
                    .build();

        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve configuration details of tenant '{}'", tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (IllegalArgumentException | InvalidArgumentException e) {
            log.error("Failed to retrieve configuration details of tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException(sm.getString("res.ten.get.config.failed", Arrays.asList(requestedConfigs), tenantName), e);
        } catch (Exception e) {
            log.error("Failed to retrieve configuration details of tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

    }

    @PUT @Path(PathParameters.TENANT_NAME_VAR + "/config")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public TenantConfigurationDTO updateConfig(@PathParam(PathParameters.TENANT_NAME) String tenantName, TenantConfigurationDTO configurationDTO) {

        TenantConfigurationDTO.Builder configBuilder = TenantConfigurationDTO.builder();
        TokenPolicyDTO tokenPolicy = configurationDTO.getTokenPolicy();
        ProviderPolicyDTO providerPolicy = configurationDTO.getProviderPolicy();
        BrandPolicyDTO brandPolicy = configurationDTO.getBrandPolicy();
        AuthenticationPolicyDTO authenticationPolicy = configurationDTO.getAuthenticationPolicy();

        try {

            // update token policy configuration of tenant
            if (tokenPolicy != null) {
                updateTokenPolicy(tenantName, tokenPolicy);
                configBuilder.withTokenPolicy(getTokenPolicy(tenantName));
            }

            // update provider policy configuration of tenant
            if (providerPolicy != null) {
                getIDMClient().setDefaultProviders(tenantName, Arrays.asList(providerPolicy.getDefaultProvider()));
                // TODO : Update default provider alias on provision of API. As interim, Can update on IdentityProviderResource
                getIDMClient().setTenantIDPSelectionEnabled(tenantName, providerPolicy.isProviderSelectionEnabled());
                configBuilder.withProviderPolicy(getProviderPolicy(tenantName));
            }

            // update branding policy configuration of tenant
            if (brandPolicy != null) {
                getIDMClient().setBrandName(tenantName, brandPolicy.getName());
                getIDMClient().setLogonBannerTitle(tenantName, brandPolicy.getLogonBannerTitle());
                getIDMClient().setLogonBannerContent(tenantName, brandPolicy.getLogonBannerContent());
                if (brandPolicy.isLogonBannerCheckboxEnabled() != null) {
                    getIDMClient().setLogonBannerCheckboxFlag(tenantName, brandPolicy.isLogonBannerCheckboxEnabled());
                }

                boolean disableFlag = brandPolicy.isLogonBannerDisabled() == null ? false : brandPolicy.isLogonBannerDisabled();
                if (disableFlag) {
                    getIDMClient().disableLogonBanner(tenantName);
                }
                configBuilder.withBrandPolicy(getBrandPolicy(tenantName));
            }

            // Update authentication policy configuration of tenant
            if (authenticationPolicy != null) {
                AuthnPolicy authnPolicy = AuthenticationPolicyMapper.getAuthenticationPolicy(authenticationPolicy);
                getIDMClient().setAuthnPolicy(tenantName, authnPolicy);
                configBuilder.withAuthenticationPolicy(getAuthenticationPolicy(tenantName));
            }

            return configBuilder.build();

        } catch (NoSuchTenantException e) {
            log.debug("Failed to update the configuration details for tenant '{}'",tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"),e);
        } catch( IllegalArgumentException | InvalidArgumentException | InvalidPasswordPolicyException e){
            log.error("Failed to update the configuration details for tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException(sm.getString("res.ten.update.config.failed", tenantName), e);
        } catch (Exception e){
            log.error("Failed to update the configuration details for tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/providers")
    public IdentityProviderResource getIdentityProviderSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new IdentityProviderResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/externalidp")
    public ExternalIDPResource getExternalIDPSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new ExternalIDPResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/certificates")
    public CertificateResource getCertificateSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new CertificateResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/groups")
    public GroupResource getGroupSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new GroupResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/users")
    public UserResource getUserSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new UserResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/solutionusers")
    public SolutionUserResource getSolutionUserSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new SolutionUserResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/relyingparty")
    public RelyingPartyResource getRelyingPartySubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new RelyingPartyResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/oidcclient")
    public OIDCClientResource getOIDCClientSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new OIDCClientResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/resourceserver")
    public ResourceServerResource getResourceServerSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new ResourceServerResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/diagnostics")
    public DiagnosticsResource getDiagnosticsSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new DiagnosticsResource(tenantName, getRequest(), getSecurityContext());
    }

    private void validateConfigType(String configName) {
        try {
            TenantConfigType.valueOf(configName.toUpperCase());
        } catch (IllegalArgumentException e) {
            log.error("Invalid tenant configuration parameter '{}'. Valid values are {}", configName, Arrays.asList(TenantConfigType.values()), e);
            throw new BadRequestException(sm.getString("valid.invalid.type", configName, TenantConfigType.values()), e);
        }
    }

    private void validateMemberType(String memberType) {
        try {
            MemberType.valueOf(memberType);
        } catch (IllegalArgumentException e) {
            log.error("Invalid tenant configuration parameter '{}'. Valid values are {}", memberType, Arrays.asList(MemberType.values()), e);
            throw new BadRequestException(sm.getString("valid.invalid.type", memberType, MemberType.values()), e);
        }
    }

    private void validateSearchBy(String searchBy) {
        try {
            SearchType.valueOf(searchBy);
        } catch (IllegalArgumentException | NullPointerException e) {
            log.debug("Invalid searchtype '{}'", searchBy);
            throw new BadRequestException(sm.getString("valid.invalid.type", "type", Arrays.toString(SearchType.values())), e);
        }
    }

    private LockoutPolicyDTO getLockoutPolicy(String tenantName) throws Exception {
        return LockoutPolicyMapper.getLockoutPolicyDTO(getIDMClient().getLockoutPolicy(tenantName));
    }

    private PasswordPolicyDTO getPasswordPolicy(String tenantName) throws Exception {
        return PasswordPolicyMapper.getPasswordPolicyDTO(getIDMClient().getPasswordPolicy(tenantName));
    }

    private AuthenticationPolicyDTO getAuthenticationPolicy(String tenantName) throws Exception {
        return AuthenticationPolicyMapper.getAuthenticationPolicyDTO(getIDMClient().getAuthnPolicy(tenantName));
    }

    private Long getClockTolerance(String tenantName) throws Exception {
        return getIDMClient().getClockTolerance(tenantName);
    }

    private Long getBearerTokenLifetime(String tenantName) throws Exception {
        return getIDMClient().getMaximumBearerTokenLifetime(tenantName);
    }

    private Long getHOKTokenLifetime(String tenantName) throws Exception {
        return getIDMClient().getMaximumHoKTokenLifetime(tenantName);
    }

    private Long getBearerRefreshTokenLifetime(String tenantName) throws Exception {
        return getIDMClient().getMaximumBearerRefreshTokenLifetime(tenantName);
    }

    private Long getHoKRefreshTokenLifetime(String tenantName) throws Exception {
        return getIDMClient().getMaximumHoKRefreshTokenLifetime(tenantName);
    }

    private Integer getRenewCount(String tenantName) throws Exception {
        return getIDMClient().getRenewCount(tenantName);
    }

    private Integer getDelegationCount(String tenantName) throws Exception {
        return getIDMClient().getDelegationCount(tenantName);
    }

    private TokenPolicyDTO getTokenPolicy(String tenantName) throws Exception {
        return TokenPolicyDTO.builder()
                .withClockToleranceMillis(getClockTolerance(tenantName))
                .withDelegationCount(getDelegationCount(tenantName))
                .withMaxBearerTokenLifeTimeMillis(getBearerTokenLifetime(tenantName))
                .withMaxHOKTokenLifeTimeMillis(getHOKTokenLifetime(tenantName))
                .withMaxBearerRefreshTokenLifeTimeMillis(getBearerRefreshTokenLifetime(tenantName))
                .withMaxHOKRefreshTokenLifeTimeMillis(getHoKRefreshTokenLifetime(tenantName))
                .withRenewCount(getRenewCount(tenantName))
                .build();
    }

        private ProviderPolicyDTO getProviderPolicy(String tenantName) throws Exception {
                ProviderPolicyDTO providerPolicyDTO = null;
                Collection<String> defaultProviders = getIDMClient().getDefaultProviders(tenantName);
                if(defaultProviders != null) {
                          IIdentityStoreData defaultIdentitySource = getIDMClient().getProvider(tenantName, defaultProviders.iterator().next());
                          providerPolicyDTO = ProviderPolicyMapper.getProviderPolicyDTO(getIDMClient().getDefaultProviders(tenantName),
                                                defaultIdentitySource.getExtendedIdentityStoreData() != null ? defaultIdentitySource.getExtendedIdentityStoreData().getAlias() : null,
                                                                  getIDMClient().isTenantIDPSelectionEnabled(tenantName));
                        }
                return providerPolicyDTO;
            }

    private BrandPolicyDTO getBrandPolicy(String tenantName) throws Exception {
        String logonBannerTitle = getIDMClient().getLogonBannerTitle(tenantName);
        String logonBannerContent = getIDMClient().getLogonBannerContent(tenantName);
        boolean disableLogonBanner = false;
        if (logonBannerTitle == null || logonBannerContent == null) {
            disableLogonBanner = true;
        }
        return BrandPolicyDTO.builder()
                .withName(getIDMClient().getBrandName(tenantName))
                .withLogonBannerTitle(logonBannerTitle)
                .withLogonBannerContent(logonBannerContent)
                .withLogonBannerCheckboxEnabled(getIDMClient().getLogonBannerCheckboxFlag(tenantName))
                .withLogonBannerDisabled(disableLogonBanner)
                .build();
    }

    private void updateTokenPolicy(String tenantName, TokenPolicyDTO tokenPolicy) throws Exception {
        if (tokenPolicy.getClockToleranceMillis() != null) {
            getIDMClient().setClockTolerance(tenantName, tokenPolicy.getClockToleranceMillis());
        }

        if (tokenPolicy.getDelegationCount() != null) {
            getIDMClient().setDelegationCount(tenantName, tokenPolicy.getDelegationCount());
        }

        if (tokenPolicy.getMaxBearerTokenLifeTimeMillis() != null) {
            getIDMClient().setMaximumBearerTokenLifetime(tenantName, tokenPolicy.getMaxBearerTokenLifeTimeMillis());
        }

        if (tokenPolicy.getMaxHOKTokenLifeTimeMillis() != null) {
            getIDMClient().setMaximumHoKTokenLifetime(tenantName, tokenPolicy.getMaxHOKTokenLifeTimeMillis());
        }

        if (tokenPolicy.getMaxBearerRefreshTokenLifeTimeMillis() != null) {
            getIDMClient().setMaximumBearerRefreshTokenLifetime(tenantName, tokenPolicy.getMaxBearerRefreshTokenLifeTimeMillis());
        }

        if (tokenPolicy.getMaxHOKRefreshTokenLifeTimeMillis() != null) {
            getIDMClient().setMaximumHoKRefreshTokenLifetime(tenantName, tokenPolicy.getMaxHOKRefreshTokenLifeTimeMillis());
        }

        if (tokenPolicy.getRenewCount() != null) {
            getIDMClient().setRenewCount(tenantName, tokenPolicy.getRenewCount());
        }
    }

    /**
     * Search person users only with matching criteria
     */
    private Set<UserDTO> searchPersonUsers(String tenantName, SearchCriteria criteria, int limit) throws Exception {
        Set<PersonUser> idmPersonUsers = getIDMClient().findPersonUsersByName(tenantName, criteria, limit);
        return UserMapper.getUserDTOs(idmPersonUsers, false);
    }

    /**
     * Search groups only with matching criteria
     */
    private Set<GroupDTO> searchGroups(String tenantName, SearchCriteria criteria, int limit) throws Exception {
        Set<Group> groups = getIDMClient().findGroupsByName(tenantName, criteria, limit);
        return GroupMapper.getGroupDTOs(groups);
    }

    /**
     * Search solution users only with matching criteria
     */
    private Set<SolutionUserDTO> searchSolutionUsers(String tenantName, SearchCriteria criteria, int limit, SearchType searchBy) throws Exception {
        Set<SolutionUserDTO> solutionUsers = new HashSet<SolutionUserDTO>();
        if (searchBy == SearchType.NAME) {
            Set<SolutionUser> idmSolutionUsers = getIDMClient().findSolutionUsers(tenantName, criteria.getSearchString(), limit);
            solutionUsers = SolutionUserMapper.getSolutionUserDTOs(idmSolutionUsers);
        } else if (searchBy == SearchType.CERT_SUBJECTDN) {
            SolutionUser idmSolutionUser = getIDMClient().findSolutionUserByCertDn(tenantName, criteria.getSearchString());
            if (idmSolutionUser != null) {
                solutionUsers.add(SolutionUserMapper.getSolutionUserDTO(idmSolutionUser));
            }
        }
        return solutionUsers;
    }

    /**
     * Search limit calculator that computes number of principal entities(users, groups and solution users) to be returned in search results
     */
    private Map<MemberType, Integer> computeSearchLimits(int limit, MemberType memberType) {
        Map<MemberType, Integer> memberTypeToLimit = new HashMap<MemberType, Integer>();
        int limitPerPrincipalType = limit;
        if (memberType == MemberType.ALL) {
            // Users + Groups
            limitPerPrincipalType = limit < 0 ? -1 : limit / (MemberType.values().length - 1);
            // Solution users
            int solutionUserLimit = limitPerPrincipalType < 0 ? -1 : limitPerPrincipalType + (limit % (MemberType.values().length - 1));
            memberTypeToLimit.put(MemberType.USER, limitPerPrincipalType);
            memberTypeToLimit.put(MemberType.GROUP, limitPerPrincipalType);
            memberTypeToLimit.put(MemberType.SOLUTIONUSER, solutionUserLimit);
        } else {
            memberTypeToLimit.put(memberType, limit);
        }
        return memberTypeToLimit;
    }

}
