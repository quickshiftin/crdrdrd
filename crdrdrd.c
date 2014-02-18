/**
 * crdrdrd.c
 * 
 * Author: 	Nathan Nobbe
 * Date:	12.04.2006
 * Developed for the Express use of Moxune as a component of the Moxune Sentinel.
 *
 * crdrdrd stands for card reader deamon
 * its purpose is to run in the background waiting for input from standard input.
 * when input is recieved the deamon will scan said input for that input which resembles
 * data from Common Grounds gift cards, sent via a USB magnetic card reader.  the deamon
 * will then extract the number from the back of the card via regex.
 * the deamon will be recieving all input from standard in.  it is important to note that
 * when in full operation, the machine running crdrdrd should not have a keyboard connected,
 * other than the magnetic card reader; however, there is no absolute control over this
 * variable.
 * also, it is notable, the regex used to search data from stdin will be based on the
 * programming of the card reader.  as the current implementation is a simple one, there is
 * no communication between crdrdrd and the card reader to absolutely determine the
 * configuration of the card reader at runtime.  therefore, such regex will be hard-coded
 * into crdrdrd until such time that support for direct runtime communication with the
 * card reader is built into crdrdrd.
 *
 * Most of the code for this application has been taken as excerpts from
 * Advanced Programming in the UNIX Environment
 * Thanks to Rago and Stevens for your hard work; you make my work possible.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <getopt.h>


#include "includes/crdrdrd.h"
#include "mxnLogger.h"
#include "includes/libusbCon.h"
#include "includes/crdrdrdDataConverter.h"
#include "insertCardNumber.h"

#include <glib.h>

char *inputfile=NULL;  //will take its input from file instead of the card reader
char *outputfile=NULL; //will output everything taken from the card reader to this file

char *dbfile=NULL;     //card numbers are placed here.

char *card_processor=NULL;  //program to be run to inform that a card has been swiped.



void usage(char *cmdname) {
  fprintf(stderr, "Usage: %s <path to database> <card processor program>\n"
                  "Database is required, card processor is optional\n\n"
                  "Optional: Set the log level.\n"
                  "  -l (EMERG|ALERT|CRIT|ERR|WARNING|NOTICE|INFO|DEBUG)\n\n"
                  "Optional: Output raw card swipe data to file.\n"
                  "  -o <filename>\n\n"
                  "Optional: Grab raw card input from file rather than card reader.\n"
                  "  -i <filename>\n\n"
                  "Optional: Run as a daemon.\n"
                  "  --daemonize\n",basename(cmdname));
}
/**
 *	command line arguments are as follows
 *	1 - path to sentinel database
 *
 *  optional
 *  -i <file>  - take input from a file instead of a card reader
 *  -o <file>  - send output from card reader (or input file) to this output file
 */
int main(int argc, char *argv[]) {
  int c, i;
  int dodaemon=0;

	mxnLogOpen(CRDRDRD_APP_NAME,MXN_DEBUG,CRDRDRD_SYSLOG_FACILITY);				// initialize the logger

  while (1) {

    int option_index = 0;

    static const struct option long_options[] = {
      {"daemon", 0, 0, 0},
      {"loglevel", 1, 0, 0},
      {0, 0, 0, 0}
    };

    c = getopt_long(argc, argv, "o:i:l:", long_options, &option_index);
    if (c==-1) break;

    switch (c) {
      case 0:
        mxnLog(MXN_INFO, "Starting card reader daemon in daemon mode.");
        dodaemon=1;
        break;
      case 'l':
        if (optarg) {
          for (i=0; i<LOG_LEVELS; i++) {
            if (strcasecmp(optarg,log_levels[i].desc)==0) {
              mxnLog(MXN_INFO,"Log level set to %s",log_levels[i].desc);
              mxnLogSetLevel(log_levels[i].type);
              break;
            }
          }
          if (i>=LOG_LEVELS)
            mxnLog(MXN_ERR, "Log level %s unrecognized, defaulting to DEBUG", optarg);
        }
        break;
      case 'i':
        inputfile = optarg;
        break;
      case 'o':
        outputfile = optarg;
        break;
      default:
        break;
    }
  }

  if (inputfile) mxnLog(MXN_DEBUG, "input file:%s\n",inputfile);
  if (outputfile) mxnLog(MXN_DEBUG, "output file:%s\n",outputfile);

  if (optind < argc) {
    dbfile=argv[optind++];
  }
  if (optind < argc) {
    card_processor=argv[optind++];
  }

  if (!dbfile) {
    fprintf(stderr,"You must specify a path to a database file.  Aborting.\n");
    usage(argv[0]);
    return -1;
  }

  if (dodaemon) daemonize(argv[0]);// convert the program to a daemon

	startSwipeListener();		// block for card swipes
	
	return 0;
}

