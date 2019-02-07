/* 
    File: my_allocator.c

    Author: Donald Elrod
            Department of Computer Science
            Texas A&M University
    Date  : 2/2/2018

    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/


#include <stdlib.h>
#include <stdio.h>
#include "my_allocator.h"
#include <math.h>


struct header {
	header* next_blk;
	int blk_size;
};//set the pointer to the header to the beginning of the address block memory
//header* head = (header*)address;


unsigned int min_block; //the minimum size a block can be split into
unsigned int len; //total length of the reserved address
char* address; //pointer to the first address of the allocated memory
int* block_av;
unsigned int num_sizes;
header** free_list;

//function declarations
unsigned int is_power_of_2(int size_to_check);
signed int size_needed(int requested_size);
char* find_available_block(int size_index);
char* split_address(char* start, int address_size_index, int needed_size_index);
void add_to_free_list(char* addr, int size_index, int new_size);
char* remove_from_free_list(int size_index, header* b);
void check_for_buddies(char* addr, int size_index);
header* find_last_free_header(int size_index);
void merge_blocks(header* A, header* B, int size_index);





extern unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length) {
		
	//this just checks if the input is valid, and makes it valid if it is not
	_length = is_power_of_2(_length);
	_basic_block_size = is_power_of_2(_basic_block_size);
	
	
	address = (char*)malloc(_length);
	min_block = _basic_block_size;
	len = _length;


	num_sizes = 0;
	int csize = min_block;
	
	while (csize <= _length) { //calculates the number of unique sizes that the block can be split into
		num_sizes++;
		csize <<= 1;
	}
	
	block_av = (int*)malloc(num_sizes*sizeof(int));
	free_list = (header**)malloc(num_sizes*sizeof(header*));
		
	//sets the possible sizes, the availability, and prints the bloock sizes possible
	for (int i = num_sizes-1; i >= 0; i--) {
		block_av[i] = 0;
		free_list[i] = NULL;
	}
	
	free_list[0] = (header*)address;
	free_list[0]->next_blk = NULL;
	free_list[0]->blk_size = len;
	block_av[0] = 1; //block_av goes in decreasing order, so at the 0 index should be max block size, and at the last should be minimim block size
	
}

extern int release_allocator() {
	free(address);
	//atexit();
}

//returns the next biggest size if it is not, or 0 for an error
unsigned int is_power_of_2(int size_to_check) {
	
	int csize = 2;
	
	bool run_while = true;
	
	while (run_while) {
		if (csize < size_to_check) //if the current csize is still less than size_to_check
			csize <<= 1;
		else if (csize >= size_to_check) //if the current csize is larger than to size_to_check, it returns the next closest power of 2 size, or if it is equal it return size_to_check
			return csize;
		else run_while = false;
	}
	
	return 0;
}

extern Addr my_malloc(unsigned int _length) {
  
  signed int size_index = size_needed(_length);
  
  if (size_index == -1)
	  return NULL;
  
  
  
  //if there is a block available of that type, this decrements the number of block available of that type and returns the address to the first empty block of that type
  if (free_list[size_index] != NULL) {
	  //printf("removing from free list\n");
	  //print_allocator();
	  return remove_from_free_list(size_index, free_list[size_index]) + sizeof(header);  
  } 
  else {
	  //if there are no available block sizes of the needed type, check if there are any bigger block sizes that can be split
	  return find_available_block(size_index) + sizeof(header);
  }
  
}
 
//finds the index of the minimum size of the block for the array free_list
signed int size_needed(int requested_size) {
	
	signed int size_index = -1;
	for (int i = 0; i < num_sizes; i++) {
		int bigger_size = (len >> i) - sizeof(header);
		int smaller_size = (len >> (i + 1)) - sizeof(header);
		if (bigger_size >= requested_size && smaller_size <= requested_size){
			return i;
		}
		else if (i+1 == num_sizes) {
			return i;
		}
	}
	return size_index;
}

//if there are no available block of the size needed, call this method to make one
//returns the address of the needed block size
char* find_available_block(int size_index) {
	//starting at the minimum block size needed, this will check if larger block sizes are available
	
	for (int i = size_index; i >= 0; i--) { 
			if (free_list[i] != NULL) { //need to split this block size_i
				return split_address((char*)free_list[i], i, size_index);
			}
	}
	return NULL;
}

//takes the starting address of the block to split, the blocks size, and the needed length of the memory
//recursively split the memory until the right size is found
char* split_address(char* start, int address_size_index, int needed_size_index) {
	
	int needed_memory_length = len >> needed_size_index;
	int address_memory_length = len >> address_size_index;
	int new_length = address_memory_length >> 1;
	
	char* free_address = (start + new_length);
	
	block_av[address_size_index]--;

	add_to_free_list(free_address, address_size_index + 1, new_length); 
	
	if (new_length > needed_memory_length) {
		return split_address(start, address_size_index + 1, needed_size_index);
	}
	else {
		header* h = (header*)start;
		h->next_blk = NULL;
		h->blk_size = new_length;
		return start;
	}
}


void add_to_free_list(char* addr, int size_index, int new_length) {
	
	header* temp = free_list[size_index];//find_last_free_header(size_index);
	header* new_block = (header*)addr;
	new_block->next_blk = NULL;
	new_block->blk_size = new_length;
	
	if (temp == NULL) {
		free_list[size_index] = new_block;
	}
	else  {
		free_list[size_index] = new_block;
		new_block->next_blk = temp;
	}
	block_av[size_index]++;
}

header* find_last_free_header(int size_index) {
	header* temp = free_list[size_index];
	if (temp == NULL)
		return free_list[size_index];
	else {
		while (temp->next_blk != NULL) {
			temp = temp->next_blk;
		}
	}
	return temp;
}

char* remove_from_free_list(int size_index, header* b) {
	
	header* temp = free_list[size_index];
	if (temp == b) {//the block is at the front
		free_list[size_index] = temp->next_blk;
	}
	else {
		while (temp->next_blk != b) {
			temp = temp->next_blk;
		}
		temp->next_blk = b->next_blk;
	}
	block_av[size_index]--;
	
	return (char*)b;
}

extern int my_free(Addr _a) {
	
	header* h = (header*)((char*)_a - sizeof(header));
	int size = h->blk_size;
	
	int size_index = (int)( log(len/size) / log(2));
	add_to_free_list((char*)h, size_index, size);
	if (size_index > 0){
		check_for_buddies((char*)h, size_index);
	}
	return 0;
}

void check_for_buddies(char* addr, int size_index) {
	if (size_index == 0) {
		free_list[0]->next_blk = NULL;
		return;
	}
	char* A_offset = (char*)(((char*)addr) - (long int)address);
	int block_size = len >> size_index;
	char* buddy_addr = (((char*)addr - address) ^ block_size) + address;
		
	header* candidate = free_list[size_index];
	int max_size = sizeof(free_list);
	int c = 0;
	while (candidate != NULL && c++<5) {
		if (candidate == (header*)buddy_addr && size_index > 0) {
			merge_blocks((header*)addr, candidate, size_index);
			return;
		}
		candidate = candidate->next_blk;
	}
}

void merge_blocks(header* A, header* B, int size_index) {
	remove_from_free_list(size_index, A);
	remove_from_free_list(size_index, B);
	header* new_block;
	if (A < B)
		new_block = A;
	else
		new_block = B;
	new_block->blk_size = A->blk_size << 1;

	new_block->next_blk = NULL;
	add_to_free_list((char*)new_block, size_index - 1, new_block->blk_size);
	if (size_index > 0)
		check_for_buddies((char*)new_block, size_index - 1);
	else
		free_list[0]->next_blk = NULL;
	
}

extern void print_allocator() {
	printf("\n\nOpen memory blocks\n");
	//for(int i = 0; i < num_sizes; i++) {
		//printf("block sizes: %d\nblocks available: %d\n", len >> i, block_av[i]);//block_sizes[i]);
	//}
	int csize = len;
	
	for(int i = 0; i < num_sizes; i++) {
		printf("free list pointers (%d): %08x \n", len >> i, free_list[i]);
		header* h = free_list[i];
		int c=0;
		while (h != NULL && c++ < 5) {
			printf("\tpoints to: %08x\n",h->next_blk);
			h=h->next_blk;
		}
	}	
}
