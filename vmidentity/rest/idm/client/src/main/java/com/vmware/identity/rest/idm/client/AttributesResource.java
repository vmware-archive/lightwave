package com.vmware.identity.rest.idm.client;

import static com.vmware.identity.rest.core.client.RequestExecutor.executeAndReturnList;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;
import java.util.Collection;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.idm.data.AttributeDTO;
import com.vmware.identity.rest.idm.data.AttributeDefinitionsDTO;
import com.vmware.identity.rest.idm.data.UpdateAttributesDTO;

public class AttributesResource extends ClientResource {

    private static final String ATTRIBUTES_URI = "/idm/tenant/%s/attributes";
    private static final String ATTRIBUTES_POST_URI = "/idm/post/tenant/%s/attributes";

    public AttributesResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request the list of attribute definitions associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to request the attribute definitions from.
     * @return a list of attribute definitions.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public Collection<AttributeDTO> getAll(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), ATTRIBUTES_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, AttributeDTO.class);
    }

    /**
     * Set the list of attribute definitions associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to set the attribute definitions.
     * @param attributes a list of attribute definitions to be set
     * @return a list of attribute definitions.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public Collection<AttributeDTO> set(String tenant, AttributeDefinitionsDTO attributes) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), ATTRIBUTES_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), attributes);
        return executeAndReturnList(parent.getClient(), post, AttributeDTO.class);
    }

    /**
     * Update the list of attribute definitions associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to update the attribute definitions.
     * @param attributes attribute definitions to be updated
     * @return a list of attribute definitions.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public Collection<AttributeDTO> update(String tenant, UpdateAttributesDTO attributesUpdate) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), ATTRIBUTES_URI, tenant);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), attributesUpdate);
        return executeAndReturnList(parent.getClient(), put, AttributeDTO.class);
    }
}