/**
 *	following example from page 425 of APUE Second Edition
 *	this is almost a verbatim copy of that code, with the exception of the logging output
 *	cmd is the name of the program that is supposed to be converted into a daemon
 */
void daemonize(const char *cmd) {
	int 				i;
	int					fd0;
	int					fd1;
	int					fd2;
  int pidfd;
	pid_t				pid;
	struct rlimit		rl;
	struct sigaction 	sa;

	/// clear file creation mask
	umask(0);

	/// get maximum number of file descriptors
  mxnLog(MXN_INFO,"attempting to get the file limit..");
	
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
      mxnLog(MXN_ERR,"%s can't get file limit",cmd);

	/// become a session leader to lose controlling TTY
  mxnLog(MXN_INFO,"attempting to become session leader to lose controlling tty");
	
	if((pid = fork()) < 0)
    mxnLog(MXN_DEBUG,"%s %s", "can't fork", cmd);

  else if(pid != 0) { // parent process terminates
    exit(0);
  }



	/// ensure future opens won't allocate controlling TTYs
  mxnLog(MXN_INFO, "ensure future opens wont allocate controlling ttys");
	
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0) {
	  mxnLog(MXN_ERR, "can't ignore SIGHUP");
	}

  //Any particular reason we are forking twice?
	if((pid = fork()) < 0)
    mxnLog(MXN_DEBUG,"%s %s", "can't fork", cmd);

	else if(pid != 0) // parent process terminates
		exit(0);


  //Since we are daemonizing, inform the init scripts of our real pid.
  pidfd = open(CRDRDRD_PID_FILE, O_CREAT | O_WRONLY | O_NOCTTY | O_TRUNC, 0600);
  if (pidfd != -1) {
    char *strpid;
    asprintf(&strpid, "%i\n",getpid());
    write(pidfd, strpid, strlen(strpid));
    free(strpid);
    close(pidfd);
  }


	/// change the current working directory to the root so we
	/// wont prevent file system from being unmounted
	mxnLog(MXN_INFO, "change current working directory to root");

	if(chdir("/") < 0)
	  mxnLog(MXN_ERR, "can't change directory to /");

	/// close all open file descriptors
	mxnLog(MXN_INFO,"close all open file descriptors");

	if(rl.rlim_max == RLIM_INFINITY) {
		rl.rlim_max = 1024;
	}
	for(i = 0; i < rl.rlim_max; i++) {
		if(i != STDIN_FILENO) {
			close(i);
		}
	}

	/// attach file descriptors 0, 1, and 2 to /dev/null
  mxnLog(LOG_INFO, "attach file descriptors to dev-null");
	
	fd1 = open("/dev/null", O_RDWR);
	fd2 = open("/dev/null", O_RDWR);
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

  mxnLog(MXN_INFO, "here is a message after detaching file descriptors");
}

GString *getLine(GString *str) {
  GString *ret;
  char *end, *tmp;;

  end = strchr(str->str,'\n');
  if (!end) return NULL;

  tmp = strndup(str->str,end-str->str+1);
  ret = g_string_new(tmp);
  free(tmp);

  g_string_erase(str,0,end-str->str+1);
  return ret;
}

void execCardProcessor(GString *card_number) {
  static char ok_chars[] = "abcdefghijklmnopqrstuvwxyz\
                            ABCDEFGHIJKLMNOPQRSTUVWXYZ\
                            1234567890_-.@";
  char *cmd=NULL;							  
  //basic sanitation of card number, hell we don't know what's on
  //that thing.
  char *cp;
  int retval;

  for (cp = card_number->str; *(cp += strspn(cp, ok_chars)); )
    *cp = '_';

  asprintf(&cmd,"%s %s&",card_processor, card_number->str);

  mxnLog(MXN_DEBUG,"Running '%s'\n",cmd);
  retval = system(cmd);
  mxnLog(MXN_DEBUG,"Returned %i\n",retval);
  free(cmd);
}

/**
 * startSwipeListener
 *
 * loop waiting for an data to become available from the card reader;
 * when raw data is recieved, invoke methods to decode and extract the card number
 * then invoke methods to insert the card number into the database
 */
