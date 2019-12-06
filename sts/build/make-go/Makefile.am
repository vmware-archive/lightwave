all: golang

export GOPATH=@abs_top_builddir@/sts/go
export GO111MODULE=on
export GOMOD=@abs_top_srcdir@/sts/go.mod
export GOOS=@go_os@
export GOARCH=@go_arch@

golang:
	@echo "Building sts" && \
	mkdir -p ${GOPATH} && \
	mkdir -p ${GOPATH}/src/github.com/vmware/lightwave && \
	cd ${GOPATH}/src/github.com/vmware/lightwave && \
	ln -sfv @abs_top_srcdir@/sts ${GOPATH}/src/github.com/vmware/lightwave/sts && \
	cd @abs_top_srcdir@/sts/internal/pkg/idm/gen && go generate && \
	cd @abs_top_srcdir@/sts/internal/pkg/oidc/gen && go generate && \
	cd @abs_top_srcdir@/sts/web/static/gen && go generate && \
	cd @abs_top_srcdir@/sts/cmd/stssetup/gen && go generate && \
	cd @abs_top_srcdir@/sts && \
	go build -mod=readonly -o @abs_top_builddir@/sts/cmd/stssrv/@go_os@/@go_arch@/stssrv@go_exe@ ./cmd/stssrv && \
	go build -mod=readonly -o @abs_top_builddir@/sts/cmd/stssetup/@go_os@/@go_arch@/stssetup@go_exe@ ./cmd/stssetup

clean-local: clean-local-check

.PHONY: clean-local-check

clean-local-check:
	@echo "cleaning sts" && \
	cd @abs_top_srcdir@/sts && \
	rm -f ${GOPATH}/src/github.com/vmware/lightwave/sts && \
	rm -f @abs_top_builddir@/sts/cmd/stssrv/@go_os@/@go_arch@/stssrv@go_exe@ && \
	rm -f @abs_top_builddir@/sts/cmd/stssetup/@go_os@/@go_arch@/stssetup@go_exe@ && \
	rm -v -f $(find . -name '*_gen.go') && \
	go clean -x -cache -testcache -modcache ./...

distclean-local: distclean-local-check

.PHONY: distclean-local-check

distclean-local-check:
	rm -f @top_srcdir@/sts/internal/pkg/config/buildinfo_gen.go
