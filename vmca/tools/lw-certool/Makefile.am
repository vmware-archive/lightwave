lwcertool_SCRIPTS = \
    @top_builddir@/vmca/tools/lw-certool/lw-certool

@top_builddir@/vmca/tools/lw-certool/lw-certool:
	CGO_CFLAGS="-I$(abs_top_srcdir)/vmafd/include/public" \
	CGO_LDFLAGS="-L$(abs_top_builddir)/vmafd/client/.libs" \
	go get github.com/vmware/lightwave/vmafd/interop/go/src/afdclient
	go get github.com/vmware/lightwave/vmidentity/goclients/oidc
	cd @top_srcdir@/vmca/tools/lw-certool && \
	go build -o @top_builddir@/build/vmca/tools/lw-certool/lw-certool -i
