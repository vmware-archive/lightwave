package diag

type TenantID string

const (
	NoneTenantID TenantID = TenantID("")
)

func (tid TenantID) String() string { return string(tid) }
func (tid *TenantID) From(v string) Error {
	// todo nil check
	*tid = TenantID(v)
	return nil
}

func TenantIDFromString(val string) TenantID {
	if len(val) > 0 {
		return TenantID(val)
	}
	return NoneTenantID
}

type DiagContext interface {
	Tenant() TenantID
	RequestID() string
}
type RequestContext interface {
	DiagContext
	Logger() Logger
}

func NewRequestContext(tenant TenantID, requestID string, l Logger) RequestContext {

	ctx := &requestCtxtImpl{
		tenant: tenant,
		reqID:  requestID,
	}

	lgr := NewCtxAwareLogger(l, ctx)
	if lgr == nil {
		lgr = l
	}

	ctx.log = lgr

	return ctx
}

type requestCtxtImpl struct {
	tenant TenantID
	reqID  string
	log    Logger
}

func (c *requestCtxtImpl) Logger() Logger { return c.log }

func (c *requestCtxtImpl) Tenant() TenantID { return c.tenant }

func (c *requestCtxtImpl) RequestID() string { return c.reqID }