void startSwipeListener() {
  usb_dev_handle *rdr_hdl;		// a handle to the minimag

  //It is simpler if we just use glib strings.
  GByteArray *swipe_data;   // raw data from a swipe.
  GString *data;            // hold the decoded string until we have enough data.
  GString *data_new;
  GString *card_number;			// the number on the back of a commonGrounds gift card
  GString *line;            // each line from the card reader is terminated by a \n

  sqlite3 *db;              // we will store the result here.
  int tries=10;             // tries to connect to database.

//  FILE *ifp=NULL, *ofp=NULL;
  FILE *ofp=NULL;
  int ifd=-1;

  //if these fail, just ignore them
  if (inputfile) {
    mxnLog(MXN_DEBUG,"preopen");
    ifd=open(inputfile,O_RDONLY);
    mxnLog(MXN_DEBUG,"postopen:%i",ifd);
    if (ifd==-1) { 
      mxnLog(MXN_DEBUG, "could not open input (%s).\n", strerror(errno));
    }
  }
  if (outputfile && (!inputfile || strcmp(outputfile,inputfile)!=0)) ofp=fopen(outputfile, "w");  //can't read and write to same file.
  
  mxnLog(MXN_INFO, "--> entering startSwipeListener; dbfile: %s", dbfile);

  /// setup the connection with the minimag

  if (!inputfile) {
    if(!initializeConnection(&rdr_hdl)) {
      mxnLog(MXN_ERR, "Unable to initialize libusb.");
      return;
    }

    claimCrdRdrInrptInterface(rdr_hdl, 0);		// attempt to disconnect from the kernel; if the kernel doesnt already control the device this wont matter
  }


  mxnLog(MXN_INFO, "Opening database:%s\n", dbfile);
  //I doubt it would fail, but eh, just to be safe.
  while (sqlite3_open(dbfile,&db)!=SQLITE_OK) {
    if (tries--==0) {
      mxnLog(MXN_CRIT, "Unable to connect to sqlite database (%s) located at %s. Terminating.", sqlite3_errmsg(db),dbfile);
      return;
    }
    mxnLog(MXN_DEBUG, "Sqlite3 error:%s\n", sqlite3_errmsg(db));
    sleep(1);
  }
  sqlite3_busy_timeout(db,5000);

    /// loop forever waiting for a card swipe		
  data = g_string_sized_new(100);
  while(1) {

    if (ifd!=-1) {
      char buf[5000];
      int buflen;
      mxnLog(MXN_DEBUG,"preread");
      buflen=read(ifd,buf,5000);
      mxnLog(MXN_DEBUG,"postread: buflen:%i",buflen);
      swipe_data = g_byte_array_new();
      if (buflen>0) {
        g_byte_array_append(swipe_data,(unsigned char*)buf,buflen);
      }
    }

    else {

      mxnLog(MXN_DEBUG,"preblock");
      swipe_data=blockForCardSwipe(rdr_hdl);
      mxnLog(MXN_DEBUG,"postblock\n");
      //mxnLog(MXN_DEBUG, "post block swipe_data->len:%i",swipe_data->len);

      if (!swipe_data) {
        mxnLog(MXN_WARNING, "Failed attempt to acquire additional card data.");
        break;
      }
    }

    if (ofp && swipe_data->len>0) {
      fwrite(swipe_data->data,1,swipe_data->len,ofp);
      fflush(ofp);
    }

    mxnLog(MXN_DEBUG,"predecode\n");
    data_new = decodeSwipe(swipe_data);
    mxnLog(MXN_DEBUG,"postdecode\n");
    g_byte_array_free(swipe_data,1);

    if (!data_new) {
      mxnLog(MXN_CRIT, "Failed attempt to decode additional card data.");
      break;
    }

    g_string_append(data,data_new->str);
    g_string_free(data_new,1);
    //      mxnLog(MXN_INFO,"full data:'%s'",data->str);

    while ((line = getLine(data))) {
      mxnLog(MXN_DEBUG,"preextract\n");
      card_number = extractCardNumber(line);
      mxnLog(MXN_DEBUG,"postextract\n");

      if (card_number) {
        mxnLog(MXN_INFO,"*********");
        mxnLog(MXN_INFO,"%s", card_number->str);
        mxnLog(MXN_INFO,"*********");

        // attempt to insert the card number into the database
        mxnLog(MXN_DEBUG,"preinsert\n");
        insertCardNumber(card_number->str, db);
        mxnLog(MXN_DEBUG,"postinsert\n");

        if (card_processor) execCardProcessor(card_number);

        g_string_free(card_number,1);
      }
    }
  }
}
