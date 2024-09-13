#include <stdio.h>
#include <stdlib.h>
#include "core.h"

typedef struct String{
	char* c_str;
	unsigned long size;
} String;


#define MKSTR(INPUT) (String){.c_str = INPUT, .size = (sizeof(INPUT) - 1) / sizeof(char)}


String read_file(const char* path){
	FILE* file = fopen(path, "r");

	if(!file){
		printf("[ERROR] Invalid File Path '%s'\n", path);
		exit(ERROR_INVALID_FILE_PATH);
	}

	Stream stream = (Stream){.data = (unsigned char*)malloc(1024), .size = 0, .capacity = 1024 / sizeof(char)};


	for(unsigned long i = 0; (!feof(file)) && (i < 10000); i+=1){
		stream.size += fread(stream.data, sizeof(char), stream.capacity, file);
		if(stream.size >= stream.capacity - 1){
			resize_stream(&stream, stream.capacity * 2);
		}
	}

	fclose(file);

	return (String){.c_str = (char*)stream.data, .size = stream.size};
}


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
	if(compare_str(str_exp, MKSTR("halt")))
		return (Exp){.index = index, .instruction = (Inst){.id = INSTRUCTION_HALT}};
	else{
		printf("[ERROR] %lu- Unknown Instruction '%s'\n", index, str_exp.c_str);
		exit(ERROR_UNKOWN_INSTRUCTION);
	}
}

Program parse(String file){

	unsigned long last_expr_index = 0;

	Program program = (Stream){.data = malloc(1024 * sizeof(Exp)), .size = 0, .capacity = 1024};

	for(unsigned long i = 0; i < file.size; i += 1){
		if(file.c_str[i] == ';'){
			const unsigned long expr_str_size = i - last_expr_index;
			Exp expr = parse_expression((String){.c_str = file.c_str + last_expr_index, .size = expr_str_size}, i);
			((Exp*)program.data)[program.size++] = expr;
			if(program.size >= program.capacity){
				printf("[ERROR] Program Size Overflow, Maximum Program Size Is %lu\n", (unsigned long)1024);
				exit(ERROR_PROGRAM_SIZE_OVERFLOW);
			}

			last_expr_index = i + 1;
		}
	}

	free(file.c_str);
	return program;
}

void write_program(const Program program, const char* output_path){
	FILE* file = fopen(output_path, "wb");

	if(!file){
		printf("[ERROR] Invalid File Path '%s'\n", output_path);
		exit(ERROR_INVALID_FILE_PATH);
	}

	for(unsigned long i = 0; i < program.size; i+=1){
		const Exp expression = ((Exp*)program.data)[i];
		fwrite(&expression.instruction, SIZEOF_CHUNK, 3, file);
	}


	fclose(file);
}


int main(int argc, char** argv){

	if(argc < 2){
		printf("[HELP] Usage: ./fictiler <path_to_executable>\n");
		printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
		return ERROR_INVALID_USAGE;
	}

	String file = read_file(argv[1]);

	Program program = parse(file);

	write_program(program, "out.virtual");

	return 0;
}
