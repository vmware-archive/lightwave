#ifdef _WIN32
#include <tchar.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lmdb.h"
#include <iomanip>
#include <unistd.h>

#ifdef _WIN32
#define SLEEP(x) _sleep(x*1000)
#else
#define SLEEP(x) sleep(x)
#endif

using namespace std;

/* default db size 1024MB */
#define DB_DEFAULT_SIZE (1L << 30)

class MdbTester
{
 public:
    MdbTester(unsigned int env_flags, int mapsize = 0, int keep_xlog = 0);
    ~MdbTester();

    void show_keys();
    void provision(int cnt, int start_idx = 0);
    int txn_begin();
    int txn_commit();
    void txn_abort();
    int test_modify(int idx);
    int test_del(int idx, const char *prefix);
    int test_modify_nested(int idx1, int in_child_txn = 0, int commit = 1);
    int largedata_modify(int i, int datasize, const char *datamark = "default");
    int largedata_search(int i);
    int test_mod_search(int idx, MDB_txn *txn = NULL);
    int test_provision_search(int idx);
    int db_init();
    int db_close();
    int shrink_db(int dbsize);
    static int backup_db();
    static int truncate_backup_db(long dbSize);
    static int restore_db();
    MDB_env *get_dbenv() { return _env; };

 private:
    int search_data(MDB_val *key, MDB_val *data);
    int search_data(MDB_val *key, MDB_val *data, MDB_txn *txn);
    int test_search(int idx, const char *key_prefix, MDB_txn *txn = NULL);

 private:
    vector<int> _keys;
    int _max_keys;
    MDB_env *_env;
    MDB_dbi _dbi;
    int _mapsize;
    unsigned int _env_flags;
    MDB_txn *_txn;
    int _keep_xlog;
};

int MdbTester::db_init()
{
  //unsigned int flags = MDB_FIXEDMAP|_env_flags;
  //vmdird doesn't use MDB_FIXEDMAP
  unsigned int flags = _env_flags;
  MDB_txn *txn = NULL;
  int rc = 0;

  _env = NULL;
  char path_buf[1024];
#ifdef _WIN32
  strcpy(path_buf, "C:\\tmp\\testdb");
#else
  strcpy(path_buf, "./testdb");
#endif

  if ((rc=mdb_env_create(&_env)))
  {
    cout<<"mdb_env_create error"<<endl;
    return rc;
  }

  if (_mapsize > 0)
  {
     if (mdb_env_set_mapsize(_env, _mapsize))
        return -1;
     printf("mapsize set to %.2f MB\n", (float)_mapsize/(float)(1024*1024));
  }

  if((rc=mdb_env_set_maxdbs(_env, 10)))
  {
      cout<<"mdb_env_set_maxdbs"<<endl;
      return rc;
  }

  if (_keep_xlog)
     flags |= MDB_KEEPXLOGS;

  rc =  mdb_env_open(_env, path_buf, flags, 0600);
  if (rc)
  {
     printf("mdb_env_open failed with error %d: %s\n", rc, mdb_strerror(rc));
     return rc;
  }

  if ((rc=mdb_txn_begin(_env, NULL, 0, &txn)) ||
      (rc=mdb_open(txn, "second", MDB_CREATE, &_dbi)) ||
      (rc=mdb_txn_commit(txn)))
  {
     cout<<"Warninig: fail to create second db."<<endl;
  }

   return 0;
}

int MdbTester::db_close()
{
  //mdb_close(_env, _dbi);
  mdb_env_close(_env);
  _env = NULL;
  return 0;
}

MdbTester::MdbTester(unsigned int env_flags, int mapsize, int keep_xlog):_env_flags(env_flags), _max_keys(400000),_env(NULL), _mapsize(mapsize), _txn(NULL),_keep_xlog(keep_xlog)
{
  int rc =0;

  for(int i=0; i<_max_keys; i++)
  {
    _keys.push_back(800000+i);
  }
  if ((rc=db_init()))
  {
    cout<<"Error db_init: "<<rc<<endl;
    exit(-1);
  }
}

int MdbTester::txn_begin()
{
  int rc = 0;
  rc = mdb_txn_begin(_env, NULL, 0, &_txn);
  return rc;
}

int MdbTester::txn_commit()
{
  int rc = 0;
  rc = mdb_txn_commit(_txn);
  if (rc)
      printf("provision: mdb_txn_commit failed %d %s\n", rc,  mdb_strerror(rc));
  _txn = NULL;
  return rc;
}

