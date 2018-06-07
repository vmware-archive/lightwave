package com.vmware.identity.cdc;

import java.util.List;

/**
 * Represents cached status information of a Domain Controller
 */
public class DCStatusInfo {
    /**
     * Name of DC
     */
    public final String dcName;
    /**
     * Last ping time of DC
     */
    public final int lastPing;
    /**
     * Last response time of DC
     */
    public final int lastResponseTime;
    /**
     * Last time of error for DC
     */
    public final int lastError;
    /**
     * Vmafd Status for DC
     */
    public final boolean isAlive;
    /**
     * Sitename DC is located in
     */
    public final String siteName;
    /**
     * List of Heartbeat Info of DC.
     * Contains status info for each registered service.
     */
    public final List<HeartbeatInfo> heartbeatInfo;

    public DCStatusInfo(String dcName,
                        int lastPing,
                        int lastResponseTime,
                        int lastError,
                        String siteName,
                        boolean isAlive,
                        List<HeartbeatInfo> heartbeatInfo)
    {
        this.dcName = dcName;
        this.lastPing = lastPing;
        this.lastResponseTime = lastResponseTime;
        this.lastError = lastError;
        this.siteName = siteName;
        this.isAlive = isAlive;
        this.heartbeatInfo = heartbeatInfo;
    }
}