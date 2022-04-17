#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpmfsys.h"
#include "diskSimulator.h"

uint8_t e_set=0;

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
	ptr=e+(32*index);

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

// TODO check if this works
// function to write contents of a DirStructType struct back to the specified index of the extent
// in block of memory (disk block 0) pointed to by e
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e){
	uint8_t *ptr;
	ptr=e+(32*index);

	//write status
	*ptr=d->status;
	ptr++;

	//write name
	int i;
	for (i=0; i<8; i++)
		*(ptr+i)=d->name[i];
	ptr+=i;

	//write extension
	for (i=0; i<3; i++)
		*(ptr+i)=d->extension[i];
	ptr+=i;
	
	//write XL, BKC, XH, and RC
	*(ptr++)=d->XL;
	*(ptr++)=d->BC;
	*(ptr++)=d->XH;
	*(ptr++)=d->RC;

	//write blocks
	for (i=0; i<BLOCKS_PER_EXTENT; i++)
		*(ptr+i)=d->blocks[i];

}

// populate the FreeList global data structure. freeList[i] == true means 
// that block i of the disk is free. block zero is never free, since it holds
// the directory. freeList[i] == false means the block is in use. 
void makeFreeList(){
	//create e pointer
	/*
	uint8_t *e;
	e=malloc(sizeof(uint8_t)*1024);
	blockRead(e, 0);
	*/
	if (!e_set){
		blockRead(e, 0);
		e_set=1;
	}
	
	int i, j;
	memset(freeList, true,  NUM_BLOCKS);
	freeList[0]=false;

	//loop for all directory entries
	for (i=0; i<32; i++){
		//make directory entry structure
		d=mkDirStruct(i, e);

		//check if block is unused
		if (d->status != 0xe5){

			//read blocks into freeList
			for (j=0; j<16; j++){
				if (d->blocks[j] != 0)
					freeList[d->blocks[j]]=false;
				else
					break;
			}
		}
		free(d);
	}
	d=NULL;
}
// debugging function, print out the contents of the free list in 16 rows of 16, with each 
// row prefixed by the 2-digit hex address of the first block in that row. Denote a used
// block with a *, a free block with a . 
void printFreeList(){
	int i, j;
	for (i=0; i<16; i++){
		for (j=0; j<16; j++){
			if (freeList[i*16+j] == true)
				printf(". ");
			else
				printf("* ");
		}
		printf("\n");
	}
}


// internal function, returns -1 for illegal name or name not found
// otherwise returns extent nunber 0-31
int findExtentWithName(char *name, uint8_t *block0){
	if (!checkLegalName(name))
		return -1;

	int len=strlen(name);
	int i;
	//find period index
	int period_idx=-1;
	for (i=0; i < len; i++){
		if (name[i] == '.')
			period_idx=i;
	}

	//set base string with spaces and end with newline
	char name_only[9];
	char ext[4];
	memset(name_only, 32, 8);
	memset(ext, 32, 4);
	name_only[8]=0;
	ext[3]=0;

	//copy strings
	if (period_idx > 0){
		memcpy(name_only, name, period_idx);
		memcpy(ext, name+period_idx+1, len-period_idx-1);
	}
	else{
		memcpy(name_only, name, len);
	}

	//check all extents
	DirStructType *ptr;
	for (i=0; i<16; i++){
		ptr=mkDirStruct(i, block0);
		if (ptr->status != 0xe5){
			int name_cmp=strcmp(name_only, ptr->name);
			int ext_cmp=strcmp(ext, ptr->extension);

			if(name_cmp == 0 && ext_cmp == 0){
				return i;
			}
			free(ptr);
		}
	}

	return -1;
}

// internal function, returns true for legal name (8.3 format), false for illegal
// (name or extension too long, name blank, or  illegal characters in name or extension)
bool checkLegalName(char *name){

	//get length of string
	int len=strlen(name);

	if (len == 0)
		return false;
	if (len > 12)
		return false;

	int i;
	//find period index
	int period_idx=-1;
	for (i=0; i < len; i++){
		if (name[i] == '.')
			period_idx=i;
	}

	if (period_idx == -1 && len > 8)
		return false;

	//return false if period is used as first character
	if (period_idx == 0)
		return false;
	
	//return false if name portion is longer than 8 characters
	if (period_idx > 8)
		return false;
		
	//check if file extension is too long
	if (period_idx > 0 && len - period_idx - 1 > 3)
		return false;

	for (i=0; i<strlen(name); i++){

		//check for period
		if (i == period_idx)
			continue;

		//check everything else
		if (name[i] < 48)
			return false;

		if (name[i] > 57 && name[i] < 65)
			return false;

		if (name[i] > 90 && name[i] < 97)
			return false;

		if (name[i] > 123)
			return false;
	}

	return true;
}


