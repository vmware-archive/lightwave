ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
    include \
    thirdparty \
    kdccommon \
    common \
    client

if ENABLE_SERVER

SUBDIRS += \
    server \
    tools \
    config \
    testing \
    post-manager

endif

install-posttemp:
	mkdir -p .postinstall/lwraft
	make DESTDIR=$(PWD)/.postinstall/lwraft install

tar-postinstall: install-posttemp
	cd .postinstall && tar -czvf lwraft.tar.gz lwraft
	-rm -rf .postinstall/lwraft

postrpm: tar-postinstall
	mkdir -p .postrpm/SOURCES
	mkdir -p .postrpm/BUILD
	mkdir -p .postrpm/RPMS/x86_64
	-rm -rf .postrpm/RPMS/*
	-rm -rf .postrpm/SOURCES/*
	mv .postinstall/lwraft.tar.gz .postrpm/SOURCES
	rpmbuild --quiet --nodeps --define="_topdir $(PWD)/.postrpm" \
	-bb $(top_srcdir)/lwraft/testing/make-test-rpm/lwraft-test.spec
	mv .postrpm/RPMS/x86_64/*.rpm .

clean-local:
	-rm -rf .postrpm
	-rm -rf .postinstall
	-rm ./lightwave-post*.rpm
