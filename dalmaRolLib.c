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
 *     Library to support piping standard out to daLogMsg
 *
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "cMsg.h"

/* redirection-pipe globals */
static pthread_t      dalmaRedirectPth;
static int dalmaPipeFD[2];
static int dalmaBufFD[2];
static void dalmaStartRedirectionThread(void);
static void dalmaRedirect(void);

static void
remexStartRedirectionThread(void)
{
  int status;

  status =
    pthread_create(&remexRedirectPth,
		   NULL,
		   (void*(*)(void *)) remexRedirect,
		   (void *)NULL);

  if(status!=0)
    {
      printf("%s: ERROR: Redirection Thread could not be started.\n",
	     __FUNCTION__);
      printf("\t pthread_create returned: %d\n",status);
    }

}

static void
remexRedirect(void)
{
  int rBytes=0, remB=0;
  int stat=0;
  char msg_buf[REDIRECT_MAX_SIZE];

  prctl(PR_SET_NAME,"remexRedirection");


#ifdef DEBUG
  printf("%s: Started..\n",__FUNCTION__);
#endif

  /* Do this until we're told to stop */
  while(1)
    {
      remB=0; rBytes=0;
      memset(msg_buf,0,REDIRECT_MAX_SIZE*sizeof(char));

#ifdef DEBUG
      printf("%s: Waiting for output\n",__FUNCTION__);
#endif
      while( (rBytes = read(remexPipeFD[0], (char *)&msg_buf[remB], 100)) > 0)
        {
	  if(remB<REDIRECT_MAX_SIZE*sizeof(char))
	    remB += rBytes;
#ifdef DEBUG
	  else
	    {
	      printf("%s: Got REDIRECT_MAX_SIZE (%d >= %d)\n",
		     __FUNCTION__,remB, REDIRECT_MAX_SIZE);
	    }
#endif
        }

#ifdef DEBUG
      printf("%s: <<%s>>\n",__FUNCTION__,msg_buf);
#endif

      pthread_testcancel();

      close(remexPipeFD[0]);

      /* Write the message back to the routine that creates the payload */
      write(remexBufFD[1],msg_buf,remB);
      close(remexBufFD[1]);

      /* Reopen the redirection pipe for future ExecuteFunction commands */
      stat = pipe(remexPipeFD);
      if(stat == -1)
	perror("pipe");

    }
}
