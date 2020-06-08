#
# Copyright 2018 VMware, Inc
#

CLEAN_TARGETS = \
	stage \
	lightwave-ui/idm/dist \
	lightwave-ui/idm/node_modules \
	lightwave-ui/idm/package-lock.json \
	lightwave-ui/lwdir/dist \
	lightwave-ui/lwdir/node_modules \
	lightwave-ui/lwdir/package-lock.json \
	lwraft-ui/node_modules \
	lwraft-ui/package-lock.json

all: lightwaveui lwraftui

stage:
	mkdir -p stage

lwraftui: stage
	rpmbuild --define "_topdir `pwd`/stage" -ba lwraft-ui/build/lwraft-ui.spec

lightwaveui: stage
	rpmbuild --define "_topdir `pwd`/stage" -ba lightwave-ui/build/lightwave-ui.spec

clean:
	rm -rf $(CLEAN_TARGETS)
