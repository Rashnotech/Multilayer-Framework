/*---------------------------------------------
 * SDBscan - SignatureDB Scan
 *
 * First version - Sat Mar 10 17:16:20 EST 2001
 *
 * Written and developed by Paul L Daniels (pldaniels@pldaniels.com)
 *
 * (C) 2001 PLDaniels.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "ringsearch.h"

#define _SIGD_FILELIST_CANNOT_OPEN 998
#define _SIGD_NO_ARGUMENTS 999

#define _DEFAULT_DB "/usr/local/signaturedb/signaturedb.db"
#define _DEFAULT_CONF "/usr/local/signaturedb/signaturedb.conf"

#define _USAGE "sdbscan - Signature Database Scanner v0.1.2\n\
written and designed by Paul L Daniels, (C) 2001 PLDaniels\n\n\
	Usage :\n\
		sdbscan [--quiet] [--no_recurse] [--fast_exit] [--full_scan] [--conf_file=<configuration file>] [--file_list=<list of files to scan>] <files to scan>\n\n"

char *_dbfile = _DEFAULT_DB;
char *_conffile = _DEFAULT_CONF;
char *_filelist = NULL;
int _fileindex = 1;
int _recurse = 1;
int _verbose = 1;
int _fastscan = 1;
int _fastexit = 0;


/*--------------------------------------------------------------------
 * load_config
 *
 * Loads the configuration file required to determine what we're going
 * to do. */
int load_config( char *fname ){
	
	FILE *f = fopen(fname,"r");
	char line[1024];
	char *name, *value;
	int linecount = 0;	
	
	if (!f) {
		fprintf(stderr,"Error: Load_config: could not open %s for reading, using defaults\n",fname);
		return 1;
		}
				
	while (fgets(line,1023,f)){
		linecount++;
		if (strlen(line) > 3){
			if (line[0] != '#'){
				name = strtok(line,"=");
				value = strtok(NULL,"\n\r");
				if (strncmp(name,"dbfile",6)==0){
					_dbfile = strdup(value);
					}
				else if (strncmp(name,"verbose",7)==0){
					_verbose = atoi(value);
					}
				else if (strncmp(name,"fastscan",8)==0){
					_fastscan = atoi(value);
					}
				else if (strncmp(name,"fastexit",8)==0){
					_fastexit = atoi(value);
					}
				else {
					fprintf(stderr,"Non-fatal-Error: Load_config: cannot understand line %d: %s\n",linecount,line);
					}
				}} /* if not a comment */
		} /* while */
		
	fclose(f);
	
	return 0;
	}
	
	
	
	
/*--------------------------------------------------------------------
 * scan_filelist
 *
 * if we're given a file containing a list of files, then
 * we will scan them using this function. */
int scan_filelist( struct _snode *p ){

	char fname[513];
	int result = 0;
	FILE *f = fopen(_filelist,"r");

	if (!f) {
		fprintf(stderr,"Error: scan_filelist(), cannot open the file \"%s\"\n",_filelist);
		exit(_SIGD_FILELIST_CANNOT_OPEN);
		}
			
	while (fgets(fname,512,f)){
		if (fname[strlen(fname)-1] == '\n') fname[strlen(fname)-1] = '\0';
		result = RS_search_stream(p,fname);
		if (result == _RS_FAST_EXIT) break;
		else result = 0;
		}
	if (f) fclose(f);
	return 0;
	}


/*--------------------------------------------------------------------
 * recurse_scan
 *
 * recursively scans the directory as given by 'adir' using dive-first
 * recursion */
