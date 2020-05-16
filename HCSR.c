#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <asm/div64.h>
#include <linux/kthread.h>
#define DEV_NAME_PREFIX "HCSR_0"
#define CONFIG_PINS 0
#define SET_PARAMETERS 1

//Input parameter (number of hcsr devices)
int device_count=1;
module_param(device_count,int,S_IRUSR);

//structure for single node in per device buffer
struct buffer_node
{
    unsigned long long timestamp;
    unsigned long long int measurement;
};

//per device structure
struct hcsr_dev {
    struct miscdevice my_misc;
    int trigger_pin_gpio;
    int trigger_pin_shield;
    int echo_pin_gpio;
    int echo_pin_shield;
    int irq_line;
    struct buffer_node dev_buffer[5];
    int read_location;
    int write_location;
    int is_full;
    int no_of_samples;
    int period;
    int is_measuring;
    unsigned long long begin;
    unsigned long long end;
}*hcsr_devices;

//Funtion prototypes
int perform_measurement(void *);
static irq_handler_t custom_irq_handler(unsigned int irq, void *device_data);
int check_and_set_trigger(int t);
int check_and_set_echo(int e,struct hcsr_dev *device);
void free_trigger(int t);
void free_echo(int e);

//method for reading current timestamp
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

int hcsr_open(struct inode *inode, struct file *dev_file)
{
	//Getting minor number from inode value
	int minor_num = iminor(inode);
	int i;
	struct hcsr_dev *hcsr_temp = NULL;
	printk("INFO: Opening device\n");
	//Matching minor number and getting per device
	for(i=0;i<device_count;i++)
	{
		if(hcsr_devices[i].my_misc.minor == minor_num)
		{
		    hcsr_temp = &hcsr_devices[i];
		    break;
		}
	}
	printk("INFO: Device with minor number %d opened\n", minor_num);
	printk("INFO: Device %s opened\n", (hcsr_temp->my_misc).name);
	dev_file->private_data = hcsr_temp;
	return 0;
}

int hcsr_release(struct inode *inode, struct file *dev_file)
{
    struct hcsr_dev *hcsr_temp;
    hcsr_temp = dev_file->private_data;
    printk("INFO: Device %s closed\n", (hcsr_temp->my_misc).name);
    return 0;
}

ssize_t hcsr_write(struct file *dev_file, const char *buffer, size_t count, loff_t *ppos)
{
	struct hcsr_dev *hcsr_temp = dev_file->private_data;
	int *input;
	int input_num;
	int i=0;
	int retval=0;
	input = kmalloc(sizeof(int),GFP_KERNEL);
	memset(input, 0 , sizeof(int));
	copy_from_user(input, buffer, count);					//copying contents from buffer to kernel space
	input_num = *(input);
	if(hcsr_temp->is_measuring==1)							//Checking for ongoing measurement
	{
		printk("INFO: There is an ongoing measurement. Try again after some time.\n");
		return -EINVAL;
	}
	else 													//If no measurement is ongoing, moving ahead.
	{
		if(input_num!=0)									//If input to write is 0, clearing the per-device buffer.
		{
			for(i=0;i<5;i++)
			{
				hcsr_temp->dev_buffer[i].timestamp = 0;
				hcsr_temp->dev_buffer[i].measurement = 0;
				hcsr_temp->write_location=0;				//resetting write and read pointers(index)
				hcsr_temp->read_location=0;
				hcsr_temp->is_full=0;
			}
		}
		retval = perform_measurement(hcsr_temp);			//else start a new measurement
		if(retval==-1)
		{
			printk("ERROR: Error in measurement. Check your pin configurations\n");
			return -EINVAL;
		}
		//kthread_run(perform_measurement,hcsr_temp,"worker_thread");  //spawing a kernel thread for doing a new measurement
	}
	kfree(input);
	return 0;
}

