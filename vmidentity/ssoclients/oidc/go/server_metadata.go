package oidc

/*
#include <stdlib.h>
#include "ssotypes.h"
#include "ssoerrors.h"
#include "oidc_types.h"
#include "oidc.h"
*/
import "C"

import "runtime"

type ServerMetadata struct {
    p C.POIDC_SERVER_METADATA
}

func ServerMetadataAcquire(
        server string,
        portNumber int,
        tenant string) (result *ServerMetadata, err error) {
    serverCStr := goStringToCString(server)
    tenantCStr := goStringToCString(tenant)

    defer freeCString(serverCStr)
    defer freeCString(tenantCStr)

    var p C.POIDC_SERVER_METADATA = nil
    var e C.SSOERROR = C.OidcServerMetadataAcquire(
        &p,
        serverCStr,
        C.int(portNumber),
        tenantCStr)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    result = &ServerMetadata { p }
    runtime.SetFinalizer(result, serverMetadataFinalize)
    return
}

func serverMetadataFinalize(this *ServerMetadata) {
    this.Close()
}

func (this *ServerMetadata) Close() error {
    if (this.p != nil) {
        C.OidcServerMetadataDelete(this.p)
        this.p = nil
    }
    return nil
}

func (this *ServerMetadata) GetTokenEndpointUrl() string {
    return cStringToGoString(C.OidcServerMetadataGetTokenEndpointUrl(this.p))
}

func (this *ServerMetadata) GetSigningCertificatePEM() string {
    return cStringToGoString(C.OidcServerMetadataGetSigningCertificatePEM(this.p))
}
