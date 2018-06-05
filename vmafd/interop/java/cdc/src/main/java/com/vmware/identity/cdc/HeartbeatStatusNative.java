package com.vmware.identity.cdc;

import java.util.List;

class HeartbeatStatusNative {
    int isAlive;
    int dwCount;
    List<HeartbeatInfoNative> hbInfoArr;
}