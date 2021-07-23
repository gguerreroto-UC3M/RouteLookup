#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "io.h"
#include "utils.h"

int MyInsertion(uint16_t *FixedArray, uint16_t **SecondArray){
  uint32_t prefix;
  int prefixLength, outInterface;
  uint32_t netmask;
  int i,j,h,result,aux;
  uint16_t n = 0;

  result = readFIBLine(&prefix, &prefixLength, &outInterface);
  
  while(result == OK){//Could be check if different from EOF but this way we avoid all the errors
    getNetmask(prefixLength, &netmask);
    if(prefixLength <= 24){
      for(i = (prefix & netmask) >> 8 ; i < ((prefix & netmask) >> 8) + (1 << (24 - prefixLength)); i++){
	FixedArray[i] = outInterface;
      }
    }
    else{   
      if((FixedArray[prefix>>8] & 0x8000) == 0){ //MSB to 0
	
	*SecondArray = (uint16_t*)realloc((*SecondArray),((n+1) << 8)*sizeof(uint16_t));
	if(!(*SecondArray)) return MALLOC_ERROR;
	
	
	//for(k = n << 8; k < (n << 8) + (1 << 8); k++){
	//  SecondArray[k] = FixedArray[prefix >> 8];
	//}
	
	aux = FixedArray[prefix >> 8];
	memset((*SecondArray) + (n << 8), aux,(size_t)(1 << 8));
	FixedArray[prefix >> 8] = n | 0x8000;
	
	for(j = (n << 8) + (prefix & 255); j < (n << 8) + (prefix & 255) + (1 << (32 - prefixLength)); j++){
	  (*SecondArray)[j] = (uint16_t)outInterface;
	}
	
	n++;
      }
      else{ //If we have a link to the second level
	for(h = (FixedArray[prefix >> 8] & 0x7FFF)*(1<<8) + (prefix & 255) ; h < (FixedArray[prefix >> 8] & 0x7FFF)*(1<<8) + (prefix & 255) + (1 << (32 - prefixLength)); h++){
	  //(find the value of n)*2^8 + the 8 LSB
	  (*SecondArray)[h] = outInterface;
	}
      }
    }
    result = readFIBLine(&prefix, &prefixLength, &outInterface);
  }
  if(result != REACHED_EOF) return result;
  else return OK;
}

int MySearch(uint16_t *FixedArray, uint16_t *SecondArray, int *processedPackets, double *totalTableAccesses, double *totalPacketProcessingTime){
  uint32_t IPAddress;
  int AccTables = 0;
  struct timespec initialTime, finalTime;
  double searchingTime;
  int result;

  result = readInputPacketFileLine(&IPAddress);
  while(result == OK){
    (*processedPackets) = (*processedPackets) + 1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime);
    if((FixedArray[IPAddress >> 8] & 0x8000) == 0){
      clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime); 
      AccTables = 1;
      (*totalTableAccesses) = (*totalTableAccesses) + 1;
      printOutputLine(IPAddress, FixedArray[IPAddress >> 8], &initialTime, &finalTime, &searchingTime, AccTables);
    }
    else{
      clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime);
      AccTables = 2;
      (*totalTableAccesses) = (*totalTableAccesses) + 2 ;
      printOutputLine(IPAddress, SecondArray[((FixedArray[IPAddress >> 8] & 0x7FFF) << 8) + (IPAddress & 255)], &initialTime, &finalTime, &searchingTime, AccTables);
    }
    (*totalPacketProcessingTime) = (*totalPacketProcessingTime) + searchingTime;
    result = readInputPacketFileLine(&IPAddress);
  }
  if(result != REACHED_EOF) return result;
  else return OK;
}


int main(int args, char** argv){
  int error = OK;
  if(args != 3) return -1;

  error = initializeIO(argv[1], argv[2]);
  if(error != OK){
   printIOExplanationError(error);
   return -1;
  }

  uint16_t *FixedArray = (uint16_t*)calloc(1 << 24, sizeof(uint16_t));
  uint16_t *SecondArray = (uint16_t*)calloc(1 << 8, sizeof(uint16_t)); 
  
  if(!FixedArray){
	error = MALLOC_ERROR;
	printIOExplanationError(error);
    //liberar Fixarray y Second array??
	return -1;
  }

  error = MyInsertion(FixedArray, &SecondArray);
  if(error != OK){
  printIOExplanationError(error);
  return -1;
  }

  int processedPackets = 0;
  double totalTableAccesses = 0;
  double totalPacketProcessingTime = 0;
  
  error = MySearch(FixedArray, SecondArray, &processedPackets, &totalTableAccesses, &totalPacketProcessingTime);
  if(error != OK){
   printIOExplanationError(error);
   return -1;
  }

  
  double averageTableAccesses = totalTableAccesses/processedPackets;
  double averagePacketProcessingTime = totalPacketProcessingTime/processedPackets;

  printSummary(processedPackets, averageTableAccesses, averagePacketProcessingTime);

  free(SecondArray);

  free(FixedArray);
  
  freeIO();
}
