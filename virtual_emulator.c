#include <stdlib.h>
#include <stdio.h>
#include "core.h"

#define STACK_CAP 1024

enum States{
    STATE_NONE = 0,

    STATE_ACTIVE
};


struct VirtualMachine{

unsigned char* internal_memory;
unsigned long internal_memory_size;

Var* stack;
unsigned long stack_size;
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

    virtual_machine.internal_memory = (unsigned char*)malloc(stream.size + STACK_CAP);
    virtual_machine.internal_memory_size = stream.size;
    
    virtual_machine.stack = (Var*)(virtual_machine.internal_memory + stream.size);
    virtual_machine.stack_size = 0;

    for(unsigned long i = 0; i < virtual_machine.internal_memory_size; i += 1){
        virtual_machine.internal_memory[i] = stream.data[i];
    }

    free(stream.data);
}

// returns the number of bytes taken by the instruction, so to be convinient to jump to next inrtuction
unsigned long eval_inst(unsigned long inst_address){
    const Inst inst = *(Inst*)(virtual_machine.internal_memory + inst_address);
    switch (inst)
    {
    case INSTRUCTION_HALT:
        exit(0);
        return sizeof(Inst);
    case INSTRUCTION_DUMP_STACK:
        for(unsigned long i = 0; i < virtual_machine.stack_size; i+=1){
            const Var var = virtual_machine.stack[i];
            printf(
                "%lu-- i: %li, u: %lu, f: %f, ptr: %p\n",
                i, var.as_int64, var.as_uint64, var.as_float64, var.as_ptr
            );
        }
        return sizeof(Inst);

    case INSTRUCTION_PUSH:
        virtual_machine.stack[virtual_machine.stack_size++] =
        *(Var*)(virtual_machine.internal_memory + inst_address + sizeof(Inst));
        return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_POP:
        virtual_machine.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_CLEAN:
        virtual_machine.stack_size = 0;
        return sizeof(Inst);
    case INSTRUCTION_DUP:
        virtual_machine.stack[virtual_machine.stack_size] = virtual_machine.stack[virtual_machine.stack_size - 1];
        virtual_machine.stack_size += 1;
        return sizeof(Inst);
    case INSTRUCTION_READ:{
        const unsigned long i = virtual_machine.stack[virtual_machine.stack_size - 1].as_uint64;
        virtual_machine.stack[virtual_machine.stack_size - 1] = virtual_machine.stack[i];
    } return sizeof(Inst);
    case INSTRUCTION_SET:
        virtual_machine.stack[virtual_machine.stack[virtual_machine.stack_size - 2].as_uint64] =
        virtual_machine.stack[virtual_machine.stack[virtual_machine.stack_size - 1].as_uint64];
        virtual_machine.stack_size -= 2;
        return sizeof(Inst);
    
    default:
        printf("[ERROR] Unkown Instruction %u\n", (unsigned int)inst);
        exit(ERROR_UNKOWN_INSTRUCTION);
        return sizeof(Inst);
    }
}




int main(int argc, char** argv){

	if(argc < 2){
		printf("[HELP] Usage: ./vimulator <path_to_executable>\n");
		printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
		return ERROR_INVALID_USAGE;
	}

	load_program(argv[1]);

    for(
        unsigned long i = 0;
        i < virtual_machine.internal_memory_size;
        i += eval_inst(i)
    );


    return 0;
}

