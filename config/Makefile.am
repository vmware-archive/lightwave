ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
    jdepends \
    cfgutils \
    tools \
    scripts

stage:
	if test -n '$(STAGEDIR)'; then \
	  list='$(SUBDIRS)' ; \
	  for subdir in $$list; do \
	    echo "Staging install in $$subdir"; \
	    $(MAKE) -C $$subdir $(AM_MAKEFLAGS) DESTDIR=$(STAGEDIR) install; \
	  done; \
	fi

all-local: stage

.PHONY: stage
