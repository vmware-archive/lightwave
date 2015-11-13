#!/usr/bin/env python

import os
import sys
import re
import string
import subprocess
import ctypes
import shutil
import shlex
import time
import traceback
import fileinput
import json
from cis.defaults import *
from cis.utils import *
from cis.tools import *
from cis.exceptions import *
from cis.firstboot import *
from cis.l10n import msgMetadata as _T
from cis.l10n import localizedString
from cis.progressReporter import ProgressReporter
from pprint import pprint

if os.name != 'posix':
    import _winreg
    import win32api
    import win32con
    import win32service
    import win32security
    import win32net
    import ntsecuritycon as con

if not os.environ['VMWARE_PYTHON_PATH'] in sys.path:
    sys.path.append(os.environ['VMWARE_PYTHON_PATH'])

SSO_COMP_NAME = "sso"

INSTALL_PARAMETER_URL_HOSTNAME = 'system.urlhostname'
INSTALL_PARAMETER_HOSTNAME = 'system.hostname'
INSTALL_PARAMETER_HOSTNAME_TYPE = 'system.hostname.type'
INSTALL_PARAMETER_VMDIR_ADMIN_DN = 'vmdir.admin-dn'
INSTALL_PARAMETER_VMDIR_PWD = 'vmdir.password'
INSTALL_PARAMETER_RHTTPPROXY_CERT = 'rhttpproxy.cert'
INSTALL_PARAMETER_VMCA_CERT_DIR = 'vmca.cert.dir'
INSTALL_PARAMETER_VMDIR_DOMAIN_NAME = 'vmdir.domain-name'
INSTALL_PARAMETER_STS_PORT = 'sts.ext.port1'
INSTALL_PARAMETER_DEPLOYMENT_TYPE = 'deployment.node.type'
DOMAIN_EXPORT_PROPERTY_FILE = "ssodomainexport.properties"
STS_DEFAULT_PORT = '7444'
STS_STATUS_STOPPED = 9
STS_STATUS_NOT_INSTALLED = 0
STS_STATUS_SERVERFILE_NOT_EXIST = 1
STS_STATUS_RUNNING = 11

if is_linux():
	installbin = os.path.normpath('/opt/vmware/lib64/')
else:
	installbin = os.path.normpath(get_cis_install_dir() + '/VMware Identity Services/')
sys.path.append(installbin)