int recurse_scan( struct _snode *p, char *adir ){
	
	int result = 0;
	int filecount;
	struct stat buf;
	struct dirent **namelist;
	char fpn[10240];
	
	/* first check that we are actually being pointed to 
	 * a directory, and not a file! */
	stat(adir, &buf);
	
	/* if what we have here is a file, not a dir, then
	 * scan it, and return */
	if (S_ISREG(buf.st_mode)){
		result = RS_search_stream(p,adir);
		return result;
		}
	else if (S_ISDIR(buf.st_mode)){
		/* scan and build our filelist for the directory */
		filecount = scandir(adir, &namelist, 0, alphasort);
	
		if (filecount < 0){
			 perror("scandir");
			 }
		/* if we got more than 0 files in our scan... */
		else while(filecount--){
			/* make sure we're not trying to scan either ourselves
			 * or our parent... */
			if ((strcmp(namelist[filecount]->d_name,".")!=0)){
				if ((strcmp(namelist[filecount]->d_name,"..")!=0)){

					/* create the new full-path */
					sprintf(fpn,"%s/%s",adir,namelist[filecount]->d_name);
				
					/* get the information on this new file */
					stat(fpn,&buf);

					/* if it's a REGULAR file, then scan it */
					if (S_ISREG(buf.st_mode)){
						result = RS_search_stream(p,fpn);
						}
					/* if it's a DIRECTORY AND we have _recurse on, then
					 * traverse it by recursively calling this function */
					else if ( (_recurse)&&(S_ISDIR(buf.st_mode)) ){
						result = recurse_scan(p, fpn);				
 	 	    		} /* if file-mode */
 		     	} /* if this isn't either ourselves or our parent dir */
	 	     }
  	    } /* while more files to process */
		} /* if what we were asked to scan is a directory */
		
	return result;
	}
	
	
	
/*--------------------------------------------------------------------
 * set_parms
 *
 * Sets the internal flags/variables according to what we read off
 * the command line parameters */
int set_parms( int argc, char **argv ){
	
	int i;
	
	/* for every parameter, excluding the actual filename of course... */
	for (i = 1; i < argc; i++){
		
		/* if we have a -- prefixed parameter, then we should check it
		 * to see if it's one we know of */
		if ( (argv[i][0] == '-') && (argv[i][1] == '-') ){
			
			/* shuffle our focus up two more chars */
			argv[i] += 2;
			
			/* check the new string we're focus'd on for matches */
			if (strncmp(argv[i],"fast_exit",9)==0) { RS_set_fastexit(1);  }
			else if (strncmp(argv[i],"full_scan",9)==0) { RS_set_fastscan(0);  }
			else if (strncmp(argv[i],"quiet",5)==0) RS_set_verbose(0);
			else if (strncmp(argv[i],"no_recurse",10)==0) _recurse = 0;
			else if (strncmp(argv[i],"file_list",9)==0) {				
				argv[i] += strlen("file_list=");
				_filelist = argv[i];
				}
			else if (strncmp(argv[i],"conf_file",9)==0) {				
				argv[i] += strlen("conf_file=");
				_conffile = argv[i];
				}
			/* else if nothing matched, then we're just going to have to moan about it */
			else fprintf(stderr,"Unknown parameter %s\n",argv[i]);
			}
		else {
			_fileindex = i;
			break;
			}
		} /* for */
	
	return 0;
	}




/*--------------------------------------------------------------------
 * main
 *
 */
int main( int argc, char **argv ){

	struct _snode *p = RS_init();	/* the parent node of our sig-tree */
	int i;
	int result=0;
	int kcount=0;
	
	/* if we dont have enough arguments, then let the user know how
	 * to use our software */
	if (argc < 2) {
		fprintf(stderr,"%s",_USAGE);
		exit(_SIGD_NO_ARGUMENTS);
		}

	/* set our _defaults_ */
	RS_set_verbose(_verbose);
	RS_set_fastscan(_fastscan);
	RS_set_fastexit(_fastexit);
	
	/* see what the caller asked for... */
	set_parms(argc, argv);

	load_config(_conffile);
	
	/* load up the signature database keys */	
	
	kcount = RS_load_keys(p,_dbfile);
			
	/* if we were given a file list, then go through it
	 * one file at a time */
	if (_filelist){
		result = scan_filelist(p);		
		}
	/* else if we have no file list, then our trailing parms
	 * on the CLI will be our files/dirs to search */
	else {
		/* for every remaining parameter */
		for (i = _fileindex; i < argc; i++){
			
			/* if we have recurse mode on, then use the recurse
			 * scan technique */
			if (_recurse) {
				result |= recurse_scan(p,argv[i]);
				}
			/* else just scan this and only this file */
			else {
				result |= RS_search_stream(p,argv[i]);
				}
			
			/* if we have a FAST_EXIT flag set, then stop scanning */
			if (result == _RS_FAST_EXIT) break;
			}
		}
	
	/* if we had either a fast_exit or a fast_scan result, then
	 * let the calling program know that we did detect a hit in
	 * our scanning */
	if ((result & _RS_FAST_EXIT)||(result & _RS_FAST_SCAN)){
		result = _SDB_FILE_MATCHED;
		}
	else result = _SDB_OK;
	
	return result;
	}
	
/*------------------------------------------END.--*/