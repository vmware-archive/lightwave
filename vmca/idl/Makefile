IDL=/opt/likewise/bin/dceidl
CFLAGS=-I. -I../include -I /usr/include -I /opt/likewise/include

vmca_h.h vmca_cstub.c vmca_sstub.c: vmca.idl
	$(IDL) -keep c_source -header vmca_h.h vmca.idl

clean :
	rm -rf  vmca_sstub.c vmca_cstub.c vmca_h.h
