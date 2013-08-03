/**
 * TieMa(Tiny&Mini) TDBM (like dbm-style database)
 * Copyright (C) 2010 heiyeluren. All rights reserved.
 *
 * tmdb is a easy use, high performance like dbm-style database
 *  
 * Use and distribution licensed under the BSD license.  See
 * the LICENSE file for full text.
 *
 * To learn more open-source code, visit: http://heiyeluren.googlecode.com
 * My blog: http://blog.csdn.net/heiyeshuwu
 *
 * $Id: tm_util.h 2010-5-1, 2010-7-9 20:42 heiyeluren $
 */

#include "tm_util.h"
#include "tmdb.h"



/*TDBHANDLE  tdb_open(const char *, int, ...);
void      tdb_close(TDBHANDLE);
char     *tdb_fetch(TDBHANDLE, const char *);
int       tdb_store(TDBHANDLE, const char *, const char *, int);
int       tdb_delete(TDBHANDLE, const char *);
void      tdb_rewind(TDBHANDLE);
char     *tdb_nextrec(TDBHANDLE, char *);
*/



/*
 * Internal functions.
 */
void	p_tdb(TDB *, char *);
void	p_data_record(struct tdb_data_record_t *);
void	p_key_record(struct tdb_key_record_t *);

TDB		*_db_open(const char *, char *);
TDB		*_db_alloc(int);
void	_db_free(TDB *);
STATUS	_db_create(TDB *);
TDBHASH _db_hash(TDB *, const char *);
STATUS	_db_find_and_lock(TDB *, const char *, int);
STATUS	_write_data_record(TDB *, struct tdb_data_record_t *);
STATUS	_db_store(TDB *, const char *, const char *, int);
STATUS	_db_delete(TDB *, const char *);
char	*_db_fetch(TDB *, const char *);
void	_db_rewind(TDB *);
char	*_db_nextrec(TDB *, char *);





//-----------------------------
//
//    internal functions
//
//-----------------------------


/**
 * TDB structure information debug funciton
 *
 */
void p_tdb(TDB *db, char *msg)
{
	printf("\n--------------TDB Structure Data %s--------------\n", msg);
	printf("Hash BUCKET: %d\n", db->nhash);
	printf("Index-name: %s\n", db->idx_name);
	printf("Data-name: %s\n", db->dat_name);
	printf("Index-fp: %d\n", (db->idx_fp!=NULL)?fileno(db->idx_fp):0);	
	printf("Data-fp: %d\n", (db->dat_fp!=NULL)?fileno(db->dat_fp):0);	
	printf("Index-Key-Len: %d\n", db->idxlen);
	printf("-------------------Done-----------------------\n");
}

/**
 * print data record
 */
void p_data_record(struct tdb_data_record_t *data_record){
	printf("\n--------------TDB Data Record--------------\n");
	printf("data_record->flag : %d\n", data_record->flag);
	printf("data_record->len : %d\n", data_record->len);
	printf("data_record->data : %s\n", data_record->data);
	printf("data_record->next : %d\n", data_record->next);
	printf("-------------------Done--------------------\n");
}

/**
 * print key record
 */
void p_key_record(struct tdb_key_record_t *key_record){
	printf("\n--------------TDB Key Record--------------\n");
	printf("key_record->flag : %d\n", key_record->flag);
	printf("key_record->key : %s\n", key_record->key);
	printf("key_record->data_ptr : %d\n", key_record->data_ptr);
	printf("-------------------Done-------------------\n");
}



/**
 * Open db 
 *
 * @param string path  access db path, example: /path/to/test, create db is /path/to/test.tdb 
 * @param char mode open db mode: r/c/w, r:read only, c:create/truncate db, w:read/write db
 * @return db handle
 */
