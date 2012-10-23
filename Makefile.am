SUBDIRS = src
AUTOMAKE_OPTIONS = no-dist-gzip dist-bzip2

if BUILD_EXAMPLES
SUBDIRS += examples
endif
if BUILD_TESTS
SUBDIRS += tests
endif

package: dist
	cp wsmancli.spec /usr/src/packages/SPEC
	cp wsmancli-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba wsmancli.spec
