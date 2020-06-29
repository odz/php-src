$(srcdir)/ircg_scanner.c: $(srcdir)/ircg_scanner.re
	re2c -b $(srcdir)/ircg_scanner.re > $@
