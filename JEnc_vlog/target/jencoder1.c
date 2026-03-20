/*
	Xtensa Xplorer GENERATED MAIN!

	On Win32 platforms, you must build XTMP applications in an appropriate
	version of Visual Studio.
	
	Further, you should scan the file for two things. First, you should
	sanity check to make sure that your system looks right. Second, XX 
	will in some cases not be able to generate a complete XTMP_main. If 
	such a case occurs you will see a comment noting that in the code
	below.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define XtensaToolsVersion 60003
#include "iss/mp.h"
//#include "iss/queue_iss.h"
#include "my_queue.h"
#include "iss/port_iss.h"

#define ADDFIFO(AA,BB,CC,DD,EE,FF) FIFOarray[FIFOcount++] = XTMP_createFIFO(AA, BB, cores[procIndex(CC)], DD, cores[procIndex(EE)], FF);
#define MEMORYDELAY(MEMORY,AA,BB,CC,DD) XTMP_setReadDelay(MEMORY, AA);XTMP_setBlockReadDelays(MEMORY, AA, BB);XTMP_setWriteDelay(MEMORY, CC);XTMP_setBlockWriteDelays(MEMORY, CC, DD);

static void loadPrograms( XTMP_core *cores, int numProcs );
static int initCoresFromFile( FILE *fp, XTMP_core *cores, XTMP_params *params, int numProcs );
static void printUsage();
static int procIndex( char * );

static int synchronizedStep = 0;

// number of processors and PIF width
#define NUM_PROCESSORS 6
#define PIF_WIDTH 16

struct procInfo {
    char *name;     // processor name.
    char *config;   // configuration name.
    char *exePath;  // path to the executable
};

/* changed by haris */
struct procInfo gProcessors[NUM_PROCESSORS] = {
	{ "u1_S4_6o", "BaseS4", 0 },
	{ "u2_S4_3i1o", "BaseS4", 0 },
	{ "u3_S4_1i1o", "BaseS4", 0 },
	{ "u4_S4_2i1oA", "BaseS4", 0 },
	{ "u5_S4_2i2o", "BaseS4", 0 },
	{ "u6_S4_3i", "BaseS4", 0 }
};

char **argvX = NULL;
struct procFIFO gFIFO[NUM_PROCESSORS];

// Added by haris to get cycles and stalls for a core
void GetCoreData(struct procFIFO *array, XTMP_core core, int first_second){
#ifdef STAT2

    long long unsigned cycles, stalls;
    char *cycles_s, *stalls_s;
    const char *summary  = XTMP_getSummary(core);

    stalls_s = strstr(summary, "TIE global stalls");

    //first_second = 0 means cycles[0] is updated else cycles[1] is updated
    array->cycles[first_second] = XTMP_coreTime(core);

    if(stalls_s != NULL)
      sscanf(stalls_s,"TIE global stalls %llu",&(array->stalls[first_second]));
    else
      array->stalls[first_second]=0;
#endif
}

