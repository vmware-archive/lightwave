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
import os
import sys
import subprocess
import _winreg
import ctypes
from ctypes import wintypes, windll
import socket
import shutil
import time



class VMwareCertificateIntallTest:
	def __init__(self):
		self.__installerPath = "c:\\Users\\Anu Engineer\\Downloads\\vmware-certificate.msi"

	def Install(self):
		command = "msiexec /i \"%s\" /q" %(self.__installerPath)
		print command
		p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		output = p.communicate()[0].rstrip()
		print output
		if p.returncode != 0:
			raise Exception('Command Execution Failure [%s] Error : %d '% (command, p.returncode))

	def Uninstall(self):
		print "Running Uninstall"
		command = 'msiexec /x \"%s\" /qn' %(self.__installerPath)
		p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		output = p.communicate()[0].rstrip()
		if p.returncode != 0:
		    raise Exception('Command Execution Failure [%s] Error : %d '% (command, p.returncode))
		print output
		shutil.rmtree("c:\\vmca")

	def RunTest(self):
		print "Running Install"
		self.Install();
		print "Running first Boot"
		p = subprocess.Popen("python vmca-firstboot.py", stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		output = p.communicate()[0].rstrip()
		print output		
		print "Running Uninstall"
		self.Uninstall();


if __name__ == "__main__":
	for i in range(0,300):
		test = VMwareCertificateIntallTest()
		test.RunTest()


