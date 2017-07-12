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

## Download and Install GIT

Inorder to checkout source code (from github), Install git client from https://git-scm.com/

## Checkout Source code

```
git clone https://github.com/vmware/lightwave.git
```
The source code for windows tools are under lightwave/tools/win

## Download and Install .NET Framework

1. Download .NET from https://www.microsoft.com/en-in/download/details.aspx?id=30653
2. Confirm installation by following steps at https://msdn.microsoft.com/en-in/library/hh925568(v=vs.110).aspx#net_b

 Note : .NET Framework 3.5+ is recommended

## Download, Install and Configure WIX

1. Download WIX binary from http://wixtoolset.org/
   (Downloaded file : wix38-binaries.zip from http://wixtoolset.org/releases/v3.8/stable)
2. Extract wix38-binaries.zip (Downloaded from above step #1)
3. Configure environmental variable 'WIXPATH' set to value <downloaded_wix_38_path>/wix38-binaries

 Note : WIX 38 is recommended and ensured working.

## Download and Copy Pre-requisites for Build

1. Download pre-requisite binaries from https://vmware.bintray.com/lightwave_ui/v1.0/rc/for_developers/win/
2. Copy the above downloaded pre-requisites to lightwave/tools/interop/client_msi folder. [Please create folder structure if doesn't exist]

## Build

We can build the LIGHTWAVE UI tools via two methods :

1. via CLI (Command Line Interface)
2. via Visual Studio IDE (Integrated Dev Environment)

### via CLI

The process of building via CLI uses a pre-existing script(build-lw-win-ui.cmd) in source code.

```
1. cd lightwave/tools/win [Navigate to windows tools folder]
2. build-lw-win-ui.cmd [Invoke build command]
```

If the above command succeeds, It should generate the following files:

* VMIdentityTools_Installer.msi - Contain Lightwave-MMC tools + Pre-requisite client libraries
* VMIdentityTools_Prerequisite.exe - Contain only Pre-requisite client libraries
* VMIdentityTools_Standalone.exe - Contain Lightwave-MMC + Pre-requisite client libraries

#### Troubleshooting/Common Issues:
The installer logs can be found at lightwave\tools\win\logs folder.

WIX related errors :
On wix related errors e.g. light.exe error, then delete all obj folders from wininstaller projects and run script again.

### via Visual Studio

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

Modify DNS Server Settings

Open Network and Preferences (In Control Panel) → Network Icon→Network Settings→Change Adapter Options→Right click on Ethernet→Properties→Double click IPv4→Add the IP of Lightwave Instance.
Make sure you add secondary DNS server too (8.8.8.8 or 8.8.4.4 i.e google public DNS) to reach other www
