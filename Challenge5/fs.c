#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "disk.h"
#include "fs.h"

#define MAX_FILES 64

struct super_block { //note all meta info will be stored in block 0, so location is not necessary
  uint16_t used_block_bitmap_offset;
  uint16_t inode_metadata_offset;
  uint16_t directory_offset;
};

struct inode {
  bool isUsed;
  uint16_t singleIndirectBlock; //points to block 
  uint16_t fileSize;
};

struct dir_entry {
  bool isUsed;
  uint16_t inodeNum;
  char name[15];
};

struct file_descriptor {
  bool isUsed;
  uint16_t inodeNum;
  int offset;
};

//// Global Vars ////

struct inode inodes[MAX_FILES];

struct dir_entry directory[MAX_FILES];

struct file_descriptor fds[32];

uint32_t usedBlocksBitmap[256]; //32 bits * 256 entries = 8192 bits

bool readyForUse = 0;

struct super_block sb;

//// Helper functions ////

int findInode(){

  for(int i = 0; i < MAX_FILES; i++){

    if(inodes[i].isUsed == 0){ //if looking for available position

      return i;
    }    
  }

  perror("No available inodes");
  
  return -1; //returns -1 if available index is not found
}

int findDirectory(bool newEntry, const char *filename){

  int index = -1;

  bool check = newEntry;
  
  for(int i = 0; i < MAX_FILES; i++){

    if(check && directory[i].name[0] == 0){ //if looking for a new entry and one is found, save its index, Note: check goes to 0 after first available spot

      index = i;

      check = 0;
    }

    if(directory[i].name[0] != 0){ //only strcmp if name != NULL
    
      if(!strcmp(filename,directory[i].name)){

	if(newEntry){ //if looking for new entry and matching filename is found, return -1

	  perror("File already Exists");

	  return -1;
	}

	else{ //if not looking for new entry, return dir entry with matching file name

	  index = i;
	}
      }
    }
  }

  if(newEntry && index == -1){ //if index is still -1 while looking for new entry, no entries were avaialble

    perror("No available directory entries");
  }

  else if(index == -1){ //if index is still -1 while not looking for new entry, filename was not found

    perror("Filename not found");
  }
  
  return index;
}

int findFD(){

  for(int i = 0; i < 32; i++){

    if(fds[i].isUsed == 0){

      return i;
    }
  }

  perror("No available file descriptors");
  
  return -1;
}

int findUsedBlocksBit(){
  
  int index, bit, flag;

  bool taken = 1;

  for(int i = 0; i < 8192; i++){

    index = i/32;

    bit = i%32;
    
    flag = 1 << bit;

    taken = usedBlocksBitmap[index] & flag; //calculate value for current bit
    
    if(taken == 0){

      return i;
    }
  }

  perror("No available blocks");
  
  return -1;
}

void writeUsedBlocksBit(int position, bool clearSet){ //clearSet = 0 for clear, 1 for set

  uint32_t index = position/32, bit = position%32, flag = 1;

  flag = flag << bit; //000...010...000

  if(clearSet){ //setting bit to 1

    usedBlocksBitmap[index] = usedBlocksBitmap[index] | flag;
  }
  
  else{

    flag = ~flag; ///111...101...111

    usedBlocksBitmap[index] = usedBlocksBitmap[index] & flag;
  }
}

int make_fs(const char *disk_name){
  
  void *writeBuf = malloc(BLOCK_SIZE);

  unsigned long int addr = (unsigned long int) writeBuf;

  //Set superblock data
  
  sb.used_block_bitmap_offset = sizeof(sb);

  sb.inode_metadata_offset = sizeof(sb) + sizeof(usedBlocksBitmap);

  sb.directory_offset = sizeof(sb) + sizeof(usedBlocksBitmap) + sizeof(inodes);

  //Fill arrays with 0's
  
  memset(inodes, 0, sizeof(inodes));

  memset(directory, 0, sizeof(directory));

  memset(usedBlocksBitmap, 0, sizeof(usedBlocksBitmap));

  writeUsedBlocksBit(0, 1); //set bit 0 in uBB
  
  //Write arrays to buf
  
  memcpy((void*) addr, &sb, sizeof(sb));

  addr += sizeof(sb);
  
  memcpy((void*) addr, usedBlocksBitmap, sizeof(usedBlocksBitmap));

  addr += sizeof(usedBlocksBitmap);

  memcpy((void*) addr, inodes, sizeof(inodes));

  addr += sizeof(inodes);

  memcpy((void*) addr, directory, sizeof(directory));

  addr += sizeof(directory);

  //Create, open, write to, and close disk
  
  if(make_disk(disk_name) == -1){

    perror("Error making disk\n");

    return -1;
  }

  if(open_disk(disk_name) == -1){

    perror("Error opening disk\n");

    return -1;
  }
  
  if(block_write(0, writeBuf) == -1){ // All metadata written to block 0

    perror("Error writing to disk");

    return -1;
  }
  
  if(close_disk() == -1){

    perror("Error closing disk");

    return -1;
  }

  free(writeBuf); //free malloced data after no longer needed
  
  return 0;
}

