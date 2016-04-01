package com.vmware.pscsetup;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.configure.PlatformInstallObserverDefault;

public class PlatformServicesControllerTest {
    private static String hostname = "";
    private static String domainName = "vsphere.local";
    private static String password = "Ca$hc0w1";
    private static String site = "Bellevue";
    private static String server = "10.160.90.212";

    @Test
    public void testsetupInstanceStandalone() throws Exception {
	DomainControllerStandaloneParams standaloneParams = new DomainControllerStandaloneParams();
	standaloneParams.setHostname(hostname);
	standaloneParams.setDomainName(domainName);
	standaloneParams.setPassword(password);
	standaloneParams.setSite(site);
	try {
	    PlatformServicesController psc = new PlatformServicesController();
	    psc.setPlatformInstallObserver(new PlatformInstallObserverDefault());
	    psc.setupInstanceStandalone(standaloneParams);
	} catch (Exception e) {
	    Assert.fail(e.toString());
	}
    }

    // @Test
    // public void testsetupInstancePartner() {
    // DomainControllerPartnerParams partnerParams = new
    // DomainControllerPartnerParams();
    // partnerParams.setHostname(hostname);
    // partnerParams.setServer(server);
    // partnerParams.setPassword(password);
    // partnerParams.setDomainName(domainName);
    // partnerParams.setSite(site);
    // try {
    // PlatformServicesController psc = new PlatformServicesController();
    // psc.setPlatformInstallObserver(new PlatformInstallObserver());
    // psc.setupInstancePartner(partnerParams);
    // } catch (Exception e) {
    // Assert.fail(e.toString());
    // }
    // }

    // @Test
    // public void testGetPartnerDomain() {
    // try {
    // new PlatformServicesController().getPartnerDomain(server);
    // } catch (DomainControllerNativeException e) {
    // Assert.fail(e.toString());
    // }
    //
    // }
    //
    // @Test
    // public void testValidatePartnerCredentials() {
    // try {
    // new PlatformServicesController().validatePartnerCredentials(server,
    // password, domainName);
    // } catch (DomainControllerNativeException e) {
    // Assert.fail(e.toString());
    // }
    //
    // }
}
