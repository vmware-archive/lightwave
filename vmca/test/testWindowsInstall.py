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


