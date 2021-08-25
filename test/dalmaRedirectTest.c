/*
 * File:
 *    dalmaRedirectTest
 *
 * Description:
 *    Test redirection with the dalmaRol Lib
 *
 */

#include <unistd.h>
#include <stdio.h>
#include "dalmaRolLib.h"

int32_t
main(int32_t argc, char *argv[])
{
  dalmaInit(1);
  printf("\nthis is happy\n");
  dalmaRedirectEnable(1);
  printf("this is good\n");
  dalmaRedirectDisable();
  printf("\nthis is nuked\n");

  dalmaRedirectEnable(1);
  printf("that's a lot of rats and uhm\n");

  dalmaRedirectDisable();
  printf("\nall right, that should be enough\n");

  dalmaRedirectEnable(1);
  printf("TK421\n");

  dalmaRedirectDisable();
  printf("it was sure nice talking to you\n");

  dalmaClose();

  return 0;
}

/*
  Local Variables:
  compile-command: "make dalmaRedirectTest"
  End:
 */
