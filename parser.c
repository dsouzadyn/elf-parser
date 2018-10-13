#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>


int main(int argc, char **argv) {
	int fd;
	char* StringTable;
	char* interp;
	uint8_t *mem;
	struct stat st;
	// elf header
	Elf32_Ehdr *ehdr;
	// elf program header
	Elf32_Phdr *phdr;
	// elf section header
	Elf32_Shdr *shdr;

	if (argc < 2) {
		printf("Usage: %s <executable>\n", argv[0]);
	}

	// Open the file
	
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("open");
		exit(-1);
	}

	// Get file status information (needed for size).
	if (fstat(fd, &st) < 0) {
		perror("fstat");
		exit(-1);
	}

	// Map the file into memory
	mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	// Get the elf header
	ehdr = (Elf32_Ehdr *)mem;

	// Get the phdr and shdr located at their respective offsets
	phdr = (Elf32_Phdr *)&mem[ehdr->e_phoff];
	shdr = (Elf32_Shdr *)&mem[ehdr->e_shoff];

	// Check if the file is an ELF
	if (mem[0] != 0x7f && strcmp(&mem[1], "ELF")) {
		fprintf(stderr, "%s is not an ELF file\n", argv[1]);
		exit(-1);
	}

	// Check if the file is executable
	if (ehdr->e_type != ET_EXEC) {
		fprintf(stderr, "%s is not executable\n", argv[1]);
		fprintf(stderr, "%s is of type %u\n", argv[1], ehdr->e_type);
		exit(-1);
	}

	// Get the program entry point
	printf("Program entry point: 0x%x\n", ehdr->e_entry);

	// Get the string table
	StringTable = &mem[shdr[ehdr->e_shstrndx].sh_offset];

	// Print each of the section headers
	printf("Section header list:\n\n");
	for (int i = 1; i < ehdr->e_shnum; i++)
		printf("%-24s: 0x%08x\n", &StringTable[shdr[i].sh_name], shdr[i].sh_addr);

	// Print the program header list
	printf("\nProgram header list:\n\n");

	for(int i = 0; i < ehdr->e_phnum; i++) {
		switch (phdr[i].p_type) {
			case PT_LOAD:
				if (phdr[i].p_offset == 0)
					printf("Text segment: 0x%08x\n", phdr[i].p_vaddr);
				else
					printf("Data segment: 0x%08x\n", phdr[i].p_vaddr);
				break;
			case PT_INTERP:
				interp = strdup((char *)&mem[phdr[i].p_offset]);
				printf("Interpreter: 0x%08x\n", interp);
				break;
			case PT_NOTE:
				printf("Note segment: 0x%08x\n", phdr[i].p_vaddr);
				break;
			case PT_DYNAMIC:
				printf("Dynamic segment: 0x%08x\n", phdr[i].p_vaddr);
				break;
			case PT_PHDR:
				printf("Phdr segment: 0x%08x\n", phdr[i].p_vaddr);
				break;
			case PT_GNU_STACK:
				printf("Gnu stack segment: 0x%08x\n", phdr[i].p_vaddr);
		}
	}

	exit(0);

}
