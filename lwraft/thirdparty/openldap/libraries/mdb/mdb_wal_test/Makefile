all: mdb_wal_test

mdb_wal_test:
	cp ../lmdb.h lmdb.h
	cp ../midl.h midl.h
	cp ../midl.c midl.c
	cp ../mdb.c mdb.c
	gcc -g -DMDB_USE_PWRITEV -o mdb_wal_test -fpic mdb_wal_test.cpp mdb.c midl.c -lpthread -lstdc++

clean:
	rm -rf mdb.c midl.c lmdb.h mdb.c midl.h *.o mdb_wal_test backupdb testdb

cleandbs:
	rm -rf backupdb testdb
