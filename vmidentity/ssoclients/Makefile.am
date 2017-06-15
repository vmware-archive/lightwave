SUBDIRS = \
    common \
    oidc \
    restclient

stage:
	if test -n '$(STAGEDIR)'; then \
          list='$(SUBDIRS)' ; \
          for subdir in $$list; do \
            echo "Staging install in $$subdir"; \
            $(MAKE) -C $$subdir $(AM_MAKEFLAGS) DESTDIR=$(STAGEDIR) install; \
          done; \
          sed -i~ -e "s;/opt/vmware/lib64;$(STAGEDIR)/opt/vmware/lib64;" $(STAGEDIR)/opt/vmware/lib64/libssooidc.la; \
        fi

all-local: stage

.PHONY: stage