TDB  *_db_open(const char *path, char *mode) {
    TDB  *db = NULL;
    int len = 0;
    size_t i = 0;

    //Check open mode
    if (strcmp(mode, "r")==-1 && strcmp(mode, "c")==-1 && strcmp(mode, "w")==-1){
        err_dump("db_open: param mode not r,c or w");
        return NULL;
    }
    len = strlen(path);
	if ( NULL == (db = _db_alloc(len))){
		err_dump("db_open: _db_alloc error for TDB");
        return NULL;
    }
	//Init name
	strcpy(db->idx_name, path);
	strcat(db->idx_name, TDB_IDX_SUFFIX);
	strcpy(db->dat_name, path);
	strcat(db->dat_name, TDB_DAT_SUFFIX);

	// Check file exist and create file
    int not_exists = 0;
	not_exists = file_exists(db->idx_name);
    if ( 0 == strcmp(mode, "c") || not_exists ){
        if ( NULL == (db->idx_fp = fopen(db->idx_name, "wb+"))){
            err_dump("db_open: open index file failed"); 
            return NULL;
        }
        if ( NULL == (db->dat_fp = fopen(db->dat_name, "wb+")) ){
            err_dump("db_open: open data file failed"); 
            return NULL;
        }
        if ( TDB_OK != _db_create(db) ){
			err_dump("_db_create create db failed");
			return NULL;
		}
    }
	// Read only
	else if ( 0 == strcmp(mode, "r") ){
        if ( NULL == (db->idx_fp = fopen(db->idx_name, "rb")) ){
            err_dump("db_open: open index file failed"); 
            return NULL;
        }
        if ( NULL == (db->dat_fp = fopen(db->dat_name, "rb")) ){
            err_dump("db_open: open data file failed"); 
            return NULL;
        }		
	}
	// Write/Read
	else if ( 0 == strcmp(mode, "w") ){
        if ( NULL == (db->idx_fp = fopen(db->idx_name, "rb+")) ){
            err_dump("db_open: open index file failed"); 
            return NULL;
        }
        if ( NULL == (db->dat_fp = fopen(db->dat_name, "rb+")) ){
            err_dump("db_open: open data file failed"); 
            return NULL;
        }		
	}

	//Get fd
	db->idx_fd = fileno(db->idx_fp);
	db->dat_fd = fileno(db->dat_fp);
   
	return db;    
}

/*
 * Allocate & initialize a TDB structure and its buffers.
 */
TDB *_db_alloc(int namelen)
{
	TDB		*db = NULL;

	/*
	 * Use calloc, to initialize the structure to zero.
	 */
	if ( NULL == (db = calloc(1, sizeof(TDB))) ) {
		err_dump("_db_alloc: calloc error for TDB");
		db->idx_fp = db->dat_fp = NULL;				/* descriptors */
		return NULL;
	}

	/*
	 * Allocate room for the name.
	 * +5 for ".idx" or ".dat" plus null at end.
	 */
	if ( NULL == (db->name = calloc(namelen + 5, sizeof(char)) ) )
		err_dump("_db_alloc: malloc error for name");
	if ( NULL == (db->idx_name = calloc(namelen + 5, sizeof(char))) )
		err_dump("_db_alloc: malloc error for idx_name");
	if ( NULL == (db->dat_name = calloc(namelen + 5, sizeof(char))) )
		err_dump("_db_alloc: malloc error for dat_name");

	//db->name[0] = '\0';
	//db->idx_name[0] = '\0';
	//db->dat_name[0] = '\0';


	/*
	 * Allocate an index buffer and a data buffer.
	 * +2 for newline and null at end.
	 */
	//if ((db->idxbuf = calloc(TDB_MAX_KEY_LEN + 2, sizeof(char))) == NULL)
	//	err_dump("_db_alloc: malloc error for index buffer");
	//if ((db->datbuf = calloc(TDB_MAX_RECORD_LEN + 2, sizeof(char))) == NULL)
	//	err_dump("_db_alloc: malloc error for data buffer");

	//Hash bucket
	db->nhash = TDB_MAX_HASH_BUCKET;

	//Index key len
	db->idxlen = sizeof(struct tdb_key_record_t);

	//Init pos
	db->next_off = db->idxoff = db->idxlen = db->preidx = db->key_ptr_pos = 0;

	return(db);
}

/*
 * Free up a TDB structure, and all the malloc'ed buffers it
 * may point to.  Also close the file descriptors if still open.
 */
