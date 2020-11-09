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

  char *myName        = "dalma";
  char *myDescription = "dalogMsg Archiver";
  char *subject       = "*";
  char *type          = "rc/report/dalog";
  char *UDL           = "cMsg://multicast/cMsg/testexpid?regime=low&cmsgpassword=testexpid";
  int   err, debug = 1;
  cMsgSubscribeConfig *config;


  if (argc > 1) {
    myName = argv[1];
    if (strcmp(myName, "-h") == 0) {
      usage();
      return(-1);
    }
  }

  if (argc > 2) {
    UDL = argv[2];
    if (strcmp(UDL, "-h") == 0) {
      usage();
      return(-1);
    }
  }

  if (argc > 3) {
    usage();
    return(-1);
  }

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
  cMsgSubscribe(domainId, subject, type, callback, NULL, config, &unSubHandle);

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
