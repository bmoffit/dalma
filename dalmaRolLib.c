/*----------------------------------------------------------------------------*
 *  Copyright (c) 2021        Southeastern Universities Research Association, *
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
#include "dalmaRolLib.h"

/* redirection-pipe globals */
static pthread_t dalmaRedirectPth;
static int dalmaPipeFD[2];
static int dalmaBufFD[2];
static void dalmaStartRedirectionThread(void);
static void dalmaRedirectionThread(void);
static int32_t dalmaRedirectEcho = 0;

#define REDIRECT_MAX_SIZE 200000

pthread_mutex_t dalmaMutex = PTHREAD_MUTEX_INITIALIZER;
#define DLOCK if(pthread_mutex_lock(&dalmaMutex)<0) perror("pthread_mutex_lock");
#define DUNLOCK if(pthread_mutex_unlock(&dalmaMutex)<0) perror("pthread_mutex_unlock");


pthread_mutex_t redirect_mutex=PTHREAD_MUTEX_INITIALIZER;
#define REDIRECT_LOCK {				\
    if(pthread_mutex_lock(&redirect_mutex)<0)	\
      perror("pthread_mutex_lock");		\
  }
#define REDIRECT_UNLOCK {				\
    if(pthread_mutex_unlock(&redirect_mutex)<0)	\
      perror("pthread_mutex_unlock");		\
  }
pthread_cond_t redirect_cv = PTHREAD_COND_INITIALIZER;
#define REDIRECT_WAIT {						\
    if(pthread_cond_wait(&redirect_cv, &redirect_mutex)<0)	\
      perror("pthread_cond_wait");				\
  }
#define REDIRECT_SIGNAL {					\
    if(pthread_cond_signal(&redirect_cv)<0)		\
      perror("pthread_cond_signal");			\
  }

int32_t
dalmaInit(int32_t echo)
{
  DLOCK;
  dalmaRedirectEcho = echo ? 1 : 0;
  DUNLOCK;

  printf("%s: Enabling Redirection Pipeline\n", __func__);
  /* Create the pipe file descriptors */
  int32_t stat = pipe(dalmaPipeFD);
  if(stat == -1)
    {
      printf("%s: ERROR: Creating redirection pipe\n", __func__);
      perror("pipe");
      return -1;
    }

  stat = pipe(dalmaBufFD);
  if(stat == -1)
    {
      printf("%s: ERROR: Creating redirection buffer pipe\n", __func__);
      perror("pipe");
      return -1;
    }

  /* Spawn the redirection-pipe pthread */
  dalmaStartRedirectionThread();

  return 0;
}

int32_t
dalmaClose()
{
  printf("%s: Disabling Redirection\n", __func__);

  /* Kill the redirection-pipe pthread, if it's running */
  void *res = 0;

  if(pthread_cancel(dalmaRedirectPth) < 0)
    perror("pthread_cancel");

  write(dalmaPipeFD[0], "", 0);

  if(pthread_join(dalmaRedirectPth, &res) < 0)
    perror("pthread_join");
  if(res == PTHREAD_CANCELED)
    printf("%s: Redirecion thread canceled\n", __func__);
  else
    printf("%s: ERROR: Redirection thread NOT canceled\n", __func__);

  return 0;
}

static void
dalmaStartRedirectionThread(void)
{
  int status;

  status =
    pthread_create(&dalmaRedirectPth,
		   NULL,
		   (void *(*)(void *)) dalmaRedirectionThread, (void *) NULL);

  if(status != 0)
    {
      printf("%s: ERROR: Redirection Thread could not be started.\n",
	     __func__);
      printf("\t pthread_create returned: %d\n", status);
    }

}

static void
dalmaRedirectionThread(void)
{
  int rBytes = 0, remB = 0;
  int stat = 0;
  char msg_buf[REDIRECT_MAX_SIZE];

  prctl(PR_SET_NAME, "dalmaRedirection");


#ifdef DEBUG
  printf("%s: Started..\n", __func__);
#endif

  memset(msg_buf, 0, REDIRECT_MAX_SIZE * sizeof(char));
  /* Do this until we're told to stop */
  while(1)
    {
      remB = 0;
      rBytes = 0;

#ifdef DEBUG
      printf("%s: Waiting for output\n", __func__);
#endif

      while((rBytes = read(dalmaPipeFD[0], (char *) &msg_buf[remB], 100)) > 0)
	{
	  if(remB < REDIRECT_MAX_SIZE * sizeof(char))
	    remB += rBytes;
#ifdef DEBUG
	  else
	    {
	      printf("%s: Got REDIRECT_MAX_SIZE (%d >= %d)\n",
		     __func__, remB, REDIRECT_MAX_SIZE);
	    }
#endif
	}

#ifdef DEBUG
      printf("%s: <<%s>>\n", __func__, msg_buf);
#endif

      pthread_testcancel();

      REDIRECT_LOCK;
      close(dalmaPipeFD[0]);

      /* Write the message back to the routine that creates the payload */
      write(dalmaBufFD[1], msg_buf, remB);
      close(dalmaBufFD[1]);

      /* Reopen the redirection pipe for future ExecuteFunction commands */
      stat = pipe(dalmaPipeFD);
      if(stat == -1)
	perror("pipe");

      memset(msg_buf, 0, REDIRECT_MAX_SIZE * sizeof(char));
      REDIRECT_SIGNAL;
      REDIRECT_UNLOCK;
    }
}

static int32_t dalmaInitialIO;
static int32_t dalmaRedirectEnabled=0;

void
dalmaRedirectEnable(int echo)
{
  DLOCK;
  dalmaRedirectEcho = echo ? 1 : 0;
  DUNLOCK;

  REDIRECT_LOCK;
  fflush(stdout);
  dalmaInitialIO = dup(STDOUT_FILENO);

  dup2(dalmaPipeFD[1], STDOUT_FILENO);
  dalmaRedirectEnabled=1;
  REDIRECT_UNLOCK;
}

void
dalmaRedirectDisable()
{
  REDIRECT_LOCK;
  fflush(stdout);
  close(dalmaPipeFD[1]);
  dup2(dalmaInitialIO, STDOUT_FILENO);
  dalmaRedirectEnabled=0;
  REDIRECT_WAIT;
  REDIRECT_UNLOCK;
}