int mount_fs(const char *disk_name){

  void *readBuf = malloc(BLOCK_SIZE); // allocate read buffer

  unsigned long int addr = (unsigned long int) readBuf; //store buf in ul int

  //set strucs to 0
  
  memset(inodes, 0, sizeof(inodes));

  memset(directory, 0, sizeof(directory));

  memset(usedBlocksBitmap, 0, sizeof(usedBlocksBitmap));
  
  //Open and read from disk
  
  if(open_disk(disk_name) == -1){

    perror("Error opening disk\n");

    return -1;
  }

  if(block_read(0, readBuf) == -1){ //read metadata

    perror("Error reading from disk");

    return -1;
  }

  //Copy all array info from disk
  
  memcpy(&sb, (void*) addr, sizeof(sb));

  addr += sizeof(sb);

  memcpy(usedBlocksBitmap, (void*) addr, sizeof(usedBlocksBitmap));

  addr += sizeof(usedBlocksBitmap);

  memcpy(inodes, (void*) addr, sizeof(inodes));

  addr += sizeof(inodes);

  memcpy(directory, (void*) addr, sizeof(directory));

  addr += sizeof(directory);

  free(readBuf); //free malloced data after no longer needed
  
  readyForUse = 1;

  return 0;
}

int umount_fs(const char *disk_name){
  
  void *writeBuf = malloc(BLOCK_SIZE);

  unsigned long int addr = (unsigned long int) writeBuf;

  //Check if fs is mounted

  if(!readyForUse){

    perror("File system not mounted\n");

    return -1;
  }
  
  //Write arrays to buf

  memcpy((void*) addr, &sb, sizeof(sb));

  addr += sizeof(sb);

  memcpy((void*) addr, usedBlocksBitmap, sizeof(usedBlocksBitmap));

  addr += sizeof(usedBlocksBitmap);

  memcpy((void*) addr, inodes, sizeof(inodes));

  addr += sizeof(inodes);

  memcpy((void*) addr, directory, sizeof(directory));
  
  addr += sizeof(directory);

  //Write, and close disk

  if(block_write(0, writeBuf) == -1){ // All metadata written to block 0

    perror("Error writing to disk");

    return -1;
  }

  if(close_disk() == -1){

    perror("Error closing disk");

    return -1;
  }

  //Set all arrays to 0
  
  memset(inodes, 0, sizeof(inodes));

  memset(directory, 0, sizeof(directory));

  memset(usedBlocksBitmap, 0, sizeof(usedBlocksBitmap));

  memset(fds, 0, sizeof(fds));

  readyForUse = 0; //Mark disk fs as no longer ready for use

  return 0;
}

int fs_create(const char *name){
  
  int inodeIndex = findInode(), dirIndex = findDirectory(1, name), inodeBlocksArrayLoc, blockNum;

  uint16_t inodeBlocksArray[256];

  inodeBlocksArrayLoc = findUsedBlocksBit();

  writeUsedBlocksBit(inodeBlocksArrayLoc, 1); //mark both used blocks as used

  blockNum = findUsedBlocksBit();

  writeUsedBlocksBit(blockNum, 1);
  
  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }
  
  if(strlen(name) > 15){

    perror("Filename larger than 15 characters");

    return -1;
  }

  if(inodeIndex == -1 || dirIndex == -1 || blockNum == -1){

    return -1;
  }
    
  memset(inodeBlocksArray, 0, sizeof(inodeBlocksArray)); //starts out empty

  inodeBlocksArray[0] = blockNum; //save block number of first block for the file
  
  //set structure info
  
  inodes[inodeIndex].isUsed = true;

  inodes[inodeIndex].singleIndirectBlock = inodeBlocksArrayLoc; //set block location for list of blocks for Inode

  inodes[inodeIndex].fileSize = 0;

  directory[dirIndex].isUsed = true;

  directory[dirIndex].inodeNum = inodeIndex;
  
  strcpy(directory[dirIndex].name, name);
  
  //write to inodeBlocksArray disk. Note: nothign is actually written to the file yet

  if(block_write(inodeBlocksArrayLoc, inodeBlocksArray) == -1){ // All metadata written to block 0

    perror("Error writing to disk in file creation");

    return -1;
  }
  
  return 0;
}

