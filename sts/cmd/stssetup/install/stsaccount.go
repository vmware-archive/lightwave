package install

import (
	"bufio"
	"bytes"
	"fmt"
	"os/exec"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

func (c *setupCmd) ensureStsAccount(ctxt diag.RequestContext) error {

	var err error
	logger := ctxt.Logger()

	logger.Infof(diag.SETUP, "Starting to configure sts account...")
	// validate stsgroup exists
	conn, err := ldap.DefaultConnectionFactory.Connection(
		c.ldapUrls[0], unameDn(c.adminUname), c.adminPwd, ldap.BindTypeSimple, c.trustedCertsFunc, ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Opening connection: (%s, %s) faled with: %v", c.ldapUrls[0], unameDn(c.adminUname), err)
		return fmt.Errorf("Failed configuring sts account failing to connect to ldap server %v", err)
	}
	defer conn.Close()

	// 1. sts accounts group exists
	domainDn := types.DnFromDomain(c.systemTenant)
	stsAccountGroupDn := fmt.Sprintf("cn=%s,cn=Builtin,%s", idmconfig.StsAccountsGroup, domainDn)
	present, err := c.ldapObjectExists(conn, stsAccountGroupDn, "(objectClass=group)", ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Checking ldap object existence: (%s, %s) faled with: %v", stsAccountGroupDn, "(objectClass=group)", err)
		return fmt.Errorf("Failed configuring sts account - unable to verify '%s' group existence: %v", idmconfig.StsAccountsGroup, err)
	}
	if !present {
		logger.Errorf(diag.SETUP, "Ldap object (%s, %s) does not exist", stsAccountGroupDn, "(objectClass=group)")
		return fmt.Errorf("Failed configuring sts account - group '%s' does not exist", idmconfig.StsAccountsGroup)
	}
	// 2. sts container exists
	stsContainerDn := fmt.Sprintf("cn=%s,%s", idmconfig.StsContainer, domainDn)
	present, err = c.ldapObjectExists(conn, stsContainerDn, "(objectClass=container)", ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Checking ldap object existence: (%s, %s) faled with: %v", stsContainerDn, "(objectClass=group)", err)
		return fmt.Errorf("Failed configuring sts account - unable to verify '%s' container existence: %v", idmconfig.StsContainer, err)
	}
	if !present {
		logger.Errorf(diag.SETUP, "Ldap object (%s, %s) does not exist", stsContainerDn, "(objectClass=group)")
		return fmt.Errorf("Failed configuring sts account - group '%s' does not exist", idmconfig.StsContainer)
	}

	// create users & groups containers under sts container
	err = idmconfig.EnsureContainer(conn, stsContainerDn, idmconfig.StsUsersContainer, ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "ensure container '%s' under '%s' faled with: %v", idmconfig.StsUsersContainer, stsContainerDn, err)
		return fmt.Errorf("Failed configuring sts account - failed creating '%s' container: %v", idmconfig.StsUsersContainer, err)
	}
	err = idmconfig.EnsureContainer(conn, stsContainerDn, idmconfig.StsGroupsContainer, ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "ensure container '%s' under '%s' faled with: %v", idmconfig.StsGroupsContainer, stsContainerDn, err)
		return fmt.Errorf("Failed configuring sts account - failed creating '%s' container: %v", idmconfig.StsGroupsContainer, err)
	}

	if len(c.stsAccount) <= 0 {
		logger.Infof(diag.SETUP, "Sts account is not provided, creating a new account.")

		machineAccount, err := c.getMachineAccountName(ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Unable to get machine account info for current node: %v", err)
			return fmt.Errorf("Failed configuring sts account - failed identifying machine account name: %v", err)
		}
		c.stsAccountPwd, err = c.getAccountPwd(ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Unable to generate account pwd: %v", err)
			return fmt.Errorf("Failed configuring sts account - failed generating pwd: %v", err)
		}

		c.stsAccount = fmt.Sprintf(
			"cn=sts/%s,cn=%s,cn=%s,%s", machineAccount, idmconfig.StsUsersContainer,
			idmconfig.StsContainer, domainDn)
		// create sts account
		err = c.createStsAccount(conn, "sts/"+machineAccount, domainDn, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Unable to create sts account '%s': %v", "sts/"+machineAccount, err)
			return fmt.Errorf("Failed configuring sts account - failed creating sts account: %v", err)
		}
		logger.Infof(diag.SETUP, "Successfully created Sts account '%s'", "sts/"+machineAccount)
	}

	// ensure sts account is part of stsaccounts group
	err = c.addUserToGroup(conn, c.stsAccount, stsAccountGroupDn, (len(c.stsAccount) > 0), ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Unable to add sts account to sts account group: %v", err)
		return fmt.Errorf("Failed configuring sts account - failed adding sts account to sts accounts group: %v", err)
	}

	logger.Infof(diag.SETUP, "Successfully configured sts account...")

	return nil
}

