#include <stdio.h>
#include <xtensa/tie/portqueue3i1o.h>
#include "datatype.h"

/* Level shifting to get 8 bit SIGNED values for the data  */
void levelshift (void)
{
	INT16 i,j;
asm("dummy");
	while(1){
		for(j=0;j<64;j++){
					//xt_iss_profile_disable();
			i = (INT16)RECV1();
					//xt_iss_profile_enable();
			//printf(stderr,"Received from FIFO1 %d\n",j);
			i -= 128;
			SEND(i);
		}
		
		for(j=0;j<64;j++){
					//xt_iss_profile_disable();
			i = (INT16)RECV2();
					//xt_iss_profile_enable();
			//fprintf(stderr,"Received from FIFO2 %d\n",j);
			i -= 128;
			SEND(i);
		}
		
		for(j=0;j<64;j++){
					//xt_iss_profile_disable();
			i = (INT16)RECV3();
					//xt_iss_profile_enable();
			//fprintf(stderr,"Received from FIFO3 %d\n",j);
			i -= 128;
			SEND(i);
		}
		#pragma flush	//changed by haris - moved it up the dummy instruction
						//resulted in correct number of iterations for this pipeline stage
		asm("dummy");
		 //break;
	}
}

int main (void)
{
	levelshift ();
	
	return 0;
}
