#include <stdlib.h>
#include <stdio.h>
#include "core.h"


enum States{
    STATE_NONE = 0,

    STATE_ACTIVE
};


struct VirtualMachine{
Inst* instructions;
unsigned long inst_count;
int state;
} virtual_machine;


void load_program(const char* path){
    FILE* file = fopen(path, "rb");

    if(!file){
		printf("[ERROR] Invalid File Path '%s'\n", path);
		exit(ERROR_INVALID_FILE_PATH);
	}

    Stream stream = (Stream){.data = malloc(1024), .size = 0, .capacity = 1024};

    for(; !feof(file);){
        stream.size += fread(stream.data, SIZEOF_CHUNK, 3, file);
        if(stream.size >= stream.capacity){
            resize_stream(&stream, 2 * stream.capacity);
        }
    }

    fclose(file);

    virtual_machine.instructions = (Inst*)malloc(stream.size);
    virtual_machine.inst_count = stream.size / sizeof(Inst);

    for(unsigned long i = 0; i < virtual_machine.inst_count; i += 1){
        virtual_machine.instructions[i] = ((Inst*)stream.data)[i];
    }

    free(stream.data);
}


void eval_inst(Inst inst){
    switch (inst.id.as_int64)
    {
    case INSTRUCTION_HALT:
        exit(0);
        break;
    
    default:
        printf("[ERROR] Unkown Instruction %u\n", inst.id.as_inst_id);
        break;
    }
}




int main(int argc, char** argv){

	if(argc < 2){
		printf("[HELP] Usage: ./vimulator <path_to_executable>\n");
		printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
		return ERROR_INVALID_USAGE;
	}

	load_program(argv[1]);



    for(unsigned long i = 0; i < virtual_machine.inst_count; i += 1){
        eval_inst(virtual_machine.instructions[i]);
    }


    return 0;
}

