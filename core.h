#ifndef VIRTUAL_DROID_CORE
#define VIRTUAL_DROID_CORE

typedef union Var{
long   as_int64;
double as_float64;
void*  as_ptr;
} Var;

enum Instructions{
INSTRUCTION_NONE = 0,
INSTRUCTION_HALT,
INSTRUCTION_WRITE,
INSTRUCTION_READ,
INSTRUCTION_JUMP,

INTERNAL_INSTRUCTION_COUNT
};


typedef struct Inst{
int id;
Var operand1;
Var operand2;
} Inst;




#endif //END OF FILE ================
