tmdb
====
*作者：heiyeluren*

tmdb 是类似于DBM的小型Hash KV数据库 (Mini NoSQL DB)

TieMa?(Tiny&Mini) TDBM (like dbm-style database)(ANSI C, GCC require)

Last update：2010/7/12 v0.0.1<br />


---


#### tmdb实现核心原理

*参考代码：https://github.com/heiyeluren/tmdb/blob/master/src/tmdb.h*


The tmdb use static hash table to organize data, doing so mainly to faster query to the data, but the problem caused by the less able to accommodate the data. Currently holds the record total of 65,536, each Key length does not exceed 64 bytes.

Data storage using index files and data files separate ways, so in order to ensure the growth of the index records would not cause the file to the increase.

Hash table using open zipper method of conflict to resolve, using the more efficient hash function:

> h = ((h <<5) + h) ^ c'', with a starting hash of 5381.
> 
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


