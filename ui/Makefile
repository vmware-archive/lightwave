#
# Copyright 2018 VMware, Inc
#

all: lightwaveui lwraftui

stage:
	mkdir -p stage

lwraftui: stage
	rpmbuild --define "_topdir `pwd`/stage" -ba lwraft-ui/build/lwraft-ui.spec

lightwaveui: stage
	rpmbuild --define "_topdir `pwd`/stage" -ba lightwave-ui/build/lightwave-ui.spec

clean:
	rm -rf stage lightwave-ui/idm/dist lightwave-ui/lwdir/dist lightwave-ui/idm/node_modules lightwave-ui/lwdir/node_modules lwraft-ui/node_modules
