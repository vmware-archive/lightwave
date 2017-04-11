package com.vmware.pscsetup;

import com.vmware.pscsetup.interop.DeployUtilsAdapter;
import com.vmware.identity.configure.IPlatformComponentInstaller;
import com.vmware.identity.configure.PlatformInstallComponent;
import com.vmware.identity.interop.Validate;

public class AuthenticationFrameworkInstaller implements
	IPlatformComponentInstaller {

    private static  final String ID ="vmware-authentication-framework";
    private static  final String Name ="VMware Authentication Framework";
    private static  final String Description ="VMware Directory service, VMware Certificate service, VMware Authentication framework";
    private DomainControllerStandaloneParams params;

    public AuthenticationFrameworkInstaller(
	    DomainControllerStandaloneParams params) {
	Validate.validateNotNull(params, "Domain Controller Params");
	this.params = params;
    }

    @Override
    public void install() throws Exception {
	if (params instanceof DomainControllerPartnerParams) {
	    DeployUtilsAdapter
		    .configurePartner((DomainControllerPartnerParams) params);
	} else {
	    DeployUtilsAdapter.configureStandalone(params);
	}
    }

    @Override
    public void upgrade() {
	// TODO Auto-generated method stub

    }

    @Override
    public void uninstall() {
	// TODO Auto-generated method stub

    }
    @Override
     public void migrate() {
 
     }
 


    @Override
    public PlatformInstallComponent getComponentInfo() {
	return new PlatformInstallComponent(ID, Name, Description);
    }
}
