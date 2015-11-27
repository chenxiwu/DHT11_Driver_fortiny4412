#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main  ( void )
{
	int fd ;
	int retval ;
	char buf[5] ;
	fd = open ( "/dev/dht11" , O_RDONLY) ;
	if ( fd == -1 )
	{
		perror ( "open dht11 error\n" ) ;
		exit ( -1 ) ;
	}
	sleep ( 1 ) ;
	while ( 1 )
	{
		sleep ( 1 ) ;
		retval = read ( fd , buf , 5 );
		if ( retval == -1 )
		{
			perror ( "read dht11 error" ) ;
			printf ( "read dht11 error" ) ;
			exit ( -1 ) ;
		}
		printf ( "\nHumidity:%d.%d\%\nTemperature:%d.%d C\n", buf[0], buf[1], buf[2], buf[3] ) ;
		printf("===================");
	}
	close ( fd ) ;
}

