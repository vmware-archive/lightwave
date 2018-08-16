package com.vmware.identity.heartbeat;

import java.util.List;

/**
 * Represents vmafd heartbeat status of a node
 */
public class HeartbeatStatus {
    /**
     * Indicates whether the Status is reporting alive or not
     */
    public final boolean isAlive;
    /**
     * List of Heartbeat Info of DC.
     * Contains status info for each registered service.
     */
    public final List<HeartbeatInfo> heartbeatInfo;

    public HeartbeatStatus(List<HeartbeatInfo> heartbeatInfo, boolean alive) {
        this.heartbeatInfo = heartbeatInfo;
        this.isAlive = alive;
    }
}
