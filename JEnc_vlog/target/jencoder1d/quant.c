#include <stdio.h>
#include <xtensa/tie/portqueue2i1oA.h>
#include "datatype.h"
#include "config.h"
#include "quantdata.h"

UINT8	Lqt [BLOCK_SIZE];
UINT8	Cqt [BLOCK_SIZE];
UINT16	ILqt [BLOCK_SIZE];
UINT16	ICqt [BLOCK_SIZE];
INT16	Temp [BLOCK_SIZE];
//
/*	This function implements 16 Step division for Q.15 format data */
UINT16 DSP_Division (UINT32 numer, UINT32 denom)
{
	UINT16 i;

	denom <<= 15;

	for (i=16; i>0; i--)
	{
		if (numer > denom)
		{
			numer -= denom;
			numer <<= 1;
			numer++;
		}
		else
			numer <<= 1;
	}

	return (UINT16) numer;
}

/* Multiply Quantization table with quality factor to get LQT and CQT */
void initialize_quantization_tables (void)
{
	UINT16 i, index;
	UINT32 value;

	UINT8 luminance_quant_table [] =
	{
		16, 11, 10, 16,  24,  40,  51,  61,
		12, 12, 14, 19,  26,  58,  60,  55,
		14, 13, 16, 24,  40,  57,  69,  56,
		14, 17, 22, 29,  51,  87,  80,  62,
		18, 22, 37, 56,  68, 109, 103,  77,
		24, 35, 55, 64,  81, 104, 113,  92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103,  99
	};

	UINT8 chrominance_quant_table [] =
	{
		17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99
	};
					//xt_iss_profile_disable();
	UINT32 quality_factor = (UINT32)RECV1();
					//xt_iss_profile_enable();
	for (i=0; i<64; i++)
	{
		index = zigzag_table [i];

		/* luminance quantization table * quality factor */
		value = luminance_quant_table [i] * quality_factor;
		value = (value + 0x200) >> 10;

		if (value == 0)
			value = 1;
		else if (value > 255)
			value = 255;

		Lqt [index] = (UINT8) value;
		ILqt [i] = DSP_Division (0x8000, value);

		/* chrominance quantization table * quality factor */
		value = chrominance_quant_table [i] * quality_factor;
		value = (value + 0x200) >> 10;

		if (value == 0)
			value = 1;
		else if (value > 255)
			value = 255;

		Cqt [index] = (UINT8) value;
		ICqt [i] = DSP_Division (0x8000, value);
	}
	for (i=0; i<64; i++){
		SEND(Lqt[i]);
	}
	for (i=0; i<64; i++){
		SEND(Cqt[i]);
	}
}

/* multiply DCT Coefficients with Quantization table and store in ZigZag location */
void quantization (UINT16* const quant_table_ptr)
{
	INT16 i;
	INT32 value;

	for (i=0; i<=63; i++)
	{					//xt_iss_profile_disable();
		value = (INT16)RECV2() * quant_table_ptr [i];
							//xt_iss_profile_enable();
		value = (value + 0x4000) >> 15;

		Temp [zigzag_table [i]] = (INT16) value;
	}
	for (i=0; i<=63; i++)
	{
		SEND(Temp [i]);
	}
}

int main(void){
  while(1){
  UINT32 count  = (UINT32)RECV1();
  initialize_quantization_tables();
	asm("dummy");
	for(;count>0;count--){
		quantization (ILqt);
		quantization (ICqt);
		quantization (ICqt);
	//	#pragma flush //added by haris
	asm("dummy");
	}
	break ;	
  }
	printf("\n Done ! \n");
	
}
