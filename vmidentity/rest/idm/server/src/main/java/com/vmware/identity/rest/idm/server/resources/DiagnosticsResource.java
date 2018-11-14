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

import java.util.List;

import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;
import javax.ws.rs.core.UriInfo;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.performanceSupport.IIdmAuthStat;
import com.vmware.identity.performanceSupport.IIdmAuthStatus;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.EventLogDTO;
import com.vmware.identity.rest.idm.data.EventLogStatusDTO;
import com.vmware.identity.rest.idm.server.mapper.EventLogMapper;
import com.vmware.identity.rest.idm.server.util.Config;

import io.prometheus.client.Histogram;

/**
 * Web service resource to manage all diagnostic operations for
 * a tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class DiagnosticsResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(DiagnosticsResource.class);

    private static final String METRICS_COMPONENT = "idm";
    private static final String METRICS_RESOURCE = "DiagnosticsResource";

    public DiagnosticsResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    @GET @Path("/eventlog")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public List<EventLogDTO> getEventLog(@Context UriInfo info) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "getEventLog").startTimer();
        String responseStatus = HTTP_OK;
        try {
            List<IIdmAuthStat> stats = getIDMClient().getIdmAuthStats(tenant);
            return EventLogMapper.getEventLogs(stats);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve the event log from tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to retrieve the event log from tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "getEventLog").inc();
            requestTimer.observeDuration();
        }
    }

    @GET @Path("/eventlog/status")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public EventLogStatusDTO getEventLogStatus() {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "getEventLogStatus").startTimer();
        String responseStatus = HTTP_OK;
        try {
            IIdmAuthStatus status = getIDMClient().getIdmAuthStatus(tenant);
            return EventLogMapper.getEventLogStatus(status);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve the event log status from tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to retrieve the event log status from tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "getEventLogStatus").inc();
            requestTimer.observeDuration();
        }
    }

    @POST @Path("/eventlog/start")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void startEventLog(@QueryParam("size") Integer size) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "startEventLog").startTimer();
        String responseStatus = HTTP_OK;
        try {
            if (size != null) {
                getIDMClient().setIdmAuthStatsSize(tenant, size);
            }

            getIDMClient().enableIdmAuthStats(tenant);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to start the event log for tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to start the event log for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "startEventLog").inc();
            requestTimer.observeDuration();
        }
    }

    @POST @Path("/eventlog/stop")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void stopEventLog() {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "stopEventLog").startTimer();
        String responseStatus = HTTP_OK;
        try {
            getIDMClient().disableIdmAuthStats(tenant);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to stop the event log for tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to stop the event log for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "stopEventLog").inc();
            requestTimer.observeDuration();
        }
    }

    @DELETE @Path("/eventlog")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void clearEventLog() {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "clearEventLog").startTimer();
        String responseStatus = HTTP_OK;
        try {
            getIDMClient().clearIdmAuthStats(tenant);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to clear the event log for tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to clear the event log for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "clearEventLog").inc();
            requestTimer.observeDuration();
        }
    }

}