class VMwareIdentityFirstBoot(FirstBoot):

    def __init__(self):
        FirstBoot.__init__(self, SSO_COMP_NAME)
        self.init()

    def isWindowsOs(self):
        return is_windows()


    def defByOs(self, linux, windows):
        return def_by_os(linux, windows)

    def startService(self, svc_name, wait_time=3600):
        return service_start(svc_name, wait_time)

    def stopService(self, svc_name, wait_time=300):
        return service_stop(svc_name, wait_time)

    def canWaitForInstallParameter(self):
        return True

    def waitForInstallParameter(self, parameter, default=None):
        return wait_for_install_parameter(parameter)

    def getInstallParameter(self, parameter, default=None):
        return get_install_parameter(parameter, default)

    def getOpenSSLExePath(self):
        return get_openssl()

    def getCertoolPath(self):
        return get_certool()

    def Log(self, message):
        log(message)

    def isUpgradeMode(self):
        isUpgrading = False
        importDir = self.getSSOImportDir()
        if importDir != "":
            isUpgrading = os.path.isdir(importDir)

        log("VMware Identity Service bootstrap: isUgprading=%s" % (isUpgrading))

        return isUpgrading

    def getSSOImportDir(self):
        importDirectory = get_install_parameter("upgrade.import.directory", "")
        if importDirectory != "":
            importDirectory = os.path.join(importDirectory,SSO_COMP_NAME)

        log("VMware Identity Service bootstrap: importDirectory=%s" % (importDirectory))
        return importDirectory

    def getUpgradeSourceVersion(self):
        upgradeSourceVersion = ""
        if self.isUpgradeMode():
            importDir = self.getSSOImportDir()
            versionPath = os.path.join(importDir, 'version')
            with open(versionPath, 'r') as versionFile:
                upgradeSourceVersion = versionFile.read()
        else:
            raise Exception('Only valid during upgrades')
        if upgradeSourceVersion == "":
            raise Exception('upgrade version file is not set in import dir')
        return upgradeSourceVersion

    def getInstallFolder(self):
       return get_cis_install_dir()

    def getConfigFolderPath(self):
        return self.get_config_dir()

    def getJavaHomePath(self):
        return get_java_home()

    def getTomcatRootPath(self):
        return os.environ['VMWARE_TCROOT']

    def getSSOHomePath(self):
        return def_by_os(self.get_home_dir(), "%s\\vmware-%s" % (get_cis_install_dir(), self._component_name) )

    def getTCInstanceRootPath(self):
        return def_by_os(self.get_home_dir(), '%s' % (os.environ['VMWARE_RUNTIME_DATA_DIR']) )

    def getLogsPath(self):
        return self.get_log_dir()

    def executeCommand(self, args, quiet=False):
        return run_command(args, None, quiet)

    def init(self):
        self.Log("VMWareIdentityFirstBoot ctor begin.")

        #general constants
        self.__vmafdSvcRetryCount    = 20
        self.__vmafdSvcRetryInterval = 30
        self.__dirSvcRetryCount    = 20
        self.__dirSvcRetryInterval = 30
        self.__idmRetryCount    = 10
        self.__idmRetryInterval = 30
        self.__stsRetryCount    = 10
        self.__stsRetryInterval = 30

        # Linux install path
        self.__vmware_libdir_lin = "/opt/vmware/lib64"

        #certificate constants
        self.__leaf_cert_x509_name = "ssoserver.crt"
        self.__leaf_cert_key_name = "ssoserver.key"
        self.__leaf_cert_pub_name = "ssoserver.pub"
        self.__leaf_cert_alias = "ssoserver"

        self.__root_cert_x509_name = "ssoserverRoot.crt"
        self.__ssl_root_cert_x509_name = "sslRoot.crt"

        self.__leaf_cert_pkcs12_name = "ssoserver.p12"
        self.__leaf_sign_cert_x509_name = "ssoserverSign.crt"
        self.__leaf_sign_cert_key_name = "ssoserverSign.key"
        self.__leaf_sign_cert_pub_name = "ssoserverSign.pub"
        self.__leaf_sign_cert_alias = "ssoserverSign"
        self.__machine_cert = "machine.crt"

        self.__ssl_root_cert_x509_path = os.path.join(self.getSSOCertPath(), self.__ssl_root_cert_x509_name)
        self.__root_cert_x509_path = os.path.join(self.getSSOCertPath(), self.__root_cert_x509_name)
        self.__leaf_cert_x509_path = os.path.join(self.getSSOCertPath(), self.__leaf_cert_x509_name)
        self.__leaf_cert_key = os.path.join(self.getSSOCertPath(), self.__leaf_cert_key_name)
        self.__machine_cert_path = os.path.join(self.getSSOCertPath(), self.__machine_cert)

        self.__leaf_cert_pkcs_path = os.path.join(self.getSSOCertPath(), self.__leaf_cert_pkcs12_name)

        self.__leaf_sign_cert_x509_path = os.path.join(self.getSSOCertPath(), self.__leaf_sign_cert_x509_name)
        self.__leaf_sign_cert_key = os.path.join(self.getSSOCertPath(), self.__leaf_sign_cert_key_name)

        self.__cert_password = "changeme"
        self.__cert_password_ssl = "pass:%s" % self.__cert_password
        self.__certoolpath = self.getCertoolPath()
        self.__cert_tool_configName = ''
        self.Log("__certoolpath=%s" %  self.__certoolpath)

        #services
        self.__sts_instance_name = self.defByOs( "vmware-sts", "VMwareSTSService" )
        self.__companyName = "VMware, Inc."
        self.__ssoProductName = "VMware Identity Services"

        #java exe and settings
        self.__javaBin = os.path.join(self.getJavaHomePath(), "bin", self.defByOs("java", "java.exe"))
        self.__classpath = self.getClassPath()

        #future: consider changing the legacy linux sts service name to use instance_name "vmware-sts"
        #or better yet to component name "vmware-sso"
        self.__sts_service_name = self.defByOs("vmware-stsd", self.__sts_instance_name)
        self.__sts_local_port = STS_DEFAULT_PORT
        self.__vmwareurlhostname = ''
        self.__vmwarehostnametype = ''
        self.__vmwarehostname = ''
        self.__idm_servicename = self.defByOs("vmware-sts-idmd", "VMwareIdentityMgmtService")
        self.Log("__idm_servicename=%s" % self.__idm_servicename)

        #other configurations
        self.__vmdirDomainNameDefault = "vsphere.local"
        self.__vmdirAdminDNDefault = "cn=administrator,cn=users,dc=vsphere,dc=local"
        self.__hostnameFilePath = os.path.join(self.getConfigFolderPath(), "hostname.txt")
        self.__sts_installer_log_option = "-Dvmware.log.dir=%s" % self.getLogsPath()
        self.__sts_installer_error_option = "-XX:ErrorFile=%s" % os.path.join(self.getLogsPath(), "hs_err_stsinstaller_pid%p.log")
        self.__sts_installer_error_option += "-XX:HeapDumpPath=%s" % self.getLogsPath()
        self.Log("__hostnameFilePath=%s" % self.__hostnameFilePath)
        self.__regkeySPSystemDomainBackCompat="SPSystemDomainBackCompat"
        self.__regkeySPSystemDomainAlias="SPSystemDomainAlias"
        self.__regkeySPSystemDomainUserAliases="SPSystemDomainUserAliases"
        self.__regvalSPSystemDomainUserAliases=['__Administrators__@Administrators']
        self.__regvalSPSystemDomainAlias="SYSTEM-DOMAIN"
        self.__rhttp_service_name= self.defByOs("vmware-rhttpproxy","VMware HTTP Reverse Proxy")

        if os.environ.has_key('VMWARE_CLOUDVM_RAM_SIZE'):
            self.__cloudvm_ram_size = os.environ['VMWARE_CLOUDVM_RAM_SIZE']
        else:
            self.Log("Environment variable [VMWARE_CLOUDVM_RAM_SIZE] is not set.")
            if self.isWindowsOs():
                self.__cloudvm_ram_size = os.path.join(self.getInstallFolder(), "visl-integration\\usr\\sbin\\cloudvm-ram-size")
            else:
                self.__cloudvm_ram_size = "/usr/sbin/cloudvm-ram-size"
        self.Log("__cloudvm_ram_size=%s" % self.__cloudvm_ram_size)

        # set Tanuki wrapper dir
        if self.isWindowsOs():
            self.__vmidentity_home = os.path.join(self.getInstallFolder(), self.__ssoProductName)
            self.__wrapper_root = os.path.join(self.__vmidentity_home, "wrapper")
            self.__wrapper_bin = os.path.join(self.__wrapper_root, "bin")
            self.__wrapper_conf = os.path.join(self.__wrapper_root, "conf")
            self.__wrapper_lib = os.path.join(self.__wrapper_root, "lib")

        #sts tc instance
        self.__tcinst_root = self.getTCInstanceRootPath()
        self.__tc_sts_base = os.path.join(self.__tcinst_root, self.__sts_instance_name)
        self.__tc_sts_bin = os.path.join(self.__tc_sts_base, "bin")
        self.__tc_sts_conf = os.path.join(self.__tc_sts_base, "conf")

        #Default configuration for Xmx and XX:Maxperm.
        self.__default_sts_max_ram = self.defByOs("512m", "512m")
        self.__default_sts_max_perm = self.defByOs("128m","256m")
        #platform specific var
        self.__lw_reg_bin = "/opt/likewise/bin/lwregshell"
        self.__lw_reg_file = "/opt/vmware/share/config/idm.reg"
        self.__linux_firewall_config = "/etc/vmware/appliance/firewall/vmware-sso"
        self.__linux_firewall_config_json = "/usr/lib/vmware-sso/firewall/sso-firewall.json"
        if self.isUpgradeMode() :
            upgrade_import_dir = self.getInstallParameter('upgrade.import.directory')
            ur_ctx_file = os.path.join(upgrade_import_dir, "system-data", "UpgradeRunner.ctx")
            if os.path.exists(ur_ctx_file):
                # We are running in upgrade mode
                with open(ur_ctx_file) as fp:
                    context = json.load(fp)
                self.src_platform = context['sourcePlatform']
                self.dst_platform = context['destinationPlatform']


    def boot (self):

        self.createHostnameFile()

        if (self.canWaitForInstallParameter()):
            self.__vmcacertdir = self.waitForInstallParameter(INSTALL_PARAMETER_VMCA_CERT_DIR)
            stsLocalPort = self.getInstallParameter(INSTALL_PARAMETER_STS_PORT, STS_DEFAULT_PORT)
            self.__sts_local_port = stsLocalPort.encode('ascii')

        self.checkVmafdService(self.__vmafdSvcRetryCount, self.__vmafdSvcRetryInterval)
        self.checkDirectoryService(self.__dirSvcRetryCount, self.__dirSvcRetryInterval)
        self.checkCertificateService()
        self.generateCertificates()
        if self.isUpgradeMode() and (self.getUpgradeSourceVersion() == "5.5" or self.getUpgradeSourceVersion() == "6.0"):
            self.doLduCleanup()
        self.configureIdentityManager(self.__idmRetryCount, self.__idmRetryInterval)
        self.configureLookupService()
        self.configureSTS(self.__stsRetryCount, self.__stsRetryInterval)
        self.registerPorts(self.getStsPort(), self.__sts_local_port)
        self.checkSTS(self.__stsRetryCount, self.__stsRetryInterval)
        self.publishSSOProperties()

    def createHostnameFile(self):

        self.Log("Creating hostname file from install-parameter")
        createHostnameFile = True

        if not os.path.exists(self.getConfigFolderPath()):
            os.makedirs(self.getConfigFolderPath())

        if self.isUpgradeMode() and self.getUpgradeSourceVersion() == "6.0":
            hostnameBackupFile = os.path.join(self.getSSOImportDir(), "conf", "hostname.txt")
            if os.path.exists(hostnameBackupFile) == False:
                self.Log("hostname.txt doesn't exist in Import folder")
            else :
                createHostnameFile = False
                shutil.copy2(hostnameBackupFile,self.__hostnameFilePath)

        if (createHostnameFile == True and self.canWaitForInstallParameter()):
            self.__vmwareurlhostname = self.waitForInstallParameter(INSTALL_PARAMETER_URL_HOSTNAME)
            self.__vmwarehostnametype = self.waitForInstallParameter(INSTALL_PARAMETER_HOSTNAME_TYPE)
            open(self.__hostnameFilePath, 'w').write(self.__vmwareurlhostname)
            self.__vmwarehostname = self.waitForInstallParameter(INSTALL_PARAMETER_HOSTNAME)
        else:
            self.Log("skip creating hostname file as Install parameter bin does not exist")

    def checkDirectoryService(self, retryCount, retryInterval):

        self.Log("Checking connection to Directory Service")
        command = '"%s" -cp %s %s %s com.vmware.identity.installer.STSInstaller \
            --check-dir-svc --retry-count %d --retry-interval %d'\
            % (self.__javaBin,
            self.__classpath,
            self.__sts_installer_log_option,
            self.__sts_installer_error_option,
            retryCount,
            retryInterval)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('checkDirectoryService|Failed to check directory service.')

        # wait to make sure replication is done (install parameters are published only afterwards)
        if self.canWaitForInstallParameter():
            self.__vmdirUsername = self.waitForInstallParameter(INSTALL_PARAMETER_VMDIR_ADMIN_DN, self.__vmdirAdminDNDefault)
            self.__vmdirPassword = self.waitForInstallParameter(INSTALL_PARAMETER_VMDIR_PWD)
            self.__vmdirDomainName = self.waitForInstallParameter(INSTALL_PARAMETER_VMDIR_DOMAIN_NAME, self.__vmdirDomainNameDefault)

    def checkVmafdService(self, retryCount, retryInterval):

        self.Log("Checking connection to Vmafd Service")
        command = '"%s" -cp %s %s %s com.vmware.identity.installer.STSInstaller --check-vmafd-svc --retry-count %d --retry-interval %d'\
            % (self.__javaBin,
            self.__classpath,
            self.__sts_installer_log_option,
            self.__sts_installer_error_option,
            retryCount,
            retryInterval)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('checkVmafdService|Failed to check Vmafd service.')

    def doLduCleanup(self):

        self.Log("Checking and cleaning LDU Structure")
        command = '"%s" -cp %s %s %s com.vmware.identity.installer.STSInstaller --do-ldu-cleanup'\
            % (self.__javaBin,
            self.__classpath,
            self.__sts_installer_log_option,
            self.__sts_installer_error_option)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('doLduCleanup|Failed to cleanup LDU structure.')

    def checkCertificateService(self):

        self.Log("Checking connection to Certificate Service")

        # Wait for certificate server to start for 10 min intead of the default 3 min
        if (self.__vmwarehostname != ""):
            command = '"%s" --WaitVMCA --server="%s" --wait=10' % (self.__certoolpath, self.__vmwarehostname)
        else:
            command = '"%s" --WaitVMCA --wait=10' % (self.__certoolpath)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('checkCertificateService|Failed to check certificate service.')

    def configureIdentityManager(self, retryCount, retryInterval):

        self.Log("Configuring Identity Manager")

        if self.isWindowsOs():
            self.configProcrunRegistry()
        else:
            command = '"%s" import "%s"' % (self.__lw_reg_bin, self.__lw_reg_file)
            # Trying to import all the default registry settings
            return_code = self.runShellCommand(command)
            if return_code != 0:
                raise Exception('Failed to import IDM registry settings.')

        if self.isUpgradeMode():
            self.setupRegistryForUpgrade()

        self.startIDMService()

        retainTokenPolicy = ''
        if self.isUpgradeMode():
            retainTokenPolicy = '--retain-token-policy'

        command = '"%s" -cp %s %s %s \
             com.vmware.identity.installer.STSInstaller --install \
             --root-cert-path "%s" --cert-path "%s" --private-key-path "%s" --retry-count \
             %d --retry-interval %d %s' \
             % (self.__javaBin,
             self.__classpath,
             self.__sts_installer_log_option,
             self.__sts_installer_error_option,
             self.__root_cert_x509_path,
             self.__leaf_sign_cert_x509_path,
             self.__leaf_sign_cert_key,
             retryCount,
             retryInterval,
             retainTokenPolicy)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('Failed to configure identity manager')

    def configureLookupService(self):

        print "Configuring Lookup Service"

        log4j = "initls-log4j.properties"
        command = '"%s" -cp %s -Dlog4j.configuration=%s \
             com.vmware.vim.lookup.tools.InitializeLookupService \
             --cert-path "%s" --host-name "%s" --http-port %s' \
             % (self.__javaBin,
                self.__classpath,
                log4j,
                self.getStsCertPath(),
                self.__vmwarehostname,
                self.getStsPort())

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('Failed to configure lookup service')

        # Since we are copy the vmdir database during an upgrade we
        # don't need to set up the lookup service
        if not self.isUpgradeMode():
            cert_path = self.getStsCertPath()
            port = self.getStsPort()
            command = '"%s" -cp %s -Dlog4j.configuration=%s \
                 com.vmware.vim.lookup.tools.InitializeLookupService \
                 --cert-path "%s" --host-name "%s" --http-port %s --legacy' \
                 % (self.__javaBin,
                    self.__classpath,
                    log4j,
                    cert_path,
                    self.__vmwarehostname,
                    port)

            return_code = self.runShellCommand(command)

            if return_code != 0:
                raise Exception('Failed to configure lookup service')

        if (self.canWaitForInstallParameter()):
            if (self.getInstallParameter("lookup.hidessltrust", 'false').lower() == 'true'):
                subkey = 'Software\\VMware\\Identity\\Configuration'
                propName = "LookupHideSslTrust"
                propValue = "True"
                if self.isWindowsOs():
                    keyConfig = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, subkey, 0, _winreg.KEY_SET_VALUE)
                    _winreg.SetValueEx(keyConfig, propName, 0, _winreg.REG_SZ, propValue)
                else:
                    keyName='[HKEY_THIS_MACHINE\\%s]' % subkey
                    command = "%s delete_value '%s' %s"  \
                        % (self.__lw_reg_bin, keyName, propName)
                    self.runShellCommand(command)
                    command = "%s add_value '%s' %s REG_SZ %s"  \
                        % (self.__lw_reg_bin, keyName, propName, propValue)
                    self.runShellCommand(command)


    def unconfigureLookupService(self):

        print "Removing Lookup Service data"

        if (self.canWaitForInstallParameter()):
            hostnameOpt = '--host-name "%s"' % self.waitForInstallParameter(INSTALL_PARAMETER_HOSTNAME)
        else:
            hostnameOpt = ''

        log4j = "initls-log4j.properties"
        command = '"%s" -cp %s -Dlog4j.configuration=%s \
             com.vmware.vim.lookup.tools.InitializeLookupService \
             --cleanup %s' \
             % (self.__javaBin,
                self.__classpath,
                log4j,
                hostnameOpt)

        self.runShellCommand(command)

    def createCertToolconfig(self, filename, content):

        try:
            certool_config_path=os.path.join(self.getConfigFolderPath(),filename);
            configfile = open(certool_config_path,'w')   # Trying to create a new config file
            configfile.write(content);
            configfile.close()
            self.__cert_tool_configName = os.path.normpath(certool_config_path)

        except:
            raise Exception('Failed to write custom config file for vmidentity')

        return None

    def generateCertificates(self):
        if self.isUpgradeMode():
            self.copyCertificates()
        else:
            self.createCertificates()

    #copy certs and keys from backup
    def copyCertificates(self):
        backupKeyPath = ""
        if self.getUpgradeSourceVersion() == "5.5" and self.src_platform.lower() == 'windows':
            backupKeyPath = os.path.join(self.getSSOImportDir(), 'conf')
        else :
            backupKeyPath = os.path.join(self.getSSOImportDir(), 'conf','keys')
        confKeyPath = os.path.join(self.getConfigFolderPath(), 'keys')
        if not os.path.exists(self.getConfigFolderPath()):
            os.makedirs(self.getConfigFolderPath())

        shutil.copytree(backupKeyPath, confKeyPath)
        shutil.copy2(self.__root_cert_x509_path, self.__ssl_root_cert_x509_path)

        # apply authorized user permissions for certificate dir, for upgrade path
        self.applyAuthorizedUserPermission(confKeyPath)

    def createCertificates(self):
        #future: check out common lib if it can simplify the steps.
        self.Log("Generating SSO Certificates using VMware Certificate Service...")

        openssl_bin = self.getOpenSSLExePath()
        certDirPath = self.getSSOCertPath()
        certconfig_filename = "signcert_vmidentity.cfg"

        if not os.path.exists(certDirPath):
            os.makedirs(certDirPath)

        # apply authorized user permissions for certificate dir, for standard installation path
        self.applyAuthorizedUserPermission(certDirPath)

        leaf_sign_cert_pub = os.path.join(certDirPath, self.__leaf_sign_cert_pub_name)
        leaf_sign_cert_alias = self.__leaf_sign_cert_alias

        # step 0 root signing x509 certificate
        if (self.__vmwarehostname != ""):
            command = '"%s" --getrootca --cert="%s" --server="%s"' \
                 % (self.__certoolpath, self.__root_cert_x509_path,self.__vmwarehostname)
        else:
            command = '"%s" --getrootca --cert="%s"' \
                 % (self.__certoolpath, self.__root_cert_x509_path)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('Failed to get VMCA root certificate.')

        # step 1. create or migrate ssl certs
        if self.isUpgradeMode():
            self.migrateSSLCertificate()
        else:
            self.__ssl_root_cert_x509_path = self.__root_cert_x509_path
            self.generateSSLCertificate()

        # step 2. create sign keys
        command = '"%s" --genkey --privkey="%s" --pubkey="%s"' \
            % (self.__certoolpath,
            self.__leaf_sign_cert_key,
            leaf_sign_cert_pub)
        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('Failed to generate sso sign keys! Please make sure VMware Certificate Service is installed and up running..')

        #step 3. create sign cert
        if (self.__vmwarehostnametype == 'fqdn' and self.__vmwarehostname != ""):
            self.createCertToolconfig(certconfig_filename, "Hostname = " + self.__vmwarehostname)
            command = '"%s" --gencert --priv="%s" --Name="%s" --cert="%s" --config="%s" --server="%s"' \
            % (self.__certoolpath,
            self.__leaf_sign_cert_key,
            leaf_sign_cert_alias,
            self.__leaf_sign_cert_x509_path,
            self.__cert_tool_configName,
            self.__vmwarehostname)
        elif ((self.__vmwarehostnametype == 'ipv4' or self.__vmwarehostnametype == 'ipv6') and self.__vmwarehostname != ""):
            self.createCertToolconfig(certconfig_filename, "")
            command = '"%s" --gencert --priv="%s" --Name="%s" --cert="%s" --IPAddress="%s" --config="%s" --server="%s"' \
                % (self.__certoolpath,
                 self.__leaf_sign_cert_key,
                 leaf_sign_cert_alias,
                 self.__leaf_sign_cert_x509_path,
                 self.__vmwarehostname,
                 self.__cert_tool_configName,
                 self.__vmwarehostname)
        else:
            self.createCertToolconfig(certconfig_filename, "")
            command = '"%s" --gencert --priv="%s" --Name="%s" --cert="%s" --config="%s" ' \
            % (self.__certoolpath,
            self.__leaf_sign_cert_key,
            leaf_sign_cert_alias,
            self.__leaf_sign_cert_x509_path,
            self.__cert_tool_configName)

        return_code = self.runShellCommand(command)

        if return_code != 0:
          raise Exception('Failed to generate sso x509 certificate')

    def applyAuthorizedUserPermission(self, dir):
        if os.path.exists(dir):
            if self.isWindowsOs():
                #On Windows, only apply access permission for authorized users
                self.Log("Start to apply authorized user permission for " + dir)

                # Build authorized SIDs list
                sids = []
                sids.append(win32security.GetBinarySid("S-1-5-18"))     # SID for account "NT AUTHORITY\SYSTEM"
                sids.append(win32security.GetBinarySid("S-1-5-32-544")) # SID for account "BUILTIN\Administrators"
                self.Log("Apply user permission for these accounts: NT AUTHORITY\SYSTEM and BUILTIN\Administrators")

                try:
                    security_desc = win32security.GetFileSecurity(dir, win32security.DACL_SECURITY_INFORMATION)
                    # Create a new DACL and add ACEs
                    dacl = win32security.ACL()
                    for sid in sids:
                        dacl.AddAccessAllowedAceEx(win32security.ACL_REVISION_DS, win32security.OBJECT_INHERIT_ACE | win32security.CONTAINER_INHERIT_ACE, con.GENERIC_ALL, sid)

                    security_desc.SetSecurityDescriptorDacl(1, dacl, 0)
                    win32security.SetFileSecurity(dir, win32security.DACL_SECURITY_INFORMATION, security_desc)
                except win32security.error, (hr_got, func, msg):
                    self.Log("win32security failure detected at %s: in function: %s, with error message: %s" % (hr_got, func, msg))
                    raise Exception("Apply authorized user permission for " + dir + " failed due to win32security error.")
                self.Log("Apply authorized user permission for " + dir + " completed.")
            else:
                #On Linux, Only allow root for R/W access to dir
                command = 'sudo /bin/chown -R root "%s"' % (dir)
                return_code = self.runShellCommand(command)
                if return_code != 0:
                    self.Log("Command: %s failed." % command)
                    raise Exception('Failed to set permission on certificate key directory')

                # apply "rwx" permission on dir recursively for user, "x" is only applied on directories
                # remove all group and other permissions.
                command = 'sudo /bin/chmod -R u+rwX,go-rwx "%s"' % (dir)
                return_code = self.runShellCommand(command)
                if return_code != 0:
                    self.Log("Command: %s failed." % command)
                    raise Exception('Failed to set permission on certificate key directory')
        else:
            self.Log("Can not apply authorized user permission due to the following dir does not exist: " + dir)

    def generateSSLCertificate(self):
        print "Generating SSL Certificates.."

        openssl_bin = self.getOpenSSLExePath()

        leaf_cert_pub = os.path.join(self.getSSOCertPath(), self.__leaf_cert_pub_name)
        leaf_cert_alias = self.__leaf_cert_alias
        certconfig_filename = "sslcert_vmidentity.cfg"


        # step 1.1 create ssl keys
        command = '"%s" --genkey --privkey="%s" --pubkey="%s"' % (self.__certoolpath, self.__leaf_cert_key, leaf_cert_pub)
        return_code = self.runShellCommand(command)
        if return_code != 0:
            raise Exception('Failed to generate sso keys! Please make sure VMware Certificate Service is installed and up running..')

        #step 1.2 create ssl cert
        if (self.__vmwarehostnametype == 'fqdn' and self.__vmwarehostname != ""):
            self.createCertToolconfig(certconfig_filename, "Hostname = " + self.__vmwarehostname)
            command = '"%s" --gencert --priv="%s" --Name="%s" --cert="%s" --config="%s" --server="%s"' \
                % (self.__certoolpath,
                 self.__leaf_cert_key,
                 leaf_cert_alias,
                 self.__leaf_cert_x509_path,
                 self.__cert_tool_configName,
                 self.__vmwarehostname)
        elif ((self.__vmwarehostnametype == 'ipv4' or self.__vmwarehostnametype == 'ipv6') and self.__vmwarehostname != ""):
            self.createCertToolconfig(certconfig_filename, "")
            command = '"%s" --gencert --priv="%s" --Name="%s" --cert="%s" --IPAddress="%s" --config="%s" --server="%s"' \
                % (self.__certoolpath,
                 self.__leaf_cert_key,
                 leaf_cert_alias,
                 self.__leaf_cert_x509_path,
                 self.__vmwarehostname,
                 self.__cert_tool_configName,
                 self.__vmwarehostname)
        else:
            self.createCertToolconfig(certconfig_filename, "")
            command = '"%s" --gencert --priv="%s" --Name="%s" --cert="%s" --config="%s"' \
                % (self.__certoolpath,
                self.__leaf_cert_key,
                leaf_cert_alias,
                self.__leaf_cert_x509_path,
                self.__cert_tool_configName)

        return_code = self.runShellCommand(command)

        if return_code != 0:
          raise Exception('Failed to generate sso x509 certificate')

        #step 1.4. creates pkcs12 file for tcServer
        command = '%s pkcs12 -export -in "%s" -inkey "%s" -out "%s" -name "%s" -passout %s \
            -CAfile "%s" -caname "rootca"' \
            % (openssl_bin,
            self.__leaf_cert_x509_path,
            self.__leaf_cert_key,
            self.__leaf_cert_pkcs_path,
            leaf_cert_alias,
            self.__cert_password_ssl,
            self.__ssl_root_cert_x509_path)

        return_code = self.runShellCommand(command)

        if return_code != 0:
            raise Exception('Failed to generate PKCS12 certificate')

    def migrateSSLCertificate(self):
        try:
            print "Copying SSL Certificates from export folder.."
            importFromPath = self.getSSOImportDir()
            shutil.copy2(os.path.join(importFromPath, self.__leaf_cert_x509_name), self.__leaf_cert_x509_path)
            shutil.copy2(os.path.join(importFromPath, self.__root_cert_x509_name), self.__ssl_root_cert_x509_path)
            shutil.copy2(os.path.join(importFromPath, self.__leaf_cert_pkcs12_name), self.__leaf_cert_pkcs_path)

        except Exception, e:
            self.Log("failed to migrate ssl certificate as either the \
                import folder is missing or cert files are not in the folder!! Use the default ssl certs instead.")
            traceback.print_exc()

    def createTCInstance(self):

        self.Log("creating tc server instance..")

        os.chdir(self.__tcinst_root)

        tcruntime_name = self.defByOs("tcruntime-instance.sh", "tcruntime-instance.bat")
        tcruntime_path = os.path.join(self.getTomcatRootPath(),tcruntime_name)
        create_command = '"%s" create -t bio-custom \
            --property bio-custom.http.port=7080 --force -t bio-ssl-localhost  \
            --property bio-ssl-localhost.https.port=%s %s' \
            % (tcruntime_path, self.__sts_local_port, self.__sts_instance_name)

        return_code = self.runShellCommand(create_command)

        if return_code != 0:
            raise Exception('Failed to create the service [%s, %d]' \
                % (self.__sts_instance_name, return_code))

        os.chdir(self.getTomcatRootPath())

        self.configTCLog()

        # apply authorized user permissions for Tomcat conf dir
        self.applyAuthorizedUserPermission(self.__tc_sts_conf)

    #
    # Config log directory
    #
    def configTCLog(self):

        #for linux only create a symbolic link to the common sso log directory.
        #TODO: schai maybe we should same on windows, need to change support bundle as well
        if not self.isWindowsOs():
            log_dir = os.path.join(self.__tc_sts_base, 'logs')
            command = '/bin/rm -rf "%s"' % (log_dir)
            return_code = self.runShellCommand(command)
            if return_code != 0:
                raise Exception('Cannot delete [%s, %d]' % (log_dir, return_code))

            command = '/bin/ln -s "%s" "%s"' % (self.getLogsPath(), log_dir)
            return_code = self.runShellCommand(command)
            if return_code != 0:
                raise Exception('Cannot create [%s, %d]' % (log_dir, return_code))

    def installJarAndWarFiles(self):

        self.Log("Installing war files..")

        tc_sts_webapps_folder = os.path.join(self.__tc_sts_base, "webapps")
        source_folder = self.getWebAppFilePath()

        shutil.copy2(os.path.join(source_folder, "ims.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "websso.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "openidconnect.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "sts.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "sso-adminserver.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "lookupservice.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "idm.war"), tc_sts_webapps_folder)
        shutil.copy2(os.path.join(source_folder, "afd.war"), tc_sts_webapps_folder)


    def moveTanukiWrapperFiles(self):

        if not os.path.exists(self.__wrapper_root):
            os.makedirs(self.__wrapper_root)
        if not os.path.exists(self.__wrapper_bin):
            os.makedirs(self.__wrapper_bin)
        if not os.path.exists(self.__wrapper_conf):
            os.makedirs(self.__wrapper_conf)
        if not os.path.exists(self.__wrapper_lib):
            os.makedirs(self.__wrapper_lib)

        with open(os.path.join(self.__tc_sts_conf, "wrapper.arch"), 'r') as f:
            arch = f.readline().rstrip('\n')

        # move bin and lib files
        tanukisrc = os.path.join(self.__tc_sts_bin, arch)
        shutil.move(os.path.join(tanukisrc, "wrapper.exe"), self.__wrapper_bin)
        shutil.move(os.path.join(tanukisrc, "wrapper.dll"), self.__wrapper_bin)
        shutil.move(os.path.join(tanukisrc, "wrapper.jar"), self.__wrapper_lib)
        shutil.move(os.path.join(tanukisrc, "threaddumpwrapper.jar"), self.__wrapper_lib)

        # move config files
        shutil.move(os.path.join(self.__tc_sts_conf, "wrapper.conf"), self.__wrapper_conf)
        shutil.move(os.path.join(self.__tc_sts_conf, "wrapper.arch"), self.__wrapper_conf)

    def installInstAsWinService(self):

        if not (self.isWindowsOs() ):
            raise Exception("installInstAsWinService is unapplicable wor linux.")

        self.setSTSServiceProperties()

        # move Tanuki wrapper files from TC instance dir to VMware Indentity Service wrapper dir in Program Files
        self.moveTanukiWrapperFiles()

        # delete bin folder in TC instance dir.
        if os.path.exists(self.__tc_sts_bin):
            self.retryableRmtree(self.__tc_sts_bin)

        self.Log("Installing server...")

        os.chdir(self.__tcinst_root)

        command = os.path.join(self.__wrapper_bin, "InstallApp-NT.bat")
        return_code = self.runShellCommand(command)
        if return_code != 0:
                raise Exception('Failed to install the service %s Error code=%d' \
                    % (self.__sts_instance_name, return_code))

        command = 'sc.exe failure VMwareSTS reset= 86400 actions= restart/30000/restart/60000/restart/90000'
        return_code = self.runShellCommand(command)
        if return_code != 0:
                raise Exception('Failed to set VMwareSTS service recovery options. Error code=%d' \
                    % (return_code))

    # Retryable shutil.rmtree function
    def retryableRmtree(self, path, nRetry = 100, secToSleep = 2):
        ok = False
        for i in range(1, nRetry + 1):
            try:
                shutil.rmtree(path)
                ok = True
                break
            except OSError, err:
                self.Log("Failed to remove path %s with shutil.rmtree at attempt %d: %s" % (path, i, err))
                self.Log("Files in path %s at attempt %d: %s" % (path, i, str(os.listdir(path))))
                time.sleep(secToSleep)

        if not ok:
            self.Log("Failed to remove path %s with shutil.rmtree, even after %d attempts." % (path, nRetry))
        else:
            self.Log("Path %s successfully removed." % path)

    #set service dependencies, display name, description and name.
    def setSTSServiceProperties(self):
        self.Log("Setting STS service properties...")

        wrapper_conf_file = os.path.join(self.__tcinst_root, 'VMwareSTSService', 'conf', 'wrapper.conf')
        for line in fileinput.input(wrapper_conf_file, inplace=True):
            if 'wrapper.ntservice.dependency.1=' in line:
                line = 'wrapper.ntservice.dependency.1=VMwareIdentityMgmtService\n'
            elif 'wrapper.ntservice.name=' in line:
                line = 'wrapper.ntservice.name=VMwareSTS\n'
            elif 'wrapper.ntservice.displayname=' in line:
                line = 'wrapper.ntservice.displayname=VMware Security Token Service\n'
            elif 'wrapper.ntservice.description=' in line:
                line = 'wrapper.ntservice.description=VMware Single Sign-On STS Service\n'
            print line,

    def copyCertForTC(self):

        targetkeyfile = os.path.join(self.__tc_sts_conf, self.__leaf_cert_key_name)
        targetcrtfile = os.path.join(self.__tc_sts_conf, self.__leaf_cert_x509_name)
        targetpkcsfile = os.path.join(self.__tc_sts_conf, self.__leaf_cert_pkcs12_name)

        self.Log("Installing SSL certificate in TC Server at [%s]" % targetcrtfile)
        shutil.copy2(self.__leaf_cert_x509_path, targetcrtfile)

        if not self.isUpgradeMode():
            self.Log("Installing SSL Private Key in TC Server at [%s]" % targetkeyfile)
            shutil.copy2(self.__leaf_cert_key, targetkeyfile)

        #replace server.xml when upgrading so that keystore is configured appropriately
        if self.isUpgradeMode():
            self.Log('Upgrade: Copying server xml and pkcs12 to tcserver conf')
            exportedServerXML = os.path.join(self.getSSOImportDir(), "server.xml")
            localServerXMLPath = os.path.join(self.__tc_sts_conf, "server.xml")
            shutil.copy2(exportedServerXML, localServerXMLPath)
            #pending - handle custom p12 file names
            exportedP12Path = os.path.join(self.getSSOImportDir(), "ssoserver.p12")
            localServerP12Path = os.path.join(self.__tc_sts_conf, "ssoserver.p12")
            shutil.copy2(exportedP12Path, localServerP12Path)
        else:
            self.Log("Installing SSL certificate pkcs12 in TC Server at [%s]" % targetpkcsfile)
            shutil.copy2(self.__leaf_cert_pkcs_path, targetpkcsfile)

    def startSTSService(self):

        self.Log("Starting SSO service...")
        returnCode = 0
        if self.isWindowsOs():
            #
            #on windows use the script provided by TC. Windows service name is
            #generated from TC's STS instance name.

            start_command = os.path.join(self.__wrapper_bin, "StartApp-NT.bat")

            for n in range(1, self.__stsRetryCount):
                returnCode = self.runShellCommand(start_command)
                if (returnCode == 0):
                    break
                else:
                    time.sleep(self.__stsRetryInterval)
        else:
            #
            #on linux use vmware-stsd start
            returnCode = self.startService(self.__sts_service_name, self.__stsRetryCount * self.__stsRetryInterval)

        if returnCode != 0:
            raise Exception('startSTSService|Failed to start service [%s, %d]' % (self.__sts_service_name, returnCode))

    def stopSTSService(self):

        self.Log("Stopping SSO service...")
        returnCode = 0
        if self.isWindowsOs():
            os.chdir(self.__tcinst_root)
            #check service status, try to stop only if it is running.
            command = os.path.join(self.__wrapper_bin, "wrapper.exe -q ..\\conf\\wrapper.conf")
            returnCode = self.runShellCommand(command)
            if (returnCode == STS_STATUS_STOPPED or \
                returnCode == STS_STATUS_NOT_INSTALLED or\
                returnCode == STS_STATUS_SERVERFILE_NOT_EXIST):
                self.Log("Service is found not installed or not running!")
                return

            command = os.path.join(self.__wrapper_bin, "StopApp-NT.bat")

            for n in range(1, self.__stsRetryCount):
                returnCode = self.runShellCommand(command)
                if (returnCode == 0):
                    break
                else:
                    time.sleep(self.__stsRetryInterval)
        else:
            returnCode = self.stopService(self.__sts_service_name, self.__stsRetryCount * self.__stsRetryInterval )

        if returnCode != 0:
            raise Exception('stopSTSService|Failed to stop service [%s, %d]' % (self.__sts_service_name, returnCode))

    def uninstallSTSService(self):

        self.Log("Uninstalling server...")

        if self.isWindowsOs():
            os.chdir(self.__tcinst_root)
            #check service status, try to uninstall only if it is installed.
            command = os.path.join(self.__wrapper_bin, "wrapper.exe -q ..\\conf\\wrapper.conf")
            returnCode = self.runShellCommand(command)
            if (returnCode == STS_STATUS_NOT_INSTALLED or\
                returnCode == STS_STATUS_SERVERFILE_NOT_EXIST):
                self.Log("Service is found not installed! Skipping uninstall ...")
                return

            command = os.path.join(self.__wrapper_bin, "UninstallApp-NT.bat")

            self.Log("Uninstall command="+command)

            p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

            output = p.communicate()[0].rstrip()

            self.Log(output)

            if p.returncode != 0:
                raise Exception('uninstallSTSService|Failed to uninstall the service %s Error code=%d' % (self.__sts_service_name, p.returncode))

            os.chdir(self.getTomcatRootPath())

            # delete files moved from TC instance bin dir
            wrapper_exe_file = os.path.join(self.__wrapper_bin, "wrapper.exe")
            wrapper_dll_file = os.path.join(self.__wrapper_bin, "wrapper.dll")
            wrapper_log_file = os.path.join(self.__wrapper_bin, "wrapper.log")
            if os.path.isfile(wrapper_exe_file):
                os.remove(wrapper_exe_file)
            if os.path.isfile(wrapper_dll_file):
                os.remove(wrapper_dll_file)
            if os.path.isfile(wrapper_log_file):
                os.remove(wrapper_log_file)
            if os.path.exists(self.__wrapper_lib):
                self.retryableRmtree(self.__wrapper_lib)
            if os.path.exists(self.__wrapper_conf):
                self.retryableRmtree(self.__wrapper_conf)

            # delete manually copied Tomcat templates directories
            bio_ssl_localhost_dir = os.path.join(self.getTomcatRootPath(), "templates", "bio-ssl-localhost")
            bio_custom_dir = os.path.join(self.getTomcatRootPath(), "templates", "bio-custom")
            if os.path.exists(bio_ssl_localhost_dir):
                self.retryableRmtree(bio_ssl_localhost_dir)
            if os.path.exists(bio_custom_dir):
                self.retryableRmtree(bio_custom_dir)

        #else not needed for linux

        self.unconfigureLookupService()

    def setIDMServiceToAutoStart(self):
        # change the service configuration from demand to auto-start
        scm = win32service.OpenSCManager(None, None,
                                         win32service.SC_MANAGER_ALL_ACCESS)
        svc = win32service.OpenService(scm, self.__idm_servicename,
                                       win32service.SC_MANAGER_ALL_ACCESS)
        win32service.ChangeServiceConfig(svc, win32service.SERVICE_NO_CHANGE,
                                         win32service.SERVICE_AUTO_START,
                                         win32service.SERVICE_NO_CHANGE,
                                         None, None, 0, None, None, None, None)
        win32service.CloseServiceHandle(svc)

    def startIDMService(self):

        if self.isWindowsOs():
            self.Log("Setting IDM service to auto-start...")
            self.setIDMServiceToAutoStart()
        else:
            # Change to a directory that will continue to exist for jsvc
            os.chdir(self.__vmware_libdir_lin)

        self.Log("Starting IDM service...")
        returnCode = self.startService(self.__idm_servicename,self.__idmRetryCount * self.__idmRetryInterval)
        if returnCode != 0:
            raise Exception('startIDMService|Failed to start service [%s]' % (self.__idm_servicename))

    def stopIDMService(self):

        self.Log("Stopping IDM service...")

        returnCode = self.stopService(self.__idm_servicename)
        if returnCode != 0:
            raise Exception('stopIDMService|Failed to stop service [%s]' % (self.__idm_servicename))

    def copyTCTemplate(self):

        self.Log("Install sts tcserver template..")

        dest = os.path.join(self.getTomcatRootPath(), "templates", "bio-ssl-localhost")
        shutil.rmtree(dest, ignore_errors=1)
        source_template = os.path.join(self.getTCTemplatePath(), "bio-ssl-localhost")
        shutil.copytree(source_template, dest )

        self.Log("Install sts tcserver template (custom)..")

        dest = os.path.join(self.getTomcatRootPath(), "templates", "bio-custom")
        shutil.rmtree(dest, ignore_errors=1)
        source_template = os.path.join(self.getTCTemplatePath(), "bio-custom")
        shutil.copytree(source_template, dest)

    def configTCSetenv(self, javaOptions):

        self.Log("Configure setenv.bat..")

        setenv_name = self.defByOs("setenv.sh", "setenv.bat")
        setenv_path = os.path.join(self.__tc_sts_bin,setenv_name)
        envData = open(setenv_path).read()

        javaOptionsString = string.join(javaOptions, ' ').replace("\\", "\\\\") # two slashes so that re.sub does not crash

        #set options in setenv script file
        if self.isWindowsOs():
            envData = re.sub(r'set JVM_OPTS=.*', r'set JVM_OPTS=%s' %(javaOptionsString), envData)
            envData = re.sub(r'set JAVA_OPTS', r'set CATALINA_OPTS=-ea\nset JAVA_OPTS', envData)
        else:
            envData = re.sub(r'JVM_OPTS=.*', r'JVM_OPTS="%s"' %(javaOptionsString), envData)
            envData = re.sub(r'JAVA_OPTS=', r'CATALINA_OPTS="-ea"\nJAVA_OPTS=', envData)

        open(setenv_path, "w").write(envData)

    def processWrapperConfFile(self, file_name, javaOptions):
        args = (r'(wrapper.cpu.timeout=)',
                r'(wrapper.shutdown.timeout=)',
                r'(wrapper.jvm_exit.timeout=)')

        foundJavaOptions = False
        with open(file_name, 'r') as fp:
            array = []
            for line in fp:
                string = line.rstrip('\n')
                omitString = False # omit deployed wrapper.java.additional options in order to replace them

                if (string == 'set.CATALINA_HOME=..\\..') :
                    array.append('set.CATALINA_HOME=%VMWARE_TOMCAT%')
                    omitString = True

                if (string == 'set.CATALINA_BASE=..\\..') :
                    array.append('set.CATALINA_BASE=' + self.__tc_sts_base)
                    omitString = True

                if ('wrapper.java.classpath.1=%CATALINA_BASE%\\bin' in string and 'wrapper.jar' in string) :
                    array.append('wrapper.java.classpath.1=' + self.__wrapper_lib + '\\wrapper.jar')
                    omitString = True

                if ('wrapper.java.classpath.2=%CATALINA_BASE%\\bin' in string and 'threaddumpwrapper.jar' in string) :
                    array.append('wrapper.java.classpath.2=' + self.__wrapper_lib + '\\threaddumpwrapper.jar')
                    omitString = True

                if ('wrapper.java.library.path.1=%CATALINA_BASE%\\bin' in string) :
                    omitString = True

                for arg in args:
                    # Comment out the above three properties
                    (new_string, count) = re.subn(arg, r'#\1', string)
                    if count > 0:
                        string = new_string
                        break
                javaOptionMatch = re.search(r'wrapper\.java\.additional\.[\d]+=', string)
                if javaOptionMatch:
                    omitString = True
                    foundJavaOptions = True

                if not javaOptionMatch and foundJavaOptions:
                    # I've seen the last java option, now append
                    foundJavaOptions = False # so that we do not execute the following loop again
                    javaOptionIndex = 1
                    for option in javaOptions:
                        array.append(r'wrapper.java.additional.%d="%s"' % (javaOptionIndex, option))
                        javaOptionIndex += 1

                if not omitString:
                    array.append(string)

        # Add new Java service properties
        array.append('')
        array.append('# Timeout for how long Tanuki will wait to restart a frozen JVM')
        array.append('wrapper.ping.timeout=120')
        array.append('wrapper.ping.interval.logged=300')
        array.append('wrapper.restart.delay=120')
        array.append('wrapper.restart.reload_configuration=TRUE')
        array.append('')

        array.append('# restart on abnormal exit')
        array.append('wrapper.on_exit.default=RESTART')
        array.append('wrapper.on_exit.0=SHUTDOWN')
        array.append('')

        array.append('# restart trigger on stuck transactions')
        array.append('wrapper.filter.trigger.1=SESSION_THRESHOLD_ERROR')
        array.append('wrapper.filter.action.1=RESTART')
        array.append('')

        array.append('# restart attempts')
        array.append('wrapper.max_failed_invocations=20')
        array.append('')

        array.append('# Windows service restart')
        array.append('wrapper.ntservice.recovery.reset=86400')
        array.append('wrapper.ntservice.recovery.1.failure=RESTART')
        array.append('wrapper.ntservice.recovery.2.failure=RESTART')
        array.append('wrapper.ntservice.recovery.3.failure=RESTART')
        array.append('wrapper.ntservice.recovery.1.delay=30')
        array.append('wrapper.ntservice.recovery.2.delay=60')
        array.append('wrapper.ntservice.recovery.3.delay=90')

        with open(file_name, 'w') as fp:
            for string in array:
                fp.write(string + '\n')

    def configWrapperConf(self, javaOptions):

        if self.isWindowsOs():
            self.Log("Configure conf/wrapper.conf..")
            wrapper_name = os.path.join(self.__tc_sts_conf,"wrapper.conf")
            self.processWrapperConfFile(wrapper_name, javaOptions)

    #
    # Create, configure and start TC server instance for SSO.
    #
    def configureSTS(self, retryCount, retryInterval):

        self.Log("Configuring Secure Token Server.")
        self.Log("Creating TC instance name="+self.__sts_instance_name)

        self.Log("tc instance base="+self.__tc_sts_base)

        os.environ['JAVA_HOME'] = self.getJavaHomePath()

        self.copyTCTemplate()
        self.createTCInstance()

        tomcat_temp_dir = os.path.join(self.__tc_sts_base, "temp")
        self.Log( "creating tomcat's temp dir [%s] if does not exist " %(tomcat_temp_dir) )
        if not os.path.exists(tomcat_temp_dir):
            os.makedirs(tomcat_temp_dir)

        java_mem_args = self.getCloudVmMaxHeapAndPermSize("vmware-stsd")
        javaMaxMemory = java_mem_args.get('heapsize',self.__default_sts_max_ram)
        javaMaxPerm = java_mem_args.get('permsize',self.__default_sts_max_perm)
        sso_log_dir = self.getLogsPath()
        if self.isWindowsOs():
            sso_log_dir = os.path.join(self.__tcinst_root, 'VMwareSTSService', 'logs')
        gcLogFileName = "\\gc.log" if self.isWindowsOs() else "gclogFile"
        gcLogFilePath = sso_log_dir + gcLogFileName
        jvmStsErrorFile = os.path.join(sso_log_dir, "hs_err_sts_pid%p.log")
        javaOptionsSetEnv = (
            r'-Djdk.map.althashing.threshold=512',
            r'-Xss228K',
            r'-Xmx%s' % (javaMaxMemory),
            r'-XX:MaxPermSize=%s' % (javaMaxPerm),
            r'-XX:+HeapDumpOnOutOfMemoryError',
            r'-XX:HeapDumpPath=%s' % (sso_log_dir),
            r'-XX:+PrintGCDetails',
            r'-XX:+PrintGCDateStamps',
            r'-XX:+PrintTenuringDistribution',
            r'-Xloggc:%s' % (gcLogFilePath),
            r'-XX:+UseGCLogFileRotation',
            r'-XX:NumberOfGCLogFiles=2',
            r'-XX:GCLogFileSize=5M',
            r'-XX:ErrorFile=%s' % jvmStsErrorFile,
            r'-XX:HeapDumpPath=%s' % sso_log_dir)

        javaOptionsWrapperConf = (
            r'-Djava.endorsed.dirs=%CATALINA_HOME%\common\endorsed',
            r'-Dcatalina.base=%CATALINA_BASE%',
            r'-Dcatalina.home=%CATALINA_HOME%',
            r'-Djava.io.tmpdir=%CATALINA_BASE%\temp',
            r'-Djava.util.logging.manager=com.springsource.tcserver.serviceability.logging.TcServerLogManager',
            r'-Djava.util.logging.config.file=%CATALINA_BASE%\conf\logging.properties',
            r'-Dwrapper.dump.port=-1')

        self.configTCSetenv(javaOptionsSetEnv)
        self.configWrapperConf(javaOptionsWrapperConf + javaOptionsSetEnv)
        self.copyCertForTC()
        self.installJarAndWarFiles()
        if self.isWindowsOs():
            self.installInstAsWinService()
        self.startSTSService()
        self.configureInfraNodeHomePage()

    def configureInfraNodeHomePage(self):
        deployType= "embedded"
        if self.canWaitForInstallParameter():
            #do not wait for this install parameter as it might be unset
            deployType = self.getInstallParameter(INSTALL_PARAMETER_DEPLOYMENT_TYPE, "embedded")

        self.Log("Deployment type:"+deployType)
        webappDir = os.path.join(self.__tc_sts_base, "webapps")
        if (deployType == "infrastructure"):
            self.Log("Configure ROOT index.html on infrastructure node")

            rootPagePath = os.path.join(webappDir, "ROOT", "index.html")
            rootFile = open(rootPagePath, "w")
            rootFile.write("<html>\n")
            rootFile.write("<head>\n")
            rootFile.write('<meta http-equiv=\"refresh\" content=\"0;URL=http://%s/websso/\">\n' % (self.__vmwarehostname))
            rootFile.write("</head>\n")
            rootFile.write("<body> </body>\n")
            rootFile.write("</html>\n")
            rootFile.close()
        else:
            #remove /websso/WEB-INF/views/index.jsp
            webssoInfDir = os.path.join(webappDir, "websso", "WEB-INF")
            indexViewPath = os.path.join(webssoInfDir, "views", "index.jsp")
            if os.path.isfile(indexViewPath):
                os.remove(indexViewPath)
            else:
                self.Log("/websso/WEB-INF/views/index.jsp does not exist. Skip deleting it.")

    def checkSTS(self, retryCount, retryInterval):

        self.Log("Checking Secure Token Server")

        command = '"%s" -cp %s %s %s com.vmware.identity.installer.STSInstaller \
            --check-sts --cert-path "%s" --retry-count %d --retry-interval %d' \
            % (self.__javaBin,
            self.__classpath,
            self.__sts_installer_log_option,
            self.__sts_installer_error_option,
            self.__ssl_root_cert_x509_path,
            retryCount,
            retryInterval)

        returncode = self.runShellCommand(command)
        if returncode != 0:
            raise Exception('Failed to initialize Secure Token Server.')


    def registerPorts(self, stsPort, stsLocalPort):
        self.registerPort('StsTcPort', stsPort)
        self.registerPort('StsLocalTcPort', stsLocalPort)

    def registerPort(self, portName, portValue):
        print 'Setting registry value %s = %s' % (portName, portValue)

        if self.isWindowsOs():
            keyConfig = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'Software\\VMware\\Identity\\Configuration', 0, _winreg.KEY_SET_VALUE)

            _winreg.SetValueEx(keyConfig, portName, 0, _winreg.REG_SZ, portValue)
        else:
            keyName='[HKEY_THIS_MACHINE\Software\Vmware\Identity\Configuration]'

            command = "%s set_value '%s' %s %s"  \
            % (self.__lw_reg_bin, keyName, portName, portValue)

            returncode = self.runShellCommand(command)
            if returncode != 0:
                raise Exception('Failed to set %s key' % portName)

    # This module sets up registry on the upgraded instance.
    def setupRegistryForUpgrade(self):

        self.Log( "Setup SPSytemDomainBackCompact in registry")
        self.doCompatibilityRegImport()
        return

    def doCompatibilityRegImport(self):

        self.Log("setup SPSytemDomainBackCompact in registry")

        command = '"%s" -cp "%s"%s%s -ea \
        com.vmware.identity.migration.UpgradeCompatibilityImporter \
        "%s"' \
        % (self.__javaBin,
        os.path.join(self.getSSOImportDir(), '*'),
        ';' if self.isWindowsOs() else ':',
        self.__classpath,
        os.path.join(self.getSSOImportDir(), 'upgrade_compatibility.properties'))
        command = command.encode('ascii')
        returncode = self.runShellCommand(command)
        if returncode != 0:
            raise Exception('Failed to import SSO1 system domain compatibility registry file.')


    # Create a registry key given key, sub key and its corresponding registry value.
    def createUpgradeRegistryKey(self,keyName,subKeyName,type,value):
        # Add the key to registry
        commandToRegister = "%s add_value '%s' %s %s %s" \
            % (self.__lw_reg_bin, keyName, subKeyName, type, value)
        returncode = self.runShellCommand(commandToRegister)
        if returncode != 0:
                raise Exception("Failed to add key: "+subKeyName )

    def publishSSOProperties(self):

        self.Log("Publishing SSO Properties")

        stsPort = self.getStsPort()

        propFilePath = os.path.join(self.getConfigFolderPath(), "sso.properties")

        propFilePath_tmp = "%s.tmp" % propFilePath

        self.Log("SSO Properties file [%s]" % propFilePath)

        #initial publish sso endpoints with directout reverse proxy
        command = '"%s" -cp %s %s %s com.vmware.identity.installer.STSInstaller \
            --cert-path "%s"  --port "%s" --publish-endpoints "%s"' \
            % (self.__javaBin,
             self.__classpath,
             self.__sts_installer_log_option,
             self.__sts_installer_error_option,
             self.__ssl_root_cert_x509_path,
             stsPort,
             propFilePath_tmp)

        returncode = self.runShellCommand(command)
        if returncode != 0:
            raise Exception('Failed to publish endpoints')

        #if reverse proxy is installed, update sso property file.
        #In upgrade mode, use migrated sso ssl cert to config reverse proxy cert

        if self.canWaitForInstallParameter():

            # Open STS port 7444 for backward compatibility with VC 5.5 and reload firewall settings
            self.openSTSPort()

            #ensure look up service backward compatibily for upgrade
            if self.isUpgradeMode():
                self.addCertToVECS(self.__ssl_root_cert_x509_path)

            rhttpProxyCert = self.waitForInstallParameter(INSTALL_PARAMETER_RHTTPPROXY_CERT)

            if rhttpProxyCert == "":
                self.Log("Warning:  Empty reverse proxy cert")

            else:
                # make CM firstboot happy
                self.Log("copying [%s] to [%s]" % (rhttpProxyCert, self.__leaf_cert_x509_path))
                shutil.copy2("%s" % (rhttpProxyCert), "%s" % (self.__leaf_cert_x509_path))

                command = '"%s" -cp %s %s %s com.vmware.identity.installer.STSInstaller \
                    --cert-path "%s" --update-cert "%s"' \
                    % (self.__javaBin,
                    self.__classpath,
                    self.__sts_installer_log_option,
                    self.__sts_installer_error_option,
                    self.__leaf_cert_x509_path,
                    propFilePath_tmp)

                returncode = self.runShellCommand(command)
                if returncode != 0:
                    raise Exception('Failed to update sso endpoints with reverse \
                    proxy certificate')
        #make sure the dest file does not exist when publishing
        if os.path.isfile(propFilePath):
            os.remove(propFilePath)
        os.rename(propFilePath_tmp, propFilePath)
        #make cm firstboot happy by cloning this in /etc/vmware-identity/sso.properties
        #TODO: coordinate with cm to switch to new location and remove this copy
        cm_expected_properties_path = self.defByOs( "/etc/vmware-identity", os.path.normpath( os.path.join(self.getConfigFolderPath(), "..", "vmware-identity") ) )
        self.Log("copying [%s] to [%s]" % (propFilePath, cm_expected_properties_path))

        if not os.path.exists(cm_expected_properties_path):
            os.makedirs(cm_expected_properties_path)

        shutil.copy2("%s" % (propFilePath), "%s" % (os.path.join(cm_expected_properties_path, "sso.properties")))

    # linux only
    # return true if opened the port
    def openSTSPort(self):
        if self.isWindowsOs():
            return False

        command = '/bin/ln -s -f "%s" "%s"' % (self.__linux_firewall_config_json, self.__linux_firewall_config)
        return_code = self.runShellCommand(command)
        if return_code != 0:
            raise Exception('Cannot create simbolic link %s, returncode=%d' % (self.__linux_firewall_config, return_code))

        command = '/usr/lib/applmgmt/networking/bin/firewall-reload'
        if os.path.exists(command):
            return_code = self.runShellCommand(command)
            if return_code != 0:
                raise Exception('Failed to run firewall reload %s' %(command))
            return True

        return False

    #Add cert to VECS
    #This is currently used for adding migrated ssl in during upgrade.
    def addCertToVECS(self, cert):
        from identity import vmafd
        with open (cert, "r") as certFile:
            certString=certFile.read()
            print certString
            vmafdclient = vmafd.client("localhost")
            vmafdclient.AddTrustedRoot(certString)
            print 'Added %s to Trusted Roots Store' % (cert)



    def doDataImport(self):
        command = '"%s" -cp "%s"%s%s -ea \
        com.vmware.identity.migration.ImporterToSSO2 \
        "%s"' \
        % (self.__javaBin,
        os.path.join(self.getSSOImportDir(), '*'),
        ';' if self.isWindowsOs() else ':',
        self.__classpath,
        os.path.join(self.getSSOImportDir(), 'exported_sso.properties'))

        command = command.encode('ascii')
        returncode = self.runShellCommand(command)
        if returncode != 0:
            raise Exception('Failed to import sso data')

    #lookup service import.
    def doLSImport(self):
        displayCommand = '"%s" -cp %s -ea -Dlog4j.configuration="file:%s" \
        com.vmware.vim.install.cli.SsoLsCli importServices \
        -d "https://%s:%s/lookupservice/sdk" -ip "%s" \
        -u "administrator@%s" -p "CENSORED"'\
        % (self.__javaBin, self.__classpath,
        os.path.join(self.getVMIdentityCommonLibPath(), 'ssolscli-log4j.xml'),
        self.__vmwarehostname, self.__sts_local_port,
        os.path.join(self.getSSOImportDir(), 'exported_sso.properties'),
        self.__vmdirDomainName)
        self.Log(displayCommand)

        command = '"%s" -cp %s -ea -Dlog4j.configuration="file:%s" \
        com.vmware.vim.install.cli.SsoLsCli importServices \
        -d "https://%s:%s/lookupservice/sdk" -ip "%s" \
        -u "administrator@%s" -p "%s"'\
        % (self.__javaBin, self.__classpath,
        os.path.join(self.getVMIdentityCommonLibPath(), 'ssolscli-log4j.xml'),
        self.__vmwarehostname, self.__sts_local_port,
        os.path.join(self.getSSOImportDir(), 'exported_sso.properties'),
        self.__vmdirDomainName,
        self.__vmdirPassword)

        command = command.encode('ascii')
        returncode = self.runShellCommand(command, True)
        if returncode != 0:
            raise Exception('Failed to import lookup service data from 5.x')

    def getStsPort(self):
        stsPort = self.__sts_local_port;
        if self.canWaitForInstallParameter():
            stsPortParam = self.waitForInstallParameter('rhttpproxy.ext.port2')
            if stsPortParam > 0:
               stsPort = stsPortParam;
        return stsPort

    def getStsCertPath(self):
        from identity import vmafd
        if os.path.exists(self.__machine_cert_path):
            os.remove(self.__machine_cert_path)
        vmafdclient = vmafd.client("localhost")
        cert = vmafdclient.GetMachineCert()
        open(self.__machine_cert_path, 'w').write(cert)
        return self.__machine_cert_path

    def configProcrunRegistry(self):

       self.Log("Setting up registry keys for procrun...")

       identityInstallPath = self.getVMIdentityInstallPath()
       keyIDM = _winreg.CreateKey(_winreg.HKEY_LOCAL_MACHINE, \
           'SOFTWARE\\Wow6432Node\\Apache Software Foundation\\Procrun 2.0\\%s\\Parameters' \
           % (self.__idm_servicename))

       #Add 'Java' key
       keyJava =  _winreg.CreateKey(keyIDM, 'Java')

       _winreg.SetValueEx(keyJava, "ClassPath", 0, _winreg.REG_SZ, '%s\\lib\\*;%s\\lib\ext\*;%s\\*;%s\\*'\
            %(self.getJavaHomePath(), self.getJavaHomePath(), identityInstallPath, self.getVMIdentityCommonLibPath()))
       _winreg.SetValueEx(keyJava, "JavaHome", 0, _winreg.REG_SZ, '%s\\' %(self.getJavaHomePath()))
       _winreg.SetValueEx(keyJava, "Jvm", 0, _winreg.REG_SZ, '%s\\bin\\server\\jvm.dll' %(self.getJavaHomePath()))

       #Define options for 'Java' key

       java_mem_args = self.getCloudVmMaxHeapAndPermSize("vmware-sts-idmd")
       javaMaxMemory = java_mem_args.get('heapsize',self.__default_sts_max_ram)
       javaMaxPerm = java_mem_args.get('permsize',self.__default_sts_max_perm)

       securityPolicyOption = '-Djava.security.policy=%s\\server_policy.txt' %(identityInstallPath)
       logDirOption = '-Dvmware.log.dir=%s' % (self.getLogsPath())
       jvmIdmErrorFile = os.path.join(self.getLogsPath(), 'hs_err_idm_pid%p.log')
       valJvmOptions = [securityPolicyOption, '-Xmx%s' % (javaMaxMemory), '-XX:MaxPermSize=%s' % (javaMaxPerm), '-XX:ErrorFile=%s' % (jvmIdmErrorFile), '-XX:HeapDumpPath=%s' % self.getLogsPath(), logDirOption]
       _winreg.SetValueEx(keyJava, "Options", 0, _winreg.REG_MULTI_SZ, valJvmOptions)

       #Add 'Log' key
       keyLog =  _winreg.CreateKey(keyIDM, 'Log')
       _winreg.SetValueEx(keyLog, "Path", 0, _winreg.REG_SZ, '%s' %(self.getLogsPath()))
       _winreg.SetValueEx(keyLog, "Prefix", 0, _winreg.REG_SZ, self.__idm_servicename)

       #Add 'Start' key
       keyStart =  _winreg.CreateKey(keyIDM, 'Start')
       _winreg.SetValueEx(keyStart, "Mode", 0, _winreg.REG_SZ, "jvm")
       _winreg.SetValueEx(keyStart, "Class", 0, _winreg.REG_SZ, "com.vmware.identity.idm.server.IdmServer")
       _winreg.SetValueEx(keyStart, "Method", 0, _winreg.REG_SZ, "startserver")

       #Add 'Stop' key
       keyStop =  _winreg.CreateKey(keyIDM, 'Stop')
       _winreg.SetValueEx(keyStop, "Mode", 0, _winreg.REG_SZ, "jvm")
       _winreg.SetValueEx(keyStop, "Class", 0, _winreg.REG_SZ, "com.vmware.identity.idm.server.IdmServer")
       _winreg.SetValueEx(keyStop, "Method", 0, _winreg.REG_SZ, "stopserver")
       _winreg.CloseKey(keyIDM);

    def runShellCommandTuple(self, command, quiet=False):
       if self.isWindowsOs():
           return self.runShellCmdArgsTuple(command, quiet)
       else:
           args = shlex.split(command)
           return self.runShellCmdArgsTuple(args, quiet)

    def runShellCommand(self, command, quiet=False):
       if self.isWindowsOs():
           return self.runShellCmdArgsRetCode(command, quiet)
       else:
           args = shlex.split(command)
           return self.runShellCmdArgsRetCode(args, quiet)

    def runShellCmdArgsTuple(self, args, quiet=False):
        ret, stdout, stderr = self.executeCommand(args, quiet)
        if (stderr.rstrip() != ""):
            self.Log(">>>>stderr:")
            self.Log("%s" % (stderr.rstrip()))
            self.Log("<<<<stderr")

        self.Log(">>>>stdout:")
        self.Log("%s" % (stdout.rstrip()))
        self.Log("<<<<stdout")

        self.Log("===Return code: %d" % (ret))
        return ret, stdout, stderr

    def runShellCmdArgsRetCode(self, args, quiet=False):
        ret, stdout, stderr = self.runShellCmdArgsTuple(args, quiet)
        return ret

    #return where webapp files was installed by installer.
    def getWebAppFilePath(self):

        if self.isWindowsOs():
            return os.path.join(self.getVMIdentityInstallPath(), "Tomcat", "webapps")
        else:
            return "/opt/vmware/webapps"

    def getTCTemplatePath(self):

        if self.isWindowsOs():
            #TODO currently templates is install under Tomcat\temp. We may want to move it
            #to "webAppFilePath" folder to be consistant with linux
            return os.path.join(self.getVMIdentityInstallPath(), "Tomcat", "temp")
        else:
            return self.getWebAppFilePath()

    def getVMIdentityInstallPath(self):
        #windows only concept. on linux, the lib files were installed by rpm directly to /opt/vmware/lib64

        import _winreg
        if self.isWindowsOs():
            key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'Software\\%s\\%s' %(self.__companyName, self.__ssoProductName))
            installPath = _winreg.QueryValueEx(key, "InstallPath")[0]
            return installPath
        else:
            return ""

    def getVMIdentityCommonLibPath(self):
        return os.path.join(self.getSSOHomePath(), "commonlib")

    def getCloudVmRamSize(self, svcName):
        javaMaxMemory = self.__default_sts_max_ram
        #if cloudvm ram size available
        if os.path.exists(self.__cloudvm_ram_size):
            command = '"%s" "%s"'  % (self.__cloudvm_ram_size, svcName)
            ret, stdout, stderr = self.runShellCommandTuple(command)
            if ret != 0:
                raise Exception("Failed to get cludvm size for %s : retcode=%d" %(svcName, ret))

            javaMaxMemory = stdout.rstrip()
            self.Log( "ram size: %s" % javaMaxMemory )
        else:
            javaMaxMemory = self.__default_sts_max_ram

        return javaMaxMemory

    def getCloudVmMaxHeapAndPermSize(self, svcName):
        jvm_memory_args = dict()
        if os.path.exists(self.__cloudvm_ram_size):
            command = '"%s" -J "%s"' % (self.__cloudvm_ram_size, svcName)
            (ret, stdout, stderr) = self.runShellCommandTuple(command)
            self.Log("JVM args from cloudvm-ram : %s" % stdout)
            if ret != 0:
                raise Exception('Failed to get JVM memory params  MaxHeap(Xmx) and PermSize(XX:MaxPermSize) for %s : retcode=%d'
                                 % (svcName, ret))

            # Extract the MaxheapSize value
            XmxTokenArray = stdout.split()
            for token in XmxTokenArray:
                if '-Xmx' in token:
                    jvm_memory_args['heapsize'] = token.split('Xmx')[1]
                    self.Log("Heap size : %s" % jvm_memory_args.get('heapsize'))
            # Extract the MaxPermSize value
                if '-XX:MaxPermSize' in token:
                    jvm_memory_args['permsize'] = \
                    token.split('-XX:MaxPermSize=')[1]
                    self.Log("Perm size : %s" % jvm_memory_args.get('permsize'))
        return jvm_memory_args

    def getClassPath(self):

        if self.isWindowsOs():
            installed_lib_files = os.path.join(self.getVMIdentityInstallPath(),'*')
        else:
            installed_lib_files = os.path.join(self.__vmware_libdir_lin,'*')

        common_lib_files = os.path.join(self.getVMIdentityCommonLibPath(),'*')
        if self.isWindowsOs():
            class_path = '"%s;%s;.;*"' % (installed_lib_files, common_lib_files)
        else:
            class_path = '%s:%s:.:*' % (installed_lib_files, common_lib_files)
        self.Log("Java class path for IDM:%s" % (class_path))
        return class_path

    def getSSOCertPath(self):
        return os.path.join(self.getConfigFolderPath(), "keys")


