#include <stdio.h>
#include <stdlib.h>
#include "core.h"

void show_usage(){

printf(
"[HELP] Usage: ./vc <path_to_executable>\n"
);

}

typedef struct String{
	char* c_str;
	unsigned long size;
} String;

typedef struct Expression{
	unsigned long index;
	Inst instruction;
} Exp;


#define MKSTR(INPUT) (String){.c_str = INPUT, .size = (sizeof(INPUT) - 1) / sizeof(char)



static inline int compare_str(String str1, String str2){
if(str1.size != str2.size){
return 0;
}

for(unsigned long i = 0; i < str1.size; i+=1){
if(str1.c_str[i] != str2.c_str[i]) return 0;
}
return 1;

}

static inline Exp parse_expression(String str_exp, unsigned long index){
if(compare_str(str_exp, MKSTR("halt"))) return (Exp){.index = index, .Instruction = (Inst){.id = INSTUCTION_HALT}};
}


int main(int argc, char** argv){

if(argc < 2){
show_usage();
printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
}




return 0;
}
