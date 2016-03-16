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
package com.vmware.identity.rest.idm.client;

import static com.vmware.identity.rest.core.client.RequestExecutor.execute;
import static com.vmware.identity.rest.core.client.RequestExecutor.executeAndReturnList;
import static com.vmware.identity.rest.core.client.URIFactory.buildParameters;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;
import java.util.List;
import java.util.Map;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.idm.data.EventLogDTO;
import com.vmware.identity.rest.idm.data.EventLogStatusDTO;

public class DiagnosticsResource extends ClientResource {

    private static final String DIAGNOSTICS_URI = "/idm/tenant/%s/diagnostics";
    private static final String EVENT_LOG_URI = DIAGNOSTICS_URI + "/eventlog";
    private static final String START_EVENT_LOG_URI = EVENT_LOG_URI + "/start";
    private static final String STOP_EVENT_LOG_URI = EVENT_LOG_URI + "/stop";

    private static final String DIAGNOSTICS_POST_URI = "/idm/post/tenant/%s/diagnostics";
    private static final String EVENT_LOG_POST_URI = DIAGNOSTICS_POST_URI + "/eventlog";
    private static final String EVENT_LOG_STATUS_POST_URI = EVENT_LOG_POST_URI + "/status";

    public DiagnosticsResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request that a tenant's event log be cleared out.
     *
     * @param tenant name of the tenant to request the event log from.
     * @return a list of log entries.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void clearEventLog(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EVENT_LOG_URI, tenant);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

    /**
     * Request the event log associated with a tenant.
     *
     * @param tenant name of the tenant to request the event log from.
     * @return a list of log entries.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<EventLogDTO> getEventLog(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EVENT_LOG_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, EventLogDTO.class);
    }

    /**
     * Request the status of the event log associated with a tenant.
     *
     * @param tenant name of the tenant to request the event log status from.
     * @return a status object.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public EventLogStatusDTO getEventLogStatus(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EVENT_LOG_STATUS_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, EventLogStatusDTO.class);
    }

    /**
     * Request that the tenant begin logging events. The size of the
     * event log will be the value it was last configured with (or the
     * default size if it has never been configured).
     *
     * @param tenant name of the tenant to start the event log on.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void startEventLog(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        startEventLog(tenant, null);
    }

    /**
     * Request that the tenant begin logging events.
     *
     * @param tenant name of the tenant start the event log on.
     * @param size size of the event log.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void startEventLog(String tenant, int size) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        startEventLog(tenant, Integer.valueOf(size));
    }

    /**
     * Request that the tenant stops logging events.
     *
     * @param tenant name of the tenant to stop the event log on.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void stopEventLog(String tenant)  throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), STOP_EVENT_LOG_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        execute(parent.getClient(), post);
    }

    private void startEventLog(String tenant, Integer size) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = buildParameters("size", size);
        URI uri = buildURI(parent.getHostRetriever(), START_EVENT_LOG_URI, tenant, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        execute(parent.getClient(), post);
    }

}
