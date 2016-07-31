#Lightwave Admin UI

Lightwave Admin UI is HTML 5 based web interface to administer Lightwave identity management.
It lets you perform the following tasks:
* Provision users, solution users, groups and group membership
* Integrate with external identity sources like Active Directory and Open LDAP
* Configure policies for token, password, lockout, authentication and banner
* Update signer identity certificate chain
* Add a new tenant and setup the signer identity for it
* View the list of service providers like relying party and external identity providers


##Build Environment

To build the source code you need the following environment:

* Photon Operating System (1.0 GA)
* Git to clone the source code
* Run make commmand in the top level directory
* For any missing packages use: $tdnf install <package name>
	
Lightwave Admin UI artifact is a .war file lightwaveui.war.

lightwaveui.war is deployed as a part of RPM vmware-sts-6.6.0-0.x86_64.rpm using command:

/opt/vmware/bin/configure-lightwave-server --domain <domain> --password <pwd> --hostname <ip-address>


##Source code
```
git clone https://github.com/vmware/lightwave.git
Lightwave Admin UI files are under /vmidentity/lightwaveui

```

##Technology stack

The Lightwave Admin UI is built on the following technology stack:
* Java 8 Servlets (placed under: /vmidentity/lightwaveui/src)
* Angular JS 1.5 + Bootstrap 3.3.6 (placed under: /vmidentity/lightwaveui/web)


##Known Issues

None

