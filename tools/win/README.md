# Lightwave UI tools for Windows

Lightwave UI tools for Windows are MMC snapins built using Microsoft Visual Studio 2013 development platform.
You can download Visual Studio from https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx


##Lightwave CA MMC snapin
Lightwave CA is a MMC snapin to manage the Certificate Server in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Manage certificates
* Create and issue certificates
* View expiring certificates
* Generate keypairs/CSRs


##Lightwave CertStore MMC snapin
Lightwave CertStore is a MMC snapin to manage the endpoint certstore in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Create and manage certificate stores
* View stores information
* Create/Manage private/secret keys


##Lightwave Directory MMC snapin
Lightwave Directory is a  MMC snapin to manage the ldap directory store in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Browse directory
* Manage directory objects
* Search directory objects


##Lightwave SSO MMC snapin
Lightwave SSO is a MMC snapin to manage the SSO server in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Administer users and groups
* Integrate with OpenLDAP and AD
* Administer tenant policies and certificates
* Manage relying party and OIDC


##Lightwave PSC Site Management MMC snapin
Lightwave PSC Site Management is a MMC snapin to monitor the high availability of PSC and VC nodes
in a lightwave topology.
It lets you connect to a remote VC and do the following tasks

* Monitor the health of the sites and topology
* Change the mode of the VCs to legacy and high availability
* Displays Domain Functional Level (DFL) for the topology
* Monitors the updates done to the topology

##Lightwave Directory Schema MMC snapin
Lightwave Directory Schema is a  MMC snapin to manage the ldap directory schema in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Browse directory schema
* Manage/Edit schema
* Compare Federation and Replication metadata


## Build Environment
The source code is developed and tested against following environment:

* .NET framework 4.5
* Visual Studio 2013
  - .NET framework 3.5 may also require in case of community edition
* Wix v3.8 (stable)


## Source code
```
git clone https://github.com/vmware/lightwave.git
windows source files are under lightwave/tools/win
```



## Build

The code can be build either using the build script or manually using Visual Studio.

###Pre-requisite client binaries
Download client pre-built binaries (from the link below) and copy them to tools/interop/client_msi folder (create if it does not exists) to satisfy dependencies before attempting to build.:

https://vmware.bintray.com/lightwave_ui/v1.0/rc/for_developers/win/

####Using build script
```
Perform following steps to build the Lightwave UI installer for windows using the build script:

* Download and install .net framework 4.5 (in case it is not present on your machine) from https://www.microsoft.com/en-in/download/details.aspx?id=30653. To confirm whether .net framework 4.5 is installed in your machine please use  this link https://msdn.microsoft.com/en-in/library/hh925568(v=vs.110).aspx#net_b .
* Download and install git client from https://git-scm.com/
* Open git bash shell and run the command: git clone https://github.com/vmware/lightwave.git
* Download wix38-binaries.zip from http://wixtoolset.org/. Do not download any wix installer binaries.
* Right click wix38-binaries.zip and open properties panel and click unblock button present at bottom.
* Now, extract wix38-binaries.zip binaries.
* Create WIXPATH environment variable with value <path to folder where wix binaries are extracted>\wix38-binaries
* Go to lightwave windows folder using command prompt: cd lightwave\tools\win
* Run the windows build script: build-lw-win-ui.cmd

```
 
This will generate following three files in tools\win\x64\Debug and tools\win\x64\Release folders.

1) VMIdentityTools_Installer.msi - contains only Lightwave MMC tools and requires pre-requisite client libraries to be already installed on machine. 

2) VMIdentityTools_Prerequisite.exe - does not contains Lightwave MMC tools and used to install pre-requisite client libraries on machine

3) VMIdentityTools_Standalone.exe - contains both pre-requisite libs + Lightwave MMC tools

You can find installer log in win\logs folder.

If you get any wix related errors e.g. light.exe error, then delete all obj folders from wininstaller projects and run script again.

####Using Visual Studio