ssize_t hcsr_read(struct file *dev_file, char *buffer, size_t count, loff_t *ppos)
{
	int bytes_read = 0;
	int r=0;
	int w=0;
	int retval=0;
    struct hcsr_dev *hcsr_temp;
    struct buffer_node node; 
    hcsr_temp = dev_file->private_data;
    r=hcsr_temp->read_location;									//getting current buffer pointers
    w=hcsr_temp->write_location;
    if(w == r)   //Buffer empty									//Condition to check if buffer is empty
    {
    	if(hcsr_temp->is_measuring==1)							//If measurement is ongoing. Wait.
    	{
    		while(hcsr_temp->is_measuring==1)
    		{

    		}
    	}
    	else
    	{
    		retval = perform_measurement(hcsr_temp);			//else start a new measurement
			if(retval==-1)
			{
				printk("ERROR: Error in measurement. Check your pin configurations\n");
				return -EINVAL;
			}
    	}
    }
    node = hcsr_temp->dev_buffer[r];						
	copy_to_user(buffer,&node,sizeof(struct buffer_node));		//copying data to read buffer
	r = (r + 1) % 5;
	hcsr_temp->read_location=r;									//updating read location
	bytes_read = sizeof(struct buffer_node);
	return bytes_read;
}

long hcsr_ioctl(struct file *dev_file, unsigned int operation, unsigned long params)
{
	int *arr;
	int t=-1;
	int e=-1;
	struct hcsr_dev *hcsr_struct;
	hcsr_struct = dev_file->private_data;
	arr = (int*)params;
	if(operation==0)										//printing input
	{
		printk("INFO: IOCTL operation: CONFIG_PINS\n");
		printk("INFO: INPUT: Trigger pin: %d\n",arr[0]);
		printk("INFO: INPUT: Echo pin: %d\n",arr[1]);
	}
	else if(operation==1)
	{
		printk("INFO: IOCTL operation: SET_PARAMETERS\n");
		printk("INFO: INPUT: Number of samples: %d\n",arr[0]);
		printk("INFO: INPUT: Sampling period: %d\n",arr[1]);
	}
	switch(operation)
	{
		case 0:
			//TRIGGER PIN CONFIG
			//Checking if the trigger/echo pin is within valid range
			if(arr[0]<0 || arr[1]<0 || arr[0]>19 || arr[1]>19){
				printk("ERROR: Invalid pin numbers!\n");
				return -EINVAL;
			}
			//Checking if input trigger pin matches with previously configured trigger pin 
			if(hcsr_struct->trigger_pin_shield == arr[0])
			{	
				t = hcsr_struct->trigger_pin_gpio;
			}
			else
			{	
				//else freeing previously configured trigger pin(if any)
				free_trigger(hcsr_struct->trigger_pin_shield);
				//checking and configuring the trigger pin with proper pin multiplexing
				t = check_and_set_trigger(arr[0]);
			}
			if(t==-1)
			{	
				printk("ERROR: Trigger pin configuration failed!\n");		
				return -EINVAL;
			}
			else
			{
				//if no errors are encountered, setting trigger pin value to a variable in per-device struct
				hcsr_struct->trigger_pin_shield = arr[0];
				hcsr_struct->trigger_pin_gpio = t;
			}
			//ECHO PIN CONFIG
			//Checking if input echo pin matches with previously configured echo pin 
			if(hcsr_struct->echo_pin_shield == arr[1])
			{	
				e = hcsr_struct->echo_pin_gpio;
			}
			else
			{
				//else freeing previously configured echo pin(if any)
				free_echo(hcsr_struct->echo_pin_shield);
				//checking and configuring the echo pin with proper pin multiplexing
				e = check_and_set_echo(arr[1],hcsr_struct);
			}
			if(e==-1)
			{	
				//if error is encountered while configuring echo pin, then freeing the trigger pin
				free_trigger(t);	
				printk("ERROR: Echo pin configuration failed!\n");		
				return -EINVAL;
			}
			else
			{
				//else, setting per-device variables corresponding to echo pins
				hcsr_struct->echo_pin_shield = arr[1];
				hcsr_struct->echo_pin_gpio = e;
			}
		break;
		case 1:
			//Checking if input is valid
			if(arr[0]<=0 || arr[1]<=0){
				printk("ERROR: Invalid parameters!\n");
				return -EINVAL;
			}
			hcsr_struct->no_of_samples = arr[0];
			hcsr_struct->period = arr[1];
		break;
		default:
			//if opcode is invalid
			printk("ERROR: Invalid ioctl command\n");
			return -EINVAL;
	}
    return 0;
}

