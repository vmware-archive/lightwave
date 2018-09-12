package com.vmware.identity.heartbeat;

/**
 * Represents heartbeat info for a particular service
 */
public class HeartbeatInfo {
    /**
     * Name of registered service
     */
    public final String serviceName;
    /**
     * Port number registered with service
     */
    public final int port;
    /**
     * Time of last heartbeat
     */
    public final int lastHeartbeat;
    /**
     * Status of service
     */
    public final boolean isAlive;

    public HeartbeatInfo(String serviceName, int port, int lastHearbeat, boolean isAlive) {
        this.serviceName = serviceName;
        this.port = port;
        this.lastHeartbeat = lastHearbeat;
        this.isAlive = isAlive;
    }
}