package com.vmware.identity.heartbeat;

import java.util.List;

class HeartbeatStatusNative {
    int isAlive;
    int dwCount;
    List<HeartbeatInfoNative> hbInfoArr;
}