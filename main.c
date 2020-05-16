#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CONFIG_PINS 0
#define SET_PARAMETERS 1

#define DEV_0_TRIGGER_PIN 10
#define DEV_0_ECHO_PIN 5

#define DEV_0_NO_OF_SAMPLES 5
//please use sampling period more than 80
#define DEV_0_SAMPLING_PERIOD 100

#define DEV_1_TRIGGER_PIN 9
#define DEV_1_ECHO_PIN 4

#define DEV_1_NO_OF_SAMPLES 7
//please use sampling period more than 80
#define DEV_1_SAMPLING_PERIOD 100


struct buffer_node
{
    unsigned long long timestamp;
    unsigned long long int measurement;
};

void * device_0()
{
	//Opening device file
	int device;
	printf("\nDevice 1: Opening device file\n");
	device = open("/dev/HCSR_0", O_RDWR);
	if (device< 0){
		printf("\nDevice 1: Can not open device file.\n");		
		return 0;
	}
	
	//IOCTL CONFIG_PINS
	printf("\nDevice 1: Configuring pins\n");
	int *arr;
	arr= (int*)malloc(2*sizeof(int));
	arr[0] = DEV_0_TRIGGER_PIN;
	arr[1] = DEV_0_ECHO_PIN;
	ioctl(device,CONFIG_PINS,arr);
	
	//IOCTL SET_PARAMETERS
	printf("\nDevice 1: Setting parameters\n");
	int *arr2;
	arr2= (int*)malloc(2*sizeof(int));
	arr2[0] = DEV_0_NO_OF_SAMPLES;
	arr2[1] = DEV_0_SAMPLING_PERIOD;
 	ioctl(device,SET_PARAMETERS,arr2);
	
	//CALLING WRITE
	int input=0;
	printf("\nDevice 1: Calling write with value %d\n",input);
	write(device,&input,sizeof(int));

	//CALLING READ
	struct buffer_node temp;
	read(device,&temp,sizeof(struct buffer_node));
	printf("\nDevice 1: Distance is: %lld cm\n",temp.measurement);

	//CALLING WRITE TO CLEAR BUFFER
	input=1;
	printf("\nDevice 1: Calling write with value %d\n",input);
	write(device,&input,sizeof(int));

	//CALLING READ
	read(device,&temp,sizeof(struct buffer_node));
	printf("\nDevice 1: Distance is: %lld cm\n",temp.measurement);

	//CLOSING DEVICE FILE
	close(device);
	free(arr);
	free(arr2);
	return 0;
}

void * device_1()
{
	//Opening device file
	int device1;
	printf("\nDevice 2: Opening device file\n");
	device1 = open("/dev/HCSR_1", O_RDWR);
	if (device1< 0){
		printf("\nCan not open device file.\n");		
		return 0;
	}
	
	//IOCTL CONFIG_PINS
	printf("\nDevice 2: Configuring pins\n");
	int *arr1;
	arr1= (int*)malloc(2*sizeof(int));
	arr1[0] = DEV_1_TRIGGER_PIN;
	arr1[1] = DEV_1_ECHO_PIN;
	ioctl(device1,CONFIG_PINS,arr1);
	
	//IOCTL SET_PARAMETERS
	printf("\nDevice 2: Setting parameters\n");
	int *arr12;
	arr12= (int*)malloc(2*sizeof(int));
	arr12[0] = DEV_1_NO_OF_SAMPLES;
	arr12[1] = DEV_1_SAMPLING_PERIOD;
 	ioctl(device1,SET_PARAMETERS,arr12);
	
	//CALLING WRITE
	int input1=0;
	printf("\nDevice 2: Calling write with value %d\n",input1);
	write(device1,&input1,sizeof(int));

	//CALLING READ
	struct buffer_node temp1;
	read(device1,&temp1,sizeof(struct buffer_node));
	printf("\nDevice 2: Distance is: %lld cm\n",temp1.measurement);

	//CALLING WRITE TO CLEAR BUFFER
	input1=1;
	printf("\nDevice 2: Calling write with value %d\n",input1);
	write(device1,&input1,sizeof(int));

	//CALLING READ
	read(device1,&temp1,sizeof(struct buffer_node));
	printf("\nDevice 2: Distance is: %lld cm\n",temp1.measurement);


	//CLOSING DEVICE FILE
	close(device1);
	free(arr1);
	free(arr12);
	return 0;
}

int main(int argc, char **argv) 
{		
	pthread_t device_0_thread,device_1_thread;

	//Creating two threads to control two devices concurrently
	pthread_create(&device_0_thread, NULL, device_0, NULL);
	pthread_create(&device_1_thread, NULL, device_1, NULL);

	pthread_join(device_0_thread,NULL);
	pthread_join(device_1_thread,NULL);

	return 0;
}
