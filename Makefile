#
# Copyright 2015 VMware, Inc
#

SRCROOT := .
MAKEROOT=$(SRCROOT)/support/make
include $(MAKEROOT)/makedefs.mk

PACKAGES=\
    $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_SERVER_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_DEVEL_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_SERVER_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_DEVEL_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMCA_SERVER_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_DEVEL_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(CFG_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(LW_SERVER_RPM) \
    $(LIGHTWAVE_STAGE_DIR)/$(LW_CLIENTS_RPM)

all: $(PACKAGES)

$(LIGHTWAVE_STAGE_DIR)/$(LW_SERVER_RPM): $(LW_SERVER_PKGDIR)/$(LW_SERVER_RPM)
	$(CP) -f $< $@

$(LW_SERVER_PKGDIR)/$(LW_SERVER_RPM):
	@cd $(SRCROOT)/lw-server && make

lw-server-clean:
	@cd $(SRCROOT)/lw-server && make clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    cd $(LIGHTWAVE_STAGE_DIR) && $(RM) -f $(LW_SERVER_RPM); \
	fi

$(LIGHTWAVE_STAGE_DIR)/$(LW_CLIENTS_RPM): $(LW_CLIENTS_PKGDIR)/$(LW_CLIENTS_RPM)
	$(CP) -f $< $@

$(LW_CLIENTS_PKGDIR)/$(LW_CLIENTS_RPM):
	@cd $(SRCROOT)/lw-clients && make

lw-clients-clean:
	@cd $(SRCROOT)/lw-clients && make clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    cd $(LIGHTWAVE_STAGE_DIR) && $(RM) -f $(LW_CLIENTS_RPM); \
	fi

vmdir-client-install: $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_RPM) $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_DEVEL_RPM)
	$(RPM) -Uvh --force $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_RPM) $(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_DEVEL_RPM)

$(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_RPM):$(VMDIR_PKGDIR)/$(VMDIR_CLIENT_RPM)
	$(CP) -f $< $@

$(LIGHTWAVE_STAGE_DIR)/$(VMDIR_CLIENT_DEVEL_RPM):$(VMDIR_PKGDIR)/$(VMDIR_CLIENT_DEVEL_RPM)
	$(CP) -f $< $@

$(LIGHTWAVE_STAGE_DIR)/$(VMDIR_SERVER_RPM):$(VMDIR_PKGDIR)/$(VMDIR_SERVER_RPM)
	$(CP) -f $< $@

$(VMDIR_PKGDIR)/$(VMDIR_CLIENT_RPM):$(VMDIR_PKGDIR)/$(VMDIR_SERVER_RPM)

$(VMDIR_PKGDIR)/$(VMDIR_CLIENT_DEVEL_RPM):$(VMDIR_PKGDIR)/$(VMDIR_SERVER_RPM)

$(VMDIR_PKGDIR)/$(VMDIR_SERVER_RPM):$(LIGHTWAVE_STAGE_DIR)
	@cd $(SRCROOT)/vmdir/build && make -f Makefile.bootstrap

vmdir-clean:
	@cd $(SRCROOT)/vmdir/build && make -f Makefile.bootstrap clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    cd $(LIGHTWAVE_STAGE_DIR) && $(RM) -f $(VMDIR_RPMS); \
	fi

vmafd-client-install: $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_RPM) $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_DEVEL_RPM)
	$(RPM) -Uvh --force $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_RPM) $(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_DEVEL_RPM)

$(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_RPM):$(VMAFD_PKGDIR)/$(VMAFD_CLIENT_RPM)
	$(CP) -f $< $@

$(LIGHTWAVE_STAGE_DIR)/$(VMAFD_CLIENT_DEVEL_RPM):$(VMAFD_PKGDIR)/$(VMAFD_CLIENT_DEVEL_RPM)
	$(CP) -f $< $@

$(LIGHTWAVE_STAGE_DIR)/$(VMAFD_SERVER_RPM):$(VMAFD_PKGDIR)/$(VMAFD_SERVER_RPM)
	$(CP) -f $< $@

$(VMAFD_PKGDIR)/$(VMAFD_CLIENT_RPM) : $(VMAFD_PKGDIR)/$(VMAFD_SERVER_RPM)

$(VMAFD_PKGDIR)/$(VMAFD_CLIENT_DEVEL_RPM) : $(VMAFD_PKGDIR)/$(VMAFD_SERVER_RPM)

$(VMAFD_PKGDIR)/$(VMAFD_SERVER_RPM): $(LIGHTWAVE_STAGE_DIR) vmdir-client-install
	@cd $(SRCROOT)/vmafd/build && make -f Makefile.bootstrap

vmafd-clean:
	@cd $(SRCROOT)/vmafd/build && make -f Makefile.bootstrap clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    cd $(LIGHTWAVE_STAGE_DIR) && $(RM) -f $(VMAFD_RPMS); \
	fi

vmca-client-install: $(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_RPM) $(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_DEVEL_RPM)
	$(RPM) -Uvh --force $(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_RPM) $(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_DEVEL_RPM)

$(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_RPM):$(VMCA_PKGDIR)/$(VMCA_CLIENT_RPM)
	$(CP) -f $< $@

$(LIGHTWAVE_STAGE_DIR)/$(VMCA_CLIENT_DEVEL_RPM):$(VMCA_PKGDIR)/$(VMCA_CLIENT_DEVEL_RPM)
	$(CP) -f $< $@

$(LIGHTWAVE_STAGE_DIR)/$(VMCA_SERVER_RPM):$(VMCA_PKGDIR)/$(VMCA_SERVER_RPM)
	$(CP) -f $< $@

$(VMCA_PKGDIR)/$(VMCA_CLIENT_RPM):$(VMCA_PKGDIR)/$(VMCA_SERVER_RPM)

$(VMCA_PKGDIR)/$(VMCA_CLIENT_DEVEL_RPM):$(VMCA_PKGDIR)/$(VMCA_SERVER_RPM)

$(VMCA_PKGDIR)/$(VMCA_SERVER_RPM): $(LIGHTWAVE_STAGE_DIR) vmafd-client-install
	@cd $(SRCROOT)/vmca/build && make -f Makefile.bootstrap

vmca-clean:
	@cd $(SRCROOT)/vmca/build && make -f Makefile.bootstrap clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    cd $(LIGHTWAVE_STAGE_DIR) && $(RM) -f $(VMCA_RPMS); \
	fi

$(LIGHTWAVE_STAGE_DIR)/$(CFG_RPM) : $(CFG_PKGDIR)/$(CFG_RPM)
	$(CP) -f $< $@

$(CFG_PKGDIR)/$(CFG_RPM): $(LIGHTWAVE_STAGE_DIR) vmca-client-install
	@cd $(SRCROOT)/config/build && make -f Makefile.bootstrap

config-clean:
	@cd $(SRCROOT)/config/build && make -f Makefile.bootstrap clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    cd $(LIGHTWAVE_STAGE_DIR) && $(RM) -f $(CFG_RPM); \
	fi

clean: config-clean vmca-clean vmafd-clean vmdir-clean lw-server-clean lw-clients-clean
	@if [ -d $(LIGHTWAVE_STAGE_DIR) ]; then \
	    $(RMDIR) $(LIGHTWAVE_STAGE_DIR); \
	fi

$(LIGHTWAVE_STAGE_DIR):
	@$(MKDIR) -p $@
