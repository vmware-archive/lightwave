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

####Browsers Supported
* Chrome 39
* Mozilla Firefox 34
* Safari 8
* Microsoft Edge 13 (on Windows 10 onwards)

##Known Issues

* Does not work on Internet Explorer browser
* Improvement - Need a UI to add a user to multiple groups from Users tab
* Token auto-refresh after expiration leads to UI refresh to Home page
* Web Socket Exception is seen in browser console on Windows when user navigates to login page
* Some places error message is missing from the error banner
* Branding Policy needs all the text and Display checkbox checked before Save to enable the banner
* Branding Policy needs all the text removed and  Display checkbox un-checked before Save to disable the banner

## How To

Q: Where can I find the Lightwave UI tools?

Use the following link to download Lightwave UI tools:
https://vmware.bintray.com/lightwave_ui/v1.0/ga/



Q: How do I make Lightwave web UI to work with non-default STS port?

To make Lightwave UI work with a non-defualt STS port, you need to update the OIDC client configuration 
for Lightwave web UI using Lightwave UI tools.

Once you download and install the Lightwave UI tools, follow these steps:
1. Login to UI Tool using the server, port, username and password.
2. On successful login, expand the tree on the left pane and navigate to OIDC client node of the tree.
3. Choose (right-click for Windows MMC snapin) the OIDC entry which you need to update. In case of multple 
   entries see the clientId and/or logout URI for correct server.
4. The OIDC URIs are configured using the following format for Lightwave web UI :
	logout URI: https:<hostname>:<custom-port>/lightwaveui
	redirect URI: https:<hostname>:<custom-port>/lightwaveui/Home
	post logout URI: https:<hostname>:<custom-port>/lightwaveui
	NOTE:
	* Here, custom-port is the port on which the STS is setup.
	* In case you have multiple dockers hosted on the same machine and have configured any routing or reverse proxy 
	for it, the custom-port will be the externally visible port configured on the host for the docker.
5. For non-default STS port, you need to update the <custom-port> in Step 4 with the non-default STS port. 



Q: How do I make Lightwave web UI to work with a load balancer?

For Lightwave UI to work with the load balancer you need to make following 2 changes:
1. Make the client Id in lightwave-ui-oidc.xml same for all the load balanced servers. The xml is located at: 
	/opt/vmware/share/config/lightwave-ui-oidc.xml
2. Download and Install Lightwave UI tool. Now, login to the UI Tool, navigate to the OIDC client entry for 
   the client Id used in Step 1 and update URI for it as follows:
	Update the logout URI to: https:<load-balancer>:<lb-port>/lightwaveui
	Update the redirect URI to: https:<load-balancer>:<lb-port>/lightwaveui/Home
	Update the post logout URI to: https:<load-balancer>:<lb-port>/lightwaveui

NOTE:
	If you wish to access Lightwave Web UI using both the load balancer as well as individual servers URI, add 
	redirect URI and post logout URI for each of the individual servers in step 2 in addiiton to the load-balancer 
	URIs.



Q: I am not able to connect to Lightwave server 1.0.1 using Lightwave UI tools?

Lightwave UI tools do not work with the TLS 1.1,1.2 enabled builds which are setup by default for Lightwave server 
1.0.1+ builds.

To make the UI tools work, login to the Lightwave server and do the following:

1. Please enable TLSv1 by editing /opt/vmware/vmware-sts/conf/server.xml
From:
<Connector SSLEnabled="true"
...
sslEnabledProtocols="TLSv1.1,TLSv1.2"
To:
sslEnabledProtocols="TLSv1,TLSv1.1,TLSv1.2"

2. Restart the STS service using: systemctl restart vmware-stsd

3. You should be able to connect to the Lightwave server after step 2. You can revert the changes in step 1 and 
   restart STS service once you are done updating the OIDC client configuration.
