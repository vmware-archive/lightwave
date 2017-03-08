all: libmdb_nowal.so libmdb_wal.so mdb_wal_test.cpp
	gcc -g -o mdb_wal_test_linux mdb_wal_test.cpp -lpthread -L . -lmdb -lstdc++

libmdb_nowal.so:
	cp ../lmdb.h lmdb.h
	cp ../midl.h midl.h
	cp ../midl.c midl.c
	gcc -shared -o libmdb_nowal.so -fpic mdb_nowal.c midl.c
	ln -s libmdb_nowal.so libmdb.so

libmdb_wal.so:
	cp ../lmdb.h lmdb.h
	cp ../midl.h midl.h
	cp ../midl.c midl.c
	cp ../mdb.c mdb_wal.c
	gcc -shared -o libmdb_wal.so -fpic mdb_wal.c midl.c

clean:
	rm -rf mdb_wal.c midl.c lmdb.h midl.h *.o *.so mdb_wal_test_linux backupdb testdb
