#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: top level Makefile
# Abstract: VMware Domain Name Service.
# Created on: Sep 18, 2012
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

#
# This Makefile is used for both gobuild and local builds
#

GOBUILD_AUTO_COMPONENTS = 1

GOBUILD_TARGET ?= rd-domain-name-server

ifeq ($(OS),Windows_NT)
  #
  # Windows...
  # This Makefile is used by gobuild harness
  #

  SRCROOT := ..
  MAKEROOT=$(SRCROOT)/support/make
  include $(MAKEROOT)/makedefs.mk

  $(info SRCROOT: [$(SRCROOT)])
  $(info BUILDROOT: [$(BUILDROOT)])
  $(info PUBLISH_DIR: [$(PUBLISH_DIR)])
  $(info TCROOT: [$(TCROOT)])

  all: gobuild-deps-copy
	win_build.cmd 
	$(CP) build.log $(BUILDLOG_DIR)
	$(MKDIR) -p $(PUBLISH_DIR)/vmdns_msms/Debug
	$(MKDIR) -p $(PUBLISH_DIR)/vmdns_msms/Release
	$(MKDIR) -p $(PUBLISH_DIR)/vmdns_setup/Debug
	$(MKDIR) -p $(PUBLISH_DIR)/vmdns_setup/Release
	$(MKDIR) -p $(PUBLISH_DIR)/win64/Debug
	$(MKDIR) -p $(PUBLISH_DIR)/win64/Release
	$(CP) -frv wininstaller/vmware-dns-client-msm/x64/Debug/*.msm $(PUBLISH_DIR)/vmdns_msms/Debug
	$(CP) -frv wininstaller/vmware-dns-client-msm/x64/Release/*.msm $(PUBLISH_DIR)/vmdns_msms/Release
	$(CP) -frv wininstaller/vmware-dns-server-msm/x64/Debug/*.msm $(PUBLISH_DIR)/vmdns_msms/Debug
	$(CP) -frv wininstaller/vmware-dns-server-msm/x64/Release/*.msm $(PUBLISH_DIR)/vmdns_msms/Release
	$(CP) -frv wininstaller/vmware-dns-client-msi/x64/Debug/*.msi $(PUBLISH_DIR)/vmdns_setup/Debug
	$(CP) -frv wininstaller/vmware-dns-client-msi/x64/Release/*.msi $(PUBLISH_DIR)/vmdns_setup/Release
	$(CP) -frv wininstaller/vmware-dns-server-msi/x64/Debug/*.msi $(PUBLISH_DIR)/vmdns_setup/Debug
	$(CP) -frv wininstaller/vmware-dns-server-msi/x64/Release/*.msi $(PUBLISH_DIR)/vmdns_setup/Release
	$(CP) -frv x64/Debug/*.pdb $(PUBLISH_DIR)/win64/Debug
	$(CP) -frv x64/Release/*.pdb $(PUBLISH_DIR)/win64/Release
	$(CP) -frv x64/Debug/*.exe $(PUBLISH_DIR)/win64/Debug
	$(CP) -frv x64/Release/*.exe $(PUBLISH_DIR)/win64/Release
	$(CP) -frv x64/Debug/*.dll $(PUBLISH_DIR)/win64/Debug
	$(CP) -frv x64/Debug/libvmdnsclient.lib $(PUBLISH_DIR)/win64/Debug
	$(CP) -frv x64/Release/*.dll $(PUBLISH_DIR)/win64/Release
	$(CP) -frv x64/Release/libvmdnsclient.lib $(PUBLISH_DIR)/win64/Release

  #include $(MAKEROOT)/makeimpl.mk

  ifdef GOBUILD_TARGET
    ifndef GOBUILD_SPECINFO_MK
      GOBUILD_SPECINFO_MK = 1
      .PHONY: $(GOBUILD_SPECINFO_MK)
      $(GOBUILD_SPECINFO_MK):

    endif

    .PHONY: gobuild-deps-copy deps

    deps: $(GOBUILD_SPECINFO_MK) gobuild-deps-copy

    gobuild-deps-copy: $(GOBUILD_SPECINFO_MK)
   endif

else

GOBUILD_AUTO_COMPONENTS_HOSTTYPE = linux64
GOBUILD_TARGET ?= vmware-dns-linux-x64

SRCROOT=..
MAKEROOT=$(SRCROOT)/support/make
include $(MAKEROOT)/makedefs.mk

MAKEFILENAME = Makefile.bootstrap
DIRS = build

CLEANMORE = \
    obj            \
    aclocal.m4     \
    autom4te.cache \
    Makefile.in    \
    config.guess   \
    config.sub     \
    configure      \
    depcomp        \
    include/config.h.in* \
    install-sh     \
    ltmain.sh      \
    missing

include $(MAKEROOT)/makeimpl.mk

clean:
	@for i in `find $(CURDIR) -name Makefile.in`; do \
		$(RM) -f $$i; \
	done

endif


