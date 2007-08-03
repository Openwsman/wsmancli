SUBDIRS = src examples tests
AUTOMAKE_OPTIONS = no-dist-gzip dist-bzip2

package: dist
	cp wsmancli.spec /usr/src/packages/SPEC
	cp wsmancli-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba wsmancli.spec
