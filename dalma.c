/*----------------------------------------------------------------------------*
 *  Copyright (c) 2020        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Author:  Bryan Moffit                                                   *
 *             moffit@jlab.org                   Jefferson Lab, MS-12B3       *
 *             Phone: (757) 269-5660             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *
 * Description:
 *     Program to capture dalogMsg's from specified components
 *
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>

#include "cMsg.h"

void *domainId;
void *unSubHandle;


/******************************************************************/
static void callback(void *msg, void *arg) {
  char *logText, *name, *severity;

  struct timespec ts_msgtime;

  char   senderTimeBuf[32];
  struct tm tBuf;

  cMsgGetSenderTime(msg, &ts_msgtime);
  /* convert struct timespect to struct tm */
  localtime_r(&ts_msgtime.tv_sec, &tBuf);
  /* convert struct tm to readable time */
  strftime(senderTimeBuf, 32, "%d%b%Y %T", &tBuf);
  /* Null to end the time string */
  senderTimeBuf[strlen(senderTimeBuf)]='\0';
  printf("%s: ", senderTimeBuf);

  cMsgGetSender(msg, (const char **)&name);
  printf("%s ", name);

  cMsgGetString(msg, "severity", (const char **)&severity);
  printf("%s: ", severity);

  cMsgGetString(msg, "dalogText", (const char **)&logText);
  printf("%s\n", logText);


  // in case we need to respond back... here is the canned response
  int response;
  void *rmsg;
  if(cMsgGetGetRequest(msg,&response)==CMSG_OK)
    {
      if(response==1)
	{
	  printf("%s: Need to respond to this one\n",__FUNCTION__);
	  rmsg = cMsgCreateResponseMessage(msg);
	  cMsgAddString(rmsg, "payloadItem", "any string you want");
	  cMsgSetSubject(rmsg, "RESPONDING");
	  cMsgSetType(rmsg, "TO MESSAGE");
	  cMsgSetText(rmsg, "responder's text");
	  cMsgSend(domainId, rmsg);

	  cMsgFreeMessage(&rmsg);
	}
    }
  else
    printf("%s: Something wrong here\n",__FUNCTION__);


  /* user MUST free messages passed to the callback */
  cMsgFreeMessage(&msg);
}

/******************************************************************/
static void usage() {
  printf("Usage:  consumer <name> <UDL>\n");
}


/******************************************************************/
int main(int argc,char **argv) {

  /* Parameters configurable from the command line */
  int opt_param, option_index = 0;
  static char *pHost = NULL;
  static char *pEXPID = NULL;
  static char *cName = "*";
  static char *pPort = "45000";
  static int Verbose = 1;

  /* cMsg Connection and Subscription information */
  char *myName        = "dalma";
  char *myDescription = "dalogMsg Archiver";
  char *type          = "rc/report/dalog";
  char UDL[64];
  char hostinfo[32];
  int   err, debug = 1;
  cMsgSubscribeConfig *config;

  /* structure for getopt_long */
  static struct option long_options[] =
    {
     /* {const char *name, int has_arg, int *flag, int val} */
     {"host", 1, NULL, 'h'},
     {"port", 1, NULL, 'p'},
     {"name", 1, NULL, 'n'},
     {"expid", 1, NULL, 'e'},
     {"verbose", 0, &Verbose, 1},
     {0, 0, 0, 0}
    };

  /* Parse the commandline parameters */
  while(1)
    {
      option_index = 0;
      opt_param = getopt_long (argc, argv, "h:p:n:e:",
			       long_options, &option_index);

      if (opt_param == -1) /* No more option parameters left */
	break;

      switch (opt_param)
	{
	case 0:
	  break;

	case 'h': /* Platform Host */
	  pHost = optarg;
	  if(Verbose) printf("-- Platform Host (%s)\n",pHost);
	  break;

	case 'p': /* Platform Port */
	  pPort = optarg;
	  if(Verbose) printf("-- Platform Port (%s)\n",pPort);
	  break;

	case 'n': /* Component Name */
	  cName = optarg;
	  if(Verbose) printf("-- Component Name (%s)\n",cName);
	  break;

	case 'e': /* EXPID */
	  pEXPID = optarg;
	  if(Verbose) printf("-- EXPID (%s)\n",pEXPID);
	  break;

	case '?': /* Invalid Option */
	default:
	  usage();
	  exit(1);
	}

    }

  /* Construct the cMsg hostname:port for the UDL */
  if(pHost)
    {
      sprintf(hostinfo,
	      "%s:%s",
	      pHost, pPort);
    }
  else
    {
      /* If the host is not specified on the commandline, use multicast */
      sprintf(hostinfo,"multicast");
    }

  /* if the EXPID is not specified, try to get it from the environment */
  if(pEXPID == NULL)
    {
      pEXPID = getenv("EXPID");
      if(pEXPID == NULL)
	{
	  usage();
	  exit(-1);
	}
      if(Verbose) printf("-- env: EXPID (%s)\n",pEXPID);
    }

  /* Construct the cMsg UDL */
  sprintf(UDL,
	  "cMsg://%s/cMsg/%s?regime=low&multicastTO=5&cmsgpassword=%s",
	  hostinfo, pEXPID, pEXPID);

  if (debug) {
    printf("Running the cMsg consumer, \"%s\"\n", myName);
    printf("  connecting to, %s\n", UDL);
  }

  /* connect to cMsg server */
  err = cMsgConnect(UDL, myName, myDescription, &domainId);
  if (err != CMSG_OK) {
      if (debug) {
          printf("cMsgConnect: %s\n",cMsgPerror(err));
      }
      exit(1);
  }

  /* start receiving messages */
  cMsgReceiveStart(domainId);

  /* set the subscribe configuration */
  config = cMsgSubscribeConfigCreate();
  cMsgSubscribeSetMaxCueSize(config, 10);
  /*
  cMsgSubscribeSetSkipSize(config, 20);
  cMsgSubscribeSetMaySkip(config,0);
  cMsgSubscribeSetMustSerialize(config, 0);
  cMsgSubscribeSetMaxThreads(config, 10);
  cMsgSubscribeSetMessagesPerThread(config, 10);
  */
  cMsgSetDebugLevel(CMSG_DEBUG_NONE);

  /* subscribe */
  cMsgSubscribe(domainId, cName, type, callback, NULL, config, &unSubHandle);

  cMsgSubscribeConfigDestroy(config);
  printf("Press <Enter> to end\n");
  getchar();

  printf("Unsubscribing\n");
  cMsgUnSubscribe(domainId, unSubHandle);
  printf("sleep\n");
  sleep(4);
  cMsgDisconnect(&domainId);
  pthread_exit(NULL);
  return(0);
}
