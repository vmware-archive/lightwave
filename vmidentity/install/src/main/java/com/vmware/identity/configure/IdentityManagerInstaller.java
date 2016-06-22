package com.vmware.identity.configure;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class IdentityManagerInstaller implements IPlatformComponentInstaller {
    private static final String ID = "vmware-identity-manager";
    private static final String Name = "VMware Identity Manager";
    private static final String Description = "VMware Identity Manager";
    private static final Logger log = LoggerFactory
            .getLogger(IdentityManagerInstaller.class);

    private String hostnameURL = null;
    private boolean initialized = false;
    private String domainName;
    private String password;
    private String username;
    private boolean setReverseProxy = false;

    public IdentityManagerInstaller(String username, String domainName,

    String password, boolean setReverseProxy) {
        Validate.validateNotEmpty(username, "Username");
        Validate.validateNotEmpty(domainName, "Domain name");
        Validate.validateNotEmpty(password, "Password");

        this.domainName = domainName;
        this.password = password;
        this.username = username;
        this.setReverseProxy = setReverseProxy;
    }

    @Override
    public void install() throws Exception {
        initialize();
        log.info("Writing hostname file");

        writeHostnameFile();

        if(setReverseProxy){
		    log.info("Setting reverse proxy port");
		    setReverseProxyPort();
        }

        log.info("Configuring registry setting for IDM");

        InstallerUtils.getInstallerHelper().configRegistry();

        log.info("Configuring Identity Manager");
        
        Thread.sleep(20000);
        configureIDM();
    }

    @Override
    public void upgrade() {
        // TODO Auto-generated method stub

    }

    @Override
    public void uninstall() {
        // TODO Auto-generated method stub

    }

    private void initialize() {
        if (!initialized) {
            String hostnameURL = VmAfClientUtil.getHostnameURL();

            this.hostnameURL = hostnameURL;

            initialized = true;
        }
    }

    private void writeHostnameFile() throws IdentityManagerInstallerException {
        Validate.validateNotEmpty(hostnameURL, "Host name URL");

        HostnameWriter writer = new HostnameWriter(hostnameURL);
        try {
            writer.write();
        } catch (HostnameCreationFailedException e) {
            log.debug(e.getStackTrace().toString());
            throw new IdentityManagerInstallerException(
                    "Failed to create hostname file", e);
        }
    }

    private void setReverseProxyPort() {
        VmAfClientUtil.setReverseProxyPort(InstallerUtils.REVERSE_PROXY_PORT);
    }

    private void configureIDM() throws Exception {
        IdentityManagerUtil idmUtil = new IdentityManagerUtil(username,
                domainName, password);

        idmUtil.install();
    }

    @Override
    public PlatformInstallComponent getComponentInfo() {
        return new PlatformInstallComponent(ID, Name, Description);
    }

}
