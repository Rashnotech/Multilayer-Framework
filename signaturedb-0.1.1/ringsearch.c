/*----------------------------------------------------
 * ringsearch.c
 *
 * Written/Designed by Paul L Daniels (C)2001 PLDaniels
 *
 * RingSearch is a system which allows us to build a 
 * search tree for massive numbers of keys, and permitting
 * a search order time of O(log(n)) where 'n' is the lenght
 * of the longest string to search for.
 *
 * the final structure is very much like a b-tree.
 *
 * There are improvements which can be made in order to speed
 * up the search process, such as ordering the siblings so
 * that they are in "order" rather than as they are added.
 *
 * Also, case-insensitive operation can be facilitated at minimal
 * expense by adding an additional 'value' field which is the 
 * apposing case to the 'value' given (assuming this is a a-z, A-Z
 * value).  
 * 
 * These things will be left until further development and stability
 *
 * Regards, Paul L Daniels, pldaniels@pldaniels.com
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "ringsearch.h"

#define _V_END 0
#define _RS_VERSION "0.1.3"
#define _RS_ADD_SIBLING 1
#define _RS_ADD_DOWN 2
#define _BUFFER_SIZE 10240

char *RB_front, *RB_back, *RB_start, *RB_stop, *RB_tmp;
int RB_size;
int RB_waterlevel = 0;
int RS_length = 0;

/* Our hex conversion table */
int hexconv[256];

/* Our file-read buffer */
char _databuffer[_BUFFER_SIZE];
int _RS_FB_highmark = 0;
int _RS_FB_lowmark = 0;
int _RS_FB_eof = 0;
FILE *_RS_FB_stream = NULL;

/* Operational flags */
int verbose = 0;
int fastexit;
int fastscan;
char *currentfilename = NULL;


/*------------------------------------------------
 * get_fname
 *
 * Simple function just to return the file-name 
 * portion of a full path */
char  *get_fname( char *fullname ){
	char *p;
	p = strrchr(fullname,'/');
	p++;
	return p;
	}



/*-----------------------------------------------
 * RS_init_FB()
 *
 * Initialises and cleans the file buffer */
int RS_FB_init( void ){
	_RS_FB_highmark = -1;
	_RS_FB_lowmark = 0;
	_RS_FB_eof = 0;	
	return 0;
	}	

/*------------------------------------------------
 * RS_FB_setstream
 *
 * Sets the stream which the file-buffer routines
 * should be using to get more data from */
int RS_FB_setstream( FILE *f ){
	
	_RS_FB_stream = f;
	
	return 0;
	}
	
	
/*------------------------------------------------
 * RS_FB_feof
 *
 * Just like the normal feof() call, except it 
 * returns the status of the FileBuffer stream */
int RS_FB_feof( void ){
	
	/* if we've had the EOF flag marked from a previous
	 * RS_FB_getblock() call, then we just check to see
	 * if our data has run out yet from this last block */
	if (_RS_FB_eof && (_RS_FB_lowmark > _RS_FB_highmark))
		return 1;
	else return 0;
	}

/*------------------------------------------------
 * RS_FB_getblock
 *
 * Gets a _BLOCK_SIZE data block from the stream as
 * specified in _RS_FB_stream.  If the stream ends
 * then we set our _RS_FB_eof flag in here */	
int RS_FB_getblock( void ){
	
	size_t bytesread;

	/* try read in _BUFFER_SIZE bytes from the stream */
	bytesread = fread( _databuffer, sizeof(char), _BUFFER_SIZE, _RS_FB_stream);

	/* if we didn't get all of what we asked, for, then we have
	 * reached the end of the file, hence mark the EOF flag */
	if (bytesread != _BUFFER_SIZE) {
		_RS_FB_eof = 1;	
		}

	/* Now that we have a full tank of "data", set the high/low
	 * marks back to their "full" values */
	_RS_FB_highmark = bytesread -1;
	_RS_FB_lowmark = 0;	
	
	/* as a courtosy, return the number of bytes read just incase
	 * we want to do something from the calling function */
	return bytesread;
	}
	
	