void _db_free(TDB *db)
{
	if ( NULL == db ){
		return;
	}
	if ( NULL != db->idx_fp )
		fclose(db->idx_fp);
	if ( NULL != db->idx_fp )
		fclose(db->idx_fp);
	if ( NULL != db->idxbuf ){
		free(db->idxbuf);
		db->idxbuf = NULL;
	}
	if ( NULL != db->datbuf ){
		free(db->datbuf);
		db->datbuf = NULL;
	}
	if ( NULL != db->name ){
		free(db->name);
		db->name = NULL;
	}
	free(db);
	db = NULL;	
}

/**
 * Create init database
 *
 */
STATUS _db_create(TDB *db) {
    if ( !db ){
        return TDB_FALSE;
    }
    /**
     * init index file 
     */
    rewind(db->idx_fp);
    //Write index header record total
	int zero = 0;
	int i = 0;
    if ( fwrite(&zero, TDB_INT_SIZE, 1, db->idx_fp) != 1){
		err_dump("_db_create: write index header record total fail");
	}
    //Init index file: header + hash buckets
    off_t offset = TDB_INDEX_HEADER_LEN+(TDB_MAX_HASH_BUCKET*TDB_PRT_SIZE) - TDB_INT_SIZE; 
    if ( fseek(db->idx_fp, offset, SEEK_SET) != 0){
		err_dump("_db_create: init index file fseek() fail");
	}
	if ( fwrite(&zero, TDB_INT_SIZE, 1, db->idx_fp) != 1){
		err_dump("_db_create: init index fwrite() fail");
	}
	fflush(db->idx_fp);

	/**
     * init data file  (init data file header)
     */
    rewind(db->dat_fp);
    if ( fseek(db->dat_fp, TDB_DATA_HEADER_LEN-TDB_INT_SIZE, SEEK_SET) != 0 ){
		err_dump("_db_create: init data file fseek() fail");
	}
    if ( fwrite(&zero, TDB_INT_SIZE, 1, db->dat_fp) != 1 ){
		err_dump("_db_create: init data file fwrite() fail");
	}
	fflush(db->dat_fp);
	
	return TDB_OK;
}


/**
 * Hash core function
 *
 * @desc BKDR Hash 
 */
TDBHASH _db_hash(TDB *db, const char *str) {
	TDBHASH seed = 131; //31 131 1313 13131 131313 etc..
	TDBHASH hash = 0; 
	while (*str) {
		hash = hash * seed + (*str++);
	} 
	return (hash & 0x7FFFFFFF) % db->nhash;
}


/**
 * Find a key for index file
 *
 * @return int ret
 *
 */