To build the tools indiviudally using Visual Studio, you need to build the pre-requisite interops first.

If you have opened the .sln file for a tool, you would need to close it before you perform these steps.
Perform the following steps before you open the solution for a tool.

There are 3 pre-requisite interop projects that you need to build. 

These are placed at:

\lightwave\vmafd\dotnet\VMAFD.Client\VMAFD.Client.sln

\lightwave\vmdir\dotnet\VMDIR.Client\VMDIR.Client.sln

\lightwave\vmdir\interop\csharp\VmDirInterop\VmDirInterop.sln  - Build only VMDirInterop project

The artifacts for the above projects can be found under \bin\Debug or \bin\Release folders 
under respective (VMAFD.Client, VMDIR.Client or VmDirInterop) project folder.

The above project would generate following 6 artifacts:

VMAFD.Client.dll
VMAFD.Client.dll.config
VMDIR.Client.dll
VMDIR.Client.dll.config
VmDirInterop.dll
VmDirInterop.dll.config

Now, copy the above artifacts to lightwave/tools/interop/lib64 folder.
You may need to create the above folder if one does not exist.

Make a copy of configuration file Brand_lw.config present inside tools\common\VMIdentity.CommonUtils project and rename it to "VMIdentity.CommonUtils.dll.config" (You can use below command)
```
copy /Y tools\common\VMIdentity.CommonUtils\Brand_lw.config tools\common\VMIdentity.CommonUtils\VMIdentity.CommonUtils.dll.config
```
Now, you can open the solution files for the individual tool and build it using visual Studio.

Following are the location of the .sln files of the tools:

Lightwave CA tool: \lightwave\tools\win\VMCASnapIn\VMCASnapIn.sln

Lightwave Directory tool: \lightwave\tools\win\VMDirSnapIn\VMDirSnapIn.sln

Lightwave Certificate tool: \lightwave\tools\win\VMCertStoreSnapIn\VMCertStoreSnapIn.sln

Lightwave SSO tool: \lightwave\tools\win\VMRestSsoAdminSnapIn\RestSsoAdminSnapIn.sln

Lightwave PSC Site Management tool: \lightwave\tools\win\VMPscHighAvailabilitySnapIn\VMPscHighAvailabilitySnapIn.sln

Lightwave Directory Schema tool: \lightwave\tools\win\VMDirSchemaSnapIn\VMDirSchemaSnapIn.sln

The assembly files will be created under tools\win\x64\Debug foler by default.

##Installer

* You need to download and install wix38.exe from http://wixtoolset.org/ otherwise you will get project not supported error.

* You need to download wix38-binaries.zip and configure WIXPATH as mentioned in build script steps previously (if not done already)

* Now, build wininstaller solution present at lightwave\tools\win\wininstaller.
This will generate three installers as mentioned before in the tools\win\x64\Debug folder by default, which can be used to install on other machines. 




## Known Issues

```
*Installer
	1. Only administrator users are allowed to install tools.

* Lightwave REST SSO Tool : 
	1. Tool doesn't work with the latest super-main (TSL enabled) vSphere builds.

* Lightwave PSC Site Management Tool :  
	1. Tool does not support partial topology load.
	2. Tool does not show PSC status as UNKNOWN when Heartbeat API throws error.

* Lightwave Directory Schema Tool :  
	1. Many attribute types are showing syntax as System.String in Right Pane
	2. UI tools allows attributes to be created for 34 different attribute syntax at present.

```

## How To

I. PSC Site Management UI tool does not login to the MXN topology once a topology is deployed.

Edit the hosts file on all the nodes of the topology and the machine running UI tool as follows:
 <IP>	<FQDN> 	<HOSTNAME>
	 
example:
190.160.1.2	contoso.vmware.com	photon-contoso

Add entry for all the nodes in the hosts file

For linux, hosts file is located under: /etc/hosts
For windows, hosts file is located under: C:\Windows\System32\drivers\etc\hosts