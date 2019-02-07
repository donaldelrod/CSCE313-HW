#include "ackerman.h"
#include "my_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void exiting(){
	print_allocator();
	release_allocator();
}

int main(int argc, char ** argv) {
	
	atexit(exiting);
	int basic_block_size, memory_length;
	basic_block_size = 128;
	memory_length = 524288;
	
	int tag;
	
	while ((tag = getopt(argc, argv, "b:s:")) != -1) {
		switch (tag) {
			case 's':
				memory_length = atoi(optarg);
				break;
			case 'b':
				basic_block_size = atoi(optarg);
				break;
			default:
				printf("Inproper arg input. Try again/n");
				return 0;
				break;
		}
	}
	
	init_allocator(basic_block_size, memory_length);
  
	//char* t1 = (char*)my_malloc(524288-16);
	//print_allocator();
	//my_free(t1);
	//print_allocator();
	//char* t2 = (char*)my_malloc(12);
	//print_allocator();
	
	//printf("t1: %08x\nt2: %08x\n\n", t1, t2);
	
	
	//my_free(t1);
	//print_allocator();
	//my_free(t2);
	//print_allocator();
  
	ackerman_main(); // this is the full-fledged test. 
	// The result of this function can be found from the ackerman wiki page or https://www.wolframalpha.com/. If you are not getting correct results, that means that your allocator is not working correctly. In addition, the results should be repetable - running ackerman (3, 5) should always give you the same correct result. If it does not, there must be some memory leakage in the allocator that needs fixing
  
	// please make sure to run small test cases first before unleashing ackerman. One example would be running the following: "print_allocator (); x = my_malloc (1); my_free(x); print_allocator();" the first and the last print should be identical.
	print_allocator();
	//release_allocator();
}