/*------------------------------------------------
 * RS_FB_getbyte
 *
 * sets the suppled 'c' char with the next byte in
 * the filebuffer.  NOTE - RS_FB_eof() should be 
 * called before this function, because we cannot
 * give a suitable response back via the char value */
int RS_FB_getbyte( char *c ){
	
	/* if we've run out of buffer space... */
	if (_RS_FB_highmark < _RS_FB_lowmark) {
		RS_FB_getblock();
		}
		
	/* return the next char */
	*c = _databuffer[_RS_FB_lowmark];
	
	/* increment our buffer mark */
	_RS_FB_lowmark++;
	
	return 1;
	}
	


/*-----------------------------------------------
 * RS_set_verbose
 *
 * Makes RS talk as it walks */
int RS_set_verbose( int level ){
	
	verbose = level;
	
	return 0;
	}
	
/*-----------------------------------------------
 * RS_set_fastexit
 * 
 * Makes RS exit as soon as it detects on occurance
 * of a key in its currently scanned file */
int RS_set_fastexit( int level ){
	
	fastexit = level;
	return 0;
	}
	
	
	
/*-----------------------------------------------
 * RS_set_fastscan
 * 
 * Makes RS exit as soon as it detects on occurance
 * of a key in its currently scanned file */
int RS_set_fastscan( int level ){
	
	fastscan = level;
	return 0;
	}
	
	
	
/*---------------------------------------------------
 * RS_init_hexconv()
 *
 * Initialises the hex->dec conversion
 *
 */
int RS_init_hexconv( void ){
	
	hexconv['0'] = 0;
	hexconv['1'] = 1;	
	hexconv['2'] = 2;	
	hexconv['3'] = 3;	
	hexconv['4'] = 4;	
	hexconv['5'] = 5;	
	hexconv['6'] = 6;	
	hexconv['7'] = 7;	
	hexconv['8'] = 8;	
	hexconv['9'] = 9;	
	hexconv['a'] = 10;	
	hexconv['b'] = 11;	
	hexconv['c'] = 12;	
	hexconv['d'] = 13;	
	hexconv['e'] = 14;	
	hexconv['f'] = 15;	
	hexconv['A'] = 10;	
	hexconv['B'] = 11;	
	hexconv['C'] = 12;	
	hexconv['D'] = 13;	
	hexconv['E'] = 14;	
	hexconv['F'] = 15;	
	
	return 0;
	}
	


/*-------------------------------------------------
 * RS_decode_PQ
 *
 * This function just decodes a single print-quotable line into
 * it's normal/original format
 */
int RS_decode_PQ( char *line ) {
	
	char c;								/* The Character to output */
	int op, ip; 						/* OutputPointer and InputPointer */
	int slen = strlen(line); /* Length of our line */

	/* Initialise our "pointers" to the start of the encoded string */
	ip=op=0;

	/* for every character in the string... */
	for (ip = 0; ip < slen; ip++){
		
		c = line[ip];
		
		/* if we have the quote-printable esc char, then lets get cracking */
		if (c == '=') {

			/* if we have another two chars... */
			if (ip <= (slen-2)){

				/* convert our encoded character from HEX -> decimal */
				c = (char)hexconv[(int)line[ip+1]]*16 +hexconv[(int)line[ip+2]];
							
				/* shuffle the pointer up two spaces */
				ip+=2;
				} /* if there were two extra chars after the ='s */
				
			/* if we didn't have enough characters, then  we'll make the char the 
			 * string terminator (such as what happens when we get a =\n
			 */
			else {
				line[ip] = '\0';
				} /* else */
				
			} /* if c was a encoding char */
			
		/* put in the new character, be it converted or not */
		line[op] = c;
		
		/* shuffle up the output line pointer */
		op++;
		} /* for */
		
	/* terminate the line */
	line[op]='\0';		
		
	return 0;
	
	}
	
	
/*------------------------------------------------
 * RS_init
 *
 * Initialises the required structures for using
 * the ringsearch system. */	
struct _snode *RS_init( void ){

	struct _snode *c;

	/* try and allocate memory... */	
	c = malloc(sizeof(struct _snode));
	