void MdbTester::txn_abort()
{
  mdb_txn_abort(_txn);
  _txn = NULL;
}

int MdbTester::shrink_db(int dbsize)
{
    db_close();
    printf("shrinking to %.2f MB\n", (float)(dbsize)/(float)(1024*1024));
    _mapsize = dbsize;
    if(db_init())
    {
	printf("failed to shrink db by half\n");
        exit(1);
    }
    return 0;
}

int MdbTester::search_data(MDB_val *key, MDB_val *data)
{
  int rc;
  MDB_txn *txn = _txn;;

  if (_txn == NULL)
  {
    rc = mdb_txn_begin(_env, NULL, MDB_RDONLY, &txn);
    if (rc)
      return rc;
  }

  rc = mdb_get(txn, _dbi, key, data);
  if (rc)
    printf("mdb_get no data found for key '%s'\n", (char *)key->mv_data);
  else
    printf("mdb_get got data with length %d on key '%s'\n", data->mv_size, (char *)key->mv_data);
  if (_txn == NULL)
    mdb_txn_abort(txn);
  return rc;
}

int MdbTester::search_data(MDB_val *key, MDB_val *data, MDB_txn *txn)
{
  int rc;

  rc = mdb_get(txn, _dbi, key, data);
  if (rc)
    printf("txn mdb_get no data found for key '%s'\n", (char *)key->mv_data);
  else
    printf("txn mdb_get got data with length %d on key '%s'\n", data->mv_size, (char *)key->mv_data);
  return rc;
}

MdbTester::~MdbTester()
{
    db_close();
}

void MdbTester::show_keys()
{
  for(int i=0; i<_max_keys; i++)
  {
      cout << _keys[i] << endl;
  }
}

void MdbTester::provision(int cnt, int start_idx)
{
  MDB_txn *txn = _txn;
  int rc, j, k, data_size;
  MDB_val key, data;
  char kval[128];
  char sval[512];
  long data_size_total = 0;
  int failed_cnt = 0;
  int si = 0;

  if (cnt > _max_keys)
     cnt = _max_keys;

  for(int i=0, si=start_idx; i<cnt; i++, si++)
  {
      string small_data;
      key.mv_data = kval;
      sprintf(kval, "provision key %d", _keys[si]);
      if (i==0)
          printf("provision %d data items starting with key '%s'\n", cnt, kval);
      key.mv_size = strlen(kval);
      j = 0;
      k = 0;
      data_size = rand()%997;
      while (j < data_size)
      {
          sprintf(sval, "Dummy small data -------------------------------- key %d, i=%d,", _keys[si], k++);
          small_data += sval;
          j += (int)strlen(sval);
      }
      data.mv_size = small_data.size();
      data.mv_data = (char *)small_data.c_str();
	  data_size_total += (long)data.mv_size;

      if (_txn == NULL){
          rc = mdb_txn_begin(_env, NULL, 0, &txn);
          if (rc)
             return;
      }

      //printf("======== Adding data - key '%s' with len %d\n", kval, data.mv_size);
      rc = mdb_put(txn, _dbi, &key, &data, MDB_NOOVERWRITE);
      if (rc == 0)
      {
        if (_txn == NULL)
        {
           rc = mdb_txn_commit(txn);
           if (rc) {
             printf("provision: mdb_txn_commit failed %d %s\n", rc,  mdb_strerror(rc));
	     goto failed;
           }
        }
      } else {
        failed_cnt++;
        if (failed_cnt < 3)
            printf("provision: mdb_put failed %d %s\n", rc,  mdb_strerror(rc));
        if (_txn == NULL)
          mdb_txn_abort(txn);
      }
  }

  printf("provisioned %d data items with avg data size %d, failed counts: %d\n",
         cnt, data_size_total/cnt, failed_cnt);
  return;

failed:
    printf("provison stoped with last data key '%s'\n", kval);
    return;
}

int MdbTester::test_mod_search(int idx, MDB_txn *txn)
{
    return test_search(idx, "modify key", txn);
}

int MdbTester::test_provision_search(int idx)
{
    return test_search(idx, "provision key");
}

