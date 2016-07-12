/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 * ************************************************************************/
package com.vmware.identity.websso.client;

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.joda.time.DateTime;

/**
 * Message is a structure which holds single WebSSO message data. It has a
 * number of properties with getters and setters. Many values are get-only
 * though.
 *
 */
public class Message implements Serializable {

    private static final long serialVersionUID = 1L;

    private MessageType type; // not settable

    private String id; // not settable

    private String relayState; // not settable

    private DateTime issueInstant;

    private String source; // not settable

    private String target; // not settable

    private String status; // not settable

    private String substatus; // not settable

    private String sessionIndex; // not settable

    private MessageData messageData; // not settable

    private Object tag; // client-defined additional information

    private List<Message> associatedMessages; // not settable here, use
                                              // MessageStore

    private ValidationResult validationResult; // setable. default is null.
                                               // client library side validation
                                               // result.
    private boolean isIdpInitiated;    //idp-inited SSO or SLO response. Applicable to response message types.

    // required by subclass of serializable
    protected Message() {
    }

    public Message(MessageType type, String id, String relayState, DateTime issueInstant, String source, String target,
            String status, String substatus, String sessionIndex, MessageData messageData, Object tag, boolean isIdpInitiated) {

        this.type = type;
        this.id = id;
        this.relayState = relayState;
        this.issueInstant = issueInstant;
        this.messageData = messageData;
        this.sessionIndex = sessionIndex;
        this.source = source;
        this.target = target;
        this.status = status;
        this.substatus = substatus;
        this.tag = tag;
        this.associatedMessages = new LinkedList<Message>();
        this.validationResult = null;
        this.isIdpInitiated = isIdpInitiated;
    }

    public MessageType getType() {
        return type;
    }

    public String getId() {
        return id;
    }

    public String getRelayState() {
        return relayState;
    }

    public DateTime getIssueInstant() {
        return issueInstant;
    }

    public String getSource() {
        return source;
    }

    public String getTarget() {
        return target;
    }

    /**
     * available only in a response message
     *
     */
    public String getStatus() {
        return status;
    }

    /**
     * available only in a response message
     *
     */
    public String getSubstatus() {
        return substatus;
    }

    public String getSessionIndex() {
        return sessionIndex;
    }

    public MessageData getMessageData() {
        return messageData;
    }

    public Object getTag() {
        return tag;
    }

    /**
     * Adding a message as associated to this message
     */
    public void addToAssociated(Message asMessage) {
        associatedMessages.add(asMessage);
    }

    /**
     * Return a collection of messages that are associated to this message
     */
    public Collection<Message> getAssociatedMessages() {
        return Collections.unmodifiableCollection(associatedMessages);
    }

    /**
     * @return the client library validation result
     *
     *         Client code should always check validationResult for
     *         authentication error discovered.
     */
    public ValidationResult getValidationResult() {
        return validationResult;
    }

    /**
     * @param validationResult
     *            the client library validation result to set
     */
    public void setValidationResult(ValidationResult validationResult) {
        this.validationResult = validationResult;
    }

    public boolean isIdpInitiated() {

        return isIdpInitiated;
    }


}
