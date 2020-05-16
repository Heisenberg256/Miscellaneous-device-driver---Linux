Name: SAHIL PATIL
ASU ID: 1217436155

EOSI assignment 2

Part 1: A Linux kernel module to enable user-space device interface for HC-SR04

Steps to use the software:

0. Configure the pin numbers in main.c according to your own setup

		for example:

		#define DEV_0_TRIGGER_PIN 10
		#define DEV_0_ECHO_PIN 5

		#define DEV_0_NO_OF_SAMPLES 5
		#define DEV_0_SAMPLING_PERIOD 100

		change the above variables as needed

1. Make/build/compile the the source code using the given Makefile

	run the following command in the source directory:

	make 

2. To install kernel module

	insmod HCSR.ko device_count= <number of devices required>

3. To test driver functions using user program, run
	
	./hcsr_tester

4. After testing remove the kernel module using following command

	rmmod HCSR.ko
	
5. To clean source directory use:
	
	make clean

Note: 
1. In the hcsr_tester program, I have written the code to test operations on 2 hcsr devices concurrently. Please change the code according to what you will be testing.
2. I have only allowed configuration of those pins which support both-edge interrupt triggering. Please use only those pins for echo.
3. Please use sampling period above 80 for proper execution
4. For testing purpose, I have tested OPEN, IOCTL with both modes(CONFIG_PINS & SET_PARAMETERS), WRITE with both modes(1 & 0) and READ functions. Please feel free to test any other condition.



SAMPLE OUTPUT:

1. ON STDOUT:

	root@quark:/home/sahil# insmod HCSR.ko device_count=2
	root@quark:/home/sahil# ./hcsr_tester 

	Device 2: Opening device file

	Device 1: Opening device file

	Device 2: Configuring pins

	Device 1: Configuring pins

	Device 2: Setting parameters

	Device 2: Calling write with value 0

	Device 1: Setting parameters

	Device 1: Calling write with value 0

	Device 1: Distance is: 26 cm

	Device 1: Calling write with value 1

	Device 2: Distance is: 26 cm

	Device 2: Calling write with value 1

	Device 1: Distance is: 27 cm

	Device 2: Distance is: 26 cm
	root@quark:/home/sahil# ./hcsr_tester 

	Device 2: Opening device file

	Device 1: Opening device file

	Device 2: Configuring pins

	Device 2: Setting parameters

	Device 1: Configuring pins

	Device 2: Calling write with value 0

	Device 1: Setting parameters

	Device 1: Calling write with value 0

	Device 1: Distance is: 19 cm

	Device 1: Calling write with value 1

	Device 2: Distance is: 18 cm

	Device 2: Calling write with value 1

	Device 1: Distance is: 19 cm

	Device 2: Distance is: 19 cm
	root@quark:/home/sahil# ./hcsr_tester 

	Device 2: Opening device file

	Device 1: Opening device file

	Device 2: Configuring pins

	Device 1: Configuring pins

	Device 2: Setting parameters

	Device 1: Setting parameters

	Device 2: Calling write with value 0

	Device 1: Calling write with value 0

	Device 1: Distance is: 52 cm

	Device 1: Calling write with value 1

	Device 2: Distance is: 40 cm

	Device 2: Calling write with value 1

	Device 1: Distance is: 40 cm

	Device 2: Distance is: 40 cm
	root@quark:/home/sahil# rmmod HCSR.ko 
	root@quark:/home/sahil# 


