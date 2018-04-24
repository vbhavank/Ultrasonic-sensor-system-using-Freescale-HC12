/**stand-alone QNX Neutrino program to measure the distance between the rear bumper of  
 * your car and any objects behind the vehicle while parking. 
Authors= Bhavan Kumar Vasu  
 */ 
 
 
 
#include <stdlib.h> 
#include <stdio.h> 
#include <pthread.h> 
#include <time.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <sys/neutrino.h> 
#include <unistd.h> 
#include <sys/mman.h> 
#include <hw/inout.h> 
#include <stdint.h> 
#include <sys/select.h> 
#define PORTA_address 0x288 
#define PORTB_address 0x289 
#define control_address 0x28B 
 
 unsigned long number = 0; 
 unsigned int interval=0; 
 unsigned char sensor = 0; 
 uint64_t start_timer,endTime; 
 timer_t tickTimer; 
 struct sigevent   tickTimer_event; 
 struct itimerspec tickTimer_info; 
 struct sigaction  tickTimer_action; 
 uintptr_t PORTA_handler,PORTB_handler; 
  
 void *input_data(void* arg) 
 { 
 	  char key = 0; 
      while( key != 'S' || key !='f') 
            key = getchar(); 
      start_tim(); 
      sensor = 1; 
 
     while( key != 'F' || key !='f') 
            key = getchar(); 
     sensor = 0; 
     return 0; 
 } 
 
//Starts the  timer 
void start_tim() 
 
  { 
 
     tickTimer_info.it_value.tv_sec     = 0; 
     tickTimer_info.it_value.tv_nsec    = 100000000; 
     tickTimer_info.it_interval.tv_sec  = 0; 
     tickTimer_info.it_interval.tv_nsec = 100000000; 
     tickTimer_action.sa_sigaction = &timerTick; 
     tickTimer_action.sa_flags = SA_SIGINFO; 
     sigaction(SIGUSR1, &tickTimer_action, NULL); 
     SIGEV_SIGNAL_INIT(&tickTimer_event, SIGUSR1); 
 
     timer_create(CLOCK_REALTIME, &tickTimer_event, &tickTimer); 
 
     timer_settime(tickTimer, 0, &tickTimer_info, 0); 
  } 
 
 
//This function is called everytime a timer ticks 
void timerTick() 
  { 
     struct timespec t1; 
     if(sensor) 
      { 
 
    	 out8( PORTA_handler, 0x01 ); 
 
 	    t1.tv_sec = 0; 
 	    t1.tv_nsec = 10000; 
 	    nanospin(&t1); 
 
 		out8( PORTA_handler, 0x00); 
 
 		while( (in8(PORTB_handler)&0x01) == 0x00); //Loops till the signal is low 
 		if((in8(PORTB_handler)&0x01) == 0x01) 
 		 { 
 			start_timer = ClockCycles(); 
 		 } 
 		while( ((in8(PORTB_handler)&0x01) == 0x01) &&( 
 					   (ClockCycles()-start_timer))); 
 		if((in8(PORTB_handler)&0x01)==0x00)  //Check if the signal is low 
 		 { 
 			endTime = ClockCycles(); 
 		 } 
        interval = ((endTime - start_timer)/58000); 
        if(interval>24||interval<3) 
         { 
            printf("out of range   n"); 
         } 
        else 
         { 
 			printf("%lu:%d inchs  n",number,interval); 
         } 
      } 
     number++; 
  } 
  
 int main() 
  { 
    pthread_t thread1; 
 
    int err; 
	uintptr_t ctrl_handle; 
 
	err = ThreadCtl( _NTO_TCTL_IO, NULL ); 
	if ( ERROR == -1 ) 
	 { 
		printf( "error  n" ); 
		return -1; 
	 } 
 
 
	ctrl_handle = mmap_device_io( 1, control_address  ); //Initialise the I/o port 
	out8( ctrl_handle,0x02); 
	PORTA_handler = mmap_device_io( 1, PORTA_address );  //Gives the handle to port's data register 
	PORTB_handler = mmap_device_io( 1, PORTB_address ); 
	out8( PORTA_handler, 0x00 ); 
 
    pthread_create(&thread1,NULL,input_data,NULL); 
    printf("Enter 'S' to start:  n"); 
    pthread_join(thread1,NULL); 
    return 0; 
  } 
 
  
 
 
 }
