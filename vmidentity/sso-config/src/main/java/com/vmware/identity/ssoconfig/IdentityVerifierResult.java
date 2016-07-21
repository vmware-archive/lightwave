package com.vmware.identity.ssoconfig;

public class IdentityVerifierResult {
    private int total;
    private int failed;

    public IdentityVerifierResult(int total, int failed)
    {
        this.total = total;
        this.failed = failed;
    }

    public int getTotal()
    {
        return total;
    }

    public int getFailed()
    {
        return failed;
    }
}
