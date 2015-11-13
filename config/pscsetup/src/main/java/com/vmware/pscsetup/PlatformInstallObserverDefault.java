package com.vmware.pscsetup;

import java.util.List;

public class PlatformInstallObserverDefault implements IPlatformInstallObserver {

    @Override
    public void beginInstall(List<PlatformInstallComponent> components) {
	System.out.println("Begin installing components: ");
	for (PlatformInstallComponent info : components)
	{
	    System.out.println(info.getName());
	}
    }

    @Override
    public void beginComponentInstall(String component) {
	 System.out.println("Begin installing component: "+ component);

    }

    @Override
    public void componentInstallationProgress(String component,
	    int percentCompleted) {
	// TODO Auto-generated method stub
    }

    @Override
    public void endComponentInstall(String component, boolean status) {
	System.out.println("End installing component " + component + " "
		+ status);
    }

    @Override
    public void endInstall(boolean status) {
	System.out.println("End install: "+ status);
    }

}
