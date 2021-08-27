#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/

#include <linux/slab.h>
MODULE_LICENSE("GPL");
#include "message_slot.h"

//================== DATA STRUCTURES ===========================



/*linked-list of channel nodes - each device has its own message_chanells nodes*/
typedef struct channel_node
{
	int iminor;
	int id;
	int len;
	char* message;
	struct channel_node* next;
} channel_node;


/*a device has a channel_node list containing its nodes*/
typedef struct Device
{

	struct channel_node* list;
} device;


/*IMPORTANT: the main structure for all devices and its message channels will be handled using a
 * CONSTANT (MEMORY SPACE WISE) array.*/
static device device_array[256];

//================== MINOR OPERATIONS ===========================




static void create_node(channel_node* node, int k, struct file*  file, int channel_id, int flag){
			if (flag == 0)(*(device_array+k)).list = node;
			node->len = 0;
			node->id = channel_id;
			node->iminor = k;
			node->next= NULL;
			node->message= NULL;
			file->private_data = node;
}

static int set_channel(channel_node* ptr, unsigned long channel_id, struct file* file, int index){
	channel_node* last;
	/*if (ptr->id == 0) {
		  ptr->id = channel_id;
		  file->private_data = (void*) ptr;
		  return SUCCESS;
	  }*/
	  while(ptr->next != NULL) {
		  if (ptr->id == channel_id) {
			  file->private_data = (void*) ptr;
			  return 0;
		  }
		  ptr = ptr->next;
	  }
	  last = ptr;
	  if (last->id == channel_id) {
	 		  file->private_data = (void*) last;
	 		  return 0;
	 	  }
	  else {
		  channel_node* node = kmalloc(sizeof(channel_node), GFP_KERNEL);
		  if (node == NULL) return -1;
		  last->next = node;
		  create_node(node, index, file, channel_id,1);
	  }
	  return 0;

}


static void reset_structure(void){
	int k = 0;
	  while(k<256){
		  (*(device_array+k)).list = NULL;
		  k++;}
}

static void clean_structure(void){
	int k = 0;
		channel_node *curr_node, *prev_node;
		/*iterate over the device array*/
		while(k < 256){
			/*obtain pointer to the start of linked-list*/
			curr_node = (*(device_array+k)).list;
			/*iterate over nodes- all channels for device slot*/
			while (curr_node != NULL){
				/*
				 * remember current node*/
						prev_node = curr_node;
						/*advance current node to the next one*/
						curr_node = curr_node->next;
						/*
						 * if message exists in slot - free string*/
						if(prev_node->message!= NULL) kfree(prev_node->message);
						/*
						 * free the channel node*/
						kfree(prev_node);
					}
			k = k+1;
		}
}


//================== DEVICE FUNCTIONS ===========================

static int device_open(struct inode* inode, struct file*  file){

	int k;
	k = iminor(inode);
	if ((*(device_array+k)).list == NULL) {
		channel_node* node = kmalloc(sizeof(channel_node), GFP_KERNEL);
		if (node == NULL) return -ENOMEM;
		create_node(node, k, file,0,0);
	}
	else{
		file->private_data = (*(device_array+k)).list;
	}



  return SUCCESS;

}


//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset )
{
	int len, i, bool = 0;
	len = (int)((channel_node*)file->private_data)->len;


	  if ((((channel_node*)file->private_data)->id) == 0) {
		  return -EINVAL;
	  }
	  if (len == 0) {
		  return -EWOULDBLOCK;;
	  }


	 if (length < len) {
	 		  return -ENOSPC;
	 	  }
	 for( i = 0; i < len; ++i ){
	    if ((put_user(((channel_node*)file->private_data)->message[i], &buffer[i]))) bool = 1;
	  }
	 if (bool) return -EIO;

	return i;


}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write(struct file*  file, const char __user* buffer, size_t  length, loff_t* offset){
  int i, bool = 0;
  char the_message[BUF_LEN];
  if ((length > 128) || (length <=0 )) {
	  return -EMSGSIZE;
  }

  if ((((channel_node*)file->private_data)->id) == 0) {
	  return -EINVAL;
  }
  if (buffer == NULL) {
	  return -EINVAL;
  }

  for( i = 0;  i < length && i < BUF_LEN ; ++i )
  {
    if ((get_user(the_message[i], &buffer[i]))) bool = 1;
  }
  if (bool == 1) {
	  return -EIO;
  }
  ((channel_node*)file->private_data)->len = length;
  if(((channel_node*)file->private_data)->message!=NULL) kfree(((channel_node*)file->private_data)->message);
  ((channel_node*)file->private_data)->message = NULL;
  ((channel_node*)file->private_data)->message = kmalloc(sizeof(char)*length,GFP_KERNEL);
  if (((channel_node*)file->private_data)->message == NULL) return -ENOMEM;
  for( i = 0;  i < length && i < BUF_LEN ; ++i )
  {
	  ((channel_node*)file->private_data)->message[i] = the_message[i];
  }

  // return the number of input characters used
  return i;
}


//----------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long channel_id )
{
	channel_node* ptr;
	int index, x = 0;
	index = (int)(((channel_node*)file->private_data)->iminor);

  if(MSG_SLOT_CHANNEL != ioctl_command_id)
  {
	  return -EINVAL;}
  if (channel_id == 0) {
	 return -EINVAL;
  }

  ptr = (*(device_array+index)).list;
  x = set_channel(ptr, channel_id, file, index);
  if (x == -1) return -ENOMEM;


  return SUCCESS;
}


//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created


struct file_operations Fops =
{
  .owner	  = THIS_MODULE,
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl =          device_ioctl,
};

//---------------------------------------------------------------
// Initialize the module - Register the device
static int __init simple_init(void)
{
  int rc = -1;

  // Register driver capabilities. Obtain major num
  rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );

  // Negative values signify an error
  if( rc < 0 )
  {
    printk(KERN_ALERT "%s registraion failed for  %d\n", DEVICE_FILE_NAME, MAJOR_NUM );
    return rc;
  }
  /*reset the list paramter for all device cells in main memory structure*/
  reset_structure();
  return 0;
}

/*clean up function - iterates over our device array and in an inner loop free
 * the message in each channel_node and its message within (if exists)
 */
static void __exit simple_cleanup(void)
{

clean_structure();
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}


//---------------------------------------------------------------
/*INITIALIZE AND CLEANUP BUILT-IN FUNCTION*/
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================