static struct file_operations hcsr_fileops = {
    .owner		= THIS_MODULE,           
    .open		= hcsr_open,        
    .release    = hcsr_release,     
    .write		= hcsr_write,       
    .read		= hcsr_read,        
    .unlocked_ioctl	= hcsr_ioctl,
};

int __init hcsr_init(void)
{
	int i=0;
	int retval=0;
	char character;
	char *name = DEV_NAME_PREFIX;
	char *temp;
	//Allocating memory for per device struct according to the number of devices user has specified
	hcsr_devices = (struct hcsr_dev*)kmalloc(device_count * sizeof(struct hcsr_dev), GFP_KERNEL);
	memset(hcsr_devices, 0, device_count * sizeof( struct hcsr_dev));
	while(i<device_count)
	{
		//Dynamically allocating minor number
		hcsr_devices[i].my_misc.minor = MISC_DYNAMIC_MINOR;

		//Creating device name string
		temp = kmalloc(strlen(name) * sizeof(char),GFP_KERNEL);
		strcpy(temp,name);
		character = i + '0';
		temp[5] = character;

		//misc device initialization
		hcsr_devices[i].my_misc.name = temp;
		hcsr_devices[i].my_misc.fops = &hcsr_fileops;
		retval = misc_register(&hcsr_devices[i].my_misc);

		//initializing other device variables
		hcsr_devices[i].trigger_pin_gpio=-1;
	    hcsr_devices[i].trigger_pin_shield=-1;
	    hcsr_devices[i].echo_pin_gpio=-1;
	    hcsr_devices[i].echo_pin_shield=-1;
	    hcsr_devices[i].read_location=0;
	    hcsr_devices[i].write_location=0;
	    hcsr_devices[i].is_full=0;
	    hcsr_devices[i].no_of_samples=0;
	    hcsr_devices[i].period=0;
	    hcsr_devices[i].is_measuring=0;
	    hcsr_devices[i].irq_line=-1;
		i++;
		printk("INFO: HCSR device %s initialized.\n",temp);
	}
	printk("INFO: All HCSR devices initialized.\n");
	return 0;
}

void __exit hcsr_exit(void)
{
	int i=0;
	struct hcsr_dev *dev_data;
	printk("INFO: EXITING\n");
	while(i<device_count)
	{
		//freeing all the pins and IRQ
		dev_data = &hcsr_devices[i];
		if(dev_data->irq_line!=-1)
		{
			free_irq(dev_data->irq_line,(void *)&hcsr_devices[i]);
		}
		free_echo(dev_data->echo_pin_shield);
		free_trigger(dev_data->trigger_pin_shield);	
		//Deregistering misc device	
		misc_deregister(&dev_data->my_misc);
		printk("INFO: Misc driver deregistered\n");
		i++;
	}
	//Freeing device structure array
	kfree(hcsr_devices);
	printk("INFO: All device structures freed\n");
	printk("GOODBYE!\n");
}

module_init(hcsr_init);
module_exit(hcsr_exit);
MODULE_LICENSE("GPL");

