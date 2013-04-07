
publicdir=public
install_publicdir=public
privatedir=private
dist_public_DATA=public
install_public_DATA=public
dist_private_DATA=private

public:
	mkdir public

private:
	mkdir private

dist-hook:
	doxygen Doxyfile.private
	doxygen Doxyfile

distclean-local:
	$(RM) -rf public private

.PHONY: all
