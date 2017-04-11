/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.apache.commons.lang.Validate;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * MessageStore implementation class. Provides a basic implementation of the
 * interface.
 * 
 * Here is an example of specifying it through Spring XML (note that bean id
 * should be messageStore for controller autowiring to work).
 * 
 * <bean id="messageStore"
 * class="com.vmware.identity.websso.client.MessageStoreImpl"/>
 */
/*
 * todo: think about better data structure than linked list, for better
 * performance on the searching type supported.
 */
public class MessageStoreImpl implements MessageStore {

    private final LinkedList<Message> messages;

    private final Logger logger = LoggerFactory.getLogger(MessageStoreImpl.class);

    private final ReentrantReadWriteLock readWriteLock;
    private final Lock readLock;
    private final Lock writeLock;
    private final static int DEFAULT_MAX_CONCURRENT_REQUEST = 5000;
    private final int maxSize;

    /**
     * Return a MessageStore object
     */
    public MessageStoreImpl() {
        this(DEFAULT_MAX_CONCURRENT_REQUEST);
    }

    /**
     * Return a MessageStore object maxConcurrentRequests: max number of
     * concurrent request
     */
    public MessageStoreImpl(int maxConcurrentReqeusts) {
        if (maxConcurrentReqeusts > 0) {
            this.maxSize = maxConcurrentReqeusts;
        } else {
            this.maxSize = DEFAULT_MAX_CONCURRENT_REQUEST;
        }
        this.messages = new LinkedList<Message>();
        this.readWriteLock = new ReentrantReadWriteLock();
        this.readLock = this.readWriteLock.readLock();
        this.writeLock = this.readWriteLock.writeLock();
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * com.vmware.identity.websso.client.MessageStore#remove(java.lang.String)
     */
    @Override
    public void remove(String messageId) throws WebssoClientException {

        Validate.notNull(messageId);
        try {
            this.writeLock.lock();
            Iterator<Message> iterator = this.messages.iterator();
            while (iterator.hasNext()) {
                Message message = iterator.next();
                if (message.getId().equals(messageId)) {
                    this.messages.remove(message);
                    break;
                }
            }
        } finally {
            this.writeLock.unlock();
        }
    }

    /**
     * Associate the two given messages.
     */
    public void associate(String messageId1, String messageId2) {
        if (messageId1 == null || messageId2 == null) {
            return;
        }

        try {
            this.writeLock.lock();
            Message a = get(messageId1);
            Message b = get(messageId2);
            if (a == null || b == null) {
                return;
            }

            a.addToAssociated(b);
            b.addToAssociated(a);
        } finally {
            this.writeLock.unlock();
        }
    }

    public Collection<Message> getAll() {
        return this.messages;
    }

    public Collection<Message> getAllByType(MessageType type) {
        if (type == null) {
            return null;
        }
        List<Message> resultMessages = new LinkedList<Message>();
        try {
            this.readLock.lock();
            Iterator<Message> iterator = this.messages.iterator();
            while (iterator.hasNext()) {
                Message message = iterator.next();
                if (message.getType() == type) {
                    resultMessages.add(message);
                }
            }
        } finally {
            this.readLock.unlock();
        }
        return resultMessages;
    }

    public Collection<Message> getAllByRelayState(String relayState) {
        if (relayState == null) {
            return null;
        }

        List<Message> resultMessages = new LinkedList<Message>();
        try {
            this.readLock.lock();
            Iterator<Message> iterator = this.messages.iterator();
            while (iterator.hasNext()) {
                Message message = iterator.next();
                String rState = message.getRelayState();
                if (rState != null && rState.equals(relayState)) {
                    resultMessages.add(message);
                }
            }
        } finally {
            this.readLock.unlock();
        }
        return resultMessages;
    }

    @Override
    public Message get(String messageId) {
        if (messageId == null) {
            return null;
        }

        Message message = null;
        try {
            this.readLock.lock();
            Iterator<Message> iterator = this.messages.iterator();
            while (iterator.hasNext()) {
                message = iterator.next();
                if (message.getId().equals(messageId)) {
                    break;
                }
            }
        } finally {
            this.readLock.unlock();
        }
        if (null == message) {
            logger.error("No request with id=:%s found.  Message store size:%s", messageId, this.size());
        }
        return message;
    }

    /**
     * Return all Message with given type and relayState.
     */
    public Collection<Message> get(MessageType type, String relayState) {
        if (relayState == null || type == null) {
            return null;
        }

        List<Message> resultMessages = new LinkedList<Message>();
        try {
            this.readLock.lock();
            Iterator<Message> iterator = this.messages.iterator();
            while (iterator.hasNext()) {
                Message message = iterator.next();
                String rState = message.getRelayState();
                if (rState != null && rState.equals(relayState) && message.getType().equals(type)) {
                    resultMessages.add(message);
                }
            }
        } finally {
            this.readLock.unlock();
        }
        return resultMessages;
    }

    /**
     * Remove all messages in the store.
     */
    public void clear() {
        try {
            this.writeLock.lock();
            this.messages.clear();
        } finally {
            this.writeLock.unlock();
        }
    }

    @Override
    public void add(Message message) {
        if (message != null && message.getId() != null && !message.getId().isEmpty()) {
            try {
                this.writeLock.lock();
                if (this.size() == getMaxSize()) {
                    this.messages.removeFirst();
                    assert this.size() < getMaxSize();
                }
                this.messages.add(message);
            } finally {
                this.writeLock.unlock();
            }
            this.logger.debug(String.format("New MessageStore entry added:%s , store size: %s ", message.getType(),
                    this.messages.size()));
        }
    }

    /**
     * return size of the store in term of number of element
     * 
     * @return store size
     */
    public int size() {
        return messages.size();
    }

    public int getMaxSize() {
        return maxSize;
    }
}
