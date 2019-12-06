package install

import (
	"fmt"
	"os/exec"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

func (c *setupCmd) registerStsWithDns(ctxt diag.RequestContext) error {

	var err error

	logger := ctxt.Logger()

	if !c.firstInstance {
		logger.Tracef(diag.SETUP, "No need to register secondary instances with dns...")
		return nil
	}

	logger.Infof(diag.SETUP, "Registering STS service with dns")

	// todo: this is dns-cli work-around,
	// it always adds zone to the target automatically
	target := strings.TrimSuffix(c.publicHost, "."+c.systemTenant)

	logger.Tracef(diag.SETUP, "running vmdns-cli add record command with target='%s', port='%s'", target, c.publicPort)
	cmd := exec.Command(
		c.VmDNSCli(), "add-record",
		"--zone", c.systemTenant+".",
		"--type", "SRV",
		"--server", c.vmdirHost,
		"--username", c.adminAccount,
		"--domain", c.systemTenant,
		"--password", c.adminPwd,
		"--service", "sts",
		"--protocol", "tcp",
		"--target", target,
		"--priority", "1",
		"--weight", "1",
		"--port", c.publicPort)
	out, err := cmd.CombinedOutput()

	if err != nil {
		logger.Errorf(
			diag.SETUP,
			"failed running '%s add-record --zone %s --type %s --server %s --username %s --domain %s --password '***' --service %s --protocol 5s --target %s --priority %s --weight %s --port %s': %v",
			c.VmDNSCli(),
			c.systemTenant+".", "SRV", c.vmdirHost, c.adminAccount, c.systemTenant,
			"sts", "tcp", target, "1", "1", c.publicPort, err)
		logger.Errorf(
			diag.SETUP,
			"command output: %s", string(out))
		return fmt.Errorf("Failed registering STS service in dns : %v", err)
	}

	logger.Infof(diag.SETUP, "successfully registered STS service in dns")

	return nil
}