//Function for performing new measurement
int perform_measurement(void *dev)
{
	struct hcsr_dev *device = (struct hcsr_dev *)dev; 
	int i=0;
	int j=0;
	int m=0;
	int sampling_period=0;
	unsigned long long int sum=0;
	unsigned long long int largest_value=0;
	unsigned long long int smallest_value=ULLONG_MAX;
	unsigned long long int t=0;
	printk("INFO: Starting measurement\n");
	device->is_measuring=1;
	m = device->no_of_samples;
	sampling_period = device->period;
	if(m == 0 || sampling_period == 0)
	{
		printk("ERROR: Number of samples/period not configured!\n");
		return -1;
	}
	if(device->trigger_pin_gpio==-1 || device->echo_pin_gpio==-1)
	{
		printk("ERROR: Trigger/Echo pin not configured!\n");
		return -1;
	}
	for (i=0;i<m+2;i++)
	{
		//Setting trigger pin to 1 for triggering the HCSR sensor for getting a sample 
		gpio_set_value_cansleep(device->trigger_pin_gpio,1);
		udelay(12);
		gpio_set_value_cansleep(device->trigger_pin_gpio,0);
		//Sleeping for sampling period defined by the user
		msleep(sampling_period);
		//Calculating the time difference between RISING and FALLING edges
	 	t = device->end - device->begin;
	 	printk("INFO: Time difference is %llu \n",t);
	 	t=div_u64(t,400); //to get time in micro seconds
	 	t=div_u64(t,58); //to get distance in cm
		printk("INFO: Sample distance is %llu cm\n",t);
	 	sum=sum +t;
	 	//Calulating the two outliers in the measurement
	 	if(t > largest_value)
	 	{
	 		largest_value=t;
	 	}
	 	if(t < smallest_value)
	 	{
	 		smallest_value=t;
		}
	}	
 	//Subtracting the two outliers from sum
	sum = sum - largest_value;
	sum = sum - smallest_value;
	//Taking average of m samples
	sum = div_u64(sum,m);
	printk("INFO: Average measured distance is %llu cm\n",sum);
	//Getting read and write pointers (index)
	i = device->write_location;
	j = device->read_location;
	i = i%5;
	//Checking if device is full
	if(device->is_full==1)
	{
		//If full, then checking if the location to be overwritten is read or not
		if(i==j)  
		{
			//If value to be overwritten is not read yet, then moving the read pointer to next oldest value
			device->read_location = (j+1)%5;
		}
	}
	//Setting buffer node's values
	device->dev_buffer[i].timestamp = native_read_tsc();
	device->dev_buffer[i].measurement = sum;
	printk("INFO: Distance is %llu cm\n",sum);
	//Updating write pointer
	i = (i+1)%5;
	if(i==j)
	{
		//Buffer full;
		device->is_full=1;
	}
	else
	{	
		//Buffer not full
		device->is_full=0;
	}
	device->write_location=i;
	device->is_measuring=0;
	printk("INFO: Measurement done!\n");
	return 0;
}

static irq_handler_t custom_irq_handler(unsigned int irq, void *device_data)
{
	struct hcsr_dev *dev_data = (struct hcsr_dev*)device_data;
	//Checking echo pin signal value
	if(gpio_get_value(dev_data->echo_pin_gpio))
	{
		dev_data->begin = native_read_tsc();
		//printk("INFO: RISING edge interrupt\n");
	}
	else
	{
		dev_data->end = native_read_tsc();
		//printk("INFO: FALLING edge interrupt\n");
	}
	return (irq_handler_t) IRQ_HANDLED;
}

