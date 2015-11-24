# Lightwave UI tools for Mac OSX

Lightwave UI tools for Mac OSX are native apps built using Xamarin.Mac development platform from Xamarin.

##Lightwave CA.app
Lightwave CA is a native Mac OSX app to manage the Certificate Server in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Manage certificates
* Create and issue certificates
* View expiring certificates
* Generate keypairs/CSRs

##Lightwave CertStore.app
Lightwave CertStore is a native Mac OSX app to manage the endpoint certstore in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Create and manage certificate stores
* View stores information
* Create/Manage private/secret keys

##Lightwave Directory.app
Lightwave Directory is a native Mac OSX app to manage the ldap directory store in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* View directory data
* Manage directory data
* Browse ldap schema

##Lightwave SSO.app
Lightwave SSO is a native Mac OSX app to manage the SSO server in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Administer users and groups
* Integrate with OpenLDAP and AD
* Administer tenant policies and certificates
* Manage relying party and OIDC

## Source code
```
git clone https://github.com/vmware/lightwave.git
mac ui source files are under lightwave/tools/mac
```

## Build Environment
The source code is developed and tested against following environment:

* Mono 4.0.4
* XCode 6.4
* Xamarin mac 2.0.2
* Xamarin Studio 5.9.4.5

## Pre-built binaries
Pre-built binaries for Mac are available at the following location
https://vmware.bintray.com/lightwave_ui/ 

## Build
Note that you will require a Xamarin license to build Xamarin.Mac apps that link with 64bit libraries.
LightwaveUI Mac projects will require a Xamarin Indie or Business license.
Trial licenses can be created from the Xamarin Studio IDE.

If you have a trial license, you will have to build from the IDE using Xamarin Studio.

If you have a purchased license, you can use tools/mac/build.sh to build all apps.

Install the pre-built binaries (link above) to satisfy dependencies before attempting to build.


```
 cd lightwave/tools/mac
 ./build.sh
```

app files will be created under x64/Debug folder by default. If you need a Release build, use ./build.sh Release

##Installer
To create a MacOSX installer, use ./buildproduct.sh. This script will package all the apps from the x64/Debug folder into an installer which can be used to install on other machines. To package Release files, use ./buildproduct.sh Release 


##Known Issues

I. Move Lightwave Tools from user folder back to Applications folder.

Steps to move the Lightwave Tools apps back from a user folder back to
Application/LightwaveTools folder (typically useful only on developer
machines):

1. Install the LightwaveToolsInstaller.pkg package

2. Check the path where the app contained in this pkg are installed using
the following command:
tail -n 100 /var/log/install.log

NOTE: It should contain details as below:
Nov 18 20:41:15 LOCALMACHINE.local Installer[13136]: PackageKit:
Registered bundle file:///Applications/LightwaveTools/Lightwave%20CA.app/
for uid 93024
Nov 18 20:41:15 LOCALMACHINE.local Installer[13136]: PackageKit:
Registered bundle
file:///Applications/LightwaveTools/Lightwave%20Certificate%20Store.app/
for uid 93024
Nov 18 20:41:15 LOCALMACHINE.local Installer[13136]: PackageKit:
Registered bundle
file:///Applications/LightwaveTools/Lightwave%20Directory.app/ for uid
93024
Nov 18 20:41:15 LOCALMACHINE.local Installer[13136]: PackageKit:
Registered bundle file:///Applications/LightwaveTools/Lightwave%20SSO.app/
for uid 93024



3. If the paths in install.log in step 2. Are something other than above
then run the following steps:

	i. Run any of the following command based on the "culprit" app that is
not under LightwaveTools:
	   sudo pkgutil --forget com.vmware.LightwaveDirectory OR
	   sudo pkgutil --forget com.vmware.LightwaveSSO OR
	   sudo pkgutil --forget com.vmware.LightwaveCertStore OR
           sudo pkgutil --forget com.vmware.LightwaveCA

	ii. Go to the folder that the "culprit" app is unstalled under and move
that app to Trash.

	iii. Now re-run step 1.

4. Continue this until all the apps shown in Step 2 show the path as
Applications/LightwaveTools OR you see all the apps listed under
Applications UI.

5. You can now safely uninstall the LightwaveTools from Application (by
dragging the Application/LightwaveTools to trash) and re-install them
using step 1.


NOTE: Each time the developer builds or runs the code from Xamarin Studio,
the path of the above packages will be changed. So the developer needs to
do step 1-4 to use the installer post this.
