/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. 
 * ************************************************************************/

package com.vmware.identity.websso.client;

/**
 * The library will throw WebssoClientException in case of error. The exception
 * includes a message and an optional inner exception.
 * 
 */
public class WebssoClientException extends Exception {
    /**
	 * 
	 */
    private static final long serialVersionUID = 1L;

    /**
	 * 
	 */
    public WebssoClientException() {

    }

    /**
     * @param message
     */
    public WebssoClientException(String message) {
        super(message);
    }

    /**
     * @param cause
     */
    public WebssoClientException(Throwable cause) {
        super(cause);
    }

    /**
     * @param message
     * @param cause
     */
    public WebssoClientException(String message, Throwable cause) {
        super(message, cause);
    }
}
