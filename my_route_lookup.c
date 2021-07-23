
/***********************************************************************
 * ROUTE LOOKUP LAB
 * SWITCHING 2020/2021
 * GROUP 3
 * AUTHORS: MARIA SANZ GOMEZ & GONZALO GUERRERO TORIJA
 * NIAS: 100406560 & 100406534
 ***********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "io.h"
#include "utils.h"


/***********************************************************************
 * Inserts the output interfaces given in the opened routing table
 * file into the fixed and second tables 
 * 
 * It reallocates the n table memory automatically if necessary
 ***********************************************************************/

int MyInsertion(uint16_t *FixedArray, uint16_t **SecondArray){
  uint32_t prefix, netmask;
  int prefixLength, outInterface, result, aux, i, j, k, n;

  n = 0;
  result = readFIBLine(&prefix, &prefixLength, &outInterface);
  
  while(result == OK){
    
    getNetmask(prefixLength, &netmask);
    /*netmask is a 32 bit binary number filled with as many
      1s as the prefixLength starting from MSB to LSB*/ 
    
    if(prefixLength <= 24){
      
      for(i = (prefix & netmask) >> 8 ;
	  i < ((prefix & netmask) >> 8)+ (1 << (24 - prefixLength));
	  i++){
	/****************************************************************
	 * ->(prefix & netmask) >> 8 takes the 24 MSB of prefix
	 * ->(1 << (24 - prefixLength)) travels the table given 
	 *   the prefix length starting in the index set by 
	 *   (prefix & netmask) >> 8
	 ***************************************************************/
	FixedArray[i] = outInterface;
      }
    }
    else{
      
      if((FixedArray[prefix>>8] & 0x8000) == 0){
	/*Check if the second n table does not exist:
	  if the MSB is set to 0*/
	
	*SecondArray = (uint16_t*)realloc((*SecondArray),((n+1) << 8)*sizeof(uint16_t));
	/*Expand the second array 256 more values every time that there 
	 is no prior n table created and it is demanded*/
	if(!(*SecondArray)) return MALLOC_ERROR;
	
	aux = FixedArray[prefix >> 8];
	memset((*SecondArray) + (n << 8), aux,(size_t)(1 << 8));
	/****************************************************************
	 * ->FixedArray[prefix >> 8] gives the output interface stored
	 *   in the fixed table, and such value is stored in aux
	 * ->memset sets all the values of the newly created n table 
	 *   to the value of the stored output interface from the
	 *   fixed table 
	 ***************************************************************/
	
	FixedArray[prefix >> 8] = n | 0x8000;
	//Set the MSB of the fixed table to 1 since there is a n table
	
	for(j = (n << 8) + (prefix & 255);
	    j < (n << 8) + (prefix & 255) + (1 << (32 - prefixLength));
	    j++){
	  /**************************************************************
	   * ->(n << 8) + (prefix & 255) takes the 8 LSB of prefix and
	   *   adds them to the position of the n table, according to n
	   * ->(1 << (24 - prefixLength)) travels the table given 
	   *   the prefix length starting in the index set by 
	   *   (n << 8) + (prefix & 255)
	   *************************************************************/
	  (*SecondArray)[j] = outInterface;
	}
	
	n++;
	//Increase the value of index n
      }
      else{
	//A second n table already exists

	for(k = (FixedArray[prefix >> 8] & 0x7FFF)*(1<<8) + (prefix & 255);
	    k < (FixedArray[prefix >> 8] & 0x7FFF)*(1<<8) + (prefix & 255) + (1 << (32 - prefixLength));
	    k++){
	  /**************************************************************
	   * ->(FixedArray[prefix >> 8] & 0x7FFF)*(1<<8) gets the the 
	   *   value of n stored in the fixed array, discards the MSB,
	   *   which inicates a n table existing and adds the 8 LSB of
	   *   the prefix
	   * ->(1 << (32 - prefixLength)) travels the table given 
	   *   the prefix length starting in the index set by 
	   *   (FixedArray[prefix >> 8] & 0x7FFF)*(1<<8)
	   *************************************************************/
	  (*SecondArray)[k] = outInterface;
	}
       }
    }
    
    result = readFIBLine(&prefix, &prefixLength, &outInterface);
  }
  
  if(result != REACHED_EOF) return result;
  else return OK;
}


