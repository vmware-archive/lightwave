/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : prototypes.h
 *
 * Abstract :
 *
 */

/**
 * @brief Joins the local system to the specified VMware directory domain
 *
 * @param[in]          pszDomain   Name of the domain to join
 * @param[in,optional] pszUsername Account to be used to perform the join.
 *                                 The "Administrator" account is used as the
 *                                 default.
 * @param[in]          pszPassword Password of the account used to perform the
 *                                 join.
 * @param[in,optional] pszOrgUnit  Organizational Unit to contain machine account
 */
DWORD
VmAfdJoinDomain(
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszOrgUnit
    );

/**
 * @brief Detaches the local system from the VMware directory domain it is
 *        joined to. If credentials are specified, the current machine
 *        account will be deleted. Otherwise, the current machine account will
 *        be disabled.
 *
 * @param[in,optional] pszUsername Account to be used to administer leaving the
 *                                 domain
 * @param[in,optional] pszPassword Password of the account specified as part of
 *                                 pszUsername
 * @param[in,optional] pbIsForce If domain leave force option is passed
 */

DWORD
VmAfdLeaveDomain(
    PCSTR pszUsername,
    PCSTR pszPassword,
    DWORD dwLeaveFlags
    );

/**
 * @brief Retrieves the domain name that the current system is joined to.
 *
 * @param[in,out] ppszDomain Name of the domain if the system is in the joined
 *                           state.
 *
 * @return Returns ERROR_NOT_JOINED if the system is not joined.
 *         Otherwise, returns ERROR_SUCCESS and the domain name.
 */
DWORD
VmAfdGetJoinStatus(
    PSTR*    ppszDomain,
    PBOOLEAN pbIsDC
    );