#Error Catalog Message Definitions
# To make the exception message localizable, throw exception in the following format
# raise Exception("<ErrorKey>|<ErrorMsg>")
# And define the  Error def and resolution in the following dictionary to be picked up by the Error framework. append 'Def' and 'Resolution'
# to ErrorKey to make it a dictionary key .
# Eg : raise Exception("checkSTS|STS failed")
#  Dictionary Entry:
# checkSTSDef = _T('<errorid>','<errormsg>')
# checkSTSResolution = _T('<errorresolutionid>','<resolution>')
errorDictionary = {
'doLduCleanupDef' : _T('install.vmidentity.doLduCleanup', 'Failed to update LDU structure in VMware Directory'),
'doLduCleanupResolution' : _T('install.vmidentity.doLduCleanup.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'checkDirectoryServiceDef' : _T('install.vmidentity.checkDirectoryService', 'Failed to check the status of VMware Directory Service.'),
'checkDirectoryServiceResolution' : _T('install.vmidentity.checkDirectoryService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'checkVmafdServiceDef' : _T('install.vmidentity.checkVmafdService', 'Failed to check VMware Authentication Framework Service.'),
'checkVmafdServiceResolution' : _T('install.vmidentity.checkVmafdService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'checkCertificateServiceDef' : _T('install.vmidentity.checkCertificateService', 'Failed to check VMware Certificate Service.'),
'checkCertificateServiceResolution' : _T('install.vmidentity.checkCertificateService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'startIDMServiceDef' : _T('install.vmidentity.startIDMService', 'Failed to start VMware Identity Management Service.'),
'startIDMServiceResolution' : _T('install.vmidentity.startIDMService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'startSTSServiceDef' : _T('install.vmidentity.startSTSService', 'Failed to start VMware Security Token Service.'),
'startSTSServiceResolution' : _T('install.vmidentity.startSTSService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'stopSTSServiceDef' : _T('install.vmidentity.stopSTSService', 'Failed to stop VMware Security Token Service.'),
'stopSTSServiceResolution' : _T('install.vmidentity.stopSTSService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'stopIDMServiceDef' : _T('install.vmidentity.stopIDMService', 'Failed to stop VMware Identity Management Service.'),
'stopIDMServiceResolution' : _T('install.vmidentity.stopIDMService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.'),
'uninstallSTSServiceDef' : _T('install.vmidentity.uninstallSTSService', 'Failed to uninstall VMware Security Token Service.'),
'uninstallSTSServiceResolution' : _T('install.vmidentity.uninstallSTSService.resolution','Please search of these symptoms in the VMware Knowledge Base for any known issues and possible workarounds. If none can be found, please collect a support bundle and open a support request.')
}
getCoreExecutionSettings().componentName = "vmidentity"
reporter = ProgressReporter(statusFile=getFBStatusReportInternalFile())

class VMIdentityInstallException(BaseInstallException):
    def __init__(self, msg, resolution = None, problemId = None):
        '''
        @param msg: a LocalizableMessage that describes what operation failed
        '''
        BaseInstallException.__init__(self, ErrorInfo([msg], resolution = resolution, problemId = problemId))

def main (argv):

    try:
        log("Begin VMware Identity Service Bootstrap")

        vmidentityFB = VMwareIdentityFirstBoot()
        if vmidentityFB.get_action() == FBActions.FIRSTBOOT:
            log("Processing VMware Identity Service bootstrap - FIRSTBOOT.")
            vmidentityFB.boot()
            log("Successfully processed VMware Identity Service bootstrap - FIRSTBOOT.")

        elif vmidentityFB.get_action() == FBActions.START:
            log("Processing VMware Identity Service bootstrap - START.")
            vmidentityFB.startIDMService()
            vmidentityFB.startSTSService()
            log("Successfully processed VMware Identity Service bootstrap - START.")

        elif vmidentityFB.get_action() == FBActions.STOP:
            log("Processing VMware Identity Service bootstrap - STOP.")
            vmidentityFB.stopSTSService()
            vmidentityFB.stopIDMService()
            log("Successfully processed VMware Identity Service bootstrap - STOP.")

        elif vmidentityFB.get_action() == FBActions.UNINSTALL:
            log("Processing VMware Identity Service bootstrap - UNINSTALL.")
            vmidentityFB.stopSTSService()
            vmidentityFB.stopIDMService()
            vmidentityFB.uninstallSTSService()
            log("Successfully processed VMware Identity Service bootstrap - UNINSTALL.")

        log("VMware Identity Service bootstrap completed successfully.")
        sys.exit(0)

    except Exception, e:
        log("VMware Identity Service bootstrap failed.")
        exceptionContent = str(e)
        tokens = exceptionContent.split('|')
        if len(tokens) == 2:
            exceptionID = tokens[0]
            exceptionDefKey = exceptionID + 'Def'
            exceptionResolutionKey = exceptionID + 'Resolution'
            if(errorDictionary.has_key(exceptionDefKey) and errorDictionary.has_key(exceptionResolutionKey)):
                msgDef = localizedString(errorDictionary[exceptionDefKey]);
                msgResolution = localizedString(errorDictionary[exceptionResolutionKey]);
                problemId = exceptionID
                raise VMIdentityInstallException(msgDef, msgResolution, problemId)
            else:
                log(exceptionContent);
        log("Exception: %s" % traceback.format_exc())
        ex = composeFBIntErr(traceback.format_exc())
        ex.componentKey = getCoreExecutionSettings().componentName
        reporter.failure(ex)

        sys.exit(1)

if __name__ == "__main__":

	main(sys.argv)
