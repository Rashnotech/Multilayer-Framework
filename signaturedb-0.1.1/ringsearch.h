#ifndef _RS_HEADER
#define _RS_HEADER

#define _RS_TRANS 1
#define _RS_HIT 2
#define _RS_FAIL 0

#define _FLAG_CASE_INSENSITVE 1
#define _RS_FAST_EXIT 10
#define _RS_FAST_SCAN 11

/* flags for info */
#define _SDB_IS_BINARY 1
#define _SDB_INSENSITIVE 2

/* Exit error codes */
#define _SDB_OK 0
#define _SDB_FILE_MATCHED 1
#define _SDB_DB_CANT_OPEN 100
#define _SDB_DB_CANT_LOAD 101
#define _SDB_DB_CANT_BUILD 102
#define _SDB_DB_CANT_SEARCH 103
#define _SDB_SCAN_NO_MEMORY 200
#define _SDB_SCAN_IMPROPER_START 201
#define _SDB_BUILD_NO_MEMORY 210
#define _SDB_PARENT_NULL 220
#define _SDB_STREAM_CANT_READ 300


/* This struct is present at the end of each search-string 
 * if the *info is non-null in the _snode, then this is what
 * will be at the end of it.*/
struct _infonode {
	char key[20];  /* The unique key-id of the string */
	char *comment; /* The comment to be returned to the scanner */
	int major;     /* Catagory MAJOR, ie, Spam, Hoax, Virus */
	int minor; 	 /* Catagory MINOR, ie, Hoax-Virus, Hoax-Offer, Hoax-Pity */
	int flags;     /* Depicts information about what is the string about / how to handle */
	};

/* The _snode is the single-char/byte node which builds up the
 * search-tree */
struct _snode {
	struct _snode *sibling; /* Points to other chars which may match */
	struct _snode *down;    /* Points to the next level to progress to if matched */
	char value;      	    /* The actual value we're supposed to match */
	struct _infonode *info; /* Information node, non-NULL if we're at the end of a string */
	};



struct _snode *RS_add_node( struct _snode *parent, char value, struct _infonode *info );
struct _snode *RS_init( void );
int RS_add_string( struct _snode *parent, char *ss, int length, struct _infonode *info );
int RS_search_string( struct _snode *parent, char *string );
int RS_search_stream( struct _snode *parent, char *streamname );
int RS_load_keys( struct _snode *parent, char *fname );
int RS_set_verbose( int level );
int RS_set_fastscan( int level );
int RS_set_fastexit( int level );

#endif _RS_HEADER