STATUS _db_find_and_lock(TDB *db, const char *key, int is_write) {
	int key_pos = 0;
	int key_ptr_pos = 0; 
	int key_max_pos = 0;
	int key_len = 0; 
	int find = 0;
	struct tdb_key_record_t *key_record = NULL;

	db->hash = _db_hash(db, key);

	//Lock file
	if (is_write){
		flock(db->idx_fd, LOCK_EX);
		flock(db->dat_fd, LOCK_EX);
	} else {
		flock(db->idx_fd, LOCK_SH);
		flock(db->dat_fd, LOCK_SH);
	}

	//Find a key postion
	key_pos = TDB_INDEX_HEADER_LEN + (db->hash * TDB_INT_SIZE);
	key_max_pos = TDB_INDEX_HEADER_LEN + (TDB_MAX_HASH_BUCKET * TDB_INT_SIZE);
	key_len = sizeof(struct tdb_key_record_t);
	fseek(db->idx_fp, key_pos, SEEK_SET);
	if ( fread(&key_ptr_pos, TDB_PRT_SIZE, 1, db->idx_fp) != 1){
		if (TDB_DEBUG){
			off_t i_off = ftell(db->idx_fp);
			printf("_db_find_and_lock(): fread key_ptr_pos fail, idx_fp addr:%d\n", i_off);
		}
		return KEY_ERROR;
	}
	/*if (key_ptr_pos > key_max_pos){
		if (TDB_DEBUG){
			off_t i_off = ftell(db->idx_fp);
			printf("_db_find_and_lock(): key_ptr_pos:%d > key_max_pos:%d,  idx_fp addr:%d\n", key_ptr_pos, key_max_pos, i_off);
		}
		return KEY_ERROR;
	}*/
	db->key_ptr_pos = key_ptr_pos;
	if (key_ptr_pos == 0) {
		db->idxoff = 0;
		return KEY_FIRST;
	}	

	//Seek key to real key record
	if ( (key_record = (struct tdb_key_record_t *)malloc(key_len)) == NULL) {
		if (TDB_DEBUG){
			printf("_db_find_and_lock(): key_record alloc memory fail\n");
		}
		free(key_record);
		key_record = NULL;
		return KEY_ERROR;
	}
	find = 0;
	while (1) {
		//Key record not exist
		fseek(db->idx_fp, key_ptr_pos, SEEK_SET);
		fread(key_record, key_len, 1, db->idx_fp);
		db->preidx = ftell(db->idx_fp);
		if ( NULL == key_record || NULL == key_record->key ){
			db->idxoff = key_ptr_pos;
			free(key_record);
			key_record = NULL;
			return KEY_ERROR;
		}
		//Compare record[key] == key 
		if ( strcmp(rtrim(key_record->key), key) == 0){
			find = 1;
			break;
		}
		//Key postion pointer to next pointer
		if ( 0 == key_record->next_ptr ){
			break;
		}
		key_ptr_pos = key_record->next_ptr;
	}

	//Save key record postion	
	if ( !find ) {
		db->idxoff = key_ptr_pos;
		free(key_record);
		key_record = NULL;
		return KEY_NOTEXIST;
	}
	db->idxoff = key_ptr_pos;
	db->datoff = key_record->data_ptr;

	//Row level lock
	//is_write ? write_lock(db->idx_fd, key_ptr_pos, SEEK_SET, key_len) : read_lock(db->idx_fd, key_ptr_pos, SEEK_SET, key_len);

	return KEY_EXIST;
}


/**
 * write a data record to data file
 *
 	Data record
	+--------+-------+------------------+----------+
	| Flag   | len   |   Data           | Next ptr |
	+--------+-------+------------------+----------+
	  4Bytes  4Bytes   dynamics length    4Bytes
 */
STATUS _write_data_record(TDB *db, struct tdb_data_record_t *data_record){
	fwrite(&(data_record->flag), sizeof(data_record->flag), 1, db->dat_fp);
	fwrite(&(data_record->len), sizeof(data_record->len), 1, db->dat_fp);
	fwrite(data_record->data, data_record->len, 1, db->dat_fp);
	fwrite(&(data_record->next), sizeof(data_record->next), 1, db->dat_fp);

	return TDB_SUCCESS;
}


/**
 * Store data
 *
 * @param TDB *db  db handler
 * @param char *key Data key
 * @param char *value Data value
 * @param int mode TDB_INSERT/TDB_REPLACE/TDB_STORE
 *		  TDB_INSERT - record exist return TDB_OK, else store data
 *		  TDB_REPLACE - record not exist return TDB_OK, else replace data
 *		  TDB_STORE - drecord exist: replace, not exist: insert
 */