int check_and_set_trigger(int t)
{
	//Initializing gpio pin values
	int linux_gpio=-1;
	int level_shifter=-1;
	int mux_1=-1;
	int mux_2=-1;
	//Depending on user input, configuring the necessary pins accordingly
	switch(t)
	{
		case 0:
		    linux_gpio = 11;
		    level_shifter = 32;
		    break;
		case 1:
		    linux_gpio = 12;
		    level_shifter = 28;
		    mux_1 = 45;
		    break;
		case 2:
		    linux_gpio = 13;
		    level_shifter = 34;
		    mux_1 = 77;
		    break;
		case 3:
		    linux_gpio = 14;
		    level_shifter = 16;
		    mux_1 = 76;
		    mux_2 = 64;
		    break;
		case 4:
		    linux_gpio = 6;
		    level_shifter = 36;
		    break;
		case 5:
		    linux_gpio = 0;
		    level_shifter = 18;
		    mux_1 = 66;
		    break;
		case 6:
		    linux_gpio = 1;
		    level_shifter = 20;
		    mux_1 = 68;
		    break;
		case 7:
		    linux_gpio = 38;
		    break;
		case 8:
		    linux_gpio = 40;
		    break;
		case 9:
		    linux_gpio = 4;
		    level_shifter = 22;
		    mux_1 = 70;
		    break;
		case 10:
		    linux_gpio = 10;
		    level_shifter = 26;
		    mux_1 = 74;
		    break;
		case 11:
		    linux_gpio = 5;
		    level_shifter = 24;
		    mux_1 = 44;
		    mux_2 = 72;
		    break;
		case 12:
		    linux_gpio = 15;
		    level_shifter = 42;
		    break;
		case 13:
		    linux_gpio = 7;
		    level_shifter = 30;
		    mux_1 = 46;
		case 14:
		    linux_gpio = 48;
		    break;
		case 15:
		    linux_gpio = 50;
		    break;
		case 16:
		    linux_gpio = 52;
		    break;
		case 17:
		    linux_gpio = 54;
		    break;
		case 18:
		    linux_gpio = 56;
		    mux_1 = 60;
		    mux_2 = 78;
		    break;
		case 19:
		    linux_gpio = 58;
		    mux_1 = 60;
		    mux_2 = 79;
		    break;
		default:
		    printk("ERROR: Invalid pin selected for trigger\n");
		    return -1;
	}
	//Requesting all the pins required
	//If any pin configuration fails then freeing all the previously requested pins
	if(gpio_request(linux_gpio, "gpio_out") != 0 )
	{
		printk("ERROR: linux_gpio error!\n");
		return -1;
	}
	if(level_shifter!=-1)
	{
		if( gpio_request(level_shifter, "dir_out") != 0 )
		{
			printk("ERROR: level_shifter error!\n");
			gpio_free(linux_gpio);
			return -1;
		}
	}
	if(mux_1!=-1)
	{
		if( gpio_request(mux_1, "pin_mux_1") != 0 )
		{
			printk("ERROR: pin_mux_1 error!\n");
			gpio_free(linux_gpio);
			if(level_shifter!=-1)
			{
				gpio_free(level_shifter);
			}
			return -1;
		}
	}
	if(mux_2!=-1)
	{
		if( gpio_request(mux_2, "pin_mux_2") != 0 )
		{
			printk("ERROR: pin_mux_2 error!\n");
			gpio_free(linux_gpio);
			if(level_shifter!=-1)
			{
				gpio_free(level_shifter);
			}
			gpio_free(mux_1);
			return -1;
		}
	}
	//Setting values to all requested pins as required
	if(level_shifter!=-1)
	{
		gpio_direction_output(level_shifter, 0);
	}
	if(mux_1!=-1)
	{
		if(mux_1==60)
		{
			gpio_direction_output(mux_1, 1);
		}
		else
		{
			if(mux_1>63)
			{
				gpio_set_value_cansleep(mux_1, 0);
			}
			else
			{
				gpio_direction_output(mux_1, 0);
			}
		}
	}
	if(mux_2!=-1)
	{
		if(mux_2==78 || mux_2==79)
		{
			gpio_set_value_cansleep(mux_2, 1);
		}
		else
		{
			if(mux_2>63)
			{
				gpio_set_value_cansleep(mux_2, 0);
			}
			else
			{
				gpio_direction_output(mux_2, 0);
			}
		}
	}
	gpio_direction_output(linux_gpio,0);
	//returning gpio pin for echo
	return linux_gpio;
}