2. On dmesg:

	root@quark:/home/sahil# dmesg
	[ 1493.711956] INFO: HCSR device HCSR_0 initialized.
	[ 1493.734199] INFO: HCSR device HCSR_1 initialized.
	[ 1493.738950] INFO: All HCSR devices initialized.
	[ 1507.554667] INFO: Opening device
	[ 1507.558148] INFO: Device with minor number 56 opened
	[ 1507.563280] INFO: Device HCSR_1 opened
	[ 1507.571339] INFO: Opening device
	[ 1507.574816] INFO: Device with minor number 57 opened
	[ 1507.579824] INFO: Device HCSR_0 opened
	[ 1507.593058] INFO: IOCTL operation: CONFIG_PINS
	[ 1507.597751] INFO: INPUT: Trigger pin: 9
	[ 1507.601734] INFO: INPUT: Echo pin: 4
	[ 1507.611682] INFO: IOCTL operation: CONFIG_PINS
	[ 1507.616391] INFO: INPUT: Trigger pin: 10
	[ 1507.620461] INFO: INPUT: Echo pin: 5
	[ 1507.630781] INFO: IRQ number: 60
	[ 1507.634121] INFO: IRQ Success!
	[ 1507.643111] INFO: IOCTL operation: SET_PARAMETERS
	[ 1507.648126] INFO: INPUT: Number of samples: 7
	[ 1507.652642] INFO: INPUT: Sampling period: 100
	[ 1507.663033] INFO: Starting measurement
	[ 1507.670240] INFO: IRQ number: 54
	[ 1507.673582] INFO: IRQ Success!
	[ 1507.679044] INFO: IOCTL operation: SET_PARAMETERS
	[ 1507.684250] INFO: INPUT: Number of samples: 5
	[ 1507.688647] INFO: INPUT: Sampling period: 100
	[ 1507.701428] INFO: Starting measurement
	[ 1507.770128] INFO: Time difference is 623612 
	[ 1507.774443] INFO: Sample distance is 26 cm
	[ 1507.810148] INFO: Time difference is 632218 
	[ 1507.814463] INFO: Sample distance is 27 cm
	[ 1507.880129] INFO: Time difference is 614258 
	[ 1507.884443] INFO: Sample distance is 26 cm
	[ 1507.920433] INFO: Time difference is 632946 
	[ 1507.924747] INFO: Sample distance is 27 cm
	[ 1507.990141] INFO: Time difference is 611136 
	[ 1507.994456] INFO: Sample distance is 26 cm
	[ 1508.030142] INFO: Time difference is 620404 
	[ 1508.034456] INFO: Sample distance is 26 cm
	[ 1508.100150] INFO: Time difference is 622552 
	[ 1508.104455] INFO: Sample distance is 26 cm
	[ 1508.140149] INFO: Time difference is 632354 
	[ 1508.144464] INFO: Sample distance is 27 cm
	[ 1508.210141] INFO: Time difference is 614980 
	[ 1508.214455] INFO: Sample distance is 26 cm
	[ 1508.250149] INFO: Time difference is 623450 
	[ 1508.254463] INFO: Sample distance is 26 cm
	[ 1508.320152] INFO: Time difference is 615388 
	[ 1508.324466] INFO: Sample distance is 26 cm
	[ 1508.360149] INFO: Time difference is 633226 
	[ 1508.364463] INFO: Sample distance is 27 cm
	[ 1508.430150] INFO: Time difference is 606552 
	[ 1508.434464] INFO: Sample distance is 26 cm
	[ 1508.470149] INFO: Time difference is 635138 
	[ 1508.474463] INFO: Sample distance is 27 cm
	[ 1508.478586] INFO: Average measured distance is 26 cm
	[ 1508.483702] INFO: Distance is 26 cm
	[ 1508.487226] INFO: Measurement done!
	[ 1508.501079] INFO: Starting measurement
	[ 1508.540146] INFO: Time difference is 618120 
	[ 1508.544460] INFO: Sample distance is 26 cm
	[ 1508.610150] INFO: Time difference is 634424 
	[ 1508.614464] INFO: Sample distance is 27 cm
	[ 1508.650149] INFO: Time difference is 621172 
	[ 1508.654463] INFO: Sample distance is 26 cm
	[ 1508.658586] INFO: Average measured distance is 26 cm
	[ 1508.663702] INFO: Distance is 26 cm
	[ 1508.667226] INFO: Measurement done!
	[ 1508.681810] INFO: Starting measurement
	[ 1508.720146] INFO: Time difference is 638490 
	[ 1508.724460] INFO: Sample distance is 27 cm
	[ 1508.790153] INFO: Time difference is 614534 
	[ 1508.794467] INFO: Sample distance is 26 cm
	[ 1508.830149] INFO: Time difference is 638822 
	[ 1508.834463] INFO: Sample distance is 27 cm
	[ 1508.900128] INFO: Time difference is 621980 
	[ 1508.904443] INFO: Sample distance is 26 cm
	[ 1508.940149] INFO: Time difference is 638734 
	[ 1508.944463] INFO: Sample distance is 27 cm
	[ 1509.010150] INFO: Time difference is 620136 
	[ 1509.014464] INFO: Sample distance is 26 cm
	[ 1509.050150] INFO: Time difference is 625502 
	[ 1509.054464] INFO: Sample distance is 26 cm
	[ 1509.120150] INFO: Time difference is 619132 
	[ 1509.124464] INFO: Sample distance is 26 cm
	[ 1509.160149] INFO: Time difference is 638314 
	[ 1509.164463] INFO: Sample distance is 27 cm
	[ 1509.230150] INFO: Time difference is 621000 
	[ 1509.234464] INFO: Sample distance is 26 cm
	[ 1509.270149] INFO: Time difference is 638640 
	[ 1509.274463] INFO: Sample distance is 27 cm
	[ 1509.278588] INFO: Average measured distance is 27 cm
	[ 1509.283704] INFO: Distance is 27 cm
	[ 1509.287227] INFO: Measurement done!
	[ 1509.298881] INFO: Device HCSR_0 closed
	[ 1509.340126] INFO: Time difference is 624620 
	[ 1509.344440] INFO: Sample distance is 26 cm
	[ 1509.450131] INFO: Time difference is 617602 
	[ 1509.454445] INFO: Sample distance is 26 cm
	[ 1509.560131] INFO: Time difference is 619220 
	[ 1509.564445] INFO: Sample distance is 26 cm
	[ 1509.670150] INFO: Time difference is 630400 
	[ 1509.674465] INFO: Sample distance is 27 cm
	[ 1509.678587] INFO: Average measured distance is 26 cm
	[ 1509.683704] INFO: Distance is 26 cm
	[ 1509.687228] INFO: Measurement done!
	[ 1509.698805] INFO: Device HCSR_1 closed
	[ 1519.334650] INFO: Opening device
	[ 1519.338135] INFO: Device with minor number 56 opened
	[ 1519.343267] INFO: Device HCSR_1 opened
	[ 1519.352891] INFO: Opening device
	[ 1519.356372] INFO: Device with minor number 57 opened
	[ 1519.361482] INFO: Device HCSR_0 opened
	[ 1519.368218] INFO: IOCTL operation: CONFIG_PINS
	[ 1519.372874] INFO: INPUT: Trigger pin: 9
	[ 1519.376748] INFO: INPUT: Echo pin: 4
	[ 1519.389727] INFO: IOCTL operation: SET_PARAMETERS
	[ 1519.394651] INFO: INPUT: Number of samples: 7
	[ 1519.399040] INFO: INPUT: Sampling period: 100
	[ 1519.406561] INFO: IOCTL operation: CONFIG_PINS
	[ 1519.411207] INFO: INPUT: Trigger pin: 10
	[ 1519.415167] INFO: INPUT: Echo pin: 5
	[ 1519.423185] INFO: Starting measurement
	[ 1519.430805] INFO: IOCTL operation: SET_PARAMETERS
	[ 1519.435762] INFO: INPUT: Number of samples: 5
	[ 1519.440265] INFO: INPUT: Sampling period: 100
	[ 1519.456452] INFO: Starting measurement
	[ 1519.530130] INFO: Time difference is 430692 
	[ 1519.534444] INFO: Sample distance is 18 cm
	[ 1519.570149] INFO: Time difference is 437028 
	[ 1519.574463] INFO: Sample distance is 18 cm
	[ 1519.640150] INFO: Time difference is 437058 
	[ 1519.644464] INFO: Sample distance is 18 cm
	[ 1519.680129] INFO: Time difference is 459980 
	[ 1519.684444] INFO: Sample distance is 19 cm
	[ 1519.750150] INFO: Time difference is 443418 
	[ 1519.754464] INFO: Sample distance is 19 cm
	[ 1519.790153] INFO: Time difference is 466868 
	[ 1519.794467] INFO: Sample distance is 20 cm
	[ 1519.860149] INFO: Time difference is 457058 
	[ 1519.864463] INFO: Sample distance is 19 cm
	[ 1519.900149] INFO: Time difference is 458300 
	[ 1519.904463] INFO: Sample distance is 19 cm
	[ 1519.970141] INFO: Time difference is 445354 
	[ 1519.974455] INFO: Sample distance is 19 cm
	[ 1520.010182] INFO: Time difference is 461732 
	[ 1520.014496] INFO: Sample distance is 19 cm
	[ 1520.080149] INFO: Time difference is 438758 
	[ 1520.084464] INFO: Sample distance is 18 cm
	[ 1520.120149] INFO: Time difference is 465616 
	[ 1520.124463] INFO: Sample distance is 20 cm
	[ 1520.190142] INFO: Time difference is 457652 
	[ 1520.194456] INFO: Sample distance is 19 cm
	[ 1520.230150] INFO: Time difference is 466940 
	[ 1520.234464] INFO: Sample distance is 20 cm
	[ 1520.238588] INFO: Average measured distance is 19 cm
	[ 1520.243705] INFO: Distance is 19 cm
	[ 1520.247229] INFO: Measurement done!
	[ 1520.261023] INFO: Starting measurement
	[ 1520.300145] INFO: Time difference is 451582 
	[ 1520.304460] INFO: Sample distance is 19 cm
	[ 1520.370158] INFO: Time difference is 464008 
	[ 1520.374472] INFO: Sample distance is 20 cm
	[ 1520.410164] INFO: Time difference is 459090 
	[ 1520.414478] INFO: Sample distance is 19 cm
	[ 1520.418601] INFO: Average measured distance is 18 cm
	[ 1520.423718] INFO: Distance is 18 cm
	[ 1520.427242] INFO: Measurement done!
	[ 1520.436419] INFO: Starting measurement
	[ 1520.480145] INFO: Time difference is 463568 
	[ 1520.484460] INFO: Sample distance is 19 cm
	[ 1520.550150] INFO: Time difference is 653774 
	[ 1520.554464] INFO: Sample distance is 28 cm
	[ 1520.590149] INFO: Time difference is 465596 
	[ 1520.594464] INFO: Sample distance is 20 cm
	[ 1520.660129] INFO: Time difference is 457510 
	[ 1520.664443] INFO: Sample distance is 19 cm
	[ 1520.700149] INFO: Time difference is 459628 
	[ 1520.704463] INFO: Sample distance is 19 cm
	[ 1520.770150] INFO: Time difference is 455058 
	[ 1520.774463] INFO: Sample distance is 19 cm
	[ 1520.810149] INFO: Time difference is 470752 
	[ 1520.814463] INFO: Sample distance is 20 cm
	[ 1520.880130] INFO: Time difference is 459606 
	[ 1520.884444] INFO: Sample distance is 19 cm
	[ 1520.920149] INFO: Time difference is 460824 
	[ 1520.924463] INFO: Sample distance is 19 cm
	[ 1520.990131] INFO: Time difference is 466350 
	[ 1520.994445] INFO: Sample distance is 20 cm
	[ 1521.030142] INFO: Time difference is 473340 
	[ 1521.034456] INFO: Sample distance is 20 cm
	[ 1521.038579] INFO: Average measured distance is 19 cm
	[ 1521.043695] INFO: Distance is 19 cm
	[ 1521.047219] INFO: Measurement done!
	[ 1521.058845] INFO: Device HCSR_0 closed
	[ 1521.100126] INFO: Time difference is 467162 
	[ 1521.104441] INFO: Sample distance is 20 cm
	[ 1521.210152] INFO: Time difference is 466990 
	[ 1521.214466] INFO: Sample distance is 20 cm
	[ 1521.320131] INFO: Time difference is 461038 
	[ 1521.324445] INFO: Sample distance is 19 cm
	[ 1521.430132] INFO: Time difference is 465166 
	[ 1521.434446] INFO: Sample distance is 20 cm
	[ 1521.438569] INFO: Average measured distance is 19 cm
	[ 1521.443684] INFO: Distance is 19 cm
	[ 1521.447208] INFO: Measurement done!
	[ 1521.458834] INFO: Device HCSR_1 closed
	[ 1527.863480] INFO: Opening device
	[ 1527.866960] INFO: Device with minor number 56 opened
	[ 1527.872105] INFO: Device HCSR_1 opened
	[ 1527.880189] INFO: Opening device
	[ 1527.883513] INFO: Device with minor number 57 opened
	[ 1527.888513] INFO: Device HCSR_0 opened
	[ 1527.901871] INFO: IOCTL operation: CONFIG_PINS
	[ 1527.906567] INFO: INPUT: Trigger pin: 9
	[ 1527.910549] INFO: INPUT: Echo pin: 4
	[ 1527.920535] INFO: IOCTL operation: CONFIG_PINS
	[ 1527.925237] INFO: INPUT: Trigger pin: 10
	[ 1527.929198] INFO: INPUT: Echo pin: 5
	[ 1527.938760] INFO: IOCTL operation: SET_PARAMETERS
	[ 1527.943680] INFO: INPUT: Number of samples: 7
	[ 1527.948078] INFO: INPUT: Sampling period: 100
	[ 1527.958848] INFO: IOCTL operation: SET_PARAMETERS
	[ 1527.963766] INFO: INPUT: Number of samples: 5
	[ 1527.968164] INFO: INPUT: Sampling period: 100
	[ 1527.979237] INFO: Starting measurement
	[ 1527.985611] INFO: Starting measurement
	[ 1528.090137] INFO: Time difference is 2304916 
	[ 1528.094537] INFO: Sample distance is 99 cm
	[ 1528.098722] INFO: Time difference is 968012 
	[ 1528.103243] INFO: Sample distance is 41 cm
	[ 1528.200128] INFO: Time difference is 47675440 
	[ 1528.204617] INFO: Sample distance is 2054 cm
	[ 1528.212206] INFO: Time difference is 47944816 
	[ 1528.216693] INFO: Sample distance is 2066 cm
	[ 1528.310142] INFO: Time difference is 954034 
	[ 1528.314456] INFO: Sample distance is 41 cm
	[ 1528.330146] INFO: Time difference is 962074 
	[ 1528.334460] INFO: Sample distance is 41 cm
	[ 1528.420132] INFO: Time difference is 953642 
	[ 1528.424446] INFO: Sample distance is 41 cm
	[ 1528.440146] INFO: Time difference is 963380 
	[ 1528.444460] INFO: Sample distance is 41 cm
	[ 1528.530149] INFO: Time difference is 953310 
	[ 1528.534464] INFO: Sample distance is 41 cm
	[ 1528.550146] INFO: Time difference is 953044 
	[ 1528.554460] INFO: Sample distance is 41 cm
	[ 1528.640149] INFO: Time difference is 949726 
	[ 1528.644464] INFO: Sample distance is 40 cm
	[ 1528.660146] INFO: Time difference is 950080 
	[ 1528.664460] INFO: Sample distance is 40 cm
	[ 1528.750150] INFO: Time difference is 946546 
	[ 1528.754464] INFO: Sample distance is 40 cm
	[ 1528.758587] INFO: Average measured distance is 52 cm
	[ 1528.763704] INFO: Distance is 52 cm
	[ 1528.767228] INFO: Measurement done!
	[ 1528.776146] INFO: Time difference is 946040 
	[ 1528.780590] INFO: Sample distance is 40 cm
	[ 1528.790513] INFO: Starting measurement
	[ 1528.890128] INFO: Time difference is 958496 
	[ 1528.894443] INFO: Sample distance is 41 cm
	[ 1528.901857] INFO: Time difference is 942924 
	[ 1528.906171] INFO: Sample distance is 40 cm
	[ 1529.000182] INFO: Time difference is 953248 
	[ 1529.004496] INFO: Sample distance is 41 cm
	[ 1529.008619] INFO: Average measured distance is 40 cm
	[ 1529.013760] INFO: Distance is 40 cm
	[ 1529.017284] INFO: Measurement done!
	[ 1529.026205] INFO: Time difference is 941112 
	[ 1529.030646] INFO: Sample distance is 40 cm
	[ 1529.040501] INFO: Starting measurement
	[ 1529.140128] INFO: Time difference is 938740 
	[ 1529.144443] INFO: Sample distance is 40 cm
	[ 1529.151840] INFO: Time difference is 940800 
	[ 1529.156154] INFO: Sample distance is 40 cm
	[ 1529.250148] INFO: Time difference is 944046 
	[ 1529.254462] INFO: Sample distance is 40 cm
	[ 1529.270146] INFO: Time difference is 939728 
	[ 1529.274459] INFO: Sample distance is 40 cm
	[ 1529.360149] INFO: Time difference is 934106 
	[ 1529.364464] INFO: Sample distance is 40 cm
	[ 1529.380145] INFO: Time difference is 951540 
	[ 1529.384460] INFO: Sample distance is 41 cm
	[ 1529.470149] INFO: Time difference is 933514 
	[ 1529.474463] INFO: Sample distance is 40 cm
	[ 1529.490146] INFO: Time difference is 942476 
	[ 1529.494460] INFO: Sample distance is 40 cm
	[ 1529.580149] INFO: Time difference is 932426 
	[ 1529.584464] INFO: Sample distance is 40 cm
	[ 1529.588588] INFO: Average measured distance is 40 cm
	[ 1529.593704] INFO: Distance is 40 cm
	[ 1529.597228] INFO: Measurement done!
	[ 1529.606069] INFO: Time difference is 942192 
	[ 1529.610510] INFO: Sample distance is 40 cm
	[ 1529.618292] INFO: Device HCSR_0 closed
	[ 1529.720129] INFO: Time difference is 950052 
	[ 1529.724443] INFO: Sample distance is 40 cm
	[ 1529.830129] INFO: Time difference is 941360 
	[ 1529.834444] INFO: Sample distance is 40 cm
	[ 1529.940132] INFO: Time difference is 941498 
	[ 1529.944446] INFO: Sample distance is 40 cm
	[ 1530.050144] INFO: Time difference is 928252 
	[ 1530.054458] INFO: Sample distance is 40 cm
	[ 1530.058581] INFO: Average measured distance is 40 cm
	[ 1530.063697] INFO: Distance is 40 cm
	[ 1530.067221] INFO: Measurement done!
	[ 1530.078847] INFO: Device HCSR_1 closed
	[ 1535.480803] INFO: EXITING
	[ 1535.487684] INFO: Misc driver deregistered
	[ 1535.501834] INFO: Misc driver deregistered
	[ 1535.505979] INFO: All device structures freed
	[ 1535.510493] GOODBYE!