STATUS	_db_store(TDB *db, const char *key, const char *value, int mode){
	//Check param
	if ( NULL == db || NULL == key || NULL == value ){
		return TDB_ERROR;
	}
	if ( TDB_INSERT != mode && TDB_REPLACE != mode && TDB_STORE != mode){
		if (TDB_DEBUG){
			printf("_db_store() ill insert mode:%d, key:%s\n", mode, key);
		}
		return TDB_ERROR;
	}

	int keylen = 0;
	int datalen = 0;

	keylen = strlen(key);	
	datalen = strlen(value);

	if ( keylen <= 0 || keylen > TDB_MAX_KEY_LEN){
		if (TDB_DEBUG){
			printf("_db_store() ill key, too long or is null, key:%s\n", key);
		}
		return TDB_ERROR;
	}
	if ( datalen > TDB_MAX_RECORD_LEN){
		if (TDB_DEBUG){
			printf("_db_store() ill value, too long or is null, key:%s\n", key);
		}
		return TDB_ERROR;
	}

	//Find key
	STATUS ret = _db_find_and_lock(db, key, TDB_TRUE);
	if (TDB_DEBUG){
		printf("_db_store() call _db_find_and_lock() key:%s  status:%d\n", key, ret);
	}
	if (ret == KEY_ERROR){
		return TDB_ERROR;
	}

	//If key exist and record is insert mode
	if (TDB_INSERT == mode && KEY_EXIST == ret ){
		if (TDB_DEBUG){
			printf("_db_store() mode:TDB_INSERT, key:%s exist\n", key);
		}
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);
		return TDB_OK;
	}
	//If key not exist and is replace mode
	if (TDB_REPLACE == mode && (KEY_FIRST == ret || KEY_NOTEXIST == ret)){
		if (TDB_DEBUG){
			printf("_db_store() mode:TDB_REPLACE, key:%s not exist\n", key);
		}
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);
		return TDB_OK;
	}


	/**
	 * Save record data
	 */
	//Pkg data record
	struct tdb_data_record_t *data_record = NULL;
	
	data_record = malloc(sizeof(struct tdb_data_record_t));

	data_record->flag = TDB_FLAG_NOR;
	data_record->len  = strlen(value);
	data_record->data = (char *)malloc(data_record->len+1);
	strcpy(data_record->data, value);
	data_record->data[data_record->len+1] = '\0';
	data_record->next = 0;

	if (TDB_DEBUG){
		p_data_record(data_record);
	}

	//Write data record
	int record_pos = 0; 
	int data_slen = 0; 

	fseek(db->dat_fp, 0, SEEK_END);
	record_pos = ftell(db->dat_fp);
	data_slen = sizeof(struct tdb_data_record_t);

	//If first data record
	if ( record_pos < (TDB_DATA_HEADER_LEN+data_slen) ){
		_write_data_record(db, data_record);
		//fwrite(data_record, data_slen, 1, db->dat_fp);
		if (TDB_DEBUG){
			printf("write first data record key:%s\n", key);
		}
	} else {
		//update pre record next ptr address 
		fseek(db->dat_fp, (0-TDB_PRT_SIZE), SEEK_END);
		fwrite(&record_pos, TDB_PRT_SIZE, 1, db->dat_fp);
		//write new data record
		_write_data_record(db, data_record);
		//fwrite(data_record, data_slen, 1, db->dat_fp);
		if (TDB_DEBUG){
			printf("write normal data record key:%s\n", key);
		}
	}
	fflush(db->dat_fp);


	/**
	 * Save index key
	 */
	//Pkg key record
	struct tdb_key_record_t *key_record = NULL;
	int key_pos = 0;
	int key_ptr_pos = 0;
	int key_slen = 0; 
	int	key_len = 0;

    //key record
	key_record = malloc(sizeof(struct tdb_key_record_t));
	key_slen = sizeof(struct tdb_key_record_t);
    key_len  = strlen(key);
	key_record->flag = 0; //TDB_FLAG_NOR;
	bzero(key_record->key, TDB_MAX_KEY_LEN);
	strncpy(key_record->key, key, key_len);
	key_record->data_ptr = record_pos;

	if (TDB_DEBUG){
		p_key_record(key_record);
	}

	//Key not exist (write first key)
	if (KEY_FIRST == ret || KEY_NOTEXIST == ret){
		//write key record
		key_record->next_ptr = 0;
		fseek(db->idx_fp, 0, SEEK_END);
		key_ptr_pos = ftell(db->idx_fp);
		fwrite(key_record, key_slen, 1, db->idx_fp);

		//first key update hash bucket pointer addr
		if (KEY_FIRST == ret){
			key_pos = TDB_INDEX_HEADER_LEN + (db->hash * TDB_INT_SIZE);
			fseek(db->idx_fp, key_pos, SEEK_SET);
			if (TDB_DEBUG){
				printf("write not exist key (first key), key:%s\n", key);
			}
		} 
		//Normal key write data_record->next_ptr address
		else if (KEY_NOTEXIST == ret){
			fseek(db->idx_fp, (db->preidx - TDB_PRT_SIZE), SEEK_SET);
			if (TDB_DEBUG){
				printf("write not exist key (normal key), key:%s\n", key);
			}
		}
		fwrite(&key_ptr_pos, TDB_PRT_SIZE, 1, db->idx_fp);
	
		//flush & unlock
		fflush(db->idx_fp);
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);

		//free
		free(data_record);
		data_record = NULL;

		free(key_record);
		key_record = NULL;

		return TDB_SUCCESS;
	}


	//Record is exist (replace old key)
	if (KEY_EXIST == ret){
		//fseek offset to key_record->data_ptr postion
		fseek(db->idx_fp, (db->idxoff + TDB_INT_SIZE + TDB_MAX_KEY_LEN), SEEK_SET);
		//modify key_record->data_ptr to new record address
		fwrite(&record_pos, TDB_PRT_SIZE, 1, db->idx_fp);

		//flush & unlock
		fflush(db->idx_fp);
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);

		if (TDB_DEBUG){
			printf("replace exist key (normal key), key:%s\n", key);
		}
		return TDB_SUCCESS;
	}
	return TDB_OK;
}


