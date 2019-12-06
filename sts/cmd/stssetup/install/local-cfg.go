package install

import (
	"fmt"
	"os"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

func (c *setupCmd) localConfig(ctxt diag.RequestContext) error {

	var err error
	logger := ctxt.Logger()

	logger.Infof(diag.SETUP, "creating local sts config in '%s'", c.cfgFile)
	cfgb := config.NewInstanceConfigBuilder()

	if c.shutdownTimeoutSecs > 0 {
		cfgb.ShutdownWait(time.Second * time.Duration(c.shutdownTimeoutSecs))
	}

	if len(c.enabledHeads) > 0 {
		cfgb.EnabledHttpHeads(c.enabledHeads)
	}

	if c.port > 0 {
		cfgb.Port(c.port)
	}

	cfgb.TLSCert(c.httpscert)
	cfgb.TLSPrivateKey(c.httpskey)

	cfgb.Site(c.site)
	cfgb.SystemTenant(c.tenant)
	cfgb.Addresses(c.ldapUrls)
	cfgb.Certificates(c.vmdirCerts)
	cfgb.AuthType(types.AuthTypeSimple) // todo: change when srp is supported
	cfgb.UserName(unameDn(c.stsAccount))
	cfgb.Pwd([]string{c.stsAccountPwd})

	c.cfg, err = cfgb.Build()
	if err != nil {
		logger.Errorf(diag.SETUP, "creating local sts config failed: %v", err)
		return fmt.Errorf("Failed to create local sts config: %v", err)
	}

	err = config.SaveConfig(c.cfg, c.cfgFile, ctxt.Logger())
	if err != nil {
		logger.Errorf(diag.SETUP, "saving local sts config to '%s' failed: %v", c.cfgFile, err)
		return fmt.Errorf("Failed to save local sts config: %v", err)
	}
	logger.Infof(diag.SETUP, "created local sts config in '%s'", c.cfgFile)

	logger.Tracef(diag.SETUP, "setting chmod 600 on sts cfg file '%s'", c.cfgFile)
	err = os.Chmod(c.cfgFile, 0440) // u=r,g=r
	if err != nil {
		logger.Errorf(diag.SETUP, "chmod 600 on sts config '%s' failed: %v", c.cfgFile, err)
		return fmt.Errorf("Failed to create local sts config: %v", err)
	}

	logger.Infof(diag.SETUP, "successfully created local sts config in '%s'", c.cfgFile)

	return nil
}
