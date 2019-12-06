package install

import (
	"bufio"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"

	"github.com/vmware/lightwave/sts/cmd/stssetup/static"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

func (c *setupCmd) ldapSchema(ctxt diag.RequestContext) error {

	logger := ctxt.Logger()

	logger.Tracef(diag.SETUP, "Opening ldap schema stream '%s'", schemaFile)
	schema, err := static.SetupAssets.Open(schemaFile)
	if err != nil {
		logger.Errorf(diag.SETUP, "Opening ldap schema stream '%s' failed: %v", schemaFile, err)
		return fmt.Errorf("Failed registering ldap schema: %v", err)
	}
	defer schema.Close()

	scanner := bufio.NewScanner(schema)
	if scanner.Scan() {
		line := scanner.Text()
		if !strings.HasPrefix(line, "# schema_version=") {
			logger.Errorf(diag.SETUP, "Unable to identify schema version from preamble '%s'; expected format='# schema_version=<version>'", line)
			return fmt.Errorf("Unable to identify schema version from schema file")
		}
		c.schemaVersion = strings.TrimSpace(strings.TrimPrefix(line, "# schema_version="))
	}
	if scanner.Err() != nil {
		logger.Errorf(diag.SETUP, "Unable to identify schema version as failed to scan schema stream: %v", scanner.Err())
		return fmt.Errorf("Failed registering ldap schema: %v", scanner.Err())
	}
	if len(c.schemaVersion) <= 0 {
		logger.Errorf(diag.SETUP, "Unable to identify schema version from schema file; expected format='# schema_version=<version>'")
		return fmt.Errorf("Failed registering ldap schema: Unable to identify schema version from schema file")
	}
	logger.Infof(diag.SETUP, "Ldap schema version is '%s'", c.schemaVersion)
	_, err = schema.Seek(0, io.SeekStart)
	if err != nil {
		logger.Errorf(diag.SETUP, "failed to re-set to beginning of schema file stream: %v", err)
		return fmt.Errorf("Failed registering ldap schema: %v", err)
	}

	if !c.firstInstance {
		logger.Infof(diag.SETUP, "setting up secondary instance, no need to register ldap schema")
		return nil
	}

	// save schema file to temp file
	file, err := ioutil.TempFile("", "sts_ldapschema_*.ldif")
	if err != nil {
		logger.Errorf(diag.SETUP, "failed to get temporary file to store ldap schema: %v", err)
		return fmt.Errorf("Failed registering ldap schema: %v", err)
	}
	logger.Tracef(diag.SETUP, "created temporary file for schema '%s'", file.Name())

	_, err = io.Copy(file, schema)
	if err != nil {
		logger.Errorf(diag.SETUP, "failed to copy schema content to temporary file '%s': %v", file.Name(), err)
		e := file.Close()
		if e != nil {
			logger.Warningf(diag.SETUP, "failed to close schema temporary file '%s': %v", file.Name(), e)
		}
		e = os.Remove(file.Name())
		if e != nil {
			logger.Warningf(diag.SETUP, "failed to remove schema temporary file '%s': %v", file.Name(), e)
		}
		return fmt.Errorf("Failed registering ldap schema: %v", err)
	}
	logger.Tracef(diag.SETUP, "copied schema file content to temp file '%s'", file.Name())

	err = file.Close()
	if err != nil {
		logger.Errorf(diag.SETUP, "failed to close schema temporary file '%s': %v", file.Name(), err)
		return fmt.Errorf("Failed registering ldap schema: %v", err)
	}

	logger.Tracef(diag.SETUP, "running vdcschema command")
	cmd := exec.Command(
		c.VdcSchemaCli(), "patch-schema-defs",
		"--file", file.Name(),
		"--domain", c.systemTenant,
		"--host", c.vmdirHost,
		"--login", c.adminAccount,
		"--passwd", c.adminPwd)
	out, err := cmd.CombinedOutput()

	e := os.Remove(file.Name())
	if e != nil {
		logger.Errorf(diag.SETUP, "failed to remove schema temporary file '%s': %v", file.Name(), e)
	}

	if err != nil {
		logger.Errorf(
			diag.SETUP,
			"failed running '%s patch-schema-defs --file %s --domain %s --host %s --login %s --passwd ***': %v",
			c.VdcSchemaCli(),
			file.Name(), c.systemTenant, c.vmdirHost, c.adminAccount, err)
		logger.Errorf(
			diag.SETUP,
			"command output: %s", string(out))
		return fmt.Errorf("Failed registering ldap schema: %v", err)
	}

	logger.Infof(diag.SETUP, "successfully registered ldap schema")
	return nil
}
