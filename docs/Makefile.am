
EXTRA_DIST=public private

public:
	doxygen Doxyfile

private:
	doxygen Doxyfile.private

install-data-local:
	@find $(srcdir)/public -type d -exec $(INSTALL) -d '{}' $(DISTDIR)$(pkgdatadir)/docs/{} \;
	@find $(srcdir)/public -type f -exec $(INSTALL) '{}' $(DISTDIR)$(pkgdatadir)/docs/{} \;
	@find $(srcdir)/private -type d -exec $(INSTALL) -d '{}' $(DISTDIR)$(pkgdatadir)/docs/{} \;
	@find $(srcdir)/private -type f -exec $(INSTALL) '{}' $(DISTDIR)$(pkgdatadir)/docs/{} \;

uninstall-local:
	@find $(srcdir)/public -type f -exec $(RM) $(DISTDIR)$(pkgdatadir)/docs/{} \;
	@find $(srcdir)/public -type d -exec $(RM) -r $(DISTDIR)$(pkgdatadir)/docs/{} \;
	@find $(srcdir)/private -type f -exec $(RM) $(DISTDIR)$(pkgdatadir)/docs/{} \;
	@find $(srcdir)/private -type d -exec $(RM) -r $(DISTDIR)$(pkgdatadir)/docs/{} \;

install-exec-hook: public private

clean-local:
	$(RM) -rf public private

