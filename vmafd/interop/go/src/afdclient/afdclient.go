// Package afdclient provides some go wrapper functions to call C based
// Lightwave vmafd client functions for Go based applications
package afdclient

/*
#cgo CFLAGS: -I/opt/vmware/include -I/opt/likewise/include
#cgo LDFLAGS: -L/opt/vmware/lib64 -lvmafdclient -Wl,-rpath=/opt/vmware/lib64
#include <stdlib.h>
#include <vmafdtypes.h>
#include <vmafdclient.h>
#include <cdcclient.h>
*/
import "C"

import (
	"fmt"
	"runtime"
	"unsafe"
)

// VmAfdServer holds a C pointer to VMAFD_SERVER struct.
type VmAfdServer struct {
	p C.PVMAFD_SERVER
}

// Close closes the connection to VmAfdServer.
// The caller must call Close() when the server handle is no longer needed.
func (server *VmAfdServer) Close() {
	if server.p != nil {
		C.VmAfdCloseServer(server.p)
		server.p = nil
	}
}

// VmAfdGetHeartbeatStatus gets the heartbeat status of vmafd service running on the specified server.
// The caller must call FreeHeartBeatStatus when the status is no longer needed.
func (server *VmAfdServer) VmAfdGetHeartbeatStatus() (status *VmAfdHbStatus, err error) {
	var s C.PVMAFD_HB_STATUS_A = nil
	var e C.DWORD = C.VmAfdGetHeartbeatStatusA(server.p, &s)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to get heartbeat status (%s)", cErrorToGoError(e))
		return
	}

	status = &VmAfdHbStatus{s}
	runtime.SetFinalizer(status, vmAfdHbStatusFinalize)
	return
}

// CdcEnumDCEntries gets the DC entries in client side cache. CDC client side affinity must be enabled.
func (server *VmAfdServer) CdcEnumDCEntries() (dcEntries []string, err error) {
	var s *C.PSTR = nil
	var c C.DWORD = 0
	var e C.DWORD = C.CdcEnumDCEntriesA(server.p, &s, (*C.LW_UINT32)(unsafe.Pointer(&c)))
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to enumerate DC Entries (%s)", cErrorToGoError(e))
	}
	defer C.CdcFreeStringArrayA(s, c)

	count := int(c)
	dcEntries = make([]string, count)
	for i := 0; i < count; i++ {
		dcEntries[i] = cStringToGoString((*[1 << 30]C.PSTR)(unsafe.Pointer(s))[i])
	}

	return
}

// CdcGetDCStatusInfo gets the DC status and heartbeat of a specific DC entry. CDC client side affinity must be enabed.
// The caller must call FreeStatusInfo when CdcDcStatusInfo is no longer needed.
// The caller must call FreeHeartbeatStatus when VmAfdHbStatus is no longer needed.
func (server *VmAfdServer) CdcGetDCStatusInfo(dcName string) (info* CdcDcStatusInfo, hb* VmAfdHbStatus, err error) {
	var s C.PSTR = goStringToCString(dcName)
	defer freeCString(s)
	var i C.PCDC_DC_STATUS_INFO_A = nil
	var h C.PVMAFD_HB_STATUS_A = nil
	var e C.DWORD = C.CdcGetDCStatusInfoA(server.p, s, nil, &i, &h)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to getDCStatusInfo of %s (%s)", dcName, cErrorToGoError(e))
	}

	info = &CdcDcStatusInfo{i}
	hb = &VmAfdHbStatus{h}

	runtime.SetFinalizer(info, cdcDCStatusInfoFinalize)
	runtime.SetFinalizer(hb, vmAfdHbStatusFinalize)

	return
}

// VmAfdHbHandle holds a C pointer to a VMAFD_HB_HANDLE struct.
type VmAfdHbHandle struct {
	p C.PVMAFD_HB_HANDLE
}

