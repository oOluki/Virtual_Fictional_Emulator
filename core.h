#ifndef VIRTUAL_DROID_CORE
#define VIRTUAL_DROID_CORE

#include <stdio.h>
#include <stdlib.h>


enum Errors{
ERROR_NONE = 0,
ERROR_INVALID_FILE_PATH,
ERROR_INVALID_USAGE,
ERROR_UNKOWN_INSTRUCTION,
ERROR_INVALID_OPERAND,
ERROR_INVALID_OPERAND_COUNT,
ERROR_INVALID_SYNTAX,

ERROR_PROGRAM_SIZE_OVERFLOW,

// for internal errors
INTERNAL_ERROR_INTERNAL,

// for internal counting purposes
INTERNAL_ERROR_COUNT

};

typedef struct Stream{
    unsigned char* data;
    unsigned long size;
    unsigned long capacity;
} Stream;

void resize_stream(Stream* stream, unsigned long new_capacity){

    unsigned char* old_data = stream->data;
    stream->data = malloc(new_capacity);

    const unsigned long range = (stream->size < new_capacity)? stream->size : new_capacity;

    for(unsigned long i = 0; i < range; i+=1){
        stream->data[i] = old_data[i];
    }

    stream->capacity = new_capacity;
}

void stream_to_stream(Stream* stream, unsigned char* data, unsigned long size){
    for(unsigned long i = 0; i < size; i+=1){
        if(stream->size >= stream->capacity) resize_stream(stream, stream->capacity * 2);
        stream->data[stream->size++] = data[i];
    }
}

typedef enum Instructions{
INSTRUCTION_NONE = 0,

INSTRUCTION_HALT,
INSTRUCTION_DUMP_STACK,

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

// TODO: Implement arithmetic

INSTRUCTION_PLUSI,
INSTUCTION_MINUSI,
INSTRUCTION_MULI,
INSTRUCTION_DIVI,

INSTRUCTION_PLUSU,
INSTUCTION_MINUSU,
INSTRUCTION_MULU,
INSTRUCTION_DIVU,

INSTRUCTION_PLUSF,
INSTUCTION_MINUSF,
INSTRUCTION_MULF,
INSTRUCTION_DIVF,

INSTRUCTION_PLUSP,
INSTUCTION_MINUSP,
INSTRUCTION_MULP,
INSTRUCTION_DIVP,

// for internal counting purposes
INTERNAL_INSTRUCTION_COUNT
} Inst;


typedef union Var{
long           as_int64;
unsigned long  as_uint64;
double         as_float64;
void*          as_ptr;
Inst           as_inst;
} Var;

typedef struct Expression{
    int num_of_operands;
	Inst instruction;
    Var operand;
} Exp;

typedef Stream Program;

#define SIZEOF_CHUNK sizeof(unsigned char)


#endif //END OF FILE ================