int XTMP_main(int argc,char **argv)
{
	XTMP_core cores[NUM_PROCESSORS];
	XTMP_params params[NUM_PROCESSORS];
	XTMP_memory *memories;
	int foundFile = 0;
    int foundArg  = 0, iArg = 0;

    int i = 0;
    while( i < argc )
    {
        if( foundArg )
        {
            fprintf(stderr,"Trying:\n");
            argvX[iArg++] = argv[i];
            fprintf(stderr,"%s\n",argv[i]);
        }
        else if( !strncmp( argv[i], "--args", strlen( "--args" ) ) )
        {
            argvX = (char **)malloc(sizeof(char *)*(argc-i));
            foundArg = 1;
        }
        else if( !strncmp( argv[i], "--initfile", 10 ) )
		{
			char *filename = argv[i] + 11;
			FILE *fp = fopen( filename, "r" );
			
			foundFile = 1;

			if( fp )
			{
				int ret = initCoresFromFile( fp, cores, params, NUM_PROCESSORS );
				fclose( fp );

				if( ret < 0 )
					return 0;
			}
		}
		else if( !strncmp( argv[i], "--synchronized", strlen( "--synchronized" ) ) )
		{
			synchronizedStep = 1;
		}
		i++;
	}
    if ( foundArg )
        argvX[iArg] = NULL;
	
	if( !foundFile )
	{
		printUsage();
		return -1;
	}
	
	/**
		Automatically generated memory connections. You will see one
		connector per used processor port. The connector name will 
		include the processor name and port name. This connector is
		connector to that processor's port.
		
		After connector creation and connection, you will see memory creation
		and connection. Each memory is attached to the appropriate connector
		based on the memory map specified for your system.
	**/
	{
		/*Declare the connectors for each processor port used*/
		XTMP_connector connector_u1_S4_6o_PIF;
		XTMP_connector connector_u2_S4_3i1o_PIF;
		XTMP_connector connector_u3_S4_1i1o_PIF;
		XTMP_connector connector_u4_S4_2i1oA_PIF;
		XTMP_connector connector_u5_S4_2i2o_PIF;
		XTMP_connector connector_u6_S4_3i_PIF;

		/*Create the connectors and attach them to the appropriate processor ports*/
		connector_u1_S4_6o_PIF = XTMP_connectorNew( "connector_u1_S4_6o_PIF", XTMP_byteWidth(cores[procIndex("u1_S4_6o")]), 0x0);
		XTMP_connectToCore( cores[procIndex("u1_S4_6o")], XTMP_PT_PIF, 0, connector_u1_S4_6o_PIF, 0x0);

		connector_u2_S4_3i1o_PIF = XTMP_connectorNew( "connector_u2_S4_3i1o_PIF", XTMP_byteWidth(cores[procIndex("u2_S4_3i1o")]), 0x0);
		XTMP_connectToCore( cores[procIndex("u2_S4_3i1o")], XTMP_PT_PIF, 0, connector_u2_S4_3i1o_PIF, 0x0);

		connector_u3_S4_1i1o_PIF = XTMP_connectorNew( "connector_u3_S4_1i1o_PIF", XTMP_byteWidth(cores[procIndex("u3_S4_1i1o")]), 0x0);
		XTMP_connectToCore( cores[procIndex("u3_S4_1i1o")], XTMP_PT_PIF, 0, connector_u3_S4_1i1o_PIF, 0x0);

		connector_u4_S4_2i1oA_PIF = XTMP_connectorNew( "connector_u4_S4_2i1oA_PIF", XTMP_byteWidth(cores[procIndex("u4_S4_2i1oA")]), 0x0);
		XTMP_connectToCore( cores[procIndex("u4_S4_2i1oA")], XTMP_PT_PIF, 0, connector_u4_S4_2i1oA_PIF, 0x0);

		connector_u5_S4_2i2o_PIF = XTMP_connectorNew( "connector_u5_S4_2i2o_PIF", XTMP_byteWidth(cores[procIndex("u5_S4_2i2o")]), 0x0);
		XTMP_connectToCore( cores[procIndex("u5_S4_2i2o")], XTMP_PT_PIF, 0, connector_u5_S4_2i2o_PIF, 0x0);

		connector_u6_S4_3i_PIF = XTMP_connectorNew( "connector_u6_S4_3i_PIF", XTMP_byteWidth(cores[procIndex("u6_S4_3i")]), 0x0);
		XTMP_connectToCore( cores[procIndex("u6_S4_3i")], XTMP_PT_PIF, 0, connector_u6_S4_3i_PIF, 0x0);


		/*Create the array of system memories*/
		memories = malloc( sizeof( XTMP_memory ) * 12);

		/*Create memories and attach to processor ports*/
		memories[0] = XTMP_pifMemoryNew("sram", XTMP_byteWidth(cores[procIndex("u1_S4_6o")]), XTMP_isBigEndian(cores[procIndex("u1_S4_6o")]), 0x800000);
		XTMP_addMapEntry( connector_u1_S4_6o_PIF, memories[0], 0x60000000, 0 );

		memories[1] = XTMP_pifMemoryNew("srom", XTMP_byteWidth(cores[procIndex("u1_S4_6o")]), XTMP_isBigEndian(cores[procIndex("u1_S4_6o")]), 0x20000);
		XTMP_addMapEntry( connector_u1_S4_6o_PIF, memories[1], 0x40000000, 0 );

		memories[2] = XTMP_pifMemoryNew("sram_2", XTMP_byteWidth(cores[procIndex("u2_S4_3i1o")]), XTMP_isBigEndian(cores[procIndex("u2_S4_3i1o")]), 0x800000);
		XTMP_addMapEntry( connector_u2_S4_3i1o_PIF, memories[2], 0x60000000, 0 );

		memories[3] = XTMP_pifMemoryNew("srom_2", XTMP_byteWidth(cores[procIndex("u2_S4_3i1o")]), XTMP_isBigEndian(cores[procIndex("u2_S4_3i1o")]), 0x20000);
		XTMP_addMapEntry( connector_u2_S4_3i1o_PIF, memories[3], 0x40000000, 0 );

		memories[4] = XTMP_pifMemoryNew("sram_3", XTMP_byteWidth(cores[procIndex("u3_S4_1i1o")]), XTMP_isBigEndian(cores[procIndex("u3_S4_1i1o")]), 0x800000);
		XTMP_addMapEntry( connector_u3_S4_1i1o_PIF, memories[4], 0x60000000, 0 );

		memories[5] = XTMP_pifMemoryNew("srom_3", XTMP_byteWidth(cores[procIndex("u3_S4_1i1o")]), XTMP_isBigEndian(cores[procIndex("u3_S4_1i1o")]), 0x20000);
		XTMP_addMapEntry( connector_u3_S4_1i1o_PIF, memories[5], 0x40000000, 0 );

		memories[6] = XTMP_pifMemoryNew("sram_4", XTMP_byteWidth(cores[procIndex("u4_S4_2i1oA")]), XTMP_isBigEndian(cores[procIndex("u4_S4_2i1oA")]), 0x800000);
		XTMP_addMapEntry( connector_u4_S4_2i1oA_PIF, memories[6], 0x60000000, 0 );

		memories[7] = XTMP_pifMemoryNew("srom_4", XTMP_byteWidth(cores[procIndex("u4_S4_2i1oA")]), XTMP_isBigEndian(cores[procIndex("u4_S4_2i1oA")]), 0x20000);
		XTMP_addMapEntry( connector_u4_S4_2i1oA_PIF, memories[7], 0x40000000, 0 );

		memories[8] = XTMP_pifMemoryNew("sram_5", XTMP_byteWidth(cores[procIndex("u5_S4_2i2o")]), XTMP_isBigEndian(cores[procIndex("u5_S4_2i2o")]), 0x800000);
		XTMP_addMapEntry( connector_u5_S4_2i2o_PIF, memories[8], 0x60000000, 0 );

		memories[9] = XTMP_pifMemoryNew("srom_5", XTMP_byteWidth(cores[procIndex("u5_S4_2i2o")]), XTMP_isBigEndian(cores[procIndex("u5_S4_2i2o")]), 0x20000);
		XTMP_addMapEntry( connector_u5_S4_2i2o_PIF, memories[9], 0x40000000, 0 );

		memories[10] = XTMP_pifMemoryNew("sram_6", XTMP_byteWidth(cores[procIndex("u6_S4_3i")]), XTMP_isBigEndian(cores[procIndex("u6_S4_3i")]), 0x800000);
		XTMP_addMapEntry( connector_u6_S4_3i_PIF, memories[10], 0x60000000, 0 );

		memories[11] = XTMP_pifMemoryNew("srom_6", XTMP_byteWidth(cores[procIndex("u6_S4_3i")]), XTMP_isBigEndian(cores[procIndex("u6_S4_3i")]), 0x20000);
		XTMP_addMapEntry( connector_u6_S4_3i_PIF, memories[11], 0x40000000, 0 );

		{
			/*==========================================================
			 * Declare, create and connect the rest of your system here:
			 *--------------------------------------------------------*/


			/*----------------------------------------------------------
			 * End of system creation.
			 ==========================================================*/
		}
	}

    /* Set the system memory delays */
    int memCount = 0;
    //MEMORYDELAY(memories[0], 27, 6, 16, 6);
    //MEMORYDELAY(memories[1], 27, 6, 16 ,6); 43, 22, 32, 22
    for(memCount = 0; memCount < 12; memCount++){
        MEMORYDELAY(memories[memCount], 27, 6, 16, 6);
    }

    int FIFOcount = 0;
    FIFO *FIFOarray[100];

    ADDFIFO( 64, "fifo_a2b1", "u1_S4_6o",   "FIFO_OUT1", "u2_S4_3i1o", "FIFO_IN1")
    ADDFIFO( 64, "fifo_a2b2", "u1_S4_6o",   "FIFO_OUT2", "u2_S4_3i1o", "FIFO_IN2")
    ADDFIFO( 64, "fifo_a2b3", "u1_S4_6o",   "FIFO_OUT3", "u2_S4_3i1o", "FIFO_IN3")
    ADDFIFO( 2,  "fifo_a2d",  "u1_S4_6o",   "FIFO_OUT4", "u4_S4_2i1oA","FIFO_IN1")
    ADDFIFO( 3,  "fifo_a2e",  "u1_S4_6o",   "FIFO_OUT5", "u5_S4_2i2o", "FIFO_IN1")
    ADDFIFO( 100,"fifo_a2f",  "u1_S4_6o",   "FIFO_OUT6", "u6_S4_3i",   "FIFO_IN1")
    ADDFIFO( 64, "fifo_b2c",  "u2_S4_3i1o", "FIFO_OUT",  "u3_S4_1i1o", "FIFO_IN")
    ADDFIFO( 64, "fifo_c2d",  "u3_S4_1i1o", "FIFO_OUT",  "u4_S4_2i1oA","FIFO_IN2")
    ADDFIFO( 64, "fifo_d2e",  "u4_S4_2i1oA","FIFO_OUT",  "u5_S4_2i2o", "FIFO_IN2")
    ADDFIFO( 64, "fifo_e2f1", "u5_S4_2i2o", "FIFO_OUT1", "u6_S4_3i",   "FIFO_IN2")
    ADDFIFO( 1,  "fifo_e2f2", "u5_S4_2i2o", "FIFO_OUT2", "u6_S4_3i",   "FIFO_IN3")

    loadPrograms( cores, NUM_PROCESSORS );
    
    if( synchronizedStep )
        XTMP_setSynchronizedSimulation(1);
    
    /* this call prints information required for the Xplorer debugger to connect */
    fflush( stdout );

    /* prints out the command argument which will be used in the simulated cores */
    int xx = 0;
    /*commented by haris
    fprintf(stderr,"\nArguments passed:");
        while(argvX != NULL && argvX[xx]!=NULL){
        fprintf(stderr," %s",argvX[xx++]);
    }
    fprintf(stderr,"\n\n");
    */

    /* enabling profiling information that can be viewed when running it in single processor mode */
    int x;
    for(x=0;x<NUM_PROCESSORS;x++){
        char profile_opt[1000] = "profile --force-suffix --cycles --instructions --branch-delay --interlock --icmiss --dcmiss --icmiss-cycles --dcmiss-cycles ";
        char profile_path[500];
        char trace_opt[500];
        char *profile_ptr;
        char profile_gmon[] = "gmon.xx";
        strcpy(profile_path,gProcessors[x].exePath);
        if ((profile_ptr=rindex(profile_path,'/'))!=NULL){
            profile_ptr++;
            strcpy(profile_ptr,profile_gmon);
        }else{
            strcpy(profile_path,profile_gmon);
        }
        strcat(profile_opt,profile_path);

//        XTMP_clientLoad(cores[x], profile_opt);
//        fprintf(stderr,"%s: %s\n",gProcessors[x].name,profile_opt);

        sprintf(trace_opt,"trace --level 3 %s_trace3.txt",gProcessors[x].name);
        XTMP_clientLoad(cores[x], trace_opt);
        fprintf(stderr,"%s: %s\n",gProcessors[x].name,trace_opt);

        // assume that gProcessors[i] == cores[i]
    }

    //XTMP_start(-1);

    /*added by haris*/
    /*for(x=0; x<NUM_PROCESSORS; x++)
      {
	printf("\nProcessor: %s\nFirst Dummy Address is %x\nSecond Dummy Address is %x\n", gProcessors[x].name, gFIFO[x].addr[0], gFIFO[x].addr[1]);
      }

    printf("%s\n", XTMP_getCoreInformation());
    fflush(stdout);
    */

    int lastCoreIndex = procIndex("u6_S4_3i");
    while (!XTMP_hasCoreExited(cores[lastCoreIndex])){
        // execute statement when DUMMY instruction is encountered
#ifdef STAT2
        for(x=0; x<NUM_PROCESSORS; x++){
            // second address
            if ( (XTMP_getStagePC(cores[x],1) == gFIFO[x].addr[1]) || (XTMP_getStagePC(cores[x],1) == gFIFO[x].addr[0]) ){
                if ( gFIFO[x].started ){
                    long long unsigned old_cycles = gFIFO[x].cycles[1];
                    long long unsigned old_stalls = gFIFO[x].stalls[1];
                    /*
		    gFIFO[x].cycles[1] = XTMP_clockTime();
		    gFIFO[x].stalls[1] = 0;
		    */
		    //added by haris
		    GetCoreData(&(gFIFO[x]), cores[x], 1);
		    gFIFO[x].iterations++;
		    		   
		    /* added by haris 
		    printf("Processor: %s\nIterations: %lu", gProcessors[x].name, gFIFO[x+1].iterations);
		    scanf("%c", &a);
		    */
		    /*commented by haris
                    for(xx=0; xx<FIFOcount; xx++){
		      if( strcmp( XTMP_getComponentName(FIFOarray[xx]->core2), XTMP_getComponentName(cores[x]) ) == 0 ){
                            gFIFO[x].stalls[1] += FIFOarray[xx]->emptyTotal;
                        }
			//added by haris to account for stalls encountered while writing in full FIFO
			else if( strcmp( XTMP_getComponentName(FIFOarray[xx]->core1), XTMP_getComponentName(cores[x]) ) == 0 ){
			  gFIFO[x].stalls[1] += FIFOarray[xx]->fullTotal;
			  }
		    }
		    */
                    
		    // calculate running variance variables
                    long long unsigned latency =  (gFIFO[x].cycles[1] - gFIFO[x].stalls[1]) - (old_cycles - old_stalls);
                    gFIFO[x].sample_squared    += latency * latency;
		    //added by haris for first latency
		    if(gFIFO[x].iterations == 1)
		      gFIFO[x].first_latency = latency;
		    //added by haris for all latencies  
		    //gFIFO[x].latency[gFIFO[x].iterations-1] = latency; // for alllatencies
				    
                // first address
                }else{
		    gFIFO[x].started = 1;
		    //added by haris
		    GetCoreData(&(gFIFO[x]), cores[x], 0);
		    
		    /*commented by haris
		    gFIFO[x].cycles[0] = XTMP_clockTime();
                    gFIFO[x].stalls[0] = 0;
		    //XTMP_getComponentName used by haris
                    for(xx=0; xx<FIFOcount; xx++){
                        if( strcmp( XTMP_getComponentName(FIFOarray[xx]->core2), XTMP_getComponentName(cores[x]) ) == 0 ){
                            gFIFO[x].stalls[0] += FIFOarray[xx]->emptyTotal;
                        }//added by haris to account for stalls encountered while writing in full FIFO
			else if( strcmp( XTMP_getComponentName(FIFOarray[xx]->core1), XTMP_getComponentName(cores[x]) ) == 0 ){
			  gFIFO[x].stalls[0] += FIFOarray[xx]->fullTotal;
			  }
                    }
		    */
                    // duplicate to 2nd cell
                    gFIFO[x].cycles[1] = gFIFO[x].cycles[0];
                    gFIFO[x].stalls[1] = gFIFO[x].stalls[0];
                }
            }
        }
#endif
        XTMP_stepSystem(1);
        XTMP_printFIFOarrayfirstdata(FIFOarray,FIFOcount);
    }

    XTMP_printFIFOarraystalls(FIFOarray,FIFOcount);
    XTMP_printFIFOstatistics(gFIFO,NUM_PROCESSORS);
    
    XTMP_printFIFOfinalizationtime(&(gFIFO[lastCoreIndex]), cores[lastCoreIndex], FIFOarray, FIFOcount);

    fprintf(stderr, "XTMP Total Simulation time: %llu\n\n", XTMP_clockTime());

    for(x=0;x<NUM_PROCESSORS;x++){
        printf("Processor(%s):\n",gProcessors[x].name);
        XTMP_printSummary(cores[x],0);
        printf("\n");       
    }
    
    /* Must return to main, which does cleanup */
    return 0;
}