func (c *setupCmd) getMachineAccountName(ctxt diag.RequestContext) (string, error) {

	logger := ctxt.Logger()

	cmd := exec.Command(
		c.VmAfdCli(), "get-machine-account-info")
	out, err := cmd.Output()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed running:'%s get-machine-account-info' with:%v", c.VmAfdCli(), err)
		logger.Errorf(diag.SETUP, "Command output:%s", string(out))
		return "", err
	}
	scanner := bufio.NewScanner(bytes.NewReader(out))
	for scanner.Scan() {
		outputStr := scanner.Text()
		if strings.HasPrefix(outputStr, "MachineAccount: ") {
			return strings.TrimPrefix(outputStr, "MachineAccount: "), nil
		}
	}
	if err := scanner.Err(); err != nil {
		logger.Errorf(diag.SETUP, "Failed scanning cmd output for machine account info: %v", err)
		return "", err
	}

	logger.Errorf(diag.SETUP, "Was not able to get machine account info from '%s get-machine-account-info'", c.VmAfdCli())
	return "", fmt.Errorf("Unable to identify machine account name")
}

func (c *setupCmd) getAccountPwd(ctxt diag.RequestContext) (string, error) {
	logger := ctxt.Logger()
	cmd := exec.Command(
		c.VmDirCli(), "password", "create",
		"--login", c.adminAccount, "--password", c.adminPwd)
	out, err := cmd.Output()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed running:'%s password create ...' with:%v", c.VmDirCli(), err)
		logger.Errorf(diag.SETUP, "Command output %s'", string(out))
		return "", err
	}
	scanner := bufio.NewScanner(bytes.NewReader(out))
	for scanner.Scan() {
		return scanner.Text(), nil
	}
	if err := scanner.Err(); err != nil {
		logger.Errorf(diag.SETUP, "Failed scanning cmd output forgen pwd cmd: %v", err)
		return "", err
	}
	return string(out), nil
}

func (c *setupCmd) createStsAccount(conn ldap.Connection, accountName string, domainDn string, ctxt diag.RequestContext) error {

	logger := ctxt.Logger()

	dn := fmt.Sprintf(
		"cn=%s,cn=%s,cn=%s,%s", accountName, idmconfig.StsUsersContainer,
		idmconfig.StsContainer, domainDn)

	attrs := make([]ldap.Attribute, 0, 8)
	var attr ldap.Attribute
	var ldapVal ldap.Value
	var err diag.Error

	// objectClass
	ldapVal, err = ldap.ValueForString("user")
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for 'user': %v", err)
		return err
	}
	attr = ldap.NewAttribute("objectClass", ldapVal)
	attrs = append(attrs, attr)
	ldapVal, err = ldap.ValueForString(accountName)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for '%s': %v", accountName, err)
		return err
	}
	attr = ldap.NewAttribute("cn", ldapVal)
	attrs = append(attrs, attr)
	attr = ldap.NewAttribute("samAccountName", ldapVal)
	attrs = append(attrs, attr)
	attr = ldap.NewAttribute("givenName", ldapVal)
	attrs = append(attrs, attr)
	ldapVal, err = ldap.ValueForString(c.systemTenant)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for '%s': %v", c.systemTenant, err)
		return err
	}
	attr = ldap.NewAttribute("sn", ldapVal)
	attrs = append(attrs, attr)
	ldapVal, err = ldap.ValueForString(accountName + "@" + c.systemTenant)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for '%s': %v", accountName+"@"+c.systemTenant, err)
		return err
	}
	attr = ldap.NewAttribute("userPrincipalname", ldapVal)
	attrs = append(attrs, attr)
	ldapVal, err = ldap.ValueForBool(true)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for 'true': %v", err)
		return err
	}
	attr = ldap.NewAttribute("vmwPasswordNeverExpires", ldapVal) // todo: remove once pwd refresh is implemented
	attrs = append(attrs, attr)
	ldapVal, err = ldap.ValueForString(c.stsAccountPwd)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for '<pwd>': %v", err)
		return err
	}
	attr = ldap.NewAttribute("userPassword", ldapVal)
	attrs = append(attrs, attr)

	err = conn.Add(dn, attrs, ctxt)
	if err != nil && ldap.IsAlredyExistsError(err) {
		err = conn.Delete(dn, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed adding '%s' as already exists and unable to delete existing with: %v", dn, err)
			return err
		}
		err = conn.Add(dn, attrs, ctxt)
	}
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed adding '%s' with: %v", dn, err)
		return err
	}
	return nil
}

func (c *setupCmd) addUserToGroup(conn ldap.Connection, userDn string, groupDn string, ignoreExists bool, ctxt diag.RequestContext) error {

	logger := ctxt.Logger()

	attrMods := make([]ldap.AttributeMod, 0, 1)
	var ldapVal ldap.Value
	var err diag.Error

	ldapVal, err = ldap.ValueForString(userDn)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed getting ldap value for '%s': %v", userDn, err)
		return err
	}
	mod := ldap.NewAttributeMod(ldap.ModTypeAdd, "member", ldapVal)
	attrMods = append(attrMods, mod)

	err = conn.Modify(groupDn, attrMods, ctxt)
	if err != nil && ldap.IsAttributeOrValueExistsError(err) && ignoreExists {
		err = nil
	}
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed adding user '%s' to group '%s' with: %v", userDn, groupDn, err)
		return err
	}
	return nil
}
