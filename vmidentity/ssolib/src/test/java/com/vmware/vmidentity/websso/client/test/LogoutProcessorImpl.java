/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import javax.servlet.http.HttpServletRequest;
import static org.junit.Assert.fail;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.websso.client.Error;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.ValidationResult;
import com.vmware.identity.websso.client.endpoint.SsoResponseListener;

/**
 * LogoutProcessor implementation for test
 * 
 */
public class LogoutProcessorImpl implements com.vmware.identity.websso.client.LogoutProcessor {

    private static final Logger logger = LoggerFactory.getLogger(SsoResponseListener.class);

    public LogoutProcessorImpl() {
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * com.vmware.identity.websso.client.LogoutProcessor#logoutSuccess(com.vmware
     * .identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void logoutSuccess(Message message, HttpServletRequest request, HttpServletResponse response) {
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * com.vmware.identity.websso.client.LogoutProcessor#logoutError(com.vmware
     * .identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void logoutError(Message message, HttpServletRequest request, HttpServletResponse response) {
        if (message == null || message.getValidationResult() == null) {
            fail();
        }
        ValidationResult validationResult = message.getValidationResult();
        try {
            response.sendError(validationResult.getResponseCode(), validationResult.getSubstatus());
        } catch (Exception e) {
            logger.error("Unexpected error in sending out error response. ");
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * com.vmware.identity.websso.client.LogoutProcessor#logoutRequest(com.vmware
     * .identity.websso.client.Message)
     */
    @Override
    public void logoutRequest(Message message, HttpServletRequest request, HttpServletResponse response, String tenant) {
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * com.vmware.identity.websso.client.LogonProcessor#internalError(com.vmware
     * .identity.websso.client.WebssoClientException,
     * javax.servlet.http.HttpServletRequest)
     */
    @Override
    public void internalError(Exception internalError, HttpServletRequest request, HttpServletResponse response) {
        logger.error(Error.INTERNAL_ERROR_TYPE, internalError);
    }

}
