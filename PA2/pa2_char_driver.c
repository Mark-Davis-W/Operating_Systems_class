#include<linux/init.h>
#include<linux/module.h>

MODULE_AUTHOR ("Mark Davis");
MODULE_LICENSE ("GPL");
MODULE_INFO (intree, "Y");

#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

#define BUFFER_SIZE 1024

#define DEVICE_NAME "pa2_char_device"
#define major_number 240
static char* device_buff;

/* Define device_buffer and other global data structures you will need here */


ssize_t pa2_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/* length is the length of the userspace buffer*/
	/* offset will be set to current position of the opened file after read*/
	/* copy_to_user function: source is device_buffer and destination is the userspace buffer *buffer */
	
	if(length > BUFFER_SIZE-*offset)
	{
		printk(KERN_ALERT "Error: bytes to read is bigger than available size left: %lld\n",(BUFFER_SIZE-*offset));
		return -1;
	}
	
	if(copy_to_user(buffer, device_buff+*offset, length))
	{
		printk(KERN_ALERT "Error: bytes were unable to copy for read\n");
		return -1;
	}
	else
	{
		printk(KERN_ALERT "%zd bytes read\n",length);
	}

	*offset = *offset+length;
	
	return length;
}



ssize_t pa2_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/* length is the length of the userspace buffer*/
	/* current position of the opened file*/
	/* copy_from_user function: destination is device_buffer and source is the userspace buffer *buffer */
	
	if(length > BUFFER_SIZE-*offset)
	{
		printk(KERN_ALERT "Error: write attempted outside of available device size: %lld\n",(BUFFER_SIZE-*offset));
		return -1;
	}
	
	if(copy_from_user(device_buff+*offset, buffer, length))
	{
		printk(KERN_ALERT "Error: bytes were unable to write\n");
		return -1;
	}
	else
	{
		printk(KERN_ALERT "Success, bytes written: %zd\n", length);
	}
	
	*offset = *offset+length;
	
	return length;
}


int pa2_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	
	static int open_count = 0;
	open_count++;
	printk(KERN_ALERT "Pa2_char_device has been opened %d times\n", open_count);
	return 0;
}

int pa2_char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	
	static int close_count = 0;
	close_count++;
	printk(KERN_ALERT "Pa2_char_device has been closed %d times\n",close_count);
	return 0;
}

loff_t pa2_char_driver_seek (struct file *pfile, loff_t offset, int whence)
{
	/* Update open file position according to the values of offset and whence */
	
	loff_t n_pos = 0;
	
	switch(whence)
	{
	case 0:		// Start from beginning
		n_pos = offset;
		break;
	
	case 1:		// Start from Current
		n_pos = pfile->f_pos+offset;
		break;
	
	case 2:		// Start from end
		n_pos = BUFFER_SIZE+offset;
		break;
	
	}
	
	// If beyond or before file size leave f_pos alone and return -1
	if(n_pos > BUFFER_SIZE)
	{
		printk(KERN_ALERT "Error: This seek attempt is beyond end of file.\n");
		return -1;
	}
	else if(n_pos < 0)
	{
		printk(KERN_ALERT "Error: This seek attempt is beyond beginning of file.\n");
		return -1;
	}
	
	pfile->f_pos = n_pos;
	
	return 0;
}

struct file_operations pa2_char_driver_file_operations = {

	/* Add the function pointers to point to the corresponding file operations.
	   Look at the file fs.h in the linux souce code*/
	.owner   = THIS_MODULE,
	
	.open   = pa2_char_driver_open,		// int my_open  (struct inode *, struct file *);
	.release = pa2_char_driver_close,	// int my_close (struct inode *, struct file *);
	.read    = pa2_char_driver_read,	// ssize_t my_read  (struct file *, char __user *, size_t, loff_t *);
	.write   = pa2_char_driver_write,	// ssize_t my_write (struct file *, const char __user *, size_t, loff_t *);
	.llseek  = pa2_char_driver_seek		// loff_t  my_seek  (struct file *, loff_t, int);
};

static int pa2_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	/* register the device */
	
	device_buff = kmalloc(BUFFER_SIZE,GFP_KERNEL);
	memset(device_buff, '\0', BUFFER_SIZE);
	
	if(register_chrdev(major_number, DEVICE_NAME, &pa2_char_driver_file_operations) < 0)
	{
		printk(KERN_ALERT "Registering pa2 device failed with %d\n",major_number);
		return -1;
	}
	
	printk(KERN_ALERT "Inside %s function and major number set to: %d\n",__FUNCTION__,major_number);
	
	return 0;
}

static void pa2_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	/* unregister the device using the register_chrdev() function. */
	
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_ALERT "Inside %s function, unregistering device and freeing memory.\n", __FUNCTION__);
	
	memset(device_buff, '\0', BUFFER_SIZE);
	kfree(device_buff);
	
	return;
}

/* add module_init and module_exit to point to the corresponding init and exit function*/

module_init(pa2_char_driver_init);
module_exit(pa2_char_driver_exit);


