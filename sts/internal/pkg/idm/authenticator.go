package idm

import (
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

type Authenticator interface {
	Authenticate(tenant diag.TenantID, creds types.Credentials, ctxt diag.RequestContext) (types.UserID, diag.Error)
	GetUserAttributes(tenant diag.TenantID, ctxt diag.RequestContext,
		user types.UserID, attributes ...types.AttributeID) (types.AttributeValues, diag.Error)

	//LookupActiveUserByIdentity(tenant diag.TenantID, user string, ctxt diag.RequestContext) (types.UserID, diag.Error)
}