#define MAX_PATH_LENGTH 1024

/* changed senglin to haris in the following paths*/
char *gSystemDirs[] = {
	"/home/haris/xtensa/XtDevTools/install/builds/RB-2007.1-linux/BaseS4/config",
	"/home/haris/xtensa/XtDevTools/install/builds/RB-2007.1-linux/BaseS4/config",
	"/home/haris/xtensa/XtDevTools/install/builds/RB-2007.1-linux/BaseS4/config",
	"/home/haris/xtensa/XtDevTools/install/builds/RB-2007.1-linux/BaseS4/config",
	"/home/haris/xtensa/XtDevTools/install/builds/RB-2007.1-linux/BaseS4/config",
	"/home/haris/xtensa/XtDevTools/install/builds/RB-2007.1-linux/BaseS4/config",
	0
};

static char gArgBuffer[256];
static char gProcessorName[MAX_PATH_LENGTH];
static char gExePathBuffer[MAX_PATH_LENGTH];
static char gTdkBuffer[MAX_PATH_LENGTH];
char *gTdkFiles[ 2 ] = 
{
	gTdkBuffer,
	NULL
};

/**
 * Computes the index for the processor given. Returns
 * -1 for no processor match.
 */
static int procIndex( char *procName )
{
	int i;
	for( i= 0 ; i < NUM_PROCESSORS ; i++ )
	{
		if( !strcmp( procName, gProcessors[i].name ) )
				return i;
	}

	return -1;
}

