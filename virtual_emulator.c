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
} vm;


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

    vm.internal_memory = (unsigned char*)malloc(stream.size + STACK_CAP);
    vm.internal_memory_size = stream.size;
    
    vm.stack = (Var*)(vm.internal_memory + stream.size);
    vm.stack_size = 0;

    for(unsigned long i = 0; i < vm.internal_memory_size; i += 1){
        vm.internal_memory[i] = stream.data[i];
    }

    free(stream.data);
}

// returns the number of bytes taken by the instruction, so to be convinient to jump to next instruction
unsigned long eval_inst(unsigned long inst_address){
    const Inst inst = *(Inst*)(vm.internal_memory + inst_address);
    switch (inst)
    {
    case INSTRUCTION_HALT:
        exit(0);
        return sizeof(Inst);
    case INSTRUCTION_DUMP_STACK:
        for(unsigned long i = 0; i < vm.stack_size; i+=1){
            const Var var = vm.stack[i];
            printf(
                "%lu-- i: %li, u: %lu, f: %f, ptr: %p\n",
                i, var.as_int64, var.as_uint64, var.as_float64, var.as_ptr
            );
        }
        return sizeof(Inst);
    case INSTRUCTION_GSP:
        vm.stack[vm.stack_size - 1].as_uint64 = (vm.stack_size++) - 1;
        return sizeof(Inst);

    case INSTRUCTION_PUSH:
        vm.stack[vm.stack_size++] =
        *(Var*)(vm.internal_memory + inst_address + sizeof(Inst));
        return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_POP:
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_CLEAN:
        vm.stack_size = 0;
        return sizeof(Inst);
    case INSTRUCTION_DUP:
        vm.stack[vm.stack_size] = vm.stack[vm.stack_size - 1];
        vm.stack_size += 1;
        return sizeof(Inst);
    case INSTRUCTION_READ:{
        const unsigned long i = vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack[vm.stack_size - 1] = vm.stack[i];
    } return sizeof(Inst);
    case INSTRUCTION_SET:
        vm.stack[vm.stack[vm.stack_size - 2].as_uint64] =
        vm.stack[vm.stack[vm.stack_size - 1].as_uint64];
        vm.stack_size -= 2;
        return sizeof(Inst);

    case INSTRUCTION_PLUSI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 1].as_int64 + vm.stack[vm.stack_size - 2].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MINUSI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 2].as_int64 - vm.stack[vm.stack_size - 1].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MULI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 2].as_int64 * vm.stack[vm.stack_size - 1].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_DIVI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 2].as_int64 / vm.stack[vm.stack_size - 1].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);

    case INSTRUCTION_PLUSU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 1].as_uint64 + vm.stack[vm.stack_size - 2].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MINUSU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 - vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MULU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 * vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_DIVU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 / vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);

    case INSTRUCTION_PLUSF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 1].as_float64 + vm.stack[vm.stack_size - 2].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MINUSF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 2].as_float64 - vm.stack[vm.stack_size - 1].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MULF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 2].as_float64 * vm.stack[vm.stack_size - 1].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_DIVF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 2].as_float64 / vm.stack[vm.stack_size - 1].as_float64;
        vm.stack_size -= 1;
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
        i < vm.internal_memory_size;
        i += eval_inst(i)
    );


    return 0;
}

