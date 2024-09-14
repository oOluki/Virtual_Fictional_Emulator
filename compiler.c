#include <stdio.h>
#include <stdlib.h>
#include "core.h"

typedef struct String{
	char* c_str;
	unsigned long size;
} String;


#define MKSTR(INPUT) (String){.c_str = INPUT, .size = (sizeof(INPUT) - 1) / sizeof(char)}
#define INGORED_CHAR(CHAR) (CHAR == ' ' || CHAR == '\n' || CHAR == '\t')


String read_file(const char* path){
	FILE* file = fopen(path, "r");

	if(!file){
		printf("[ERROR] Invalid File Path '%s'\n", path);
		exit(ERROR_INVALID_FILE_PATH);
	}

	Stream stream = (Stream){.data = (unsigned char*)malloc(1024), .size = 0, .capacity = 1024 / sizeof(char)};


	for(unsigned long i = 0; (!feof(file)) && (i < 10000); i+=1){
		stream.size += sizeof(char) * fread(stream.data + stream.size, sizeof(char), stream.capacity, file);
		if(stream.size >= stream.capacity - 1){
			resize_stream(&stream, stream.capacity * 2);
		}
	}

	fclose(file);


	return (String){.c_str = (char*)stream.data, .size = stream.size};
}

static inline long find_char(String str, char c, unsigned long off_set){
	for(unsigned long i = off_set; i < str.size; i+=1){
		if(str.c_str[i] == c) return i - off_set;
	}

	return -1;
}

static inline long find_next_significant_char(String str, unsigned long offset){
	for(unsigned long i = offset; i < str.size; i+=1){
		if(!INGORED_CHAR(str.c_str[i])) return i - offset;
	}
	return -1;
}

static inline long find_next_insignificant_char(String str, unsigned long offset){
	for(unsigned long i = offset; i < str.size; i+=1){
		if(INGORED_CHAR(str.c_str[i])) return i - offset;
	}
	return -1;
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

static inline void print_str(const String str){
	for(unsigned long i = 0; i < str.size; i+=1){
		printf("%c", str.c_str[i]);
	}
}

unsigned long ul_pow(unsigned long base, unsigned long exponent){
	unsigned long output = 1;
	for(unsigned long i = 0; i < exponent; i+=1){
		output *= base;
	}
	return output;
}

static inline int get_int(char c){
	if(c > '9' || c < '0'){
		printf("[ERROR] Invalid Digit '%c'\n", c);
		exit(ERROR_INVALID_SYNTAX);
	}
	return c - '0';
}

static inline Var get_operand(String str){
	unsigned long aux = 0;

	unsigned long exp_factor = 0;
	int is_float = 0;
	int is_negative = str.c_str[0] == '-';

	for(unsigned long i = is_negative; i < str.size; i+=1){
		if(str.c_str[i] == 'f'){
			if(i != str.size - 1){
				printf("[ERROR] Invalid Syntex: "); print_str(str); printf("\n");
				exit(ERROR_INVALID_SYNTAX);
			}
			is_float = 1;
			continue;
		}
		if(str.c_str[i] == '.'){
			if(exp_factor){
				printf("[ERROR] Invalid Operand "); print_str(str); printf("\n");
				exit(ERROR_INVALID_OPERAND);
			}
			is_float = 1;
			exp_factor = str.size - i;
			continue;
		}
		aux += (unsigned long)(get_int(str.c_str[i])) * ul_pow(10, str.size - i - 1);
	}
	Var output;
	if(is_float == 1) output.as_float64 = aux / (double)(ul_pow(10, exp_factor));
	else if(is_negative == 1) output.as_int64 = -aux;
	else output.as_uint64 = aux;

	return output;
}

static inline Var parse_operand(String str){
	String operand_str;
	unsigned long i = 0;
	for(; i < str.size && INGORED_CHAR(str.c_str[i]); i+=1);
	const long eo_op = find_next_insignificant_char(str, i);
	operand_str.c_str = str.c_str + i;
	operand_str.size = (eo_op < 0)? str.size - i : eo_op;
	if(operand_str.c_str[operand_str.size - 1] == ';') operand_str.size -= 1;
	return get_operand(operand_str);
}

static inline Exp parse_expression(String str_exp, unsigned long index){

	String inst_str;

	{
		unsigned long i = 0;

		for(; i < str_exp.size && INGORED_CHAR(str_exp.c_str[i]); i+=1);

		const long eo_inst = find_next_insignificant_char(str_exp, i);
		inst_str.c_str = str_exp.c_str + i;
		inst_str.size = (eo_inst < 0)? str_exp.size - i : eo_inst;
	}

	if(compare_str(inst_str, MKSTR("halt")))
		return (Exp){.index = index, .instruction = (Inst){.id = INSTRUCTION_HALT}};
	if(compare_str(inst_str, MKSTR("dump")))
		return (Exp){.index = index, .instruction = (Inst){.id = INSTRUCTION_DUMP_STACK}};
	if(compare_str(inst_str, MKSTR("push"))){
		const String operand_str = (String){
			.c_str = inst_str.c_str + inst_str.size,
			.size = str_exp.size - inst_str.size
		};
		return (Exp){
			.index = index,
			.instruction = (Inst){
				.id = INSTRUCTION_PUSH,
				.operand1 = parse_operand(operand_str)
			}
		};
	}
	if(compare_str(inst_str, MKSTR("pop"))){
		return (Exp){
			.index = index,
			.instruction = (Inst){
				.id = INSTRUCTION_POP,
			}
		};
	}

	printf("[ERROR] %lu- Unknown Instruction '", index); print_str(inst_str); printf("'\n");
	exit(ERROR_UNKOWN_INSTRUCTION);
	
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
