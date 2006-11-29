SUBDIRS = src examples tests

package: dist
	cp wsmancli.spec /usr/src/packages/SPEC
	cp wsmancli-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba wsmancli.spec