/**
 * delete record
 *
 * @param TDB *db 
 * @param char *key 
 */
STATUS	_db_delete(TDB *db, const char *key){
	//Check param
	if (NULL == db || NULL == key){
		return TDB_ERROR;
	}
	int keylen = 0;
	keylen = strlen(key);
	if ( keylen <= 0 || keylen > TDB_MAX_KEY_LEN){
		if (TDB_DEBUG){
			printf("_db_delete() ill key, too long or is null, key:%s\n", key);
		}
		return TDB_ERROR;
	}

	//Find key
	STATUS ret = 0;
	ret = _db_find_and_lock(db, key, TDB_TRUE);
	if (TDB_DEBUG){
		printf("_db_delete() call _db_find_and_lock() key:%s  status:%d\n", key, ret);
	}
	if (KEY_ERROR == ret){
		return TDB_ERROR;
	}
	if (KEY_FIRST == ret || KEY_NOTEXIST == ret){
		return TDB_OK;
	}

	//set deleted flag
	int flag_del = TDB_FLAG_DEL;
	int key_next_ptr = 0;

	//If key is first key
	if (KEY_EXIST == ret && db->key_ptr_pos == db->idxoff){
		//Key record set flag is deleted
		//fseek offset to key_record->flag postion
		//modify key_record->data_ptr to new record address
		fseek(db->idx_fp, db->idxoff, SEEK_SET);
		fwrite(&flag_del, TDB_INT_SIZE, 1, db->idx_fp);

		//read next pointer
		fseek(db->idx_fp, (TDB_MAX_KEY_LEN+TDB_PRT_SIZE), SEEK_CUR);
		fread(&key_next_ptr, TDB_PRT_SIZE, 1, db->idx_fp);
		//key_next_ptr = (key_next_ptr==NULL || key_next_ptr==0) ? 0 : key_next_ptr;

		//Set a key postion
		int key_pos = 0;
		key_pos = TDB_INDEX_HEADER_LEN + (db->hash * TDB_INT_SIZE);
		fseek(db->idx_fp, key_pos, SEEK_SET);
		fwrite(&key_next_ptr, TDB_INT_SIZE, 1, db->idx_fp);

		//flush & unlock
		fflush(db->idx_fp);
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);

		if (TDB_DEBUG){
			char *s = key_next_ptr ? "has next ptr" : "only";
			printf("delete exist key done, key:%s (first key - %s)\n", key, s);
		}

		return TDB_SUCCESS;
	}

	//If not first key
	if ( KEY_EXIST == ret ){
		//Key record set flag is deleted
		//fseek offset to key_record->flag postion
		//modify key_record->data_ptr to new record address
		fseek(db->idx_fp, db->idxoff, SEEK_SET);
		fwrite(&flag_del, TDB_INT_SIZE, 1, db->idx_fp);

		//seek to next pointer, read next pointer
		fseek(db->idx_fp, (TDB_MAX_KEY_LEN+TDB_PRT_SIZE), SEEK_CUR);
		fread(&key_next_ptr, TDB_PRT_SIZE, 1, db->idx_fp);
		
		//write previous key next pointer
		fseek(db->idx_fp, (db->preidx+TDB_INT_SIZE+TDB_MAX_KEY_LEN+TDB_PRT_SIZE), SEEK_SET);
		fwrite(&key_next_ptr, TDB_PRT_SIZE, 1, db->idx_fp);

		//flush & unlock
		fflush(db->idx_fp);
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);

		if (TDB_DEBUG){
			printf("delete exist key done, key:%s (normal key)\n", key);
		}

		return TDB_SUCCESS;
	}

	return TDB_OK;
}


