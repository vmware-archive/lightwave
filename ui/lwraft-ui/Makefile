#
# Copyright 2018 VMware, Inc
#

NGCLI := $(shell command -v ng 2> /dev/null)
NODE := $(shell command -v npm 2> /dev/null)

all: rpm

nodejs:
ifndef NODE
        @echo 'Installing nodeJS'
	tdnf install nodejs-7.7.4 -y
endif

ng-cli: nodejs
ifndef NGCLI
	npm i -g @angular/cli@1.6.3
endif

rpm: ng-cli
	mkdir -p stage
	rpmbuild --define "_topdir `pwd`/stage" -ba build/lwraft-ui.spec

clean:
	rm -rf stage
