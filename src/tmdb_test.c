#include <stdio.h>
#include <string.h>
#include <time.h>
#include "tmdb.h"



//-----------------------------
//
//    test functions
//
//-----------------------------

int _db_insert_test(int total, char *df);
int _db_read_test(int total, char *df);


/**
 * data insert
 *
 * @param int total insert total
 * @param char *df data file name
 */
int _db_insert_test(int total, char *df){
	char n[16];
	char *dat;
	int i = 0;
	int success = 0;
	int s = 0;

	if (total <= 0){
		return 0;
	}

	clock_t start, finish;   
	double duration;   
	start=clock();

	TDB *db = tdb_open(df, "c");
	if ( !db ){
		printf("db_open() %s fail.\n", df);
		return 0;
	}
	for (i=0; i<total; i++){
		sprintf(n, "num%d", i);
		s = tdb_store(db, n, n, TDB_INSERT);
		if (TDB_SUCCESS == s){
			success++;
		}
	}
	tdb_close(db);

	finish=clock();   
	duration=(double)(finish-start)/CLOCKS_PER_SEC;   
	printf("db insert data test success: %d, used time: %fs\n", success, duration);

	return success;
}

/**
 * data read
 *
 * @param int total read total
 */
int _db_read_test(int total, char *df){
	char n[16];
	char *dat = NULL;
	int i = 0;
	int success = 0;
	int s = 0;

	if (total <= 0){
		return 0;
	}

	clock_t start, finish;   
	double duration;   
	start=clock();

	TDB *db = tdb_open(df, "r");
	if ( !db ){
		printf("db_open() %s fail.\n", df);
		return 0;
	}
	for (i=0; i<total; i++){
		sprintf(n, "num%d", i);
		dat = tdb_fetch(db, n);
		if (NULL != dat){
			success++;
		}
	}
	tdb_close(db);

	finish=clock();   
	duration=(double)(finish-start)/CLOCKS_PER_SEC;   
	printf("db read data test success: %d, used time: %fs\n", success, duration);

	return success;
}




/**
 * test main call
 *
 */
int main(){

	/**
	 * function test
	 */

	/* 
	int s;

	TDB *db = _db_open("./test/abc", "c");
	p_tdb(db, "1");

	s = _db_store(db, "aaa", "aaa_test", TDB_INSERT);
	printf("_db_store: aaa : aaa_test : %d\n", s);

	s = _db_store(db, "aaa", "aaaaaaaaaa", TDB_REPLACE);
	printf("_db_store: aaa : aaaaaaaaaa : %d\n", s);

	s = _db_store(db, "bbb", "bbbbbbbbbb", TDB_STORE);
	printf("_db_store: bbb : bbbbbbbbbb : %d\n", s);

	s = _db_delete(db, "bbb");
	printf("_db_delete: bbb : %d\n", s);

	s = _db_store(db, "ccc", "cccccccccc", TDB_STORE);
	printf("_db_store: ccc : cccccccccc : %d\n", s);

	s = _db_store(db, "ddd", "ddddddddd", TDB_STORE);
	printf("_db_store: %s : %s : %d\n", "ddd", "ddddddddd", s);

	char *dat = _db_fetch(db, "aaa");
	printf("_db_fetch: %s : %s (exists)\n", "aaa", dat);

	char *dat1 = _db_fetch(db, "ccc");
	printf("_db_fetch: %s : %s (exists)\n", "ccc", dat1);

	char *dat2 = _db_fetch(db, "bbb");
	printf("_db_fetch: %s : %s (deleted)\n", "bbb", dat2);

	//each all item
	_db_rewind(db);
	char *ndat, *nkey = (char *)calloc(64, 1);
	while ((ndat = _db_nextrec(db, nkey)) != NULL){
		printf("Each item key:%s, val:%s\n", nkey, ndat);
		bzero(nkey, strlen(nkey));
		bzero(ndat, strlen(ndat));
	}
	*/


	//insert&read test

	printf("============= performance test ===========\n");
	char *df = "tdb_data_test";
	int total = 1000;
	_db_insert_test(total, df);
	_db_read_test(total, df);
	/*total = 500000;
	_db_insert_test(total, df);
	_db_read_test(total, df);
	total = 1000000;
	_db_insert_test(total, df);
	_db_read_test(total, df);
	*/

	return 0;

}