	/* if allocation failed, then report... */
	if (!c) {
		fprintf(stderr,"Error: RS_init(), could not allocate memory for node. (%s)\n",strerror(errno));
		exit(_SDB_BUILD_NO_MEMORY);
		}
		
	
	/* else setup the values required */	
	c->sibling = NULL;
	c->down = NULL;
	c->value = 0;
	
	return c;
	}
	
	
	

/*-------------------------------------
 * RB_init
 *
 * Initialises and allocates memory for 
 * the search-window/Ring-buffer */	
int RB_init(void){
	
	/* Try and allocate enough space for our search-window */
	RB_front = malloc((RS_length+1) *sizeof(char));
	
	/* if there is a problem with the allocation, then we can't go any further */
	if (!RB_front) {
		fprintf(stderr,"Error: Cannot allocate memory for search-window! (%s)\n",strerror(errno));
		exit(_SDB_SCAN_NO_MEMORY);
		}
		
	/* if all is okay, then fill the buffer up with 0's */
	memset(RB_front, '\0', RS_length+1);
	
	/* Set our "waterlevel" (the current last-char) to zero */
	RB_waterlevel = 0;
	
	return 0;
	}


/*-------------------------------------------
 * RB_done
 *
 * Free up resources used by the RingBuffer */
int RB_done(void){
	
	/* If the RB_front actually has been alloc'd previously, then free it */
	if (RB_front) free(RB_front);
	
	return 0;
	}
	
/*--------------------------------------------
 * RB_prefill
 *
 * Prefill places new characters as close to the
 * FRONT of the buffer-window as possible */
int RB_prefill( char c ){

	/* Make sure we have got memory space to use */
	if ((!RB_front)||(!(RB_front +RB_waterlevel))){
		fprintf(stderr,"Error when in RB_prefill, was RB_init() called?\n");
		exit(_SDB_SCAN_IMPROPER_START);
		}

	/* Set the new character into place */
	*(RB_front +RB_waterlevel) = c;
	
	/* Push up our water level */
	RB_waterlevel++;
	
	/* Return our new water level */
	return RB_waterlevel;
	}
	

/*--------------------------------------------
 * RB_pushchar
 *
 * Places a new char at the -end- of the window
 * and drops the leading char  */
int RB_pushchar( char c ){
	
	if (!RB_front){
		fprintf(stderr,"Error: RB_pushchar, RB_Front is NULL, was RB_init() called?\n");
		exit(_SDB_SCAN_IMPROPER_START);
		}

	/* shuffle down our data */				
	memmove(RB_front, (RB_front+1), RS_length -1);
	/* insert the new char at the end of the window */
	*(RB_front +RS_length -1) = c;

	return 0;
	}
	
	
/*-------------------------------------------------
 * RS_add_node
 *
 * Technically this is one of those slightly harder
 * pointer functions to get your brain around.
 * it adds in a (possibly) new node from the current
 * parent, either as a new "drop", or as a sibling
 * to an already existing drop.  */
struct _snode  *RS_add_node( struct _snode *parent, char value, struct _infonode *info ){	

	struct _snode *n, *pp;

	/* allocate the memory for the node */
	n = malloc(sizeof(struct _snode));
	
	if (!n) {
		fprintf(stderr,"Error: RS_add_node, cannot allocate memory for new node, (%s)\n",strerror(errno));
		exit(_SDB_BUILD_NO_MEMORY);
		}
	
	/* initialise the node */	
	n->sibling = NULL;
	n->down = NULL;
	n->value = value;
	n->info = info;
	
	/* if there is no current "down" nodes below the parent, 
	 * then we need to create one, as we dont have anywhere
	 * to attach any future siblings */
	if (!parent->down){
		parent->down = n;
		return n;
		}
	else {
	/* else, obviously, if there IS a non-NULL node below, then
	 * we will have to add out new node as a sibling to the existing
	 * structure */
		parent = parent->down;
		do {
			/* if the value we want to insert already exists, then dont
			 * bother using the new node, just free it */
			if (parent->value == value){
				/* if we dont have any "info" attached to this node, then do so
				 * if required */
				if (!parent->info) parent->info = info;
				/* Free up our unneeded node */
				free(n);
				return parent;
				}
			else {
				/* else, try the next node along... */
				pp = parent;
				parent = parent->sibling;
				}
			} while (parent);
			
		/* if we go to the end of the sibling list and didn't find anything
		 * which matched our current value, then we need to attach our node
		 * to the end of the existing sibling chains... */
		if (!parent) {
			parent = pp;
			parent->sibling = n;
			return n;
			} /* if new sibling had to be added */
		} /* if we had to go to the sibling level */

