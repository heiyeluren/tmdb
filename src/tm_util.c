/**
 * TieMa(Tiny&Mini) DBM (like dbm-style database)
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
 * $Id: tm_util.c 2010-4-11, 2010-5-7 23:22, 2010-6-27 01:51 heiyeluren $
 */


#include "tm_util.h"

/**
 * Die alert message
 *
 */
void die(char *mess){
    perror(mess); 
    exit(1); 
}
//link to die
void err_dump(char *mess){
	die(mess);
}

/**
 * debug information
 */
void debug_log(short level, char *msg){
	char log_level[10];
	bzero(log_level, 10);
	switch(level){
		case LOG_LEVEL_DEBUG: strcpy(log_level, "DEBUG"); break;
		case LOG_LEVEL_TRACE: strcpy(log_level, "TRACE"); break;
		case LOG_LEVEL_NOTICE: strcpy(log_level, "NOTICE"); break;
		case LOG_LEVEL_WARNING: strcpy(log_level, "WARNING"); break;
		case LOG_LEVEL_FATAL: strcpy(log_level, "FATAL"); break;
		default: strcpy(log_level, "NOTICE");
	}
	printf("[%s] %s\n", log_level, msg);
}

/**
 * substr - Sub string from pos to length
 *
 */
char *substr( const char *s, int start_pos, int length, char *ret ){
    char buf[length+1];
    int i, j, end_pos;
	int str_len = strlen(s);

    if (str_len <= 0 || length < 0){
		return "";        
	}
	if (length == 0){
		length = str_len - start_pos;
	}
	start_pos = ( start_pos < 0 ? (str_len + start_pos) : ( start_pos==0 ? start_pos : start_pos-- ) );
	end_pos = start_pos + length;

    for(i=start_pos, j=0; i<end_pos && j<=length; i++, j++){
		buf[j] = s[i];        
	}
    buf[length] = '\0';
    strcpy(ret, buf);

    return(ret);
}

/**
 * explode -  separate string by separator
 *
 * @param string from - need separator 
 * @param char delim - separator
 * @param pointarray to - save return separate result
 * @param int item_num - return sub string total
 * 
 * @include stdlib.h 
 * @include string.h
 *
 * @example
 * char *s, **r;
 * int num;
 * explode(s, '\n', &r, &num);
 * for(i=0; i<num; i++){
 *     printf("%s\n", r[i]);
 * }
 * 
 */
void explode(char *from, char delim, char ***to, int *item_num){
    int i, j, k, n, temp_len;
    int max_len = strlen(from) + 1;
    char buf[max_len], **ret;
       
    for(i=0, n=1; from[i]!='\0'; i++){
        if (from[i] == delim) n++;
    }
    
    ret = (char **)malloc(n*sizeof(char *));
    for (i=0, k=0; k<n; k++){
        memset(buf, 0, max_len);     
        for(j=0; from[i]!='\0' && from[i]!=delim; i++, j++) buf[j] = from[i];
        i++;
        temp_len = strlen(buf)+1;
        ret[k] = malloc(temp_len);
        memcpy(ret[k], buf, temp_len);
    } 
    *to = ret;
    *item_num = n;
}

/**
 * strtolower - string to lowner
 *
 */
char *strtolower( char *s ){
	int i, len = sizeof(s);
	for( i = 0; i < len; i++ ){
		s[i] = ( s[i] >= 'A' && s[i] <= 'Z' ? s[i] + 'a' - 'A' : s[i] );
	}
	return(s);
}

/**
 * strtoupper - string to upper
 *
 */
char *strtoupper( char *s ){
	int i, len = sizeof(s);
	for( i = 0; i < len; i++ ){
		s[i] = ( s[i] >= 'a' && s[i] <= 'z' ? s[i] + 'A' - 'a' : s[i] );
	}
	return(s);
}

/**
 * strpos - find char at string position
 *
 */
int strpos (const char *s, char c){
	int i, len;
	if (!s || !c) return -1;
	len = strlen(s);
	for (i=0; i<len; i++){
		if (s[i] == c) return i;
	}
	return -1;	
}

/**
 * strrpos - find char at string last position
 *
 */
int strrpos (const char *s, char c){
	int i, len;
	if (!s || !c) return -1;
	len = strlen(s);
	for (i=len; i>=0; i--){
		if (s[i] == c) return i;
	}
	return -1;
}

/**
 * str_pad — Pad a string to a certain length with another string
 *
 */
int str_pad(char *s, int len, char c, char *to){
	if (!s || !c || !to) return -1;
	memset(to, c, len);
	memcpy(to, s, strlen(s));

	return 0;
}

/**
 * str_repeat — Repeat a string
 *
 */
int str_repeat(char input, int len, char *to){
	if (!input || !to) return -1;
	memset(to, input, len);
	return 0;
}



/**
 * trim - strip left&right space char
 *
 */
char *trim( char *s ){   
    int l;   
    for( l=strlen(s); l>0 && isspace((u_char)s[l-1]); l-- ){
		s[l-1] = '\0';   
	}
    return(s);   
}   


/**
 * ltrim - strip left space char
 *
 */
char *ltrim( char *s ){   
    char *p;   
    for(p=s; isspace((u_char)*p); p++ );   
    if( p!=s ) strcpy(s, p);   
    return(s);   
} 


/**
 * rtrim - strip right space char
 *
 */
char * rtrim(char *str){
	int p;
	p = strlen(str) - 1;
	while ( isspace(str[p]) )
		p--;
	str[p + 1] = '\0';
	return str;
}


/**
 * is_numeric - Check string is number 
 *
 */
int is_numeric( const char *s ){
	int i = 0;
	size_t len = strlen(s);
	for ( ; isdigit(s[i]); i++ ) ;
	return ( i==len ? 1 : 0 );
}

/**
 * Fetch current date tme
 *
 */
void getdate( char *s ){
	char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep);
	sprintf(s, "%d-%d-%d %d:%d:%d",(1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
} 

/**
 * Set socket nonblock
 *
 */
int socket_set_nonblock( int sockfd ){
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}


/**
 * File lock reister
 *
 */
int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;
	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;

	return ( fcntl(fd, cmd, &lock) );
}

/**
 * check file exists
 */
int file_exists (const char *path) {
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL){
		return -1;
	}
	return 0;
}