int MdbTester::test_search(int idx, const char *key_prefix, MDB_txn *txn)
{
  char kval[32];
  MDB_val key, data;
  int rc, i;

  i = idx;
  if (i >= _max_keys)
    i = _max_keys - 1;
  key.mv_data = kval;
  sprintf(kval, "%s %d", key_prefix,  _keys[i]);
  key.mv_size = strlen(kval);
  if (txn)
    rc = search_data(&key, &data, txn);
  else
    rc = search_data(&key, &data);
  return rc;
}

int MdbTester::largedata_search(int i)
{
  char kval[512];
  MDB_val key, data;
  int rc = 0;
  char *p;

  sprintf(kval, "%d", _keys[i]);
  key.mv_data = kval;
  key.mv_size  = strlen(kval);
  rc = search_data(&key, &data);
  if (rc == 0) {
      cout<< "largedata_search for key '" << kval << "' got data with size " << data.mv_size <<endl;
      p = (char *)malloc(data.mv_size + 1);
      memset(p, 0, data.mv_size + 1);
      memcpy(p, data.mv_data, data.mv_size);
      printf("last 80 chars of data for key '%s' is: %s\n", (char *)key.mv_data, &p[data.mv_size - 80]);
      free(p);
  }
  return rc;
}

int MdbTester::test_modify(int idx)
{
  char kval[128];
  MDB_val key, data;
  char sval[512];
  int rc = 0, i;
  MDB_txn *txn = _txn;

  i = idx;
  if (i >= _max_keys)
    i = _max_keys - 1;

  key.mv_data = kval;
  sprintf(kval, "modify key %d", _keys[i]);
  key.mv_size = strlen(kval);
  sprintf(sval, "This is the data item with key '%s' for mdb_put testing.", kval);
  data.mv_data = sval;
  data.mv_size = sizeof(sval);

  if (_txn == NULL)
  {
    rc = mdb_txn_begin(_env, NULL, 0, &txn);
    if (rc)
      return rc;
  }

  rc = mdb_put(txn, _dbi, &key, &data, 0);
  if (!rc)
  {
    printf("mdb_put succeeded on key '%s'\n", kval);
    if (_txn == NULL)
    {
      rc = mdb_txn_commit(txn);
      if (rc)
      {
        return rc;
      }
    }
  } else
  {
    printf("mdb_put for modify data Failed on key '%s'\n", kval);
    if (_txn == NULL)
      mdb_txn_abort(txn);
  }

  return rc;
}

int MdbTester::test_del(int idx, const char *prefix)
{
  char kval[128];
  MDB_val key, data;
  char sval[512];
  int rc = 0, i;
  MDB_txn *txn = _txn;

  i = idx;
  if (i >= _max_keys)
    i = _max_keys - 1;

  key.mv_data = kval;
  sprintf(kval, "%s%d", prefix, _keys[i]);
  key.mv_size = strlen(kval);
  sprintf(sval, "This is the data item with key '%s' for mdb_put testing.", kval);
  data.mv_data = sval;
  data.mv_size = sizeof(sval);

  if (_txn == NULL)
  {
    rc = mdb_txn_begin(_env, NULL, 0, &txn);
    if (rc)
      return rc;
  }

  rc = mdb_del(txn, _dbi, &key, &data);
  if (!rc)
  {
    printf("mdb_del succeeded on key '%s'\n", kval);
    if (_txn == NULL)
    {
      rc = mdb_txn_commit(txn);
      if (rc)
      {
        return rc;
      } else
        printf("mdb_del commited successfully.\n");
    }
  } else
  {
    printf("mdb_del failed on key '%s'\n", kval);
    if (_txn == NULL)
      mdb_txn_abort(txn);
  }

  return rc;
}

int MdbTester::test_modify_nested(int idx, int in_child_txn, int commit)
{
  char kval[128];
  MDB_val key, data;
  char sval[512];
  int rc = 0, i;
  MDB_txn *txn = _txn;
  MDB_txn *child_txn = NULL;

  if (idx >= _max_keys)
  {
    printf("idx out of range\n");
    return -1;
  }

  i = idx;
  key.mv_data = kval;
  sprintf(kval, "modify key %d", _keys[i]);
  key.mv_size = strlen(kval);
  sprintf(sval, "This is the data item with key '%s' for mdb_put testing.", kval);
  data.mv_data = sval;
  data.mv_size = sizeof(sval);

  if (in_child_txn)
  {
    rc = mdb_txn_begin(_env, txn, 0, &child_txn);
    if (rc)
    {
      printf("mdb_txn_begin on child transaction failed: %s\n", mdb_strerror(rc));
      return rc;
    }
  }

  if (in_child_txn)
    rc = mdb_put(child_txn, _dbi, &key, &data, 0);
  else
    rc = mdb_put(txn, _dbi, &key, &data, 0);

  if (!rc)
  {
    printf("mdb_put succeeded on key '%s'\n", kval);
  } else
  {
    printf("mdb_put for modify data Failed on key '%s'\n", kval);
  }

  if (in_child_txn)
  {
    test_mod_search(i, child_txn);
  }

  if (in_child_txn)
  {
    if (commit)
    {
      rc = mdb_txn_commit(child_txn);
      if (rc)
          printf("failed to commit child txn %s\n", mdb_strerror(rc));
    } else
      mdb_txn_abort(child_txn);
  }

  return rc;
}

