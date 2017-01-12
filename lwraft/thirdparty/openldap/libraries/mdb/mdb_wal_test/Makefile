GCC_BIN := /build/toolchain/lin32/gcc-4.1.2-8/bin/x86_64-linux5.0-gcc
GCC_LIB := /build/toolchain/lin32/gcc-4.1.2-8/x86_64-linux5.0/lib64

all: libmdb_nowal.so libmdb_wal.so mdb_wal_test.cpp mdb.c midl.c
	$(GCC_BIN) -g -DO_DIRECT=040000 -o mdb_wal_test_linux mdb_wal_test.cpp -lpthread -L $(GCC_LIB) -L . -lmdb -lstdc++

libmdb_nowal.so:
	$(GCC_BIN) -shared -o libmdb_nowal.so -fpic mdb.c midl.c
	ln -s libmdb_nowal.so libmdb.so

libmdb_wal.so:
	$(GCC_BIN) -shared -o libmdb_wal.so -fpic mdb_wal.c midl.c

clean:
	rm -rf *.o *.so mdb_wal_test_linux backupdb testdb