int fs_open(const char *name){
  
  int dirIndex = findDirectory(0, name), fdIndex = findFD();

  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }
  
  if(dirIndex == -1 || fdIndex == -1){

    return -1;
  }

  fds[fdIndex].isUsed = true;

  fds[fdIndex].inodeNum = directory[dirIndex].inodeNum;

  fds[fdIndex].offset = 0;

  return fdIndex; //increase number of files count
}

int fs_close(int fildes){

  if(fds[fildes].isUsed == 0){

    perror("File descriptor not in use");

    return -1;
  }

  fds[fildes].isUsed = false;

  fds[fildes].inodeNum = 0;

  fds[fildes].offset = 0;

  return 0;
}

int fs_delete(const char *name){

  char empty[BLOCK_SIZE]; //set to all 0's and write to each used block
  
  int dirIndex = findDirectory(0, name), inodeIndex;

  uint32_t inodeBlocksArray[256];

  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }

  if(dirIndex == -1){

    return -1;
  }
  
  memset(empty, 0, sizeof(empty));
  
  inodeIndex = directory[dirIndex].inodeNum;

  //look if file is open
  
  for(int i = 0; i < 32; i++){

    if(fds[i].inodeNum == inodeIndex && fds[i].isUsed){

      perror("File in uses, deletion fail");

      return -1;
    }
  }

  //Clear any used blocks

  if(block_read(inodes[inodeIndex].singleIndirectBlock, inodeBlocksArray) == -1){ //read in inodeBlocksArray

    perror("Error reading disk in file deletion");

    return -1;
  }
  
  for(int i = 0; i < 256; i++){

    int currentBlockNum = inodeBlocksArray[i];

    if(currentBlockNum == 0){ //if currentBlockNum = 0 then the there are no more used blocks

      break;
    }

    else{

      if(block_write(currentBlockNum, empty) == -1){ //reset back to 0

	perror("Error writing to disk in file deletion");

	return -1;
      }

      writeUsedBlocksBit(currentBlockNum, 0); //set current block as unused
    }
  }

  if(block_write(inodes[inodeIndex].singleIndirectBlock, empty) == -1){ //set inode array block back to 0

    perror("Error writing to disk in file deletion");

    return -1;
  }

  writeUsedBlocksBit(inodes[inodeIndex].singleIndirectBlock, 0); //mark inode blocks array block as unused

  //set structures back to unused
  
  inodes[inodeIndex].isUsed = false;

  inodes[inodeIndex].singleIndirectBlock = 0; //set block location for list of blocks for Inode

  inodes[inodeIndex].fileSize = 0;

  directory[dirIndex].isUsed = false;

  directory[dirIndex].inodeNum = 0;

  memset(directory[dirIndex].name,0,sizeof(directory[dirIndex].name)); //fill name array back with 0's

  return 0;
}

int fs_read(int fildes, void *buf, size_t nbyte){
  
  int blockIndex, inodeIndex, remOffset, readBytes = 0;

  void *readFromBlock = malloc(4096), *arrayRead = malloc(4096);
  
  unsigned long int destAddr = (unsigned long int) buf, diskReadAddr = (unsigned long int) readFromBlock;

  uint32_t inodeBlocksArray[256];

  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }
  
  if(fds[fildes].isUsed == 0){

    perror("File descriptor is not valid");

    return -1;
  }

  inodeIndex = fds[fildes].inodeNum;
  
  blockIndex = fds[fildes].offset/BLOCK_SIZE; //get index for block in inodeBlockArray

  remOffset = fds[fildes].offset-(blockIndex*BLOCK_SIZE); //calculate offset into current block

  if(fds[fildes].offset >= inodes[inodeIndex].fileSize){

    printf("Already at end of file\n");

    return 0;
  }
  
  if(block_read(inodes[inodeIndex].singleIndirectBlock, arrayRead) == -1){ //read info for inodeBlocksArray

    perror("Error reading disk in file read");

    return -1;
  }

  memcpy(inodeBlocksArray, arrayRead, sizeof(inodeBlocksArray));
  
  if(inodeBlocksArray[blockIndex] == 0){ //if nothing is written at this point in the file

    printf("Nothign written at this point in the file\n");
    
    return 0;
  }

  if(block_read(inodeBlocksArray[blockIndex], readFromBlock) == -1){ //read into block

    perror("Error reading disk in file read");

    return -1;
  }

  diskReadAddr += remOffset;

  printf("read block index = %d, block = %d\n", blockIndex, inodeBlocksArray[blockIndex]);
  
  memcpy((void*) destAddr, (void*)diskReadAddr, nbyte);

  readBytes += nbyte;
  /*
  readBytes++;
  
  for(int i = 0; i < nbyte; i++){ //read each byte in
    
    destAddr += 1;

    diskReadAddr += 1;
    
    memcpy((void*) destAddr, (void*)diskReadAddr, 1);

    readBytes++;
  }
*/
  free(readFromBlock);

  fds[fildes].offset += readBytes;
  
  return readBytes;
}