/** 
 * returns 0 if no , non zero if yes.
 */
static int readYesNo( FILE *fp )
{
	static char buffer[128];
	fscanf( fp, "%s", buffer );
	return strcmp( "no", buffer );
}

/**
 * printUsage
 *
 * prints how to use this simulator
 */
static void printUsage()
{
	printf( "This XTMP simulator must have an --initfile=filename parameter passed to it.\n" );
	printf( "The filename must point to a file what contains one line per processor.\n" );
	printf( "Each line must be of the following format:\n\n" );
	printf( "PROC_NAME TDK_SPEC DEBUG_SPEC PROFILE_SPEC PRID_SPEC EXE_SPEC\n" );
	printf( "PROC_NAME: must match the processor names given to Xplorer\n" );
	printf( "TDK_SPEC: either, no_tdk or path to the tdk directory for this core\n" );
	printf( "DEBUG_SPEC either, no no, or yes # [yes|no] where # is the TCP port and yes|no specifies whether to wait for debugger attach\n" );
	printf( "PROFILE_SPEC: [yes|no]\n" );
	printf( "PRID_SPEC: either, no, or yes # where # is the PRID for the core\n" );
	printf( "EXE_SPEC: either, no_load, or the path to the exe to preload for this core\n" );
}


/**
 * initCoresFromFile
 *
 * initializes the cores from the file pointer passed.
 */
