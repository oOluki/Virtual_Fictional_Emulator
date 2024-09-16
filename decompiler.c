#include <stdio.h>
#include <stdlib.h>
#include "core.h"

// returns the size taken by the instruction for convenience
size_t print_inst(const Program program, size_t inst_address){
    const Inst inst = *(Inst*)(program.data + inst_address);
    switch (inst)
    {
    case INSTRUCTION_HALT:
        printf("halt;\n");
        return sizeof(Inst);
    case INSTRUCTION_DUMP_STACK:
        printf("dump;\n");
        sizeof(Inst);
        return sizeof(Inst);
    case INSTRUCTION_GSP:
        printf("gsp;\n");
        return sizeof(Inst);
    case INSTRUCTION_IP:
        printf("ip;\n");
        return sizeof(Inst);

    case INSTRUCTION_PUSH:{
        const Var operand = *(Var*)(program.data + inst_address + sizeof(Inst));
        printf("push (f: %f; int64: %" PRId64 "; uint64: %" PRIu64 "; ptr: %p);\n",
        operand.as_float64, operand.as_int64, operand.as_uint64, operand.as_ptr);
    } return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_POP:
        printf("pop;\n");
        return sizeof(Inst);
    case INSTRUCTION_CLEAN:
        printf("clean;\n");
        return sizeof(Inst);
    case INSTRUCTION_DUP:
        printf("dup;\n");
        return sizeof(Inst);
    case INSTRUCTION_READ:
        printf("read;\n");
        return sizeof(Inst);
    case INSTRUCTION_SET:
        printf("set;\n");
        return sizeof(Inst);
    
    case INSTRUCTION_NOT:
        printf("not;\n");
        return sizeof(Inst);
    case INSTRUCTION_EQUAL:
        printf("eq;\n");
        return sizeof(Inst);
    case INSTRUCTION_JMP:
        printf("jmp;\n");
        return sizeof(Inst);
    case INSTRUCTION_JMP_IF:
        printf("jmpf;\n");
        return sizeof(Inst);
    case INSTRUCTION_JMP_IFNOT:
        printf("jmpnf;\n");
        return sizeof(Inst);

    
    case INSTRUCTION_PLUSI:
        printf("plusi;\n");
        return sizeof(Inst);
    case INSTRUCTION_MINUSI:
        printf("minusi;\n");
        return sizeof(Inst);
    case INSTRUCTION_MULI:
        printf("muli;\n");
        return sizeof(Inst);
    case INSTRUCTION_DIVI:
        printf("divi;\n");
        return sizeof(Inst);
    case INSTRUCTION_SMALLERI:
        printf("smalleri;\n");
        return sizeof(Inst);
    case INSTRUCTION_BIGGERI:
        printf("biggeri;\n");
        return sizeof(Inst);
    
    case INSTRUCTION_PLUSU:
        printf("plusu;\n");
        return sizeof(Inst);
    case INSTRUCTION_MINUSU:
        printf("minusu;\n");
        return sizeof(Inst);
    case INSTRUCTION_MULU:
        printf("mulu;\n");
        return sizeof(Inst);
    case INSTRUCTION_DIVU:
        printf("divu;\n");
        return sizeof(Inst);
    case INSTRUCTION_SMALLERU:
        printf("smalleru;\n");
        return sizeof(Inst);
    case INSTRUCTION_BIGGERU:
        printf("biggeru;\n");
        return sizeof(Inst);

    case INSTRUCTION_PLUSF:
        printf("plusf;\n");
        return sizeof(Inst);
    case INSTRUCTION_MINUSF:
        printf("minusf;\n");
        return sizeof(Inst);
    case INSTRUCTION_MULF:
        printf("mulf;\n");
        return sizeof(Inst);
    case INSTRUCTION_DIVF:
        printf("divf;\n");
        return sizeof(Inst);
    case INSTRUCTION_SMALLERF:
        printf("smallerf;\n");
        return sizeof(Inst);
    case INSTRUCTION_BIGGERF:
        printf("biggerf;\n");
        return sizeof(Inst);

    default:
        printf("[ERROR] Error At %zu Intruction, Unkown Instruction %u\n", inst_address, inst);
        exit(ERROR_UNKOWN_INSTRUCTION);
        return sizeof(Inst);
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
