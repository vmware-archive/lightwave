# Lightwave UI tools for Windows

Lightwave UI tools for Windows are MMC snapins built using Microsoft Visual Studio 2013 development platform.
You can download latest of Visual Studio from https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx


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

* View directory data
* Manage directory data
* Browse ldap schema


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


## Source code
```
git clone https://github.com/vmware/lightwave.git
windows source files are under lightwave/tools/win
```

## Build Environment
The source code is developed and tested against following environment:

* .NET framework 4.5
* Visual Studio 2013 (Minimum)
* Wix v3.8 (stable)


## Pre-built binaries
Pre-built binaries for Windows are available at the following location
https://vmware.bintray.com/lightwave_ui/ 


## Build & Installer
Note that you will require a Visual Studio 2013 license to build these MMC snapin projects that link with 64bit libraries.

Install the pre-built binaries (from the link below) to satisfy dependencies before attempting to build.
https://vmware.bintray.com/lightwave_ui/


```
Perform following steps to build the Lightwave UI installer for windows using the build script:

 * Download and install .net framework 4.5 (in case it is not present on your machine) from https://www.microsoft.com/en-in/download/details.aspx?id=30653
 * Download and extract wix v3.8 (stable) binaries from http://wixtoolset.org/
 * Run the command to set the WIXPATH environment variable: set WIXPATH= <path to folder where wix binaries are extracted>\ wix38-binaries
 * Download and install git client from https://git-scm.com/
 * Open git bash shell and run the command: git clone https://github.com/vmware/lightwave.git
 * Go to lightwave windows folder: cd lightwave\tools\win
 * Run the windows build script: build-lw-win-ui.cmd
 
```

Lightwave UI windows installer msi will be genrated in win\x64\Debug and win\x64\Release folders.


## Known Issues

```

* Lightwave REST SSO Tool : 

	1. Cannot search the solution user by certificate since some special characters are not supported. 
	2. The tools allows user to add expired certificate for STS signing.


* Lightwave Directory Tool : 

	1. To refresh the tree node just collapse and expand it. The right-click refresh option does not refresh the tree or attributes table.


* Lightwave PSC Site Management Tool :  

	1. No support for partial topology load.
	2. The UI refresh cycle is set to 60 seconds. There would be 2 minute (worst case) lag between the server state and its reflection on the UI.

```
