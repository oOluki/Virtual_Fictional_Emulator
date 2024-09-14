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

INSTRUCTION_PUSH,
INSTRUCTION_POP,
INSTRUCTION_MOVE,
INSTRUCTION_COPY,
INSTRUCTION_WRITE,

// for internal counting purposes
INTERNAL_INSTRUCTION_COUNT
} Instruction_Id;


typedef union Var{
long           as_int64;
unsigned long  as_uint64;
double         as_float64;
void*          as_ptr;
Instruction_Id as_inst_id;
} Var;


typedef struct Inst{
Var id;
Var operand1;
Var operand2;
} Inst;

typedef struct Expression{
	unsigned long index;
	Inst instruction;
} Exp;

typedef Stream Program;

#define SIZEOF_CHUNK sizeof(Var)


#endif //END OF FILE ================
