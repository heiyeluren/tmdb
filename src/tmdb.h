/**
 * TieMa(Tiny&Mini) TMDB (like dbm-style database)
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
 * $Id: tm_util.h 2010-4-11, 2010-5-8 22:21, 2010-7-9 20:42 heiyeluren $
 *
 *
 
tmdb use static hash table to organize data, doing so mainly to faster query to the data, but the problem caused by the less able to accommodate the data. Currently holds the record total of 65,536, each Key length does not exceed 64 bytes.
Data storage using index files and data files separate ways, so in order to ensure the growth of the index records would not cause the file to the increase.
Hash table using open zipper method of conflict to resolve, using the more efficient hash function:

h = ((h <<5) + h) ^ c'', with a starting hash of 5381.

Describe the entire data storage structure:

	[ Index structure describe ] Index File: xxx.tdi

	Index File Struct:
    +-----------+----------------------+--------------+----------------+
    | Header    |    Key ptr buckets   | Key Record 1 |Key Record 2 .. |
    +-----------+----------------------+--------------+----------------+
	  256Bytes		262144Bytes(256KB)     76Bytes          76Bytes
	
	Index header
	+--------------+-------------------------+
	| Record total |        Free space       |
	+--------------+-------------------------+
	  4Bytes		         252Bytes
	
	Index key buckets
	+----------+----------+----------+--------------+
	| Key ptr1 | Key ptr2 | Key ptr3 | Key ptrN ..  |
	+----------+----------+----------+--------------+
	  4Bytes      4Bytes    4Bytes    4Bytes   ...
    
	Index key record
	+-------+--------------+----------+----------+
	| Flag  |    Key       | Data ptr | Next ptr |
	+-------+--------------+----------+----------+
	  4Bytes	 64Bytes      4Bytes     4Bytes


	[ Data structure describe ] Data File: xxx.tdb

	Data File structure:
    +----------------+----------------+-----------------+
    | Header         | Data Record 1  |Data Record 2..  |
    +----------------+----------------+-----------------+
	  256bytes		   dynamics length  dynamics length
	
	Data record
	+--------+-------+------------------+----------+
	| Flag   | len   |   Data           | Next ptr |
	+--------+-------+------------------+----------+
	  4Bytes  4Bytes   dynamics length    4Bytes

 */

#pragma pack(1)

#define	TDB_DEBUG		0		/* Open Debug mode */

/* Return value define */
#define TDB_SUCCESS		1
#define TDB_OK			0
#define TDB_ERROR		-1

#define TDB_FALSE		0
#define TDB_TRUE		1
#define TDB_EXIT		2

/* Hash key operate method (mode) */
#define TDB_INSERT		1
#define	TDB_REPLACE		2
#define TDB_STORE		3

/* Key find result define */
#define KEY_FIRST		1		/* Key is first and not exist */
#define KEY_NOTEXIST	2		/* key not exist */
#define KEY_EXIST		3		/* key exist */
#define KEY_ERROR		-1		/* find key occur a error */


/* Default options */
#define DBM_NAME				"tmdb"
#define VERSION					"0.0.1"

/* Default operate set */
#define TDB_DAT_SUFFIX			".tdb"		/* database file suffix */
#define TDB_IDX_SUFFIX			".tdi"		/* index file suffix */

#define TDB_INT_SIZE			4			/* all int size */
#define TDB_PRT_SIZE			4			/* all pointer size */

#define TDB_MAX_HASH_BUCKET		65536		/* max hash bucket total */
#define TDB_MAX_KEY_LEN			64			/* key string max byte length */
#define TDB_MAX_RECORD_LEN		65536		/* max a record data length */

#define TDB_INDEX_HEADER_LEN	256			/* index file header length */
#define TDB_DATA_HEADER_LEN		256			/* data file header length */

/* Record flag */
#define TDB_FLAG_DEL			1			/* record deleted */
#define TDB_FLAG_NOR			0			/* record default flag */

/* common set */
#define BUFFER_SIZE				8192        /* Default data buffer size */
#define MAX_BUF_SIZE			1048576		/* Key data max length bytes */


//#define IDXLEN_MIN	   6	/* key, sep, start, sep, length, \n */
//#define IDXLEN_MAX	1024	/* arbitrary */
//#define DATLEN_MIN	   2	/* data byte, newline */
//#define DATLEN_MAX	1024	/* arbitrary */