	return NULL;
	}
	

/*--------------------------------------------------------------------
 * RS_add_string
 *
 * Adds a search string into the current search-tree as represented by
 * the parent pointer. */
int RS_add_string( struct _snode *parent, char *ss, int length, struct _infonode *info ){
	
	int i = 0;
	struct _snode *c;
	struct _infonode *setinfo = NULL;
	
	/* if the length of this string exceeds our current maximum  string
	 * lenght, then update the value, we need this later for when we are
	 * -searching- through the stream/strings */	
	if (length > RS_length) RS_length = length;
		
	/* Special case scenario */
	if (parent == NULL){
		fprintf(stderr,"Error: RS_add_string() parent isn't initialised, call RS_init() first.\n");
		exit(_SDB_PARENT_NULL);
		}
	else c = parent;
		
	/* for ever character in this string... */
	for (; i < length; i++){
		
		/* if this is the last character, then setup our info-node */
		if (i+1 == length){
			setinfo = info;
			}
			
		/* insert the node into the tree */
		c = RS_add_node(c, *ss, setinfo);
		
		/* move to the next char */
		ss++;
		}	/* for */

	/* if for some -very strange- reason the parent is null, then we better
	 * jump out here, because it should never happen! */
	if (!parent) {
		fprintf(stderr,"Error: Parent node is still null after string insert!\n");
		exit(_SDB_PARENT_NULL);
		}
		
	return 0;
	}
	
	
/*--------------------------------------------------------------------
 * RS_load_keys
 *
 * Loads the signature keys from the file as supplied by the fname */
int RS_load_keys( struct _snode *parent, char *fname ){
	
	struct _infonode *info;
	FILE *f;
	char line[10240];
	char *searchstr, *comment, *key, *smajor, *sminor, *sflags;
	int slength;
	int kcount = 0;
	
	/* try open up the file */
	f = fopen(fname,"r");
	
	/* if the file didn't open... */
	if (!f){
		fprintf(stderr,"While opening %s : %s\n",fname, strerror(errno));
		exit(_SDB_DB_CANT_OPEN);
		}
	
	/* Initialise our HEX->DEC conversion table */	
	RS_init_hexconv();
	
	/* set our token's to NULL */
	searchstr = comment = key = smajor = sminor = sflags = NULL;
	
	/* while we can load more keys... */
	while (fgets(line, 1023, f)){
		/* get the length of the string */
		slength = strlen(line);
		if (slength > 5) {
		
			/* if there's a terminating \n, then dispose of it */
			if (line[slength-1] == '\n'){
				line[slength-1] = '\0';
				slength--;
				}
		
			/* split the string... */
			key = strtok(line,":");
			if (key) smajor = strtok(NULL,":");
			if (smajor) sminor = strtok(NULL,":");
			if (sminor) sflags = strtok(NULL,":");
			if (sflags) { searchstr = strtok(NULL,":"); RS_decode_PQ(searchstr); }
			if (searchstr) { comment = strtok(NULL,"\n\r"); RS_decode_PQ(comment); }
			if (comment) {
				info = malloc(sizeof(struct _infonode));
				sprintf(info->key,"%s",key);
				info->major = atoi(smajor);
				info->minor = atoi(sminor);
				info->flags = atoi(sflags);
				info->comment = malloc((strlen(comment)+1) *sizeof(char));
				sprintf(info->comment,"%s",comment);			
				RS_add_string(parent, searchstr, strlen(searchstr), info);
				kcount++;
				}
			}
		}
	fclose(f);
		
	return kcount;
	}
			
	
/*--------------------------------------------------------------------
 * RS_test_next_char
 *
 * Tests the next character in the tree to see for a match */	