int MdbTester::largedata_modify(int i, int data_size, const char *datamark)
{
  char kval[128];
  MDB_val key, data;
  char sval[512];
  string big_data;
  int rc = 0, j, k;
  MDB_txn *txn = _txn;

  key.mv_data = kval;
  sprintf(kval, "%d", _keys[i]);
  key.mv_size = strlen(kval);
  j = 0;
  k = 0;
  while (j < data_size)
  {
      sprintf(sval, "largedata modify: size %d, key %d, data-mark=%s, i=%d,", data_size, _keys[i], datamark, k++);
      big_data += sval;
      j += (int)strlen(sval);
  }
  data.mv_size = big_data.size();
  data.mv_data = (char *)big_data.c_str();

  cout<<"largedata modify on key " << _keys[i] << " with data size " << data.mv_size << endl;

  if (_txn == NULL)
  {
    rc = mdb_txn_begin(_env, NULL, 0, &txn);
    if (rc)
      return rc;
  }
  rc = mdb_put(txn, _dbi, &key, &data, 0);
  if (!rc)
  {
    cout<<"mdb_put for largedata modify succeeded on key "<<_keys[i]<<endl;
    if (_txn == NULL)
    {
      rc = mdb_txn_commit(txn);
      if (rc)
	cout<<"txn_commit for largedata modify failed on key "<<_keys[i]<<endl;
    }
  } else {
    cout<<"mdb_put for largedata modify failed on key "<<_keys[i]<<endl;
    if (_txn == NULL)
      mdb_txn_abort(txn);
  }

  return rc;
}

int MdbTester::backup_db()
{
#ifdef _WIN32
	system("del \/Q C:\\tmp\\backupdb\\*");
	if(system("copy C:\\tmp\\testdb\\data.mdb C:\\tmp\\backupdb")||system("copy C:\\tmp\\testdb\\lock.mdb C:\\tmp\\backupdb"))
#else
    if(system("rm -f backupdb/*") || system("cp testdb/data.mdb backupdb"))
#endif
    {
	printf("failed to copy database from testdb to backupdb\n");
	exit(-1);
    }
    return 0;
}

//Used to trancate the extra blocks in the mdb.data larger than me_last_pgno
int MdbTester::truncate_backup_db(long dbSize)
{
#ifdef _WIN32
    return 0;
#else
    if (truncate("backupdb/data.mdb", dbSize) == 0)
        printf("truncated backupdb/data.mdb to %lu bytes ...\n", dbSize);
    else
        printf("tailed to truncat backupdb/data.mdb to %lu bytes ...\n", dbSize);
#endif
    return 0;
}

int MdbTester::restore_db()
{
#ifdef _WIN32
	int rc = 0;
	rc = system("del \/Q C:\\tmp\\testdb\\data.mdb");
	rc = system("del \/Q C:\\tmp\\testdb\\lock.mdb");
	if (system("copy C:\\tmp\\backupdb\\data.mdb testdb")||system("copy C:\\tmp\\backupdb\\lock.mdb C:\\tmp\\testdb"))
#else
	if (system("rm -f testdb/*.mdb") || system("mv backupdb/*.mdb testdb"))
#endif

    {
	printf("failed to move database from backupdb to testdb\n");
	exit(-1);
    }
	return 0;
}

static void show_usage(char *path)
{
    printf("Usage %s -w1|-w2|-w3|-w4|-w5|-w6|-r1|-r2|-r3|-r4|-b1|-b2|-b3|-b4|-b5|-d1|-d2|-s\n", path);
    exit(-1);
}

