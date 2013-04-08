
EXTRA_DIST=public private

public:
	doxygen Doxyfile

private:
	doxygen Doxyfile.private

install-data-local:
	$(MKDIR_P) $(DISTDIR)$(pkgdatadir)/docs
	cp -rf public $(DISTDIR)$(pkgdatadir)/docs

uninstall-local:
	rm -rf $(DISTDIR)$(pkgdatadir)/docs

install-exec-hook: public

distclean-local:
	$(RM) -rf public private