int RS_test_next_char( struct _snode **current, char nextchar ){
	
	int result = _RS_FAIL;
	
	/* if we can move down, then do so...*/
	if ((*current)->down){
		
		/* update the pointer...*/
		(*current) = (*current)->down;
		
		/* while we have a valid node, and it's value is not what we're looking for... */
		while ( (*current)&&((*current)->value != nextchar) ){
			/* go do the sibling... */
			(*current) = (*current)->sibling;
			}
			
		/* if current is still valid (ie, not NULL), then we have found a match */
		if (*current) {
			
			/* if the values -do- match */
			if ((*current)->value == nextchar) {
				
				/* then we have at _LEAST_ a transient OK */
				result = _RS_TRANS;
				
				/* if the INFO field is NOT NULL, then we have a HIT */
				if ((*current)->info != NULL){
					result = _RS_HIT;
					} /* if info */
				} /* if value == */
			} /* if current */
		} /* if current */
		
	return result;
	}
	
	
/*--------------------------------------------------------------------
 * RS_search_string
 *
 * Searches the search-tree to see if our string matches */
int RS_search_string( struct _snode *parent, char *string ){
	
	int sl = strlen(string);
	int i;
	int result = _RS_FAIL;
	struct _snode *node = parent;
	
	/* for every char in the string we've got presented to us ... */
	for (i = 0; i < sl; i++){
		
		/* try the current character ... */
		result = RS_test_next_char( &node, *string );
		
		/* if we were returned a HIT, it means we have indeed got a fitting string. */
		if (result == _RS_HIT) {
			
			/* report results as required by the flags given to us at runtime */
			if (verbose) fprintf(stdout, "%30s  :  %s\n", get_fname(currentfilename), node->info->comment);
			if (fastscan >0) result = _RS_FAST_SCAN;
			if (fastexit >0) result =  _RS_FAST_EXIT;
			break;
			}
		/* else if things just failed miserably, then dont bother searching the rest of the string */
		else if (result == _RS_FAIL){
			break;
			}
		/* if on the other hand we had a -transient-, then move to the next char... */
		else {
			string++;
			}
		} /* for */
				
	return result;
	}
	
	
/*--------------------------------------------------------------------
 * RS_search_stream
 *
 * Searches through a given stream for any matches against the signature
 * database */
int RS_search_stream( struct _snode *parent, char *streamname ){
	
	FILE *f;
	char c;
	int result = 0;
	
	/* try and open up the stream... */	
	f = fopen( streamname, "r");
	
	if (!f) {
		fprintf(stderr,"Error: RS_search_stream: Cannot open %s, %s\n\n",streamname, strerror(errno));
		return _SDB_STREAM_CANT_READ;
		}
	
	/* setup our global filename, so we can report correctly if we get a hit later */
	currentfilename = streamname;
		
	/* Initialise the search-window */
	RB_init();
	RS_FB_init();
	RS_FB_setstream(f);


	/* Prefill the buffer... */
	while ( (RB_waterlevel < RS_length) && (RS_FB_feof()==0) && (RS_FB_getbyte(&c)) ){
		RB_prefill(c);
		}

	/* check our newly filled up buffer... */
	result = RS_search_string( parent, RB_front );

	/* Now do the normal run, whilst not at the end of the file, or having
	 * not received a _RS_FAST_EXIT response... */
	while ( (result != _RS_FAST_SCAN)&&(result != _RS_FAST_EXIT)&&(!RS_FB_feof()) &&(RS_FB_getbyte(&c)) ){
		
		/* insert the new char... */
		RB_pushchar(c);
		/* test the new string */
		result = RS_search_string( parent, RB_front );
		}

		
	/* dont stop until we run out of buffer */
	while ( (result != _RS_FAST_SCAN)&&(result != _RS_FAST_EXIT)&&(RB_waterlevel > 0 )){
		
		/* keep pushing in \0's until we have no more buffer */
		RB_pushchar('\0');
		
		/* do the search */
		result = RS_search_string( parent, RB_front);
		
		/* decrement our water level */
		RB_waterlevel--;
		}
		
	/* close the file */
	fclose(f);
	
	/* free up anything which was alloc'd */
	RB_done();
		
	return result;
	}		
	
		
/*------------------------------------------END. */