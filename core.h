#ifndef VIRTUAL_DROID_CORE
#define VIRTUAL_DROID_CORE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


enum Errors{
ERROR_NONE = 0,
ERROR_INVALID_FILE_PATH,
ERROR_INVALID_USAGE,
ERROR_UNKOWN_INSTRUCTION,
ERROR_INVALID_OPERAND,
ERROR_INVALID_OPERAND_COUNT,
ERROR_INVALID_SYNTAX,
ERROR_UNRESOLVED_SYMBOL,

ERROR_PROGRAM_SIZE_OVERFLOW,

// for internal errors
INTERNAL_ERROR_INTERNAL,

// for internal counting purposes
INTERNAL_ERROR_COUNT

};


typedef struct String{
	char* c_str;
	size_t size;
} String;

typedef struct Stream{
    unsigned char* data;
    size_t         size;
    size_t         capacity;
} Stream;


typedef struct S_file_t{
	String path;
	String mother_dir;
	String contents;
} S_file_t;

void resize_stream(Stream* stream, size_t new_capacity){

    unsigned char* old_data = stream->data;
    stream->data = malloc(new_capacity);

    const size_t range = (stream->size < new_capacity)? stream->size : new_capacity;

    for(size_t i = 0; i < range; i+=1){
        stream->data[i] = old_data[i];
    }

    stream->capacity = new_capacity;
}

void stream_to_stream(Stream* stream, unsigned char* data, size_t size){
    for(size_t i = 0; i < size; i+=1){
        if(stream->size >= stream->capacity) resize_stream(stream, stream->capacity * 2);
        stream->data[stream->size++] = data[i];
    }
}

typedef enum Instructions{
INSTRUCTION_NONE = 0,

INSTRUCTION_HALT,
INSTRUCTION_DUMP_STACK,
// pushes the current stack position (stack size - 1) to the top of the stack as_uint64
INSTRUCTION_GSP,
// pushes the next instruction address to the top of the stack as_uint64. This basically acts as a
// convinient way to make and access labels. 
INSTRUCTION_IP,

// pushes element to the end of the stack
INSTRUCTION_PUSH,
// removes the last element from the stack
INSTRUCTION_POP,
// clears the whole stack
INSTRUCTION_CLEAN,
// duplicates the element on top of the stack
INSTRUCTION_DUP,
// reads the element at the stack position given by the element on top of the stack, replacing
// the element on top of the stack by a copy of the read element
INSTRUCTION_READ,
// takes the stack positions given by the 2 elements on top of the stack and sets the element at the first position
// to the element at the second position. This instruction consumes the 2 elements on top of the stack
INSTRUCTION_SET,

INSTRUCTION_NOT,
INSTRUCTION_EQUAL,
INSTRUCTION_JMP,
INSTRUCTION_JMP_IF,
INSTRUCTION_JMP_IFNOT,

INSTRUCTION_PLUSI,
INSTRUCTION_MINUSI,
INSTRUCTION_MULI,
INSTRUCTION_DIVI,
INSTRUCTION_SMALLERI,
INSTRUCTION_BIGGERI,

INSTRUCTION_PLUSU,
INSTRUCTION_MINUSU,
INSTRUCTION_MULU,
INSTRUCTION_DIVU,
INSTRUCTION_SMALLERU,
INSTRUCTION_BIGGERU,

INSTRUCTION_PLUSF,
INSTRUCTION_MINUSF,
INSTRUCTION_MULF,
INSTRUCTION_DIVF,
INSTRUCTION_SMALLERF,
INSTRUCTION_BIGGERF,

DEBUG_INSTRUCTION_BREAK,

INTERNAL_INSTRUCTION_INCLUDE,
INTERNAL_INSTRUCTION_DEFINE,

// for internal counting purposes
INTERNAL_INSTRUCTION_COUNT
} Inst;

typedef enum Vm_flags{
    VM_FLAG_NONE = 0,
    VM_FLAG_STD = 1 << 0,
    VM_FLAG_DEBUG = 1 << 1,
} Vm_flags;


typedef union Var{
int64_t        as_int64;
uint64_t       as_uint64;
double         as_float64;
void*          as_ptr;
Inst           as_inst;
} Var;

typedef struct Expression{
    int num_of_operands;
	Inst instruction;
    Var operand;
    Var operand_aux;
} Exp;

typedef struct Label{
    String str;
    String def;
} Label;

typedef struct InternalInst
{
    Inst inst;
    String str;
} InternalInst;

typedef union Atom
{
    Var as_operand;
    Inst as_inst;
} Atom;


typedef Stream Program;

#define MKSTR(INPUT) (String){.c_str = INPUT, .size = (sizeof(INPUT) - 1) / sizeof(char)}

#define COMMENT_SYM ';'
#define SIZEOF_CHUNK sizeof(unsigned char)
#define PROGRAM_CAP (1024)
#define MAX_COMP_STC_STRM_MEMCAP 10240
#define LABEL_CAP 1000


static inline int get_int(char c){
	switch (c)
	{
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	
	default:
		return -1;
	}
}

static inline int compare_str(String str1, String str2){
	if(str1.size != str2.size){
		return 0;
	}

	for(size_t i = 0; i < str1.size; i+=1){
		if(str1.c_str[i] != str2.c_str[i]) return 0;
	}
	return 1;

}

void throw_error(int error, const char* aux){
    switch (error)
    {
    case ERROR_INVALID_FILE_PATH:
        printf("[ERROR] Invalid File Path: '%s'\n", aux);
        break;
    case ERROR_INVALID_USAGE:
        printf("[ERROR] Invalid Usage\n%s\n", aux);
        break;
    case ERROR_UNKOWN_INSTRUCTION:
        printf("[ERROR] Unknown Instruction: '%s'\n", aux);
        break;
    case ERROR_INVALID_OPERAND:
        printf("[ERROR] Invalid Operand: '%s'\n", aux);
        break;
    case ERROR_INVALID_OPERAND_COUNT:
        printf("[ERROR] Invalid Operand Count\n%s\n", aux);
        break;
    case ERROR_INVALID_SYNTAX:
        printf("[ERROR] Invalid Syntax: '%s'\n", aux);
        break;
    case ERROR_UNRESOLVED_SYMBOL:
        printf("[ERROR] Unresolved Symbol '%s'\n", aux);
        break;
    
    case ERROR_PROGRAM_SIZE_OVERFLOW:
        printf("[ERROR] Program Size Overflow, Maximum Program Size Is %zu\n", (size_t)PROGRAM_CAP);
        break;
    
    case INTERNAL_ERROR_INTERNAL:
        printf("[INTERNAL ERROR] %s\n", aux);
        break;
    
    default:
        printf("[INTERNAL ERROR] Attempt To Throw Invalid Error %i\n", error);
        exit(INTERNAL_ERROR_INTERNAL);
        break;
    }
    exit(error);
}

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


#endif //END OF FILE ================
