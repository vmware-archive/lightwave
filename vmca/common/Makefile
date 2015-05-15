#
# ThinAppManagement VMCA Service Makefile
#

GOBUILD_AUTO_COMPONENTS = 1
GOBUILD_TARGET ?= XXXXXXXXXXXXxTalk_to_sriramXXXXXXXXXXXXXX

SRCROOT=../..
MAKEROOT=$(SRCROOT)/../support/make
include $(MAKEROOT)/makedefs.mk

LIBNAME = common

DEPPROJECTS = ../idl
PROJINC = VMCAService

CXXFILES =

CFILES = \
	fsutils.c \
	logging.c \
	globals.c \
	memory.c  \
	certutils.c \
	pkcs_csr.c

LOCALHEADERS = 

DISTINCPREFIX = VMCAService
DISTINC = 

GBDEPS = VMWARE_LIKEWISE_LINUX_X64

CFLAGS += -I../include

include $(MAKEROOT)/makeimpl.mk

