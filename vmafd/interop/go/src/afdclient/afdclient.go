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
	C.VmAfdCloseServer(server.p)
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

// VmAfdHbHandle holds a C pointer to a VMAFD_HB_HANDLE struct.
type VmAfdHbHandle struct {
	p C.PVMAFD_HB_HANDLE
}

// StopHeartbeat stops the heartbeat for the service and cleans up memory.
// The caller must call StopHeartbeat when the service is exiting.
func (handle *VmAfdHbHandle) StopHeartbeat() {
	C.VmAfdStopHeartbeat(handle.p)
}

// VmAfdHbStatus holds a C pointer to a VMAFD_HB_STATUS_A struct. It has a pointer to an array of VmAfdHbInfo structs.
type VmAfdHbStatus struct {
	p C.PVMAFD_HB_STATUS_A
}

// FreeHeartbeatStatus frees the memory for VmAfdHbStatus struct.
// The caller must call FreeHeartbeatStatus() when VmAfdHbStatus is no longer needed.
func (status *VmAfdHbStatus) FreeHeartbeatStatus() {
	C.VmAfdFreeHeartbeatStatusA(status.p)
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
	count = int((*status.p).dwCount)
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
func (status *VmAfdHbInfo) GetServiceName() string {
	return cStringToGoString((*status.p).pszServiceName)
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
