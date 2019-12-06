package param

import "github.com/vmware/lightwave/sts/internal/pkg/diag"

type Command interface {
	Name() string
	ShortDescription() string
	Parse(args []string) error
	LogLvl() int32
	LogLocation() string
	RegisterParams()
	Process(ctxt diag.RequestContext) error
}