/**
 * Fetch data
 *
 */
char *_db_fetch(TDB *db, const char *key){
	//Check param
	if ( NULL == db || NULL == key ){
		return NULL;
	}
	int keylen = 0;
	keylen = strlen(key);
	if ( keylen <= 0 || keylen > TDB_MAX_KEY_LEN){
		if (TDB_DEBUG){
			printf("_db_fetch() ill key, too long or is null, key:%s\n", key);
		}
		return NULL;
	}

	//Find key not exist or occur error
	STATUS ret = 0;
	ret = _db_find_and_lock(db, key, TDB_FALSE);
	if (TDB_DEBUG){
		printf("_db_fetch() call _db_find_and_lock() key:%s  status:%d\n", key, ret);
	}
	if ( KEY_ERROR == ret ){
		return NULL;
	}
	if ( KEY_FIRST == ret || KEY_NOTEXIST == ret ){
		return NULL;
	}

	//Find key exist, read data
	if ( KEY_EXIST == ret ){
		int len = 0, dat_len = 0;
		char *dat_buf = NULL; 
		char *ret_buf = NULL;

		dat_buf = calloc(TDB_MAX_RECORD_LEN, 1);
		fseek(db->dat_fp, (db->datoff+TDB_INT_SIZE), SEEK_SET);
		fread(&len, TDB_PRT_SIZE, 1, db->dat_fp);
		fread(dat_buf, sizeof(char), len, db->dat_fp);

		dat_len = strlen(dat_buf);
		ret_buf = calloc(dat_len+1, 1);
		memcpy(ret_buf, dat_buf, dat_len+1);
		free(dat_buf);
		dat_buf = NULL;

		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);
		free(dat_buf);
		dat_buf = NULL;

		return (char *)ret_buf;
	}

	return NULL;
}


/**
 * db rewind
 *
 */
void _db_rewind(TDB *db){
	//Check param
	if ( NULL == db ){
		return;
	}
	//Lock
	flock(db->idx_fd, LOCK_SH);
	flock(db->dat_fd, LOCK_SH);

	//fseek offset index to key start, data fp to file start
	int key_pos = 0;
	key_pos = TDB_INDEX_HEADER_LEN + (db->nhash * TDB_INT_SIZE);
	fseek(db->idx_fp, key_pos, SEEK_SET);
	fseek(db->dat_fp, 0, SEEK_SET);

	//Unlock
	flock(db->idx_fd, LOCK_UN);
	flock(db->dat_fd, LOCK_UN);

	return;
}


/**
 * fetch next record
 *
 */
