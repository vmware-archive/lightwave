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

* Browse directory
* Manage directory objects
* Search directory objects

##Lightwave SSO.app
Lightwave SSO is a native Mac OSX app to manage the SSO server in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Administer users and groups
* Integrate with OpenLDAP and AD
* Administer tenant policies and certificates
* Manage relying party and OIDC

##Lightwave PSC Site Management.app
Lightwave PSC Site Management is a native Mac OSX app to monitor the high availability of PSC and 
VC nodes in a lightwave topology.
It lets you connect to a remote VC and do the following tasks

* Monitor the health of the sites and topology
* Change the mode of the VCs to legacy and high availability
* Displays Domain Functional Level (DFL) for the topology
* Monitors the updates done to the topology

##Lightwave Directory Schema.app
Lightwave Directory Schema is a native Mac OSX app to manage the ldap directory schema in a lightwave instance.
It lets you connect to a remote server and do the following tasks

* Browse directory schema
* Manage/Edit schema
* Compare Federation and Replication metadata

##Build Environment
The source code is developed and tested against following environment:

* Mono 4.4.2
* Xamarin mac 2.8.2
* XCode 7.3.1
* Xamarin Studio 6.0.2 (https://www.xamarin.com/)

##Source code
```
git clone https://github.com/vmware/lightwave.git
mac ui source files are under lightwave/tools/mac
```

##Build

The code can be build either using the build script or manually using Xamarin Studio.

###Pre-requisite client binaries
Install the client pre-built binaries (from the link below) to satisfy dependencies before attempting to run tools after building.:

https://vmware.bintray.com/lightwave_ui/v1.0/rc/for_developers/mac


####Using build script
```
 cd lightwave/tools/mac
 sh build.sh
```

app files will be created under tools/mac/x64/Debug folder by default. If you need a Release build, use "sh build.sh Release" command.

####Using Xamarin Studio

To build the tools indiviudally using Xamarin Studio, you need to build the pre-requisite interops first.

If you have opened the .sln file for a tool, you would need to close it before you perform these steps.
Perform the following steps before you open the solution for a tool.

There are 3 pre-requisite interop projects that you need to build.

These are placed at:

/lightwave/vmafd/dotnet/VMAFD.Client/VMAFD.Client.sln

/lightwave/vmdir/dotnet/VMDIR.Client/VMDIR.Client.sln

/lightwave/vmdir/interop/csharp/VmDirInterop/VmDirInterop.sln - Building only VMDirInterop project inside solution is sufficient (change project target framework to 4.0).

Other projects in VmDirInterop solution are not required and if you want to build them then their target framework also requires to be changed to 4.0

The artifacts for the above projects can be found under /bin/Debug or /bin/Release folders 
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

Make a copy of configuration file Brand_lw.config present inside tools/common/VMIdentity.CommonUtils project and rename it to "VMIdentity.CommonUtils.dll.config" (You can use below command)
```
cp tools/common/VMIdentity.CommonUtils/Brand_lw.config tools/common/VMIdentity.CommonUtils/VMIdentity.CommonUtils.dll.config
```

Now, you can open the solution files for the individual tool and build it using Xamarin Studio.

Following are the location of the .sln files of the tools:

Lightwave CA tool: \lightwave\tools\mac\VMCASnapIn\LightwaveCA.sln

Lightwave Directory tool: \lightwave\tools\mac\VMDirSnapIn\Lightwave Directory.sln

Lightwave Certificate tool: \lightwave\tools\mac\VMCertStoreSnapIn\LightwaveCertStore.sln

Lightwave SSO tool: \lightwave\tools\mac\VMRestSsoSnapIn\Lightwave SSO.sln

Lightwave PSC Site Management tool: \lightwave\tools\mac\VMPSCHighAvailabilitySnapIn\Lightwave PSC Site Management.sln

Lightwave Directory Schema tool: \lightwave\tools\mac\VMDirSchemaEditorSnapIn\VMDirSchemaEditorSnapIn.sln

The app files will be created under tools/mac/x64/Debug foler by default.

##Installer
To create a MacOSX installer, use ./buildproduct.sh. This script will package all the apps 
from the x64/Debug folder into an installer which can be used to install on other machines. 
To package Release files, use ./buildproduct.sh Release


##Cleanup
In case you wish to cleanup the libs and links added by the Lightwave UI tool suite, use ./cleanup.sh


##Known Issues

```

* Lightwave REST SSO Tool :

	1. Tool doesn't work with the TLS 1.1,1.2 enabled builds. To make it to work,
	   please enable TLSv1 by editing /opt/vmware/vmware-sts/conf/server.xml

		From:
		       <Connector SSLEnabled="true"
		 …
		        sslEnabledProtocols="TLSv1.1,TLSv1.2"

		To:
		        sslEnabledProtocols="TLSv1,TLSv1.1,TLSv1.2"

		and restart the STS service

		        systemctl restart vmware-stsd
	2. External identity provider gives error 400 bad request.

* Lightwave PSC Site Management Tool :
	1. Tool does not support partial topology load.
	2. Tool does not show PSC status as UNKNOWN when Heartbeat API throws error.
	3. Multiple pop up thrown, stating "null argument" when vmafdd service is brought down.

* Lightwave Directory Schema Tool :
	1. Many attribute types are showing syntax as System.String in Right Pane
	2. UI tools allows attributes to be created for 34 different attribute syntax at present.
```

##How To

I. Move Lightwave Tools from user folder back to Applications folder.

Steps to move the Lightwave Tools apps back from a user folder back to
Application/LightwaveTools folder (typically useful only on developer
machines):

1. Install the LightwaveToolsInstaller.pkg package

2. Check the path where the app contained in this pkg are installed using
the following command:
tail -n 100 /var/log/install.log

NOTE: It should contain details as below:
```
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
Nov 18 20:41:15 LOCALMACHINE.local Installer[13136]: PackageKit:
Registered bundle file:///Applications/LightwaveTools/Lightwave%20Directory%20Schema.app/
for uid 93024
```


3. If the paths in install.log in step 2. Are something other than above
then run the following steps:

	i. Run any of the following command based on the "culprit" app that is
not under LightwaveTools:
	   sudo pkgutil --forget com.vmware.LightwaveDirectory OR
	   sudo pkgutil --forget com.vmware.LightwaveSSO OR
	   sudo pkgutil --forget com.vmware.LightwaveCertStore OR
       sudo pkgutil --forget com.vmware.LightwaveCA OR
       sudo pkgutil --forget com.vmware.LightwaveDirectorySchema

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


II. Access Denied error when you build a tool.

Example:
/Library/Frameworks/Mono.framework/Versions/4.2.3/lib/mono/4.5/Microsoft.Common.targets: error : Access to the path "/Users/<username>/lightwave/tools/common/VMDir.Common/obj/Debug/VMDir.Common.csproj.FilesWrittenAbsolute.txt" is denied.

The most probable cause for this error is that the folder path is inaccessible.
You can resolve it using sudo chmod 777 <tools folder path>
You need to perform this each time you are prompted this error typically after a cleanup.


III. PSC Site Management UI tool does not login to the MXN topology once a topology is deployed.

Edit the hosts file on all the nodes of the topology and the machine running UI tool as follows:
 <IP>	<FQDN> 	<HOSTNAME>

example:
190.160.1.2	contoso.vmware.com	photon-contoso

Add entry for all the nodes in the hosts file

For linux, hosts file is located under: /etc/hosts
For windows, hosts file is located under: C:\Windows\System32\drivers\etc\hosts

