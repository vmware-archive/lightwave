package afdclient

import (
	"testing"

	"flag"
	"fmt"
	"github.com/stretchr/testify/assert"
	"log"
	"os"
	"os/exec"
	"strings"
	"time"
)

const (
	host    = "localhost"
	service = "Lightwave - Test Service"
	account = "administrator"
	port    = 2000
	timeout = 31
)

var domain = flag.String("domain", "", "System tenant domain")
var password = flag.String("password", "", "System tenant password")
var otherDC = flag.String("dcname", "", "Other DC to affinitize to on force-refresh case")

func init() {
	flag.Parse()
	if *domain == "" || *password == "" {
		fmt.Printf("Error: domain and password parameters not passed in\n")
		flag.Usage()
		os.Exit(1)
	}
}

func TestVmAfdGetHeartbeatStatus(t *testing.T) {
	server, err := VmAfdOpenServer(host, account, *password)
	if err != nil {
		return
	}
	defer server.Close()

	status, err := server.VmAfdGetHeartbeatStatus()
	if err != nil {
		assert.FailNow(t, "Error in getting Heartbeat Status", "Username: %s, Password: %s, Error: %+v", account, *password, err)
	}
	defer status.FreeHeartbeatStatus()

	checkHeartbeatStatusAlive(t, status)
}

func TestVmAfdStartHeartbeat(t *testing.T) {
	server, err := VmAfdOpenServer(host, account, *password)
	if err != nil {
		return
	}
	defer server.Close()

	log.Printf("Starting Heartbeat\n")
	handle, err := VmAfdStartHeartbeat(service, port)
	if err != nil {
		assert.FailNow(t, "Error in start Heartbeat", "Service: %s, Port: %d, Error: %+v", service, port, err)
	}
	assert.NotNilf(t, handle, "VmAfdHeartBeat handle should not be nil")

	status1, err := server.VmAfdGetHeartbeatStatus()
	if err != nil {
		assert.FailNow(t, "Error in getting Heartbeat Status", "Username: %s, Password: %s, Error: %+v", account, *password, err)
	}
	defer status1.FreeHeartbeatStatus()

	info := getHbInfo(t, status1, service)
	if assert.NotNil(t, info, "Error: Could not get heartbeat info for: %s", service) {
		assert.True(t, info.IsAlive(), "Error: service %s should be alive", service)
	}
	log.Printf("Verified Heartbeat of service, Stopping heartbeat\n")

	handle.StopHeartbeat()

	log.Printf("Sleeping to timeout heartbeat...\n")
	time.Sleep(time.Second * timeout)

	status2, err := server.VmAfdGetHeartbeatStatus()
	if err != nil {
		assert.FailNow(t, "Error in getting Heartbeat Status", "Username: %s, Password: %s, Error: %+v", account, *password, err)
	}
	defer status2.FreeHeartbeatStatus()

	info = getHbInfo(t, status2, service)
	if assert.NotNil(t, info, "Error: Could not get heartbeat info for: %s", service) {
		assert.False(t, info.IsAlive(), "Error: service %s should not be alive", service)
	}
	log.Printf("Verified service heartbeat stopped\n")
}

func TestVmAfdGetDCName(t *testing.T) {
	out, err := exec.Command("/opt/vmware/bin/vmafd-cli", "get-dc-name", "--server-name", host).Output()
	if err != nil {
		assert.FailNow(t, "Failed to get DC name through afd cli")
	}

	dcName, err := VmAfdGetDCName(host)
	if err != nil {
		assert.FailNow(t, "Error in getting DC Name", "Error: %+v", err)
	}
	assert.Equal(t, strings.Trim(string(out), "\n"), dcName, "Error: DC Name not as expected")
}

func TestVmAfdForceRefreshDCName(t *testing.T) {
	if *otherDC == "" {
		log.Printf("Skipping ForceRefresh Test, dcname arg not passed in")
		return
	}

	dcName, err := VmAfdGetDCName(host)
	if err != nil {
		assert.FailNow(t, "Error in getting DC Name", "Error: %+v", err)
	}

	refreshedName, err := VmAfdForceRefreshDCName()
	if err != nil {
		assert.FailNow(t, "Error in refreshing DC Name", "Error: %+v", err)
	}

	assert.NotEqual(t, dcName, refreshedName, "Error: DC name should change after force refresh")
	assert.Equal(t, *otherDC, refreshedName, "Error: refreshed DC name is not %s", *otherDC)
	log.Printf("DcName: %s, After Refresh: %s", dcName, refreshedName)
}

func TestCdcGetDCStatusInfo(t *testing.T) {
	server, err := VmAfdOpenServer("", "", "") // Open connection to localhost
	if err != nil {
		assert.FailNowf(t, "Could not connect to localhost", "Error: %+v", err)
	}
	defer server.Close()

	entries, err := server.CdcEnumDCEntries()
	if err != nil {
		assert.FailNowf(t, "Failed to enumerate DC Entries", "Error: %+v", err)
	}

	assert.NotZero(t, len(entries), "Entries is empty")
	for _, entry := range entries {
		assert.NotEmpty(t, entry, "Entry is empty")
		info, hb, err := server.CdcGetDCStatusInfo(entry)
		if err != nil {
			assert.FailNowf(t, "Failed to get DC Status info", "DC: %s, Error: %+v", entry, err)
		}

		checkDCInfo(t, info, entry)
		checkHeartbeatStatusAlive(t, hb)

		info.FreeStatusInfo()
		hb.FreeHeartbeatStatus()
	}

}

func getHbInfo(t *testing.T, status *VmAfdHbStatus, service string) *VmAfdHbInfo {
	if status == nil {
		assert.FailNow(t, "Heartbeat Status is null")
	}

	for i := 0; i < status.GetCount(); i++ {
		info := status.GetHeartbeatInfo(i)
		if assert.NotNil(t, info, "Hearbeat Info index %i is null", i) && info.GetServiceName() == service {
			return info
		}
	}

	return nil
}

func checkHeartbeatStatusAlive(t *testing.T, status *VmAfdHbStatus) {
	if status == nil {
		assert.FailNow(t, "Heartbeat Status is null")
	}

	assert.True(t, status.IsAlive(), "Error: Heartbeat status isAlive is 0")
	assert.NotZero(t, status.GetCount(), "Error: Number of services registered in status is 0")
	for i := 0; i < status.GetCount(); i++ {
		info := status.GetHeartbeatInfo(i)
		if info == nil {
			assert.FailNowf(t, "Heartbeat info is null", "Index in heartbeat status: %d", i)
		}

		assert.NotEmpty(t, info.GetServiceName(), "Service name index %d should not be empty", i)
		assert.True(t, info.IsAlive(), "Service %s should be alive", info.GetServiceName())
	}

}

func checkDCInfo(t *testing.T, info *CdcDcStatusInfo, name string) {
	if info == nil {
		assert.FailNowf(t, "DC Status info is null", "DC: %s", name)
	}
	assert.NotZero(t, info.GetLastPing(), "DC %s - LastPing should not be 0", name)
	assert.True(t, info.IsAlive(), "DC %s should be alive", name)
	fmt.Printf("[DEBUG DCInfo] DC: %s\n\tLastPing: %d\n\tSite: %s\n\tisAlive: %+v\n", name, info.GetLastPing(), info.GetSiteName(), info.IsAlive())
}
