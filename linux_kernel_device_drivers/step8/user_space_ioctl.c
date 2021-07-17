//user-space for ioctl

#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>

#define WR_DATA _IOW('a','a',int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)

int main()
{
	int fd;
	int32_t val, num;
	
	printf("IOCTL based charater device driver operations from user space\n");
	
	//device number,
	fd=open("/dev/chr_device", O_RDWR);
	
	if(fd<0)
	{
		printf("\n cannot open device file");
		return 0;
	}
	
	printf("\nenter data to send to driver");
	scanf("%d", &num);
	
	printf("\nwriting value to the driver\n");
	ioctl(fd, WR_DATA, (int32_t*)&num);
	
	printf("reading value from driver");
	ioctl(fd, RD_DATA, (int32_t*)&val);
	
	printf("closing driver\n");
	close(fd);
	
}