// StopHeartbeat stops the heartbeat for the service and cleans up memory.
// The caller must call StopHeartbeat when the service is exiting.
func (handle *VmAfdHbHandle) StopHeartbeat() {
	if handle.p != nil {
		C.VmAfdStopHeartbeat(handle.p)
		handle.p = nil
	}
}

// VmAfdHbStatus holds a C pointer to a VMAFD_HB_STATUS_A struct. It has a pointer to an array of VmAfdHbInfo structs.
type VmAfdHbStatus struct {
	p C.PVMAFD_HB_STATUS_A
}

// FreeHeartbeatStatus frees the memory for VmAfdHbStatus struct.
// The caller must call FreeHeartbeatStatus() when VmAfdHbStatus is no longer needed.
func (status *VmAfdHbStatus) FreeHeartbeatStatus() {
	if status.p != nil {
		C.VmAfdFreeHeartbeatStatusA(status.p)
		status.p = nil
	}
}

// IsAlive returns whether the current machine is alive or not.
func (status *VmAfdHbStatus) IsAlive() (isAlive bool) {
	if (*status.p).bIsAlive == 0 {
		isAlive = false
	} else {
		isAlive = true
	}

	return isAlive
}

// GetCount returns the number of services registered by vmafd.
func (status *VmAfdHbStatus) GetCount() (count int) {
	count = int((*(status.p)).dwCount)
	return count
}

// GetHeartbeatInfo gets the VmAfdHbInfo struct from the VmAfdHbStatus array based on passed in index.
// If the index is invalid, nil is returned.
// VmAfdHbInfo does not need to be freed by the caller since it is an element of the VmAfdHbStatus array. When VmAfdHbStatus
// is freed, VmAfdHbInfo is cleaned up.
func (status *VmAfdHbStatus) GetHeartbeatInfo(index int) (info *VmAfdHbInfo) {
	if index < 0 || index > status.GetCount() {
		return info
	}

	hbInfo := (*[1 << 30]C.VMAFD_HB_INFO_A)(unsafe.Pointer((*status.p).pHeartbeatInfoArr))[index]
	info = &VmAfdHbInfo{C.PVMAFD_HB_INFO_A(unsafe.Pointer(&hbInfo))}
	return info
}

// VmAfdHbInfo holds a C pointer to a VMAFD_HB_INFO_A struct. It represents the info for a particular service.
type VmAfdHbInfo struct {
	p C.PVMAFD_HB_INFO_A
}

// IsAlive returns whether or not the service specified by VmAfdHbInfo struct is alive.
func (status *VmAfdHbInfo) IsAlive() (isAlive bool) {
	if (*status.p).bIsAlive == 0 {
		isAlive = false
	} else {
		isAlive = true
	}

	return isAlive
}

// GetServiceName gets the name of the service specified by VmAfdHbInfo struct.
func (status *VmAfdHbInfo) GetServiceName() (serviceName string) {
	serviceName = cStringToGoString((*(status.p)).pszServiceName)
	return serviceName
}

// CdcDcStatusInfo holds a C pointer to a PCDC_DC_STATUS_INFO_A struct. It represents the status for a particular DC.
type CdcDcStatusInfo struct {
	p C.PCDC_DC_STATUS_INFO_A
}

// GetLastPing gets the time of the last ping.
func (status *CdcDcStatusInfo) GetLastPing() (lastPing int) {
	lastPing = int((*(status.p)).dwLastPing)
	return lastPing
}

// GetLastResponseTime gets the time of the last response.
func (status *CdcDcStatusInfo) GetLastResponseTime() (lastResponse int) {
	lastResponse = int((*(status.p)).dwLastResponseTime)
	return lastResponse
}

// GetLastError gets the code of the last error.
func (status *CdcDcStatusInfo) GetLastError() (lastError int) {
	lastError = int((*(status.p)).dwLastError)
	return lastError
}

// IsAlive returns whether the DC is alive or not.
func (status *CdcDcStatusInfo) IsAlive() (isAlive bool) {
	if (*status.p).bIsAlive == 0 {
		isAlive = false
	} else {
		isAlive = true
	}

	return isAlive
}