char *_db_nextrec(TDB *db, char *ret_key){
	if (TDB_DEBUG){
		printf("\n\n================DB Read Next Record====================");	
	}
	if ( NULL == db ){
		if ( NULL != ret_key ){
			ret_key = NULL;
		}
		return NULL;
	}
	//Check last record
	if ( TDB_ERROR == db->next_off ){
		if (ret_key != NULL){
			ret_key = NULL;
		}
		return NULL;		
	}
	//Check is first record
	if ( 0 == db->next_off){
		_db_rewind(db);
		db->next_off = ftell(db->idx_fp);
	}

	//Lock file
	flock(db->idx_fd, LOCK_SH);
	flock(db->dat_fd, LOCK_SH);

	struct tdb_key_record_t *key_record = NULL;
	int key_len = 0;

	//Seek key to real key record
	if ( NULL == (key_record = (struct tdb_key_record_t *)calloc(key_len, 1)) ) {
		if (TDB_DEBUG){
			printf("_db_nextrec(): key_record alloc memory fail\n");
		}
		if ( NULL != ret_key ){
			ret_key = NULL;
		}
		return NULL;
	}

	//fseek to record
	key_len = sizeof(struct tdb_key_record_t);
	fseek(db->idx_fp, db->next_off, SEEK_SET);

	//loop read not deleted record
	int len = 0, dat_len = 0, rkey_len = 0;
	char *key_buf = NULL;
	char *ret_buf = NULL;
	char *dat_buf = NULL;

	while (1) {
		fread(key_record, key_len, 1, db->idx_fp);
		db->next_off = ftell(db->idx_fp);
		if (TDB_DEBUG){
			p_key_record(key_record);
			printf("_db_nextrec(): fread key_record, memory addr:%d\n", &key_record);
		}
		if ( NULL == key_record || NULL == key_record->key || 
			rtrim(key_record->key) == NULL || 0 == key_record->data_ptr ){
			if (NULL != key_record ) {
				free(key_record);
				key_record = NULL;
			}	
			if ( NULL != dat_buf ){
				free(dat_buf);
				dat_buf = NULL;
			}
			if (TDB_DEBUG){
				printf("_db_nextrec(): read data is null\n");
				printf("=======================================================\n");	
			}
			/*if (ret_key != NULL){
				ret_key = NULL;
			}*/
			return NULL;
		}
		//Skip deleted record
		if ( TDB_FLAG_DEL == key_record->flag ){
			if (TDB_DEBUG){
				printf("_db_nextrec(): read data is deleted, read next record.\n");
			}
			continue;
		}

		//read key
		if ( NULL != ret_key ){
			strcpy(ret_key, key_record->key);
			if (TDB_DEBUG){
				printf("_db_nextrec(): read ret_key is ok.\n");
			}
		}

		//read data
		dat_buf = calloc(TDB_MAX_RECORD_LEN, 1);
		if ( NULL == dat_buf ){
			if (TDB_DEBUG){
				printf("_db_nextrec(): dat_buf alloc memory fail\n");
			}
			return NULL;
		}
		fseek(db->dat_fp, (key_record->data_ptr+TDB_INT_SIZE), SEEK_SET);
		fread(&len, TDB_PRT_SIZE, 1, db->dat_fp);
		fread(dat_buf, sizeof(char), len, db->dat_fp);
		if (TDB_DEBUG){
			printf("_db_nextrec(): key:%s len:%d, data:%s\n", key_record->key, len, dat_buf);
		}

		dat_len = strlen(dat_buf);
		ret_buf = calloc(dat_len+1, 1);
		memcpy(ret_buf, dat_buf, dat_len+1);
		if (TDB_DEBUG){
			printf("_db_nextrec(): ret data:%s\n", ret_buf);
		}		

		//Free memory
		if ( NULL != key_record ) {
			//free(key_record);
			key_record = NULL;
		}	
		if ( NULL != dat_buf ){
			free(dat_buf);
			dat_buf = NULL;
		}

		if (TDB_DEBUG){
			printf("_db_nextrec(): read data success, return ret_buf.\n");
			printf("=======================================================\n");	
		}

		//Unlock
		flock(db->idx_fd, LOCK_UN);
		flock(db->dat_fd, LOCK_UN);

		return ret_buf;
	}
	return NULL;
}





//-----------------------------
//
//    external functions
//
//-----------------------------


/**
 * Open db
 */
TDB *tdb_open(const char *path, char *mode) {
	return _db_open(path, mode);
}


/**
 * Close db
 */
void tdb_close(TDB *db) {
	_db_free(db);
}


/**
 * Fetch key
 */
char *tdb_fetch(TDB *db, const char *key){
	return _db_fetch(db, key);
}


/**
 * Store data
 */
STATUS tdb_store(TDB *db, const char *key, const char *value, int mode) {
	return _db_store(db, key, value, mode);
}


/**
 * Delete key
 */
STATUS  tdb_delete(TDB *db, const char *key) {
	return _db_delete(db, key);
}


/**
 * Rewind db
 */
void  tdb_rewind(TDB *db) {
	_db_rewind(db);
}


/**
 * Read next key record
 */
char *tdb_nextrec(TDB *db, char *ret_key){
	return _db_nextrec(db, ret_key);
}




