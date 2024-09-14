#include <stdio.h>
#include <stdlib.h>
#include "core.h"


void print_inst(Inst inst){
    switch (inst.id.as_inst_id)
    {
    case INSTRUCTION_HALT:
        printf("halt;\n");
        break;
    case INSTRUCTION_DUMP_STACK:
        printf("dump;\n");
        break;
    case INSTRUCTION_PUSH:
        printf("push (f: %f; li: %li; lu: %lu; ptr: %p);\n",
        inst.operand1.as_float64, inst.operand1.as_int64, inst.operand1.as_uint64, inst.operand1.as_ptr);
        break;
    case INSTRUCTION_POP:
        printf("pop\n");
        break;
    
    default:
        printf("[ERROR] Unkown Instruction %li\n", inst.id.as_int64);
        exit(ERROR_UNKOWN_INSTRUCTION);
        break;
    }
}


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

    Stream stream = (Stream){.data = malloc(1024), .size = 0, .capacity = 1024};

    for(; !feof(file);){
        stream.size += SIZEOF_CHUNK * fread(stream.data + stream.size, SIZEOF_CHUNK, stream.capacity / SIZEOF_CHUNK, file);
        if(stream.size >= stream.capacity){
            resize_stream(&stream, 2 * stream.capacity);
        }
    }

    stream.data[stream.size] = '\0';

    fclose(file);

    if(stream.size % sizeof(Inst)){
        printf("[WARNING] Invalid Program Size, Size Of Program Is Not A Multiple Of The Size Of Instructions\n");
    }

    printf("size of program: %lu instructions\n\n", (unsigned long)(stream.size / sizeof(Inst)));

    for(unsigned long i = 0; i < stream.size; i += sizeof(Inst)){
        const Var* current_ptr = (Var*)(stream.data + i);
        Inst inst = (Inst){
            .id = *current_ptr,
            .operand1 = *(current_ptr + 1),
            .operand2 = *(current_ptr + 2)
        };
        print_inst(inst);
    }


	return 0;
}
