#SUBDIRS = \
#        vmevent \
#        vmdir \
#        config \
#        vmdns \
#        vmafd \
#        vmca \
#        vmidentity

subdirs = vmevent vmdir vmdns vmafd vmca config
destdir = "/home/brownj/workspaces/git/pslabs/lightwave/stage"

$(subdirs):
	make $(AM_MAKEFLAGS) DESTDIR=$(destdir) install -C $@

.PHONY: $(subdirs)

all: $(subdirs)
