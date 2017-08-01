#
# Copyright 2017 VMware, Inc
#

SRCROOT = @top_srcdir@
BUILDROOT = @abs_top_builddir@
STAGEDIR = $(BUILDROOT)/stage
MAKEROOT=$(SRCROOT)/support/make
include $(MAKEROOT)/makedefs.mk

SUBDIRS = \
        vmmetrics \
        vmevent \
        vmidentity/ssoclients \
        lwraft \
        vmdir \
        vmdns \
        vmafd \
        vmca \
        vmidentity \
        config

PACKAGES=\
    $(LIGHTWAVE_STAGE_DIR)/x86_64/$(LIGHTWAVE_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/x86_64/$(LIGHTWAVE_CLIENT_RPM)

RPMBUILD=/usr/bin/rpmbuild

RPMBUILD_ROOT=$(CURDIR)/rpmbuild
RPMBUILD_BUILD=$(RPMBUILD_ROOT)/BUILD
RPMBUILD_SPECS=$(RPMBUILD_ROOT)/SPECS
RPMBUILD_RPMS=$(RPMBUILD_ROOT)/RPMS
RPMBUILD_SRCS=$(RPMBUILD_ROOT)/SOURCES
RPMBUILD_SRPMS=$(RPMBUILD_ROOT)/SRPMS
RPMBUILD_TMP=$(RPMBUILD_ROOT)/tmp

RPMBUILD_DIRS= \
    $(RPMBUILD_BUILD) \
    $(RPMBUILD_SPECS) \
    $(RPMBUILD_RPMS)  \
    $(RPMBUILD_SRCS) \
    $(RPMBUILD_SRPMS) \
    $(RPMBUILD_TMP)

PKG_SPEC=lightwave.spec

.PHONY: package stage

stage:
	make -C $(BUILDROOT) DESTDIR=$(STAGEDIR) install

package: stage $(RPMBUILD_SPECS)/$(PKG_SPEC) | $(RPMBUILD_DIRS)
	cd $(RPMBUILD_ROOT) && \
	$(RPMBUILD) \
              -v \
              --bb \
              --noclean \
              --define "_topdir $(RPMBUILD_ROOT)" \
              --define "_prefix /opt/vmware" \
              --define "_bindir /opt/vmware/bin" \
              --define "_sbindir /opt/vmware/sbin" \
              --define "_lib64dir /opt/vmware/lib64" \
              --define "_datadir /opt/vmware/share" \
              --define "_logdir $(LIGHTWAVE_LOGDIR)" \
              --define "_includedir /opt/vmware/include" \
              --define "_localstatedir /var/lib/vmware" \
              --define "_likewise_open_prefix /opt/likewise" \
              --define "_vmevent_prefix /opt/vmware" \
              --define "_vmevent_ver $(VMEVENT_MAJOR_VER).$(VMEVENT_MINOR_VER)" \
              --define "_vmdir_prefix /opt/vmware" \
              --define "_vmafd_prefix /opt/vmware" \
              --define "_vmca_prefix /opt/vmware" \
              --define "_vmdns_prefix /opt/vmware" \
              --define "_vmsts_prefix /opt/vmware" \
              --define "_javahome $(JAVA_HOME)" \
              --define "_anthome $(ANT_HOME)" \
              --define "_version $(LW_SERVER_MAJOR_VER).$(LW_SERVER_MINOR_VER).$(LW_SERVER_RELEASE_VER)" \
              --define "_patch $(LW_SERVER_PATCH_VER)" \
              --buildroot $(STAGEDIR) \
              SPECS/$(PKG_SPEC)

$(RPMBUILD_SPECS)/$(PKG_SPEC) : $(CURDIR)/package/rpm/$(PKG_SPEC) | $(RPMBUILD_SPECS)
	@$(CP) -f $< $@

$(RPMBUILD_DIRS):
	@$(MKDIR) -p $@

$(RPMBUILD_SPECS):
	@$(MKDIR) -p $@

$(RPMBUILD_RPMS):
	@$(MKDIR) -p $@

$(RPMBUILD_SRCS):
	@$(MKDIR) -p $@

$(RPMBUILD_SRPMS):
	@$(MKDIR) -p $@

$(RPMBUILD_TMP):
	@$(MKDIR) -p $@

$(RPMBUILD_ROOT):
	@$(MKDIR) -p $@
