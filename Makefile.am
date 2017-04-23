#SUBDIRS = \
#        vmevent \
#        vmdir \
#        config \
#        vmdns \
#        vmafd \
#        vmca \
#        vmidentity

#SRCROOT := .
#MAKEROOT=$(SRCROOT)/support/make
#include $(MAKEROOT)/makedefs.mk

stage: all-recursive
	echo "STAGE TARGET HELLO WORLD"
	touch /tmp/STAGE_TARGET_HELLO_WORLD

all: 
	@cd ./vmevent    && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
	@cd ./vmdir      && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
	@cd ./vmdns      && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
	@cd ./vmafd      && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
	@cd ./vmca       && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
	@cd ./vmidentity && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
	@cd ./config     && make $(AM_FLAGS) DESTDIR=/home/brownj/workspaces/git/pslabs/lightwave/stage install
