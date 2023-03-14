#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#define chunk_size(chunk) (*(((unsigned long*)chunk)-1)& (~(unsigned long)0x7))
#define is_allocated(chunk) (*((unsigned long*)((void*)chunk+chunk_size(chunk)-8))&1)

void print_chunk(FILE* out, void* ptr) {
	void *pointer = ptr;
	unsigned long size = chunk_size(pointer);
	void* temp = sbrk(0);
	
	if(temp <= (pointer + size)) {
		fprintf(out,"[F%ld: %p]",size, pointer);
		return;
	}
	if(is_allocated(pointer))
		fprintf(out,"[A%ld: %p]",size, pointer);
	if(!is_allocated(pointer))
		fprintf(out,"[F%ld: %p]",size, pointer);
}

void print_heap(FILE* out, void* from) {
	void* chunk = from - 0x8;
	unsigned long size = chunk_size(chunk);
	void* temp = from + size;
	
	while(temp < sbrk(0)) {
		print_chunk(out, temp);
		size = chunk_size(temp);
		temp = temp + size;
	}
	fprintf(out, "\n");
}

void print_freelist(FILE* out, void* hdr) {
	unsigned long size = chunk_size(hdr);
	void* temp = *((void**)hdr) + 16;
	
	while(temp != hdr) {
		print_chunk(out, temp);
		temp = *((void**)temp) + 16;
	}
	fprintf(out, "\n");
}
