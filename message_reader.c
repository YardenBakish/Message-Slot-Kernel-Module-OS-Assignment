#include "message_slot.h"
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[])
{

  int file_desc;
  int ret_val, tmp;
  unsigned long id;
  char message[BUF_LEN];

  if (argc != 3) {
 	perror("Error");
 	exit(1);
   }

  file_desc = open(argv[1], O_RDWR );
  if( file_desc < 0 )
  {
	perror("Error");
    exit(1);
  }
  id = atoi(argv[2]);


  ret_val = ioctl(file_desc, MSG_SLOT_CHANNEL, id);
  if(ret_val != SUCCESS )
  {
	 perror("Error");
    exit(1);
  }

  ret_val = read(file_desc, message, BUF_LEN);
  if(ret_val <= 0 )
  {
	  perror("Error");
    exit(1);
  }
  close(file_desc);
  tmp = write(1, message, ret_val);
  if(ret_val != tmp)
  {
	  perror("Error");
    exit(1);
  }

  exit(0);
}