void free_trigger(int t)
{
	//Getting the pins required to be freed based on given shield pin
	int linux_gpio=-1;
	int level_shifter=-1;
	int mux_1=-1;
	int mux_2=-1;
	switch(t)
	{
		case 0:
		    linux_gpio = 11;
		    level_shifter = 32;
		    break;
		case 1:
		    linux_gpio = 12;
		    level_shifter = 28;
		    mux_1 = 45;
		    break;
		case 2:
		    linux_gpio = 13;
		    level_shifter = 34;
		    mux_1 = 77;
		    break;
		case 3:
		    linux_gpio = 14;
		    level_shifter = 16;
		    mux_1 = 76;
		    mux_2 = 64;
		    break;
		case 4:
		    linux_gpio = 6;
		    level_shifter = 36;
		    break;
		case 5:
		    linux_gpio = 0;
		    level_shifter = 18;
		    mux_1 = 66;
		    break;
		case 6:
		    linux_gpio = 1;
		    level_shifter = 20;
		    mux_1 = 68;
		    break;
		case 7:
		    linux_gpio = 38;
		    break;
		case 8:
		    linux_gpio = 40;
		    break;
		case 9:
		    linux_gpio = 4;
		    level_shifter = 22;
		    mux_1 = 70;
		    break;
		case 10:
		    linux_gpio = 10;
		    level_shifter = 26;
		    mux_1 = 74;
		    break;
		case 11:
		    linux_gpio = 5;
		    level_shifter = 24;
		    mux_1 = 44;
		    mux_2 = 72;
		    break;
		case 12:
		    linux_gpio = 15;
		    level_shifter = 42;
		    break;
		case 13:
		    linux_gpio = 7;
		    level_shifter = 30;
		    mux_1 = 46;
		case 14:
		    linux_gpio = 48;
		    break;
		case 15:
		    linux_gpio = 50;
		    break;
		case 16:
		    linux_gpio = 52;
		    break;
		case 17:
		    linux_gpio = 54;
		    break;
		case 18:
		    linux_gpio = 56;
		    mux_1 = 60;
		    mux_2 = 78;
		    break;
		case 19:
		    linux_gpio = 58;
		    mux_1 = 60;
		    mux_2 = 79;
		    break;
		default:
		    return;
		    break;
	}
	//freeing pins which were configured 
	if(linux_gpio!=-1)
	{
		gpio_free(linux_gpio);
	}
	if(level_shifter!=-1)
	{
		gpio_free(level_shifter);
	}
	if(mux_1!=-1)
	{
		gpio_free(mux_1);
	}
	if(mux_2!=-1)
	{
		gpio_free(mux_2);
	}
}
int check_and_set_echo(int e,struct hcsr_dev *device)
{
	//Initializing gpio pin values
	struct hcsr_dev *dev_data = device; 
	int linux_gpio=-1;
	int level_shifter=-1;
	int mux_1=-1;
	int mux_2=-1;
	int result=-1;
	int irq_no=-1;
	char *dev_name;
	dev_name = kmalloc(strlen(dev_data->my_misc.name) * sizeof(char),GFP_KERNEL);
	strcpy(dev_name,dev_data->my_misc.name);
	//Getting pin numbers based on given shield pin
	switch(e)
	{
		case 2:
		    linux_gpio = 61;
		    mux_1 = 77;
		    break;
		case 3:
		    linux_gpio = 62;
		    mux_1 = 76;
		    mux_2 = 64;
		    break;
		case 4:
		    linux_gpio = 6;
		    level_shifter = 36;
		    break;
		case 5:
		    linux_gpio = 0;
		    level_shifter = 18;
		    mux_1 = 66;
		    break;
		case 6:
		    linux_gpio = 1;
		    level_shifter = 20;
		    mux_1 = 68;
		    break;
		case 9:
		    linux_gpio = 4;
		    level_shifter = 22;
		    mux_1 = 70;
		    break;
		case 11:
		    linux_gpio = 5;
		    level_shifter = 24;
		    mux_1 = 44;
		    mux_2 = 72;
		    break;
		case 13:
		    linux_gpio = 7;
		    level_shifter = 30;
		    mux_1 = 46;
		    break;
		case 14:
		    linux_gpio = 48;
		    break;
		case 15:
		    linux_gpio = 50;
		    break;
		case 16:
		    linux_gpio = 52;
		    break;
		case 17:
		    linux_gpio = 54;
		    break;
		case 18:
		    linux_gpio = 56;
		    mux_1 = 60;
		    mux_2 = 78;
		    break;
		case 19:
		    linux_gpio = 58;
		    mux_1 = 60;
		    mux_2 = 79;
		    break;
		default:
			printk("ERROR: Invalid echo pin!\n");
			return -1;
			break;
    }
    //Requesting all necessary pins
    //Freeing all previously configured pins if any pin fails in request call
    if(gpio_request(linux_gpio, "gpio_in") != 0 )
	{
		printk("ERROR: linux_gpio error!\n");
		return -1;
	}
	if(level_shifter!=-1)
	{
		if( gpio_request(level_shifter, "dir_in") != 0 )
		{
			printk("ERROR: level_shifter error!\n");
			gpio_free(linux_gpio);
			return -1;
		}
	}
	if(mux_1!=-1)
	{
		if( gpio_request(mux_1, "pin_mux_1") != 0 )
		{
			printk("ERROR: pin_mux_1 error!\n");
			gpio_free(linux_gpio);
			if(level_shifter!=-1)
			{
				gpio_free(level_shifter);
			}
			return -1;
		}
	}
	if(mux_2!=-1)
	{
		if( gpio_request(mux_2, "pin_mux_2") != 0 )
		{
			printk("ERROR: pin_mux_2 error!\n");
			gpio_free(linux_gpio);
			if(level_shifter!=-1)
			{
				gpio_free(level_shifter);
			}
			gpio_free(mux_1);
			return -1;
		}
	}
	//Setting all the pins to appropriate values
	if(level_shifter!=-1)
	{
		gpio_direction_output(level_shifter, 1);
	}
	if(mux_1!=-1)
	{
		if(mux_1==60)
		{
			gpio_direction_output(mux_1, 1);
		}
		else
		{
			if(mux_1>63)
			{
				gpio_set_value_cansleep(mux_1, 0);
			}
			else
			{
				gpio_direction_output(mux_1, 0);
			}
		}
	}
	if(mux_2!=-1)
	{
		if(mux_2==78 || mux_2==79)
		{
			gpio_set_value_cansleep(mux_2, 1);
		}
		else
		{
			if(mux_2>63)
			{
				gpio_set_value_cansleep(mux_2, 0);
			}
			else
			{
				gpio_direction_output(mux_2, 0);
			}
		}
	}
	gpio_direction_input(linux_gpio);
	//Getting IRQ number based on gpio echo pin
	irq_no = gpio_to_irq(linux_gpio);
	printk("INFO: IRQ number: %d\n",irq_no);
	if(irq_no<0)
	{
		printk("ERROR: IRQ number error\n");
		gpio_free(linux_gpio);
		return -1;
	}
	//Requesting IRQ to be configured for both edges (RISING AND FALLING)
	result = request_irq(irq_no,(irq_handler_t) custom_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING , "both_edge", (void *)dev_data);
	if (result < 0)
	{
		printk("ERROR: in request_irq call\n");
		gpio_free(linux_gpio);
		return -1;
	}
	else
	{
		printk("INFO: IRQ Success!\n");
	}
	//Setting IRQ number in device struct for future reference (needed while freeing IRQ)
	dev_data->irq_line = irq_no;
	return linux_gpio;
}

