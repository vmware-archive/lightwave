package com.vmware.identity.idm;

public class CrlDownloadException extends IDMException
{

    /**
     *
     */

    private static final long serialVersionUID = 3325902801890343056L;


    public CrlDownloadException(String message, Throwable t) {
        super(message, t);
    }
    public CrlDownloadException(String message) {
        super(message);
    }

}