/***********************************************************************
 * Searches the addresses in the opened input file and classifies them
 * according to the routing table inserted in the fixed and second 
 * tables
 *
 * It calculates the proccessed packets, the total number of table
 * accesses and the total time processing the packets
 *
 * The classification, and other results are printed into a new 
 * output file
 ***********************************************************************/
int MySearch(uint16_t *FixedArray, uint16_t *SecondArray, int *processedPackets, double *totalTableAccesses, double *totalPacketProcessingTime){
  uint32_t IPAddress;
  int result;
  double searchingTime;
  struct timespec initialTime, finalTime;

   result = readInputPacketFileLine(&IPAddress);
  
  while(result == OK){
    
    (*processedPackets) = (*processedPackets) + 1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime);
    //A packet has been processed, and get the time before classify it
    
    if((FixedArray[IPAddress >> 8] & 0x8000) == 0){
      //MSB is 0, only needs 1 access, to the fixed table
      
      clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime); 
      (*totalTableAccesses) = (*totalTableAccesses) + 1;
      //Increase the accesses one unit and get the time after classify it
      
      printOutputLine(IPAddress, FixedArray[IPAddress >> 8], &initialTime, &finalTime, &searchingTime, 1);
      //Write the results in the output file
    }
    else{
      /*MSB is 1, needs 2 accesses, one to the fixed table and a second 
	to the n table*/
      
      clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime);
      (*totalTableAccesses) = (*totalTableAccesses) + 2;//
      //Increase the accesses two unit and get the time after classify it
      printOutputLine(IPAddress, SecondArray[((FixedArray[IPAddress >> 8] & 0x7FFF) << 8) + (IPAddress & 255)], &initialTime, &finalTime, &searchingTime, 2);
      //Write the results in the output file
    }
    
    (*totalPacketProcessingTime) = (*totalPacketProcessingTime) + searchingTime;
    //Acumulate the total searching time, for all the processed packets
    result = readInputPacketFileLine(&IPAddress);
  }
  
  if(result != REACHED_EOF) return result;
  else return OK;
}


int main(int args, char** argv){
  int error = OK;
  
  if(args != 3) return -1;
  //Only two arguments are valid
  
  error = initializeIO(argv[1], argv[2]);
  //Open the files passed as arguments
  
  if(error != OK){
   printIOExplanationError(error);
   return error;
  }

  uint16_t *FixedArray = (uint16_t*)calloc(1 << 24, sizeof(uint16_t));
  //Allocate memory for the fixed table
  
  if(!FixedArray){
    error = MALLOC_ERROR;
    printIOExplanationError(error);
    freeIO();
    return error;
  }
  
  uint16_t *SecondArray = (uint16_t*)calloc(1 << 8, sizeof(uint16_t)); 
  //Allocate at least 2^8 * 2 bytes for the second table
  
  if(!SecondArray){
    error = MALLOC_ERROR;
    printIOExplanationError(error);
    free(FixedArray);
    freeIO();
    return error;
  }
  
  error = MyInsertion(FixedArray, &SecondArray);
  //Insert the routing table into the tables
  
  if(error != OK){
    printIOExplanationError(error);
    free(SecondArray);
    free(FixedArray);
    freeIO();
    return error;
  }

  int processedPackets = 0;
  double totalTableAccesses = 0;
  double totalPacketProcessingTime = 0;
  //Set variables to get time and packet results 
  
  error = MySearch(FixedArray, SecondArray, &processedPackets, &totalTableAccesses, &totalPacketProcessingTime);
  //Search and write in the output file the decision for the packets
  
  if(error != OK){
    printIOExplanationError(error);
    free(SecondArray);
    free(FixedArray);
    freeIO();
    return error;
  }

  double averageTableAccesses = totalTableAccesses/processedPackets;
  double averagePacketProcessingTime = totalPacketProcessingTime/processedPackets;
  //Set varaibles to compute averages
  
  printSummary(processedPackets, averageTableAccesses, averagePacketProcessingTime);
  
  free(SecondArray);
  free(FixedArray);
  freeIO();
  //Free all the allocated memory
}