#ifdef _WIN32
int _tmain(int argc, char* argv[])
#else
main(int argc, char **argv)
#endif
{
  int t_modify = 0, t_search=0, t_db_shrink=0, t_delete=0, rc=0;
  int t_bigdata = 0;
  int wal_enabled = 1;
  unsigned int env_flags = 0;
  char *envp = NULL;

  if (argc != 2)
    show_usage(argv[0]);

  envp = getenv("MDB_ENV_WALL");
  if (envp && strcasecmp(envp, "0") == 0)
  {
      wal_enabled = 0;
      printf("MDB_ENV_WALL disabled\n");
  } else
  {
      env_flags = MDB_WAL;
      printf("MDB_ENV_WALL enabled\n");
  }

  if (strcmp(argv[1], "-w1")==0)
    t_modify=1;
  else if (strcmp(argv[1], "-w2")==0)
    t_modify=2;
  else if (strcmp(argv[1], "-w3")==0)
    t_modify=3;
  else if (strcmp(argv[1], "-w4")==0)
    t_modify=4;
  else if (strcmp(argv[1], "-w5")==0)
    t_modify=5;
  else if (strcmp(argv[1], "-w6")==0)
    t_modify=6;
  else if (strcmp(argv[1], "-b1")==0)
    t_bigdata = 1;
  else if (strcmp(argv[1], "-r1")==0)
    t_search = 1;
  else if (strcmp(argv[1], "-r2")==0)
    t_search = 2;
  else if (strcmp(argv[1], "-r3")==0)
    t_search = 3;
  else if (strcmp(argv[1], "-r4")==0)
    t_search = 4;
  else if (strcmp(argv[1], "-r5")==0)
    t_search = 5;
  else if (strcmp(argv[1], "-s")==0)
    t_db_shrink = 1;
  else if (strcmp(argv[1], "-d1")==0)
    t_delete = 1;
  else
    show_usage(argv[0]);

  if (t_modify || t_bigdata == 1 ||t_delete == 1 || t_db_shrink == 1)
  {
#ifdef _WIN32
    //rc = GetCurrentDirectory(sizeof(cmdbuf), cmdbuf);
	  rc = system("rmdir \/S \/Q c:\\tmp\\testdb c:\\tmp\\backupdb");
	  if(system("md C:\\tmp\\testdb")||system("md C:\\tmp\\backupdb"))
#else
    if(system("rm -rf testdb backupdb") || system("mkdir testdb backupdb"))
#endif
    {
      printf("failed cleanup directory testdb/backupdb\n");
      exit(-1);
    }
  }

  class MdbTester *mtp = NULL;

  if (t_bigdata == 1)
  {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE>>4, 1);
    if (wal_enabled)
    {
      cout << "Making a backup of mdb into backupdb"<<endl;
      MdbTester::backup_db();
    }

    mtp->provision(256);

    mtp->test_provision_search(100);
    mtp->test_provision_search(200);
    mtp->test_provision_search(255);

    cout << endl<<"Test big data modify:"<<endl;
    mtp->largedata_modify(4, 4096*4, "mark1"); //four overflow pages
    mtp->largedata_search(4);

    mtp->largedata_modify(4, 4096*4, "mark2"); //four overflow pages
    mtp->largedata_search(4);

    cout << endl<<"Test large data modify:"<<endl;
    mtp->largedata_modify(5, 500, "mark3"); //no overflow page
    mtp->largedata_search(5);

    mtp->largedata_modify(5, 500, "mark4"); //no overflow page
    mtp->largedata_search(5);
    delete mtp;

    if (wal_enabled)
    {
      cout<<endl<<"restore database..."<<endl;
      MdbTester::restore_db();
    }
    t_search = 5; // t_search 5 will restore db and search for added data.
  }

  if (t_db_shrink == 1) {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    rc = mtp->test_modify(1);
    rc = mtp->test_mod_search(1);
    rc = mtp->shrink_db(DB_DEFAULT_SIZE/2);
    rc = mtp->test_mod_search(1);
    //For Linux: must have atleast one write transaction
    //to write the new size into meta data
    delete mtp;
  }

  if (t_modify == 1) {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    mtp->provision(320);

    rc = mtp->test_modify(1);
    if (rc)
      printf("test modify failed");

    rc = mtp->test_mod_search(1);
    if (rc)
      printf("test search failed after modify.\n");

    rc = mtp->test_mod_search(2);
    if (rc == 0 ) {
      printf("test search returned a value for no-such-data\n");
      rc = -1;
    } else
      rc = 0;

    printf("Pause 32 seconds for checkpoint...\n");
    SLEEP(32); //observe checkpoint
    delete mtp;
	SLEEP(1);
  }

  if (t_modify == 2) {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    mtp->txn_begin();
    mtp->provision(16000);
    printf ("search data inside transaction...\n");
    mtp->test_provision_search(1000);
    mtp->test_provision_search(7001);
    rc = mtp->txn_commit();
    if (rc)
    {
      /* This can be triggered with a modified mdb.c
       * where wal write is limited to 2000 pags:
       * if ((pos/pgsize + pgs) >= 2000)
       * in function mdb_walbuf_cpy()
       */

      printf ("search again after commit failed\n");
      mtp->test_provision_search(1000);
      mtp->test_provision_search(7001);
    } else
    {
      printf ("search again after commit succeed outside transaction...\n");
      mtp->test_provision_search(1000);
      mtp->test_provision_search(7001);
    }

    if (rc)
    {
       mtp->txn_begin();
       mtp->provision(1000);
       mtp->test_provision_search(100);
       mtp->test_provision_search(701);
       rc = mtp->txn_commit();
       printf("transaction commit %s with 1000 data provisioned.\n", rc?"failed":"succeed");
       printf("search again outside transaction after 1000 data provisioned...\n");
       mtp->test_provision_search(100);
       mtp->test_provision_search(701);
    }

    printf("Pause 32 seconds for checkpoint...\n");
    SLEEP(32); //observe checkpoint
    delete mtp;
  }

  if (t_modify == 3) {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE>>4, 0);
    mtp->txn_begin();
    mtp->provision(8000);
    printf ("search data inside transaction...\n");
    mtp->test_provision_search(100);
    mtp->test_provision_search(201);
    mtp->txn_abort();
    printf ("search again after aborting the transaction...\n");
    mtp->test_provision_search(100);
    mtp->test_provision_search(201);

    printf("Pause 32 seconds for checkpoint...\n");
    SLEEP(32); //observe checkpoint
    delete mtp;
  }

  if (t_modify == 4) {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE>>4, 0);
    mtp->provision(200);

    mtp->txn_begin();

    printf("\nmodify and search inside of transaction...\n");
    rc = mtp->test_modify_nested(1, 0, 0);
    rc = mtp->test_mod_search(1);

    /* MDB WAL now supports nested transaction
     */
    printf("\nmodify and search inside of transaction with child txn aborted...\n");
    rc = mtp->test_modify_nested(2, 1, 0);
    rc = mtp->test_mod_search(2);

    printf("\nmodify and search inside of transaction with child txn committed...\n");
    rc = mtp->test_modify_nested(3, 1, 1);
    rc = mtp->test_mod_search(3);

    mtp->txn_commit();

    printf("\nsearch outside of transaction...\n");
    rc = mtp->test_mod_search(1);
    rc = mtp->test_mod_search(2);
    rc = mtp->test_mod_search(3);

    printf("Pause 32 seconds for checkpoint...\n");
    SLEEP(32); //observe checkpoint
    delete mtp;
  }

  if (t_modify == 5) {
    /* test map full case */
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE>>7, 1);
    mtp->largedata_modify(5, 300, "mark5");
    printf("search added large data idx 5 ...\n");
    mtp->largedata_search(5);

    mtp->txn_begin();
    mtp->provision(80000);
    printf ("search data inside transaction...\n");
    mtp->test_provision_search(1000);
    mtp->test_provision_search(70001);
    rc = mtp->txn_commit();
    if (rc)
    {
      printf ("search again after commit failed...\n");
      mtp->test_provision_search(1000);
      mtp->test_provision_search(70001);
    }
    printf("search large data idx 5 after MAP_FULL ...\n");
    mtp->largedata_search(5);

    delete mtp;

    t_search = 1; //Go through to t_search 1 which will roll forward xlogs. Check database intact by doing -r1 again.
  }

  if (t_modify == 6) {
    unsigned long current_xlog_num = 0;
    unsigned long dbSizeMb = 0;
    unsigned long dbMapSizeMb = 0;
    char cmd[1024];
	char db_path[128];
    // Test xlogs rollforward case - skip xlogs that have been applied.
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE>>1, 1);
    mtp->provision(50000, 0); // idx 0 ~ 49999
    if (wal_enabled)
    {
      MdbTester::backup_db(); //backup database with 50K items in the database
    }
    mdb_env_set_state(mtp->get_dbenv(), MDB_STATE_KEEPXLOGS, &current_xlog_num, &dbSizeMb, &dbMapSizeMb, db_path, sizeof(db_path));
    printf ("current xlog number: %lu dbsize %lu MB dbMapSize %lu MB\n", current_xlog_num, dbSizeMb, dbMapSizeMb);

    //For database hop copy (Windows), only dbSizeMb is needed to be transfered instead of the whole sparse file
    MdbTester::truncate_backup_db((dbSizeMb + 16)<<20);
    mtp->provision(20000, 50000); //provision additional 20K items idx 50000 ~ 69999

    mtp->test_provision_search(0);
    mtp->test_provision_search(49999);
    mtp->test_provision_search(50000);
    mtp->test_provision_search(69999);

    delete mtp;

    if (wal_enabled)
    {
      MdbTester::restore_db();
    }
    t_search = 4; //t_search 4 will search for added data after backup_db.
  }

  if (t_delete == 1) {
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 1);

    if (wal_enabled)
    {
      cout << "Making a backup of mdb into backupdb"<<endl;
      MdbTester::backup_db();
    }

    mtp->provision(8000);
    rc = mtp->test_modify(1000);
    printf("add idx 1000 %s\n", rc?"failed":"succeeded");

    mtp->test_mod_search(1000);

    rc = mtp->test_del(1000, "modify key ");
    printf("delete idx 1000 %s\n", rc?"failed":"succeeded");

    printf("search deleted small data idx 1000...\n");
    mtp->test_mod_search(1000);

    mtp->largedata_modify(200, 4096*4); //four overflow pages
    printf("search added big data idx 200...\n");
    mtp->largedata_search(200);

    printf("deleting big data idx 200 ...\n");
    rc = mtp->test_del(200, "");
    printf("delete bigdata idx 200 %s\n", rc?"failed":"succeeded");

    printf("search deleted big data idx 200...\n");
    mtp->largedata_search(200);

    printf("Pause 32 seconds for checkpoint...\n");

    delete mtp;

    if (wal_enabled)
    {
      MdbTester::restore_db();
    }
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);

    mtp->test_provision_search(100);
    mtp->test_provision_search(2000);

    mtp->test_mod_search(1000);
    mtp->largedata_search(200);

    printf("add idx 1000 ...");
    rc=mtp->test_modify(1000);
    printf("add idx 1000 %s\n", rc?"failed":"succeeded");

    printf("search  modified data idx 1000...\n");
    mtp->test_mod_search(1000);

    printf("add bigdata idx 200 ...\n");
    rc = mtp->largedata_modify(200, 4096*4); //four overflow pages
    printf("add bigdata idx 200 %s\n", rc?"failed":"succeeded");

    printf("search bigdata again ...\n");
    mtp->largedata_search(200);

    delete mtp;
  }

  if (t_search == 1) {
    printf("\nt_search 1:\n");
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    mtp->test_mod_search(1);
    mtp->test_provision_search(1000);
    mtp->test_provision_search(70001);
    printf("search large data idx 5 which was committed before MAP_FULL ...\n");
    mtp->largedata_search(5);
    delete mtp;
  }

  if (t_search == 2) {
    printf("\nt_search 2:\n");
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    mtp->test_provision_search(100);
    mtp->test_provision_search(701);
    delete mtp;
  }

  if (t_search == 3) {
    printf("\nt_search 3:\n");
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    mtp->test_provision_search(100);
    mtp->test_provision_search(201);
    delete mtp;
  }

  if (t_search == 4) {
    printf("\nt_search 4:\n");
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE, 0);
    mtp->test_provision_search(0);
    mtp->test_provision_search(49999);
    mtp->test_provision_search(50000);
    mtp->test_provision_search(69999);
    delete mtp;
  }

  if (t_search == 5) {
    printf("\nt_search 5:\n");
    mtp = new MdbTester(env_flags, DB_DEFAULT_SIZE>>4, 0);

    mtp->largedata_search(4);
    mtp->largedata_search(5);

    mtp->test_provision_search(100);
    mtp->test_provision_search(200);
    mtp->test_provision_search(255);
    delete mtp;
  }

  printf("mdb_wal_test completed.\n");
  exit(0);
}
