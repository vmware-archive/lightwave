#!/usr/bin/env python
#
# Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the “License”); you may not
# use this file except in compliance with the License.  You may obtain a copy
# of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an “AS IS” BASIS, without
# warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
# Helper function that gets certificates from VMWare Certificate Authority
# More details.  If this module can be used as a main program, include usage information.


""" certool.py : This is the standard library function for
    cloudVM/vcenterwindows first boot to integrate with
    VMCA Certificate Generation.

    if not running under a cloudVM, then it is assumed that
    the OS.Environment has the following defined.

    VMWARE_SKIP_VISL = True
    system.urlhostname
    vmdir.ldu-guid
    system.hostname.type
    vmca.cert.password
    vmca.cert.dir

    """

__copyright__ = "Copyright 2012, VMware Inc."
__version__ = 0.1
__author__ = "VMware, Inc."

import logging
import os
import subprocess


class CerTool:
    __vislInstall__ = ""
    __systemUrlHostname__ = ""
    __systemHosttype__ = ""
    __vmcaPassword__ = ""
    __vmcaCertPath__ = ""
    __skipInstallParams__ = False
    __certfileName__ = ""
    __privateKeyFileName__ = ""
    __publicKeyFileName__ = ""
    __pfxFileName__ = ""

    def __init__(self):
        self.FindEnvParams()
        self.GetVislParams()

    def GetHostName(self):
        return self.__systemUrlHostname__

    def GetHostType(self):
        return self.__systemHosttype__

    def GetPassword(self):
        return self.__vmcaPassword__

    def GetCertDir(self):
        return self.__vmcaCertPath__

    def GetCertFileName(self):
        return self.__certfileName__

    def GetPrivateKeyFileName(self):
        return self.__privateKeyFile__

    def GetPublicKeyFileName(self):
        return self.__publicKeyFile__

    def GetPfxFileName(self):
        return self.__pfxFileName__

    def GenCert(self, componentName):
        """ Generates the Certificates in the Cert directory"""
        # Generate full file names for all artifacts
        self.__certfileName__ = \
            os.path.join(self.GetCertDir(), componentName, componentName + ".crt")
        logging.debug("cert File Name : " + self.GetCertFileName())

        self.__privateKeyFile__ = \
            os.path.join(self.GetCertDir(), componentName, componentName + ".priv")
        logging.debug("Private Key Name : " + self.GetPrivateKeyFileName())

        self.__publicKeyFile__ = \
            os.path.join(self.GetCertDir(), componentName, componentName + ".pub")
        logging.debug("Public Key Name : " + self.GetPublicKeyFileName())

        self.__pfxFileName__ = \
            os.path.join(self.GetCertDir(), componentName, componentName + ".pfx")
        logging.debug("pfx file Name : " + self.GetPfxFileName())

        dir = os.path.join(self.GetCertDir(),componentName)
	logging.debug("Target Dir : " + dir)
        try:
            if not os.path.exists(dir):
                os.makedirs(dir)
                logging.debug("Created directory")
        except OSError as e:
            raise Exception("I/O error({0}): {1}".format(e.errno, e.strerror))


        # Generate Private Key and Public Keys First
        cmd = [self.GetCertToolPath(),
            '--genkey',
            '--priv=' + self.GetPrivateKeyFileName(),
            '--pub=' + self.GetPublicKeyFileName()]

        output = self.RunCmd(cmd)
        logging.info(output)

        cmd = [self.GetCertToolPath(),
            '--genCIScert',
            '--priv=' + self.GetPrivateKeyFileName(),
            '--cert=' + self.GetCertFileName(),
            '--Name=' + componentName]

        # if we know the host name, put that into the certificate
        if (self.GetHostType() == 'fqdn'):
            cmd.append('--FQDN=' + self.GetHostName())
        # elif (self.GetHostType() == 'ipv4'):
        #     # Possible TODO : support IPv4 in certificates
        # elif (self.GetHostType() == 'ipv6'):
        #     # Possible TODO : support IPv6 in certificates

        output = self.RunCmd(cmd)
        logging.info(output)

        # TODO : Replace this with certool PKCS12 capabilities

        cmd = [self.GetOpenSSLPath(),
            'pkcs12',
            '-export',
            '-in',
            self.GetCertFileName(),
            '-inkey',
            self.GetPrivateKeyFileName(),
            '-out',
            self.GetPfxFileName(),
            '-name',
            componentName,
            '-passout',
            'pass:' + self.GetPassword()]
        output = self.RunCmd(cmd)
        logging.info(output)

    def FindEnvParams(self):
        """ Finds the Default Environment parameters. if you are
        not running inside the cloudVM, set VMWARE_SKIP_VISL = True
        in your environment. This will enable this script to look
        for values in the env. block instead of VISL namespace."""
    # Find VISL Install Parameter
        INSTALL_PARAM_ENV_VAR = 'VMWARE_INSTALL_PARAMETER'
        VMWARE_SKIP_VISL = 'VMWARE_SKIP_VISL'

        if INSTALL_PARAM_ENV_VAR in os.environ:
            self.__vislInstall__ = os.environ[INSTALL_PARAM_ENV_VAR]

        if VMWARE_SKIP_VISL in os.environ:
            skip = os.environ[VMWARE_SKIP_VISL]
            if (skip in ['true', 'True', 'yes', '1', 'skip']):
                self.__skipInstallParams__ = True

        if (not self.__vislInstall__ and self.__skipInstallParams__ is False):
            errString = 'Unable to find install param script'
            logging.error(errString)
            raise Exception(errString)
        logging.debug('Using install param script : ' + self.__vislInstall__)

    def GetInstallParams(self, key):
        """ Waits on Install Parameter to return the value from visl.
        Or if the VMWARE_SKIP_VISL = True, then reads the value from
        the os environment"""
        if (self.__skipInstallParams__ is False):
            cmd = [self.__vislInstall__, '-d', key]
            output = self.RunCmd(cmd)
            logging.debug('Install param found :' + output)
            return output
        else:
            if val in os.environ:
                param = os.environ[key]
                logging.debug('Env. param found : ' + param)
                return param
            else:
                raise Exception('Requested Value not found in Env : ' + key)

    def RunCmd(self, args):
        """ Runs a given command"""
        logging.info('running %s' % args)
        p = subprocess.Popen(args,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
        if p.returncode:
            raise Exception('Failed to execute last cmd')
        else:
            return p.communicate()[0].rstrip()

    def GetVislParams(self):
        """ Waits for all VISL parameters that VMCA certool needs"""
        INSTALL_PARAM_SYSTEM_URL_HOSTNAME = "system.urlhostname"
        INSTALL_PARAM_LDU_GUID = "vmdir.ldu-guid"
        INSTALL_PARAM_SYSTEM_HOST_TYPE = "system.hostname.type"
        INSTALL_PARAM_PASSWORD = "vmca.cert.password"
        INSTALL_PARAM_CERT_DIR = "vmca.cert.dir"

        # Please note that each of this is a blocking call.
        # VISL will wait until these value are populated by the
        # appropriate Script
        self.__systemUrlHostname__ = \
            self.GetInstallParams(INSTALL_PARAM_SYSTEM_URL_HOSTNAME)
        self.__systemHosttype__ = \
            self.GetInstallParams(INSTALL_PARAM_SYSTEM_HOST_TYPE)
        self.__vmcaPassword__ = \
            self.GetInstallParams(INSTALL_PARAM_PASSWORD)
        self.__vmcaCertPath__ = \
            self.GetInstallParams(INSTALL_PARAM_CERT_DIR)

        # We really don't need this value,
        # it is a technique on waiting for directory
        # first boot to finish.

        discardldu = self.GetInstallParams(INSTALL_PARAM_LDU_GUID)

    def GetCertToolPath(self):
        """returns the path to certool"""
        #TODO : Publish Certool Path from VMCA First Boot
        if(os.name == "nt"):
            PROGRAM_FILES = os.environ['PROGRAMFILES']
            return os.path.normpath(PROGRAM_FILES +
                    '/VMware/CIS/Vmcad/certool.exe')
        elif (os.name == 'posix'):
            return '/opt/vmware/bin/certool'

    def GetOpenSSLPath(self):
        if(os.name == "nt"):
            PROGRAM_FILES = os.environ['PROGRAMFILES']
            return os.path.normpath(PROGRAM_FILES +
                    '/VMware/CIS/OpenSSL/openssl.exe')
        elif (os.name == 'posix'):
            return '/usr/lib/vmware-openSSL/openssl'



def main():
    """ Example Code Usage """
    testComponent = 'sso'
    VmcaCertool = CerTool()
    VmcaCertool.GenCert(testComponent)
    print 'Generated a pfx file : %s' % VmcaCertool.GetPfxFileName()
    print 'Using Password : %s' % VmcaCertool.GetPassword()


if __name__ == "__main__":
    main()
