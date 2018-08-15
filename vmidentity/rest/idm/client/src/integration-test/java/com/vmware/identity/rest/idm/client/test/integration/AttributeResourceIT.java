package com.vmware.identity.rest.idm.client.test.integration;

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertContainsAttribute;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.xml.soap.SOAPException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.AccessToken.Type;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.AttributeDTO;
import com.vmware.identity.rest.idm.data.AttributeDefinitionsDTO;
import com.vmware.identity.rest.idm.data.UpdateAttributesDTO;

@RunWith(value = Parameterized.class)
public class AttributeResourceIT extends IntegrationTestBase {

    @Parameters
    public static Object[] data() {
           return new Object[] { AccessToken.Type.JWT, AccessToken.Type.SAML };
    }

    public AttributeResourceIT(Type tokenType) throws Exception {
        super(true, tokenType);
    }

    private static String uriFormat = "urn:oasis:names:tc:SAML:2.0:attrname-format:uri";

    private static Collection<AttributeDTO> customAttributes = new ArrayList<>();

    static {
        AttributeDTO attr = new AttributeDTO.Builder()
                                            .withName("https://aws.amazon.com/SAML/Attributes/Role")
                                            .withFriendlyName("AWSRole")
                                            .withNameFormat(uriFormat)
                                            .build();
        customAttributes.add(attr);

        attr = new AttributeDTO.Builder()
                               .withName("https://aws.amazon.com/SAML/Attributes/RoleSessionName")
                               .withFriendlyName("AWSRoleSessionName")
                               .withNameFormat(uriFormat)
                               .build();

        customAttributes.add(attr);
    }

    @BeforeClass
    public static void init() throws ClientException, ClientProtocolException, GeneralSecurityException, HttpException, IOException, SOAPException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientException, ClientProtocolException, HttpException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testGetAll() throws ClientProtocolException, HttpException, ClientException, IOException {
        Collection<AttributeDTO> attrs = testAdminClient.attributes().getAll(testTenant.getName());
        assertFalse(attrs.isEmpty());
    }

    @Test
    public void testSet() throws ClientProtocolException, HttpException, ClientException, IOException {
        Collection<AttributeDTO> originalAttrs = testAdminClient.attributes().getAll(testTenant.getName());
        List<AttributeDTO> newAttrs = new ArrayList<>();
        newAttrs.addAll(originalAttrs);
        newAttrs.addAll(customAttributes);
        AttributeDefinitionsDTO dto = new AttributeDefinitionsDTO.Builder()
                .withAttributes(newAttrs)
                .build();
        Collection<AttributeDTO> attrs = testAdminClient.attributes().set(testTenant.getName(), dto);
        assertTrue(attrs.size() == newAttrs.size());
        for (AttributeDTO attr : attrs) {
            assertContainsAttribute(attr, newAttrs);
        }

        newAttrs = new ArrayList<>(originalAttrs);
        dto = new AttributeDefinitionsDTO.Builder()
                .withAttributes(newAttrs)
                .build();
        attrs = testAdminClient.attributes().set(testTenant.getName(), dto);
        assertTrue(attrs.size() == originalAttrs.size());
        for (AttributeDTO attr : attrs) {
            assertContainsAttribute(attr, originalAttrs);
        }
    }

    @Test
    public void testUpdate() throws ClientProtocolException, HttpException, ClientException, IOException {
        Collection<AttributeDTO> originalAttrs = testAdminClient.attributes().getAll(testTenant.getName());
        // add collection
        UpdateAttributesDTO updateAttrsDto = new UpdateAttributesDTO.Builder()
                                                                    .withAdd(customAttributes)
                                                                    .build();
        Collection<AttributeDTO> attrs = testAdminClient.attributes().update(testTenant.getName(), updateAttrsDto);
        assertTrue(attrs.size() == customAttributes.size() + originalAttrs.size());
        for (AttributeDTO attr : customAttributes) {
            assertContainsAttribute(attr, attrs);
        }

        List<String> remove = new ArrayList<>();
        for (AttributeDTO attr : customAttributes) {
            remove.add(attr.getName());
        }

        // remove collection
        updateAttrsDto = new UpdateAttributesDTO.Builder()
                                                .withRemove(remove)
                                                .build();
        attrs = testAdminClient.attributes().update(testTenant.getName(), updateAttrsDto);
        assertTrue(attrs.size() == originalAttrs.size());

        // add one attribute
        AttributeDTO attr = new AttributeDTO.Builder()
                                            .withName("https://aws.amazon.com/SAML/Attributes/Role")
                                            .withFriendlyName("AWSRole")
                                            .withNameFormat(uriFormat)
                                            .build();
        updateAttrsDto = new UpdateAttributesDTO.Builder()
                                                .withAddAttribute(attr)
                                                .build();
        attrs = testAdminClient.attributes().update(testTenant.getName(), updateAttrsDto);
        assertTrue(attrs.size() == originalAttrs.size() + 1);
        assertContainsAttribute(attr, attrs);

        // remove one attr
        updateAttrsDto = new UpdateAttributesDTO.Builder()
                                                .withRemoveAttribute(attr.getName())
                                                .build();
        attrs = testAdminClient.attributes().update(testTenant.getName(), updateAttrsDto);
        assertTrue(attrs.size() == originalAttrs.size());
    }
}
