package com.vmware.vim.sso.admin;

public class AuthenticationAccountInfo {

    private final String userName;
    private final String spn;
    private final boolean useMachineAccount;

    public AuthenticationAccountInfo(String userName, String spn, Boolean useMachineAccount) {

        this.userName = userName;
        this.spn = spn;
        this.useMachineAccount = useMachineAccount;
    }

    public String getUserName() {
        return this.userName;
    }

    public String getSpn() {
        return this.spn;
    }

    public boolean isUseMachineAccount()
    {
        return this.useMachineAccount;
    }
}
