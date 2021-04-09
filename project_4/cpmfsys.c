#include <stdint.h> 
#include <stdlib.h> 
#include "diskSimulator.h"
#include  <stdbool.h> 
#include <string.h> 
#include <stdio.h> 

#define EXTENT_SIZE 32
#define BLOCKS_PER_EXTENT 16 
#define debugging false

typedef struct dirStruct { 
uint8_t status; // 0xe5 = unused, 0-16 = user number
char  name[9]; // no need to support attributes in msb  of bytes 0,1,2,7
char  extension[4]; // no need to support attributes in msb  of bytes 0,1,2
uint8_t XL; // see below for these 4  bytes' meaning
uint8_t BC; 
uint8_t XH; 
uint8_t RC;  
uint8_t blocks[BLOCKS_PER_EXTENT]; // array of disk sectors used
} DirStructType;

//globals
bool freeList[256];

/* XL and XH store the extent number, bits 5-7 of XL are zero, bits 0-4 are
   low-order bits of extent number, bits 6-7 of XH are zero, bits 0-5 hold high
   -order bits of extent number
   
   BC = number of bytes in last used physical sector
   RC = number of 128 byte sectors in last used block 
   blocks are logical block numbers 0-255, 0 indicates a hole in file, 
   
*/ 

typedef uint8_t Extent[32];

//function to allocate memory for a DirStructType (see above), and populate it, given a
//pointer to a buffer of memory holding the contents of disk block 0 (e), and an integer index
// which tells which extent from block zero (extent numbers start with 0) to use to make the
// DirStructType value to return. 
DirStructType *mkDirStruct(int index,uint8_t *e) {
	DirStructType *output;
	int size = sizeof(output);
	output = malloc(size);
	//status
	int locStat = (e+index*EXTENT_SIZE)[0];
	output -> status = locStat;
	int i;
	int j = 0;//current char
	//name
	for(i = 1; i < 9; i++){
		int loc = (e+index*EXTENT_SIZE)[i];
		if (loc == ' ') {
			output -> name[j] = '\0'; //end of name
			break;
		}
		else {
			output -> name[j] = loc;
		}
		j++;
	}
	j = 0;//reset
	//extension
	for(i = 9; i < 12; i++) {
		int loc = (e+index*EXTENT_SIZE)[i];
		if (loc == ' ') {
			output -> extension[j] = '\0';
			break;
		}
		else {
			output -> extension[j] = (loc);
		}
		j++;
	}
	//bytes
	int locXL = (e+index*EXTENT_SIZE)[12];
	int locBC = (e+index*EXTENT_SIZE)[13];
	int locXH = (e+index*EXTENT_SIZE)[14];//is it needed?
	int locRC = (e+index*EXTENT_SIZE)[15];
	output -> XL = locXL;
	output -> BC = locBC;
	output -> XH = locXH; //is it needed?
	output -> RC = locRC;
	//block
	j = 0;//reset
	for(i = 16; i < 32; i++) {
		int loc = (e+index*EXTENT_SIZE)[i];
		output -> blocks[j] = loc;
		j++;
	}
	//return 
	return output;
}	

// function to write contents of a DirStructType struct back to the specified index of the extent
// in block of memory (disk block 0) pointed to by e
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e) { 
	(e+index*EXTENT_SIZE)[0] = d -> status;
	int i;
	int j = 0;//current char
	for(i = 1; i < 9; i++){
		if (d-> name[j] == '\0') {
			(e+index*EXTENT_SIZE)[i] = ' ';//end of name
		}
		else {
			(e+index*EXTENT_SIZE)[i] = d -> extension[j];
		}
		j++;
	}
	j = 0;//reset

	for(i = 9; i < 12; i++) {
		if (d -> extension[j] != '\0') {
			(e+index*EXTENT_SIZE)[i] = ' ';
		}
		else {
			(e+index*EXTENT_SIZE)[i] = d -> extension[j];
		}
		j++;
	}
	//bytes
	(e+index*EXTENT_SIZE)[12] = d -> XL;
	(e+index*EXTENT_SIZE)[13] = d -> BC;
	(e+index*EXTENT_SIZE)[14] = d -> XH;//is it needed?
	(e+index*EXTENT_SIZE)[15] = d -> RC;

	//block
	j = 0;//reset
	for(i = 16; i < 32; i++) {
		(e+index*EXTENT_SIZE)[i] = d -> blocks[j];
		j++;
	}
}

// populate the FreeList global data structure. freeList[i] == true means 
// that block i of the disk is free. block zero is never free, since it holds
// the directory. freeList[i] == false means the block is in use. 
void makeFreeList() {
	int i;
	int j;
	DirStructType *extent;
	uint8_t buff[1024];
	//set 0 to false
	freeList[0] = false;
	//set the rest to true
	for (i = 1; i < 256; i++) { //check on total number
		freeList[i] = true;
	}
	// EXTENT
	for (i = 0; i < 32; i++) {
		extent = mkDirStruct(i,buff);
		if (extent -> status != 0x5e) {
			for(j = 0; j < 16; j++) {
				if (extent -> blocks[j] != 0){
					freeList[(int)extent -> blocks[j]] = false;
				}
			}	
		}
	}
}
// debugging function, print out the contents of the free list in 16 rows of 16, with each 
// row prefixed by the 2-digit hex address of the first block in that row. Denote a used
// block with a *, a free block with a . 
void printFreeList() { 
	int i;
	int j;
	//set to 0
	int k = 0;
	printf("FREE BLOCK LIST: (* means in-use)");
	printf("\n");
	for (i = 0; i < 16; i++) {
		printf("%x0: ", i);
		for (j = 0; j < 16; j++) {
			if(freeList[k] == true) {
				printf(". ");
				k++;
			}
			else {
				printf("* ");
				k++;
			}
		}
		printf("\n");
	}
}

// internal function, returns -1 for illegal name or name not found
// otherwise returns extent nunber 0-31
int findExtentWithName(char *name, uint8_t *block0); 

// internal function, returns true for legal name (8.3 format), false for illegal
// (name or extension too long, name blank, or  illegal characters in name or extension)
bool checkLegalName(char *name); 


// print the file directory to stdout. Each filename should be printed on its own line, 
// with the file size, in base 10, following the name and extension, with one space between
// the extension and the size. If a file does not have an extension it is acceptable to print
// the dot anyway, e.g. "myfile. 234" would indicate a file whose name was myfile, with no 
// extension and a size of 234 bytes. This function returns no error codes, since it should
// never fail unless something is seriously wrong with the disk 
void cpmDir(); 

// error codes for next five functions (not all errors apply to all 5 functions)  
/* 
    0 -- normal completion
   -1 -- source file not found
   -2 -- invalid  filename
   -3 -- dest filename already exists 
   -4 -- insufficient disk space 
*/ 

//read directory block, 
// modify the extent for file named oldName with newName, and write to the disk
int cpmRename(char *oldName, char * newName); 

// delete the file named name, and free its disk blocks in the free list 
int  cpmDelete(char * name); 

// following functions need not be implemented for Lab 2 

int  cpmCopy(char *oldName, char *newName); 


int  cpmOpen( char *fileName, char mode); 

// non-zero return indicates filePointer did not point to open file 
int cpmClose(int filePointer); 

// returns number of bytes read, 0 = error 
int cpmRead(int pointer, uint8_t *buffer, int size);

// returns number of bytes written, 0 = error 
int cpmWrite(int pointer, uint8_t *buffer, int size);  