static int initCoresFromFile( FILE *fp, 
						XTMP_core *cores, 
						XTMP_params *params, 
						int numProcs 
)
{
	int i;
	int processorIndex;
	XTMP_register pidReg;
	for( i = 0 ; i < numProcs ; i++ )
	{
		fscanf( fp, "%s", gProcessorName );
		fscanf( fp, "%s", gTdkBuffer );

		if( !strcmp( gTdkBuffer, "no_tdk" ) )
			*gTdkBuffer = 0;

		processorIndex = procIndex( gProcessorName );
		
		if( processorIndex < 0 )
		{
			fprintf( stderr, "Cannot find processor %s\n", gProcessorName );
			return -1 ;
		}

		if( *gTdkBuffer == 0 )
			params[processorIndex] = XTMP_paramsNewFromPath( gSystemDirs, 
				gProcessors[processorIndex].config, 0 );
		else
		  {
		    params[processorIndex] = XTMP_paramsNewFromPath( gSystemDirs, gProcessors[processorIndex].config, gTdkFiles );
		  }

        // adds a new section in the initfile so that arguments for each core simulator can be specified; ends with no_arg
        char **arg_ptr=NULL;
        char arg_1[100];
        int ptr = 0;
        fscanf( fp, "%s", arg_1 );
        if ( strcmp(arg_1, "no_arg") ){
            arg_ptr = (char **)malloc(sizeof(char *)*100);
            arg_ptr[ptr] = (char *)malloc(sizeof(char)*(strlen(arg_1)+1));
            arg_ptr[ptr] = strcpy(arg_ptr[ptr],arg_1);
            fprintf(stderr,"%s: %s",gProcessors[processorIndex].name,arg_ptr[ptr]);
            ptr++;
            fscanf( fp, "%s", arg_1 );
            while( strcmp(arg_1, "no_arg") ){
                arg_ptr[ptr] = (char *)malloc(sizeof(char)*(strlen(arg_1)+1));
                arg_ptr[ptr] = strcpy(arg_ptr[ptr],arg_1);
                fprintf(stderr," %s",arg_ptr[ptr]);
                ptr++;
                fscanf( fp, "%s", arg_1 );
            }
            arg_ptr[ptr] = NULL;
            fprintf(stderr,"\n");
        }

        cores[processorIndex] = XTMP_coreNew( gProcessors[processorIndex].name, 
            params[processorIndex], arg_ptr );
//          params[processorIndex], 0 );

		if( readYesNo( fp ) )
		{
			unsigned int port;
			fscanf( fp, "%d", &port );
			XTMP_enableDebug( cores[processorIndex], port );
			fflush( stdout );
		}

		if( readYesNo( fp ) )
			XTMP_setWaitForDebugger( cores[processorIndex], true );
		
		// read profiling paramter, not supported by this template generation
		readYesNo( fp );

		if( readYesNo( fp ) )
		{
			int prid;
			fscanf( fp, "%d", &prid );
			pidReg = XTMP_lookupRegisterByName( cores[processorIndex], "prid" );
			XTMP_setRegisterValue( pidReg, &prid );
		}

		fscanf( fp, "%s", gExePathBuffer );
		if( strcmp( gExePathBuffer, "no_load" ) )
		{
			gProcessors[processorIndex].exePath = malloc( strlen( gExePathBuffer) + 1 );
			strcpy( gProcessors[processorIndex].exePath, gExePathBuffer );
		}

                // adds functionality to record first and last use of FIFOs
                XTMP_find_addresses(cores[processorIndex],&(gFIFO[processorIndex]),gProcessors[processorIndex].config,gSystemDirs[processorIndex],gTdkBuffer,gExePathBuffer);
	}
	return 0;
}

/*
 * loadPrograms
 *
 * Loads the xtensa cores with the requested programs
 */
static void loadPrograms( XTMP_core *cores, int numProcs )
{
	int i;

	for( i = 0 ; i < numProcs ; i++ )
	{
		if( gProcessors[i].exePath )
            //XTMP_loadProgram( cores[i], gProcessors[i].exePath, 0 );
            XTMP_loadProgram( cores[i], gProcessors[i].exePath, argvX );
	}
}
