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