void free_echo(int e)
{
	//Getting the pins required to be freed based on given shield pin
	int linux_gpio=-1;
	int level_shifter=-1;
	int mux_1=-1;
	int mux_2=-1;
	switch(e)
	{
		case 2:
		    linux_gpio = 61;
		    mux_1 = 77;
		    break;
		case 3:
		    linux_gpio = 62;
		    mux_1 = 76;
		    mux_2 = 64;
		    break;
		case 4:
		    linux_gpio = 6;
		    level_shifter = 36;
		    break;
		case 5:
		    linux_gpio = 0;
		    level_shifter = 18;
		    mux_1 = 66;
		    break;
		case 6:
		    linux_gpio = 1;
		    level_shifter = 20;
		    mux_1 = 68;
		    break;
		case 9:
		    linux_gpio = 4;
		    level_shifter = 22;
		    mux_1 = 70;
		    break;
		case 11:
		    linux_gpio = 5;
		    level_shifter = 24;
		    mux_1 = 44;
		    mux_2 = 72;
		    break;
		case 13:
		    linux_gpio = 7;
		    level_shifter = 30;
		    mux_1 = 46;
		    break;
		case 14:
		    linux_gpio = 48;
		    break;
		case 15:
		    linux_gpio = 50;
		    break;
		case 16:
		    linux_gpio = 52;
		    break;
		case 17:
		    linux_gpio = 54;
		    break;
		case 18:
		    linux_gpio = 56;
		    mux_1 = 60;
		    mux_2 = 78;
		    break;
		case 19:
		    linux_gpio = 58;
		    mux_1 = 60;
		    mux_2 = 79;
		    break;
		default:
		    return;
		    break;
    }
    //Freeing pins which were configured
	if(linux_gpio!=-1)
	{
		gpio_free(linux_gpio);
	}
	if(level_shifter!=-1)
	{
		gpio_free(level_shifter);
	}
	if(mux_1!=-1)
	{
		gpio_free(mux_1);
	}
	if(mux_2!=-1)
	{
		gpio_free(mux_2);
	}
}
