SRCROOT := ../
MAKEROOT=$(SRCROOT)/support/make
include $(MAKEROOT)/makedefs.mk

MKDIR=/bin/mkdir
RM=/bin/rm
CP=/bin/cp
LN=/bin/ln
RPMBUILD=/usr/bin/rpmbuild

RPMBUILD_ROOT=$(CURDIR)/rpmbuild
RPMBUILD_BUILD=$(RPMBUILD_ROOT)/BUILD
RPMBUILD_SPECS=$(RPMBUILD_ROOT)/SPECS
RPMBUILD_RPMS=$(RPMBUILD_ROOT)/RPMS
RPMBUILD_SOURCES=$(RPMBUILD_ROOT)/SOURCES
RPMBUILD_SRPMS=$(RPMBUILD_ROOT)/SRPMS
RPMBUILD_TMP=$(RPMBUILD_ROOT)/tmp

RPMBUILD_DIRS= \
    $(RPMBUILD_BUILD) \
    $(RPMBUILD_SPECS) \
    $(RPMBUILD_RPMS)  \
    $(RPMBUILD_SOURCES) \
    $(RPMBUILD_SRPMS) \
    $(RPMBUILD_TMP)

SRCROOT=..

CLEAN_OBJECTS = \
    rpmbuild

PKG_SPEC=vmware-lightwave-clients.spec

.PHONY: all package

all: package

package: $(RPMBUILD_SPECS)/$(PKG_SPEC) | $(RPMBUILD_DIRS)
	cd $(RPMBUILD_ROOT) && \
	$(RPMBUILD) \
              -ba \
              --define "_topdir $(RPMBUILD_ROOT)" \
              --define "_prefix /opt/vmware" \
              --define "_bindir /opt/vmware/bin" \
              --define "_sbindir /opt/vmware/sbin" \
              --define "_lib64dir /opt/vmware/lib64" \
              --define "_version $(LW_CLIENTS_MAJOR_VER).$(LW_CLIENTS_MINOR_VER).$(LW_CLIENTS_RELEASE_VER)" \
              --define "_patch $(LW_CLIENTS_PATCH_VER)" \
	      --buildroot $(RPMBUILD_ROOT)/BUILDROOT \
              SPECS/$(PKG_SPEC)

$(RPMBUILD_SPECS)/$(PKG_SPEC) : $(CURDIR)/package/rpm/$(PKG_SPEC) | $(RPMBUILD_SPECS)
	@$(CP) -f $< $@

$(RPMBUILD_BUILD): $(realpath $(SRCROOT)) | $(RPMBUILD_ROOT)
	@$(LN) -s $< $@

$(RPMBUILD_SPECS):
	@$(MKDIR) -p $@

$(RPMBUILD_RPMS):
	@$(MKDIR) -p $@

$(RPMBUILD_SOURCES):
	@$(MKDIR) -p $@

$(RPMBUILD_SRPMS):
	@$(MKDIR) -p $@

$(RPMBUILD_TMP):
	@$(MKDIR) -p $@

$(RPMBUILD_ROOT):
	@$(MKDIR) -p $@

clean:
	@$(RM) -rf $(CLEAN_OBJECTS)

