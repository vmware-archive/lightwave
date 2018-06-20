all: package

war:
	mvn package

package: war
	rpmbuild --define "_topdir `pwd`/stage" -ba build/lwdir-ui.spec

clean:
	rm -r stage dist target node_modules