// GetSiteName returns the site name of the DC.
func (status *CdcDcStatusInfo) GetSiteName() (siteName string) {
	siteName = cStringToGoString((*(status.p)).pszSiteName)
	return siteName
}

// FreeStatusInfo frees the CdcDcStatusInfo struct.
// The caller must call this when CdcDcStatusInfo is no longer needed.
func (status *CdcDcStatusInfo) FreeStatusInfo() {
	if status.p != nil {
		C.CdcFreeDCStatusInfoA(status.p)
		status.p = nil
	}
}

// VmAfdOpenServer opens a connection to the server specified by the arguments.
// The connection should be closed when the handle is no longer needed.
func VmAfdOpenServer(server, account, password string) (serverHandle *VmAfdServer, err error) {
	serverCStr := goStringToCString(server)
	accountCStr := goStringToCString(account)
	passwordCStr := goStringToCString(password)

	defer freeCString(serverCStr)
	defer freeCString(accountCStr)
	defer freeCString(passwordCStr)

	var s C.PVMAFD_SERVER = nil
	var e C.DWORD = C.VmAfdOpenServerA(
		serverCStr,
		accountCStr,
		passwordCStr,
		&s)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to connect to server (%s)", cErrorToGoError(e))
		return
	}

	serverHandle = &VmAfdServer{s}
	runtime.SetFinalizer(serverHandle, vmAfdServerFinalize)
	return serverHandle, nil
}

// VmAfdStartHeartbeat starts the heartbeat for the specified service and returns a handle.
// The caller must call handle.StopHeartbeat when the service is stopping or the handle no longer needed
func VmAfdStartHeartbeat(serviceName string, servicePort uint32) (handle *VmAfdHbHandle, err error) {
	dwPort := (C.DWORD)(servicePort)
	serviceCStr := goStringToCString(serviceName)
	defer freeCString(serviceCStr)

	var h C.PVMAFD_HB_HANDLE = nil
	var e C.DWORD = C.VmAfdStartHeartbeatA(serviceCStr, dwPort, &h)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to start vmafd heartbeat (%s)", cErrorToGoError(e))
		return
	}

	handle = &VmAfdHbHandle{h}
	runtime.SetFinalizer(handle, vmAfdHeartbeatFinalize)
	return
}

// VmAfdGetDCName returns the DC name of the server specified by the argument.
func VmAfdGetDCName(serverName string) (dcName string, err error) {
	dcNameCStr := goStringToCString(serverName)
	defer freeCString(dcNameCStr)

	var s C.PSTR = nil
	var e C.DWORD = C.VmAfdGetDCName(dcNameCStr, &s)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to get DC name (%s)", cErrorToGoError(e))
		return
	}

	dcName = vmafdStringToGoString(s)
	return
}

// VmAfdForceRefreshDCName forces refresh and gets DC Name of the server specified by the argument.
func VmAfdForceRefreshDCName() (dcName string, err error) {
	serverHandle, err := VmAfdOpenServer("", "", "") // Open connection to localhost
	if err != nil {
		err = fmt.Errorf("[ERROR] Failed to open connection to server (%s)", err.Error())
		return
	}
	defer serverHandle.Close()

	var domain C.PCSTR = nil
	var site C.PCSTR = nil
	var guid C.GUID_A = nil
	var flags C.DWORD = 1 //Force refresh flag enabled
	var info C.PCDC_DC_INFO_A = nil
	var e C.DWORD = C.CdcGetDCNameA(serverHandle.p, domain, guid, site, flags, &info)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to force refresh DC name (%s)", cErrorToGoError(e))
		return
	}

	dcName = vmafdStringToGoString(info.pszDCName)
	return
}

