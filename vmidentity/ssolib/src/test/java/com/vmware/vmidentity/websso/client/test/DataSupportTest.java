/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.MessageStoreImpl;
import com.vmware.identity.websso.client.MessageType;
import com.vmware.identity.websso.client.WebssoClientException;

/**
 * @author root
 *
 */
public class DataSupportTest {

    /**
     * @throws java.lang.Exception
     */

    private final MessageStoreImpl store = new MessageStoreImpl(50);

    @Before
    public void setUp() throws Exception {
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     * Test method for
     * {@link com.vmware.identity.websso.client.MessageStoreImpl#add(com.vmware.identity.websso.client.Message)}
     * .
     */
    @Test
    public final void testAddMessageWithNoID() {

        Message message = new Message(MessageType.AUTHN_RESPONSE, null, "relay", null, null, null, null, null, null,
                null, null, false);

        int beforeSize = store.size();
        store.add(message);
        Assert.assertEquals(beforeSize, store.size());
    }

    /**
     * Test method for
     * {@link com.vmware.identity.websso.client.MessageStoreImpl#add(com.vmware.identity.websso.client.Message)}
     * .
     */
    @Test
    public final void testAddGetMessage() {

        String messageID = "100";
        Message message = new Message(MessageType.AUTHN_RESPONSE, messageID, null, null, null, null, null, null, null,
                null, null, false);

        int beforeSize = store.size();
        store.add(message);
        Assert.assertEquals(beforeSize + 1, store.size());
        Assert.assertNotNull(store.get(messageID));
    }

    /**
     * Test method for
     * {@link com.vmware.identity.websso.client.MessageStoreImpl#add(com.vmware.identity.websso.client.Message)}
     * .
     */
    @Test
    public final void testStoreSizeControl() {

        int messageID = 100;
        int maxsize = store.getMaxSize();
        int extra = 10;
        for (int i = 0; i < maxsize; i++) {
            Message message = new Message(MessageType.AUTHN_RESPONSE, Integer.toString(messageID + i), null, null,
                    null, null, null, null, null, null, null, false);
            store.add(message);
        }

        for (int i = 0; i < extra; i++) {
            Message message = new Message(MessageType.AUTHN_RESPONSE, Integer.toString(messageID + i), null, null,
                    null, null, null, null, null, null, null, false);
            store.add(message);
            Assert.assertEquals(maxsize, store.size());
        }

    }

    /**
     * Test method for
     * {@link com.vmware.identity.websso.client.MessageStoreImpl#add(com.vmware.identity.websso.client.Message)}
     * .
     *
     * @throws WebssoClientException
     */
    @Test
    public final void testRemoveByID() throws WebssoClientException {

        int messageID = 100;
        int maxsize = 20;
        int removesize = 10;
        for (int i = 0; i < maxsize; i++) {
            Message message = new Message(MessageType.AUTHN_RESPONSE, Integer.toString(messageID + i), null, null,
                    null, null, null, null, null, null, null, false);
            store.add(message);
        }

        for (int i = removesize; i < maxsize; i++) {

            store.remove(Integer.toString(messageID + i));
        }
        Assert.assertEquals(maxsize - removesize, store.size());
    }

}
