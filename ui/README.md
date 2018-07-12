The web based by UI used by lightwave is separated into 2 RPMS. The UI RPMs are configured to on top of nginx server. Both lightwave UI & lwraft UI can be installed on the same host or on different host. They are both available on HTTPS default port 443 at different paths.

1. LightwaveUI
	Lightwave UI includes both UI for Identity management and browsing lightwave directory. Lightwave UI is available at the path /lightwaveui. /lightwaveui/idm sub-path opens the identity manager while lightwaveui/directory opens the lightwave LDAP directory.
2. LWraftUI
	This is the web UI for browsing the directory of a POST node. The lwraft UI is available at /lwraftui.

How to Build ?
Use the below command to build the 2 above mentioned UI RPMs.
	"./build_photon.sh —only-ui"
The UI rpms would be copied to the build/rpmbuild/RPMS/x86_64/

How to Deploy Lightwave UI?

1. Make sure that the firewall port 443 is open
2. Domain join the lightwave UI node/instance to the domain controller (this UI is going to be used for)
3. Install the ssl certificates at /etc/vmware/ssl for serving HTTPS protocol
	- This is scriptized using the VECS cli commands and available in "setup_certs_for_ui.sh"
4. Install lightwave-ui rpm file (it has a rpm level dependency on nginx,jq)
5. Use the provided /opt/vmware/tools/oidc-client-utils-lightwaveui to register lightwave UI instance as OIDC client on any lightwave server (provided admin creds are available).
	- Alternately we could use cascade-tools/deployment/lightwave/ scripts to register OIDC client.
6. Update the /opt/vmware/lightwaveui/config/lightwaveui.json with the OIDC client id for the server/tenant combination.

How to Deploy Lwraft UI ?
The steps 1 through 3 are the same as deploying the lightwave UI and need not be repeated if installing lwraft UI on the same node as lightwave UI.

4. Install lwraft-ui rpm.
5. Use the provided /opt/vmware/tools/oidc-client-utils-lwraftui to register lightwave UI instance as OIDC client on any lightwave server (provided admin creds are available).
	- Alternately we could use cascade-tools/deployment/lightwave/ scripts to register OIDC client.
6. Update the /opt/vmware/lightwaveui/config/lwraftui.json with the OIDC client id for the server/tenant combination.
