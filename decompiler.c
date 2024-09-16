#include <stdio.h>
#include <stdlib.h>
#include "core.h"


int main(int argc, char** argv){

	if(argc < 2){
		printf("[HELP] Usage: ./devirtual <path_to_executable>\n");
		printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
		return ERROR_INVALID_USAGE;
	}

	FILE* file = fopen(argv[1], "rb");

    if(!file){
		printf("[ERROR] Invalid File Path '%s'\n", argv[1]);
		exit(ERROR_INVALID_FILE_PATH);
	}
    unsigned char pdata[1024];

    Program program = (Program){.data = pdata, .size = 0, .capacity = 1024};

    for(; !feof(file);){
        program.size += SIZEOF_CHUNK * fread(program.data + program.size, SIZEOF_CHUNK, program.capacity / SIZEOF_CHUNK, file);
        if(program.size >= program.capacity){
            resize_stream(&program, 2 * program.capacity);
        }
    }

    fclose(file);

    printf("size of program: %zu bytes\n\n", program.size);

    for(size_t i = 0; i < program.size; i += print_inst(program, i));

	return 0;
}
