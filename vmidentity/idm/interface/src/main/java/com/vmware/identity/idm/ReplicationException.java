package com.vmware.identity.idm;

/**
 * This exception indicates an error regarding replication when performing operations on another partner DC
 */
public class ReplicationException extends IDMException {

    public ReplicationException(String message) {
        super(message);
    }

    public ReplicationException(Throwable ex) {
        super(ex);
    }

    public ReplicationException(String message, Throwable ex) {
        super(message, ex);
    }
}
