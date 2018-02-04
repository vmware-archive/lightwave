all: package

war:
	mvn package

package: war
	rpmbuild --define "_topdir `pwd`/stage" -ba build/lightwave-ui.spec

clean:
	rm -r stage dist target
