/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


int
LightwaveDomain(
    int argc,
    char* argv[]
    );

int
LightwaveDns(
    int argc,
    char* argv[]
    );

int
LightwaveCa(
    int argc,
    char* argv[]
    );

/* ca/ca.c */
int
LightwaveCaGetSignedCert(
    int argc,
    char* argv[]
    );

/* domain/domain.c */
int
LightwaveDomainPromote(
    int argc,
    char* argv[]
    );

int
LightwaveDomainDemote(
    int argc,
    char* argv[]
    );

int
LightwaveDomainJoin(
    int argc,
    char* argv[]
    );

int
LightwaveDomainLeave(
    int argc,
    char* argv[]
    );

/* dns/dns.c */
int
LightwaveDnsDelete(
    int argc,
    char* argv[]
    );

int
LightwaveInitDNS(
    int argc,
    char* argv[]
    );

int
LightwaveDCDnsDelete(
    int argc,
    char* argv[]);
