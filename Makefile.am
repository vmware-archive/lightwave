#SUBDIRS = \
#        vmevent \
#        vmdir \
#        vmdns \
#        vmafd \
#        vmca \
#        vmidentity \
#        config

subdirs = vmevent vmdir vmdns vmafd vmca config

$(subdirs):
	make -C $@ $(AM_MAKEFLAGS) DESTDIR=$(PWD)/stage install

.PHONY: $(subdirs)

all: $(subdirs)
