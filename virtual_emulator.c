#include <stdlib.h>
#include <stdio.h>
#include "core.h"

#define STACK_CAP 1024

enum States{
    STATE_NONE = 0,

    STATE_ACTIVE
};


struct VirtualMachine{

Inst* instructions;
unsigned long inst_address;
unsigned long inst_count;

Var* stack;
unsigned long stack_size;

Var reg;

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
        stream.size += (SIZEOF_CHUNK) * fread(stream.data + stream.size, SIZEOF_CHUNK, stream.capacity / SIZEOF_CHUNK, file);
        if(stream.size >= stream.capacity){
            resize_stream(&stream, 2 * stream.capacity);
        }
    }

    fclose(file);

    virtual_machine.instructions = (Inst*)malloc(stream.size + STACK_CAP);
    virtual_machine.inst_count = stream.size / sizeof(Inst);
    virtual_machine.inst_address = 0;
    virtual_machine.stack = (Var*)(virtual_machine.instructions + virtual_machine.inst_count);
    virtual_machine.stack_size = 0;

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
    case INSTRUCTION_DUMP_STACK:
        for(unsigned long i = 0; i < virtual_machine.stack_size; i+=1){
            const Var var = virtual_machine.stack[i];
            printf(
                "%lu-- i: %li, u: %lu, f: %f, ptr: %p\n",
                i, var.as_int64, var.as_uint64, var.as_float64, var.as_ptr
            );
        }
        break;

    case INSTRUCTION_PUSH:
        virtual_machine.stack[virtual_machine.stack_size++] = inst.operand1;
        break;
    case INSTRUCTION_POP:
        virtual_machine.stack_size -= 1;
        break;
    
    default:
        printf("[ERROR] Unkown Instruction %u\n", inst.id.as_inst_id);
        break;
    }
}




int main(int argc, char** argv){

	if(argc < 2){
		printf("[HELP] Usage: ./vimulator <path_to_executable> -<optional: flags>\n");
		printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
		return ERROR_INVALID_USAGE;
	}

	load_program(argv[1]);



    for(unsigned long i = 0; i < virtual_machine.inst_count; i += 1){
        eval_inst(virtual_machine.instructions[i]);
    }

    const char* show_stack_flag = "-show_stack";

    for(int i = 2; i < argc; i+=1){
        int flag_on = 1;
        for(unsigned int j = 0; argv[j] && show_stack_flag[j]; j+=1){
            if(argv[i][j] != show_stack_flag[j]){
                flag_on = 0;
                break;
            }
        }
        if(flag_on){
            printf("stack(size=%lu):\n", virtual_machine.stack_size);
            for(unsigned long j = 0; j < virtual_machine.stack_size; j += 1){
                const Var var = virtual_machine.stack[j];
                printf(
                    "%lu-- i: %li, u: %lu, f: %f, ptr: %p\n",
                    j, var.as_int64, var.as_uint64, var.as_float64, var.as_ptr
                );
            }
            break;
        }
    }


    return 0;
}

