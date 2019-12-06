package install

import (
	"fmt"
	"html/template"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"strconv"

	"github.com/vmware/lightwave/sts/cmd/stssetup/static"
	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

func (c *setupCmd) configProcess(ctxt diag.RequestContext) error {

	var err error
	logger := ctxt.Logger()

	stsExecutable := c.StsExec()

	err = os.Chmod(stsExecutable, 0555) // u=rx,g=rx,o=rx
	if err != nil {
		logger.Errorf(diag.SETUP, "chmod 555 on sts executable '%s' failed: %v", stsExecutable, err)
		return fmt.Errorf("Failed to configure sts process: %v", err)
	}

	if c.daemonUName != defaultDaemonUname || c.daemonGroup != defaultDaemonGroup {
		logger.Tracef(
			diag.SETUP,
			"sts process is set to run as non root, chown sts binary '%s' to the specified user '%v', group '%v'",
			stsExecutable, c.pid, c.gid)
		err = os.Chown(stsExecutable, c.pid, c.gid)
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"failed to chown sts binary '%s' to user '%v', group '%v'",
				stsExecutable, c.pid, c.gid)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}
		err = os.Chown(c.cfgFile, c.pid, c.gid)
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"failed to chown sts config file '%s' to user '%v', group '%v'",
				c.cfgFile, c.pid, c.gid)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}

		if c.port < 1024 {
			logger.Tracef(
				diag.SETUP,
				"sts process is set to run as non root, and use port '%v' need to set cap_net_bind_service+ep capability on %s",
				c.port, stsExecutable)

			cmd := exec.Command(
				"setcap", "cap_net_bind_service+ep", stsExecutable)
			err = cmd.Run()
			if err != nil {
				logger.Errorf(
					diag.SETUP,
					"Failed to set bind service capability on %s: %v",
					stsExecutable, err)
				return fmt.Errorf("Failed to configure sts process: %v", err)
			}
		}
	}

	if c.daemon {

		logger.Infof(diag.SETUP, "configuring sts to run as a daemon")

		// open daemon template stream
		tmplStream, err := static.SetupAssets.Open(daemonTemplate)
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed to open daemnon template %s: %v",
				daemonTemplate, err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}
		defer tmplStream.Close()

		tmplStr, err := ioutil.ReadAll(tmplStream)
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed reading daemnon template %s: %v",
				daemonTemplate, err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}

		tmpl, err := template.New("daemon").Parse(string(tmplStr))
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed parsing daemnon template %s: %v",
				daemonTemplate, err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}

		daemonFile, err := os.Create(path.Join(daemonFileLocation, daemonFileName))
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed creating daemonFile %s: %v",
				path.Join(daemonFileLocation, daemonFileName), err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}

		daemonParams := &daemonParameters{
			StsCmd:         stsExecutable,
			ProcessUserID:  strconv.FormatInt(int64(c.pid), 10),
			ProcessGroupID: strconv.FormatInt(int64(c.gid), 10),
		}
		if c.cfgFile != config.CfgFile {
			daemonParams.StsCmd = daemonParams.StsCmd + fmt.Sprintf(" -config=%s", c.cfgFile)
		}
		err = tmpl.Execute(daemonFile, daemonParams)
		e := daemonFile.Close()
		if e != nil {
			logger.Warningf(
				diag.SETUP,
				"Failed closing daemonFile %s: %v",
				daemonFile.Name(), err)
		}
		if err != nil {
			e = os.Remove(daemonFile.Name())
			if e != nil {
				logger.Warningf(
					diag.SETUP,
					"Failed removing daemonFile %s: %v",
					daemonFile.Name(), err)
			}
			logger.Errorf(
				diag.SETUP,
				"Failed executing daemon template: %v", err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}

		cmd := exec.Command(
			"/bin/systemctl", "enable", daemonFileName)
		err = cmd.Run()
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed enabling service '%s': %v", daemonFileName, err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}
		cmd = exec.Command(
			"/bin/systemctl", "daemon-reload")
		err = cmd.Run()
		if err != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed reloading daemons: %v", err)
			return fmt.Errorf("Failed to configure sts process: %v", err)
		}
	}

	logger.Infof(diag.SETUP, "successfully configured sts process")

	return nil
}

type daemonParameters struct {
	StsCmd         string
	ProcessUserID  string
	ProcessGroupID string
}
