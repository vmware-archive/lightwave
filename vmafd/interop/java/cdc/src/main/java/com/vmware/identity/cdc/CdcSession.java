/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * ********************************************************************
 */
package com.vmware.identity.cdc;

import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.cdc.CdcGenericException;
import com.vmware.identity.cdc.CdcAdapter;

/**
 * This class provides means to work with a particular CdcSession.
 * The CdcSession is created by CdcFactory. There are 2 types of CdcSession objects
 * one can create. IPC type (created by calling CdcFactory.createCdcFactoryViaIPC()) and
 * Remote type (created by calling CdcFactory.createCdcSessionViaDomainAuth()).
 * The IPC type allows you to only communicate with the local Cdc, where the authentication happens by validating the peer process credentials.
 * The Remote type allows you to talk to remote Cdc instance as well your local one,
 * however the authentication mechanism done by validating Domain user credentials in Directory Service.
 * NOTE: This class is *not* Thread-safe.
 */
public class CdcSession implements AutoCloseable {
    private PointerRef _serverHandle;
    private final String _userName;
    private final String _serverName;

    private static final String LOCALHOST = "__localhost__";
    private static final String LOCALUSER = "__localuser__";

    CdcSession(String serverName, String userName, String password) {
        PointerRef pServer = new PointerRef();
        int error = 0;
        _serverName = (serverName == null) ? LOCALHOST : serverName;
        _userName = (userName == null) ? LOCALUSER : userName;

        error = CdcAdapter.VmAfdOpenServerW(serverName, userName, password, pServer);
        BAIL_ON_ERROR(
                error,
                "Error opening server '%s' for user '%s'",
                serverName,
                userName);

        _serverHandle = pServer;

    }

    private void closeSession() {
        if (_serverHandle != null) {
            CdcAdapter.VmAfdCloseServer(_serverHandle);
        }
        _serverHandle = null;
    }

    public void enableClientAffinity() {

        int error = CdcAdapter.CdcEnableClientAffinity(_serverHandle);
        BAIL_ON_ERROR(
                error,
                "Enabling client affinity failed. [Server: %s, User: %s]",
                _serverName,
                _userName
        );
    }

    public void disableClientAffinity() {

        int error = CdcAdapter.CdcDisableClientAffinity(_serverHandle);
        BAIL_ON_ERROR(
                error,
                "Enabling client affinity failed. [Server: %s, User: %s]",
                _serverName,
                _userName
        );
    }

    public CdcDCEntry getAffinitizedDC(String domainName, int flags) {

        CdcDCEntry resultEntry = null;
        CdcDCEntryNative pEntry = new CdcDCEntryNative();
        int error = CdcAdapter.CdcGetDCNameW(
                            _serverHandle,
                            domainName,
                            flags,
                            pEntry);

        if (error == CdcAdapter.ERROR_OBJECT_NOT_FOUND) {
            resultEntry = null;
        } else {
            BAIL_ON_ERROR(
                    error,
                    "Getting affinitized DC for domain %s failed. [Server: %s, User: %s]",
                    domainName,
                    _serverName,
                    _userName);

            resultEntry = convertCdcDCEntryNativeToCdcDCEntry(pEntry);
        }
        return resultEntry;
    }

    public List <String> enumDCEntries() {
        List <String> dcEntryList = new ArrayList<String>();
        int error = CdcAdapter.CdcEnumDCEntriesW(
                _serverHandle,
                dcEntryList
        );
        BAIL_ON_ERROR(
                error,
                "Enumerating DCs failed. [Server: %s, User: %s]",
                _serverName, _userName
        );

        return dcEntryList;
    }

    public List <DCStatusInfo> enumDCStatusInfo() {
        List <DCStatusInfo> dcStatusList = new ArrayList<DCStatusInfo>();

        List <String> dcEntries = enumDCEntries();
        for (String entry : dcEntries) {
            dcStatusList.add(getDCStatusInfo(entry, null));
        }

        return dcStatusList;
    }

    public DCStatusInfo getDCStatusInfo(String dcName, String domainName) {
        DCStatusInfo resultEntry = null;
        CdcStatusInfoNative statusInfo = new CdcStatusInfoNative();
        HeartbeatStatusNative hbStatus = new HeartbeatStatusNative();

        int error = CdcAdapter.CdcGetDCStatusInfoW(
                _serverHandle,
                dcName,
                domainName,
                statusInfo,
                hbStatus);

        BAIL_ON_ERROR(
                error,
                "Getting status info for %s failed. [Server: %s, User: %s]",
                dcName,
                _serverName,
                _userName);

        return convertCdcStatusNativeToDCStatusInfo(dcName, statusInfo, hbStatus);
    }

    public CdcState getCdcState() {
        IntRef pState = new IntRef();
        CdcState cdcState;
        int error = CdcAdapter.CdcGetCurrentState(_serverHandle, pState);

        BAIL_ON_ERROR(
                error,
                "Getting CDC State failed. [Server: %s, User: %s]",
                _serverName,
                _userName);
        cdcState = CdcState.getState(pState.number);

        return cdcState;
    }

    @Override
    public void close() {
        closeSession();
    }

    private static CdcDCEntry convertCdcDCEntryNativeToCdcDCEntry(CdcDCEntryNative entryNative) {
        String dcName = entryNative.dcName;
        CdcAddressType type = CdcAddressType.getEntryType(entryNative.addressType);

        String dcAddress = entryNative.dcAddress;
        String dcSiteName = entryNative.dcSiteName;

        return new CdcDCEntry(type, dcName, dcAddress, dcSiteName);
    }

    private static DCStatusInfo convertCdcStatusNativeToDCStatusInfo(
            String dcName, CdcStatusInfoNative statusInfo, HeartbeatStatusNative hbStatus) {
        if (statusInfo == null || hbStatus == null) {
            BAIL_ON_ERROR(
                    CdcAdapter.ERROR_INVALID_PARAMETER,
                    "Failed to get DCStatusInfo from vmafd");
        }

        List<HeartbeatInfo> heartbeatInfo = new ArrayList<HeartbeatInfo>();
        for (HeartbeatInfoNative info : hbStatus.hbInfoArr) {
            HeartbeatInfo hbInfo = new HeartbeatInfo(
                        info.serviceName,
                        info.port,
                        info.lastHeartbeat,
                        (info.isAlive != 0));

            heartbeatInfo.add(hbInfo);
        }

        return new DCStatusInfo(
                dcName,
                statusInfo.lastPing,
                statusInfo.lastResponseTime,
                statusInfo.lastError,
                statusInfo.siteName,
                (statusInfo.isAlive != 0),
                heartbeatInfo);
    }

    private static void BAIL_ON_ERROR(final int error, final String format, Object...vargs) {
        switch (error) {
            case 0:
                break;
            default:
                throw new CdcGenericException(String.format(format, vargs), error);
        }
    }

    protected void finalize() throws Throwable {
        try {
            closeSession();
        } finally {
            super.finalize();
        }
    }
}