SUBDIRS = src

package: dist
	cp openwsman.spec /usr/src/packages/SPEC
	cp openwsman-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba openwsman.spec

install-data-local: openwsman.pc
	$(mkinstalldirs) $(libdir)/pkgconfig
	$(INSTALL_DATA) openwsman.pc $(DESTDIR)$(libdir)/pkgconfig

uninstall-local:
	rm -f $(DESTDIR)$(libdir)/pkgconfig/openwsman.pc