//VmAfdCreateComputerOUContainer recursively creates the OU container under ou=Computers given username, password, and orgunit
func VmAfdCreateComputerOUContainer(userName, password, orgUnit string) (err error) {
	userNameCStr := goStringToCString(userName)
	defer freeCString(userNameCStr)
	passwordCStr := goStringToCString(password)
	defer freeCString(passwordCStr)
	orgUnitCStr := goStringToCString(orgUnit)
	defer freeCString(orgUnitCStr)

	var e C.DWORD = C.VmAfdCreateComputerOUContainerA(
		userNameCStr,
		passwordCStr,
		orgUnitCStr)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to create Computer OU Container (%s)", cErrorToGoError(e))
		return
	}

	return
}

//VmAfdCreateComputerAccountWithDC creates a machine account given the DC name, username, password, machine name, and (optional) orgunit
func VmAfdCreateComputerAccountWithDC(serverName, userName, password, machineName, orgUnit string) (machinePassword string, err error) {
	serverNameCStr := goStringToCString(serverName)
	defer freeCString(serverNameCStr)
	userNameCStr := goStringToCString(userName)
	defer freeCString(userNameCStr)
	passwordCStr := goStringToCString(password)
	defer freeCString(passwordCStr)
	machineNameCStr := goStringToCString(machineName)
	defer freeCString(machineNameCStr)
	orgUnitCStr := goStringToCString(orgUnit)
	defer freeCString(orgUnitCStr)

	var s C.PSTR = nil
	var e C.DWORD = C.VmAfdCreateComputerAccountDCA(
						serverNameCStr,
						userNameCStr,
						passwordCStr,
						machineNameCStr,
						orgUnitCStr,
						&s)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to create Computer Account with DC (%s)", cErrorToGoError(e))
		return
	}

	machinePassword = vmafdStringToGoString(s)
	return
}

// VmAfdGetMachineAccountInfoA gets the machine account credentials
func VmAfdGetMachineAccountInfo() (account string, password string, err error) {
	var acc C.PSTR = nil
	var pass C.PSTR = nil
	var e C.DWORD = C.VmAfdGetMachineAccountInfoA(nil, &acc, &pass)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to get machine account info (%s)", cErrorToGoError(e))
		return
	}

	account = vmafdStringToGoString(acc)
	password = vmafdStringToGoString(pass)
	return
}

// VmAfdLeaveVmDir is used to domain leave client specified in machineName using userName and password
func VmAfdLeaveVmDir(userName, password, machineName string, forceFlag bool) (err error) {
	serverNameCStr := goStringToCString("")
	defer freeCString(serverNameCStr)
	userNameCStr := goStringToCString(userName)
	defer freeCString(userNameCStr)
	passwordCStr := goStringToCString(password)
	defer freeCString(passwordCStr)
	machineNameCStr := goStringToCString(machineName)
	defer freeCString(machineNameCStr)

	var leaveFlag C.DWORD = 0
	if forceFlag {
		leaveFlag = 1
	}

	var e C.DWORD = C.VmAfdLeaveVmDirA(
		serverNameCStr,
		userNameCStr,
		passwordCStr,
		machineNameCStr,
		leaveFlag)
	if e != 0 {
		err = fmt.Errorf("[ERROR] Failed to leave vmdir (%s)", cErrorToGoError(e))
	}

	return
}

// vmAfdServerFinalize is a wrapper function to set the finalizer for VmAfdServer
func vmAfdServerFinalize(server *VmAfdServer) {
	server.Close()
}

// vmAfdHbStatusFinalize is a wrapper function to set the finalizer for VmAfdHbStatus
func vmAfdHbStatusFinalize(status *VmAfdHbStatus) {
	status.FreeHeartbeatStatus()
}

// vmAfdHeartbeatFinalize is a wrapper function to set the finalizer for VmAfdHbHandle
func vmAfdHeartbeatFinalize(handle *VmAfdHbHandle) {
	handle.StopHeartbeat()
}

// cdcDCStatusInfoFinalize is a wrapper function to set the finalizer for CdcDcStatusInfo
func cdcDCStatusInfoFinalize(handle *CdcDcStatusInfo) {
	handle.FreeStatusInfo()
}
