#include "message_slot.h"
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[])
{
  int file_desc;
  int ret_val;
  unsigned long id;


  file_desc = open(argv[1], O_RDWR );
  if (argc != 4) {
	perror("Error");
	exit(1);
  }


  if( file_desc < 0 )
  {
	perror("Error");
    exit(1);
  }

  id = atoi(argv[2]);



  ret_val = ioctl(file_desc, MSG_SLOT_CHANNEL, id);
  if(ret_val != SUCCESS)
  {
	 perror("Error");
    exit(1);
  }



  ret_val = write(file_desc, argv[3], (int)strlen(argv[3]));
  if(ret_val != strlen(argv[3]))
  {
	  perror("Error");
    exit(1);
  }

  close(file_desc);

  exit(0);
}