int fs_write(int fildes, void *buf, size_t nbyte){
  
  int blockIndex, inodeIndex, remOffset, writeBytes = 0;

  void *writeToBlock = malloc(4096), *arrayRead = malloc(4096);

  unsigned long int destAddr = (unsigned long int) buf, diskWriteAddr = (unsigned long int) writeToBlock;

  uint32_t inodeBlocksArray[256];

  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }
  
  if(fds[fildes].isUsed == 0){

    perror("File descriptor is not valid");

    return -1;
  }

  inodeIndex = fds[fildes].inodeNum;

  blockIndex = fds[fildes].offset/BLOCK_SIZE; //get index for block in inodeBlockArray

  remOffset = fds[fildes].offset-(blockIndex*BLOCK_SIZE); //calculate offset into current block
  
  if(block_read(inodes[inodeIndex].singleIndirectBlock, arrayRead) == -1){ //read info for inodeBlocksArray

    perror("Error reading disk in file write");

    return -1;
  }

  memcpy(inodeBlocksArray, arrayRead, sizeof(inodeBlocksArray));
  
  if(block_read(inodeBlocksArray[blockIndex], writeToBlock) == -1){ //read into block

    perror("Error reading disk in file write");

    return -1;
  }
  
  diskWriteAddr += remOffset;
  
  memcpy((void*) diskWriteAddr, (void*) destAddr, nbyte);

  writeBytes += nbyte;
  /*
  writeBytes++;
  
  for(int i = 0; i < nbyte; i++){ //read each byte in

    destAddr += 1;

    diskWriteAddr += 1;

    memcpy((void*) diskWriteAddr, (void*) destAddr, 1);

    writeBytes++;
  }
  */
  if(block_write(inodeBlocksArray[blockIndex], writeToBlock) == -1){ //write into block

    perror("Error writing disk in file write");

    return -1;
  }
  
  fds[fildes].offset += writeBytes;
  
  inodes[inodeIndex].fileSize += writeBytes;
  
  free(writeToBlock);

  free(arrayRead);
  
  return writeBytes;
}

int fs_get_filesize(int fildes){

  int inodeIndex;

  if(fds[fildes].isUsed == 0){

    perror("File descriptor not in use");

    return -1;
  }

  inodeIndex = fds[fildes].inodeNum;

  return inodes[inodeIndex].fileSize;
}

int fs_listfiles(char ***files){

  int fileCount = 0, listIndex = 0;
  
  char **list;
    
  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }

  for(int i = 0;i < MAX_FILES; i++){

    if(directory[i].name[0] != 0){

      fileCount++;
    }
  }

  list = (char **) malloc(fileCount*sizeof(char*));
  
  for(int i = 0; i < MAX_FILES; i++){

    if(directory[i].name[0] != 0){
      
      list[listIndex] = directory[i].name;

      listIndex++;
    }
  }

  list[listIndex] = NULL; //set index after last file name to NULL
  
  *files = list;
  
  return 0;
}

int fs_lseek(int fildes, off_t offset){

  int inodeIndex;

  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }
  
  if(fds[fildes].isUsed == 0){

    perror("File descriptor not in use");

    return -1;
  }

  inodeIndex = fds[fildes].inodeNum;
  
  if(offset < 0){

    perror("Offset less than 0 error");

    return -1;
  }

  if(offset > inodes[inodeIndex].fileSize){

    perror("Offset larger than file size");

    return -1;
  }

  fds[fildes].offset = offset;
  
  return 0;
}

int fs_truncate(int fildes, off_t length){

  int inodeIndex;

  if(!readyForUse){ //check if fs is mounted

    perror("File system not mounted\n");

    return -1;
  }
  
  if(fds[fildes].isUsed == 0){

    perror("File descriptor not in use");

    return -1;
  }

  inodeIndex = fds[fildes].inodeNum;
  
  if(length > inodes[inodeIndex].fileSize){

    return -1;
  }

  inodes[inodeIndex].fileSize = length; ///Not done

  return 0;
}
