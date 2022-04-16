#include <stdio.h>
#include <stdlib.h>
#include "cpmfsys.h"
#include "diskSimulator.h"

void print_block_hex(uint8_t *block){
	int i, j;
	for (i=0; i<32; i++){
		for (j=0; j<32; j++){
			printf("%02x ", block[i*j+j]);
		}
		printf("\n");
	}
}


//function to allocate memory for a DirStructType (see above), and populate it, given a
//pointer to a buffer of memory holding the contents of disk block 0 (e), and an integer index
// which tells which extent from block zero (extent numbers start with 0) to use to make the
// DirStructType value to return. 
DirStructType *mkDirStruct(int index,uint8_t *e){

	DirStructType *dst;
	dst=malloc(sizeof(DirStructType));
	uint8_t *ptr;
	ptr=e;

	//read in status
	dst->status=*ptr++;

	//read in name
	int i;
	for (i=0; i<8; i++)
		dst->name[i]=*(ptr+i);
	dst->name[i]=0;
	ptr+=i;

	//read in extension
	for (i=0; i<3; i++)
		dst->extension[i]=*(ptr+i);
	dst->extension[i]=0;
	ptr+=i;

	//read in XL, BC, XH, and RC
	dst->XL=*(ptr++);
	dst->BC=*(ptr++);
	dst->XH=*(ptr++);
	dst->RC=*(ptr++);

	//read in blocks
	for (i=0; i<BLOCKS_PER_EXTENT; i++){
		dst->blocks[i]=*(ptr+i);
	}

	return dst;
}

// function to write contents of a DirStructType struct back to the specified index of the extent
// in block of memory (disk block 0) pointed to by e
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e){
	return;
}

// populate the FreeList global data structure. freeList[i] == true means 
// that block i of the disk is free. block zero is never free, since it holds
// the directory. freeList[i] == false means the block is in use. 
void makeFreeList(){
	//create e pointer
	e=malloc(sizeof(uint8_t)*1024);
	//read block 0 into memory
	blockRead(e, 0);
	print_block_hex(e);
	d=mkDirStruct(0, e);
}
// debugging function, print out the contents of the free list in 16 rows of 16, with each 
// row prefixed by the 2-digit hex address of the first block in that row. Denote a used
// block with a *, a free block with a . 
void printFreeList(){
	return;
}


// internal function, returns -1 for illegal name or name not found
// otherwise returns extent nunber 0-31
int findExtentWithName(char *name, uint8_t *block0){
	return 0;
}

// internal function, returns true for legal name (8.3 format), false for illegal
// (name or extension too long, name blank, or  illegal characters in name or extension)
bool checkLegalName(char *name){
	return true;
}


// print the file directory to stdout. Each filename should be printed on its own line, 
// with the file size, in base 10, following the name and extension, with one space between
// the extension and the size. If a file does not have an extension it is acceptable to print
// the dot anyway, e.g. "myfile. 234" would indicate a file whose name was myfile, with no 
// extension and a size of 234 bytes. This function returns no error codes, since it should
// never fail unless something is seriously wrong with the disk 
void cpmDir(){
	return;
}

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
int cpmRename(char *oldName, char * newName){
	return 0;
}

// delete the file named name, and free its disk blocks in the free list 
int  cpmDelete(char * name){
	return 0;
}

// following functions need not be implemented for Lab 2 

int  cpmCopy(char *oldName, char *newName){
	return 0;
}


int  cpmOpen( char *fileName, char mode){
	return 0;
}

// non-zero return indicates filePointer did not point to open file 
int cpmClose(int filePointer){
	return 0;
}

// returns number of bytes read, 0 = error 
int cpmRead(int pointer, uint8_t *buffer, int size){
	return 0;
}

// returns number of bytes written, 0 = error 
int cpmWrite(int pointer, uint8_t *buffer, int size){
	return 0;
}