// print the file directory to stdout. Each filename should be printed on its own line, 
// with the file size, in base 10, following the name and extension, with one space between
// the extension and the size. If a file does not have an extension it is acceptable to print
// the dot anyway, e.g. "myfile. 234" would indicate a file whose name was myfile, with no 
// extension and a size of 234 bytes. This function returns no error codes, since it should
// never fail unless something is seriously wrong with the disk 
void cpmDir(){

	if (!e_set){
		blockRead(e, 0);
		e_set=1;
	}

	int i, j;

	printf("------------file directory------------\n");
	//loop for all directory entries
	for (i=0; i<16; i++){
		//make directory entry structure
		d=mkDirStruct(i, e);
		//check if block is unused
		if (d->status != 0xe5){
			//count blocks used
			int NB=0;
			for (j=0; j<16; j++){
				if (d->blocks[j] != 0)
					NB++;
			}
			//decrement because last block is taken care of by RC and BC
			NB--;

			int fsize=NB*1024+d->RC*128+d->BC;
			printf("block %d: %s.%s %d bytes\n", i,d->name, d->extension, fsize);
		}
		free(d);
	}
	d=NULL;
	printf("--------------------------------------\n");
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
	//check if old name is legal
	if (!checkLegalName(oldName))
		return -2;

	//check if new name is legal
	if (!checkLegalName(newName))
		return -2;

	//check if old name doesn't exist
	int block_num=findExtentWithName(oldName, e);
	if (block_num == -1)
		return -1;
	
	//check if new name exists
	if (findExtentWithName(newName, e) > -1)
		return -3;
	
	if (!e_set){
		blockRead(e, 0);
		e_set=1;
	}
	DirStructType *old;
	DirStructType *ren;
	ren=malloc(sizeof(DirStructType));
	
	old=mkDirStruct(block_num, e);

	//clone old into ren
	memcpy(ren, old, sizeof(DirStructType));

	//begin renaming
	int len=strlen(newName);
	int i;
	//find period index
	int period_idx=-1;
	for (i=0; i < len; i++){
		if (newName[i] == '.')
			period_idx=i;
	}
	//set base string with spaces and end with newline
	char name_only[9];
	char ext[4];
	memset(name_only, 32, 8);
	memset(ext, 32, 4);
	name_only[8]=0;
	ext[3]=0;

	//copy strings
	if (period_idx > 0){
		memcpy(name_only, newName, period_idx);
		memcpy(ext, newName+period_idx+1, len-period_idx-1);
	}
	else{
		memcpy(name_only, newName, len);
	}

	memcpy(ren->name, name_only, 8);
	memcpy(ren->extension, ext, 3);
	ren->name[8]=0;
	ren->extension[3]=0;

	writeDirStruct(ren, block_num, e);

	free(ren);
	free(old);
}

// delete the file named name, and free its disk blocks in the free list 
int  cpmDelete(char * name){
	
	/*
	//create e pointer
	uint8_t *e;
	e=malloc(sizeof(uint8_t)*1024);
	blockRead(e, 0);
	*/

	if (!e_set){
		blockRead(e, 0);
		e_set=1;
	}
	
	if (!checkLegalName(name))
		return-2;
	int block_num=findExtentWithName(name, e);
	if (block_num == -1)
		return-1;

	d=mkDirStruct(block_num, e);

	//create replacement DirStructType
	DirStructType *del;
	del=malloc(sizeof(DirStructType));
	del->status=0xe5;
	memset(del->name, 0, 9);
	memset(del->extension, 0, 4);
	del->XL=0;
	del->BC=0;
	del->XH=0;
	del->RC=0;
	memset(del->blocks, 0, 16);

	int i;
	for (i=0; i < 16; i++){
		freeList[d->blocks[i]]=0;
	}

	writeDirStruct(del, block_num, e);

	free(d);
	free(del);
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