/**
 * Index structure
 */

/* Index header infomation */
/*
	Index header
	+--------------+-------------------------+
	| Record total |        Free space       |
	+--------------+-------------------------+
	  4Bytes		         252Bytes
*/
struct tdb_index_header_t {
	int record_total;						/* db all record total */
	char pre_alloc[252];					/* previous alloc header storage space */
};

struct tdb_index_key_ptr {
	int keys[TDB_MAX_HASH_BUCKET];			/* index key list (hash bucket) array */
};

/* Index one key structure */
/*
	Index key record
	+-------+--------------+----------+----------+
	| Flag  |    Key       | Data ptr | Next ptr |
	+-------+--------------+----------+----------+
	  4Bytes	 64Bytes      4Bytes     4Bytes
*/
struct tdb_key_record_t {
	int flag;								/* a key flag, delete */
	char key[TDB_MAX_KEY_LEN];				/* key data */
	int data_ptr;							/* target data recored pointer */
	int next_ptr;							/* next key pointer */
};


/* Data recored structure */
/*
	Data record
	+--------+-------+------------------+----------+
	| Flag   | len   |   Data           | Next ptr |
	+--------+-------+------------------+----------+
	  4Bytes  4Bytes   dynamics length    4Bytes
*/
struct tdb_data_record_head_t {
	int flag;								/* a data flag, delete */
	int len;								/* record data length */
};
struct tdb_data_record_t {
	int flag;								/* a data flag, delete */
	int len;								/* record data length */
	char *data;								/* record read data */
	int next;								/* next recored pointer */
};



/*
 * Library's private representation of the database.
 */
typedef unsigned long	TDBHASH;	/* hash values */
typedef unsigned long	COUNT;		/* unsigned counter */
typedef short  STATUS;



typedef struct {
	FILE *idx_fp;				/* file pointer for index file */
	FILE *dat_fp;				/* file pointer for data file */
	int idx_fd;					/* index file fd */
	int dat_fd;					/* data file fd */

	char  *idxbuf;				/* malloc'ed buffer for index record */
	char  *datbuf;				/* malloc'ed buffer for data record*/
	char  *name;				/* name db was opened under */
    char  *idx_name;			/* index file name */
    char  *dat_name;			/* data file name*/

	TDBHASH nhash;				/* hash table size */
	TDBHASH hash;				/* current hash postion */


	off_t  idxoff;				/* offset in index file of index record */
								/* key is at (idxoff + PTR_SZ + IDXLEN_SZ) */
	size_t idxlen;				/* length of index record */
								/* excludes IDXLEN_SZ bytes at front of record */
								/* includes newline at end of index record */
	off_t preidx;				/* previous key postion */

	off_t key_ptr_pos;			/* Key bucket item store Key pointer postion (address)*/


	off_t  datoff;				/* offset in data file of data record */
	size_t datlen;				/* length of data record */
								/* includes newline at end */

	off_t next_off;				/* Next record index offset */

	//off_t  ptrval;		/* contents of chain ptr in index record */
	//off_t  ptroff;		/* chain ptr offset pointing to this idx record */
	//off_t  chainoff;	/* offset of hash chain for this index record */
	//off_t  hashoff;		/* offset in index file of hash table */

	//COUNT  cnt_delok;    /* delete OK */
	//COUNT  cnt_delerr;   /* delete error */
	//COUNT  cnt_fetchok;  /* fetch OK */
	//COUNT  cnt_fetcherr; /* fetch error */
	//COUNT  cnt_nextrec;  /* nextrec */
	//COUNT  cnt_stor1;    /* store: TDB_INSERT, no empty, appended */
	//COUNT  cnt_stor2;    /* store: TDB_INSERT, found empty, reused */
	//COUNT  cnt_stor3;    /* store: TDB_REPLACE, diff len, appended */
	//COUNT  cnt_stor4;    /* store: TDB_REPLACE, same len, overwrote */
	//COUNT  cnt_storerr;  /* store error */
} TDB;




/**
 * define db operate api
 */
typedef	void *TDBHANDLE;

TDB      *tdb_open(const char *, char *);
void      tdb_close(TDB *);
char     *tdb_fetch(TDB *, const char *);
STATUS    tdb_store(TDB *, const char *, const char *, int);
STATUS    tdb_delete(TDB *, const char *);
void      tdb_rewind(TDB *);
char     *tdb_nextrec(TDB *, char *);



