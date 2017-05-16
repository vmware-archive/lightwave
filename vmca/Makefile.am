ACLOCAL_AMFLAGS = -I m4

PYTHON_DIR =

if ENABLE_PYTHON
    PYTHON_DIR += python
endif

SUBDIRS = \
    include \
    config  \
    common  \
    vmcadb  \
    service \
    client  \
    certool \
    jdepends \
    java \
    $(PYTHON_DIR)

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
