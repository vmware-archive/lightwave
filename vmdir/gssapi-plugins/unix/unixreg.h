DWORD
srp_reg_get_handle(
    void **pphRegistry
    );

VOID
srp_reg_close_handle(
    void *phRegistry
    );

DWORD
srp_reg_get_domain_state(
    void *hRegistry,
    PDWORD pdomainState);

DWORD
srp_reg_get_machine_acct_dn(
    void *hRegistry,
    PSTR *ppAccountDN);

DWORD
srp_reg_get_machine_acct_password(
    void *hRegistry,
    PSTR *ppMachPwd);

DWORD
srp_reg_get_machine_acct_upn(
    void *hRegistry,
    PSTR *ppAccountUpn);

DWORD
srp_reg_get_dc_name(
    void *hRegistry,
    PSTR *ppDcName);
