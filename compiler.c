#include <stdio.h>
#include <stdlib.h>
#include "core.h"


static inline void print_str(const String str){
	for(size_t i = 0; i < str.size; i+=1){
		printf("%c", str.c_str[i]);
	}
}


Stream stream;
Label labels[LABEL_CAP];
size_t label_count;

#define MKSTR(INPUT) (String){.c_str = INPUT, .size = (sizeof(INPUT) - 1) / sizeof(char)}
#define INGORED_CHAR(CHAR) (CHAR == ' ' || CHAR == '\n' || CHAR == '\t')

static inline int64_t find_char(String str, char c, size_t off_set){
	for(size_t i = off_set; i < str.size; i+=1){
		if(str.c_str[i] == c) return (int64_t)(i - off_set);
	}

	return -1;
}

static inline int64_t find_next_significant_char(String str, size_t offset){
	for(size_t i = offset; i < str.size; i+=1){
		if(str.c_str[i] == COMMENT_SYM){
			const int64_t nxt = find_char(str, '\n', i);
			if(nxt < 0) return -1;
			i += nxt;
			continue;
		}
		if(!INGORED_CHAR(str.c_str[i])) return (int64_t)(i - offset);
	}
	return -1;
}

static inline int64_t find_next_insignificant_char(String str, size_t offset){
	for(size_t i = offset; i < str.size; i+=1){
		if(str.c_str[i] == COMMENT_SYM) return (int64_t)(i - offset);
		if(INGORED_CHAR(str.c_str[i])) return (int64_t)(i - offset);
	}
	return -1;
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

static inline void add_label(Label label){
	for(size_t i = 0; i < label_count; i += 1){
		if(compare_str(labels[i].str, label.str)){
			const char c = label.str.c_str[label.str.size];
			label.str.c_str[label.str.size]= '\0';
			printf("[WANRING] Redefinition Of '%s' Label\n", label.str.c_str);
			label.str.c_str[label.str.size] = c;
			labels[i].def = label.def;
			return ;
		}
	}
	if(label_count >= LABEL_CAP) throw_error(INTERNAL_ERROR_INTERNAL, "Label Capacity Overflow\n");

	labels[label_count++] = label;
}

static inline String resolve_label(String str, int required){
	for(size_t i = 0; i < label_count; i += 1){
		if(compare_str(str, labels[i].str)){
			return labels[i].def;
		}
	}
	if(required){
		str.c_str[str.size] = '\0';
		throw_error(ERROR_UNRESOLVED_SYMBOL, str.c_str);
	}
	return (String){.c_str = NULL, .size = 0};
}

static inline String get_next_token(String string, size_t pos){

	int64_t begin = find_next_significant_char(string, pos);
	if(begin < 0){
		return (String){.c_str = NULL, .size = 0};
	}
	int64_t size = find_next_insignificant_char(string, pos + begin);
	if(size < 0) size = string.size - begin - pos;

	return (String){.c_str = string.c_str + begin + pos, .size = size};
}

S_file_t read_file(const String mother_dir, const char* path){

	S_file_t output;

	for(output.path.size = 0; path[output.path.size]; output.path.size+=1){
		if(path[output.path.size] == '/') output.mother_dir.size = output.path.size + 1;
	}

	output.path.size += mother_dir.size;

	if(output.path.size * sizeof(char) + stream.size + 1 > stream.capacity){
		throw_error(INTERNAL_ERROR_INTERNAL, "Maximum Stream Capacity Exceeded While Reading File\n");
	}

	output.path.c_str = (char*)(stream.data + stream.size);

	for(size_t i = 0; i < mother_dir.size; i+=1){
		output.path.c_str[i] = mother_dir.c_str[i];
	}

	for(size_t i = 0; i < output.path.size - mother_dir.size; i+=1){
		output.path.c_str[i + mother_dir.size] = path[i];
	}

	output.path.c_str[output.path.size] = '\0';

	output.mother_dir.c_str = output.path.c_str;

	FILE* file = fopen(output.path.c_str, "r");

	if(!file){
		throw_error(ERROR_INVALID_FILE_PATH, output.path.c_str);
	}

	stream.size += (output.path.size + 1) * sizeof(char);

	const size_t contents_begin = stream.size;

	for(size_t i = 0; (!feof(file)) && (i < 10000); i += 1){
		stream.size += sizeof(char) * fread(stream.data + stream.size, sizeof(char), stream.capacity, file);
		if(stream.size >= stream.capacity - 1){
			throw_error(INTERNAL_ERROR_INTERNAL, "Maximum Stream Capacity Exceeded While Reading File\n");
		}
	}

	output.contents.c_str = (char*)(stream.data + contents_begin);

	output.contents.size = stream.size - contents_begin;

	stream.data[stream.size++] = '\0';

	fclose(file);


	return output;
}

uint64_t u64_pow(uint64_t base, uint64_t exponent){
	uint64_t output = 1;
	for(uint64_t i = 0; i < exponent; i+=1){
		output *= base;
	}
	return output;
}

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

static inline Var get_operand(String str){
	uint64_t aux = 0;

	uint64_t exp_factor = 0;
	int is_float = 0;
	int is_negative = str.c_str[0] == '-';

	for(size_t i = is_negative; i < str.size; i+=1){
		if(str.c_str[i] == 'f'){
			if(i != str.size - 1){
				str.c_str[str.size] = '\0';
				throw_error(ERROR_INVALID_OPERAND, str.c_str);
			}
			is_float = 1;
			continue;
		}
		if(str.c_str[i] == '.'){
			if(exp_factor){
				str.c_str[str.size] = '\0';
				throw_error(ERROR_INVALID_OPERAND, str.c_str);
			}
			is_float = 1;
			exp_factor = str.size - i;
			continue;
		}
		const int i_n_t_ = get_int(str.c_str[i]);
		if(i_n_t_ < 0){
			str.c_str[str.size] = '\0';
			throw_error(ERROR_INVALID_SYNTAX, str.c_str);
		}
		aux += (uint64_t)(i_n_t_) * u64_pow(10, (uint64_t)(str.size - i - 1));
	}
	Var output;
	if(is_float == 1) output.as_float64 = (double)(aux / (double)(u64_pow(10, exp_factor)));
	else if(is_negative == 1) output.as_int64 = -(int64_t)(aux);
	else output.as_uint64 = aux;

	return output;
}

static inline InternalInst parse_macro(String str_exp){

	const String inst_str = get_next_token(str_exp, 0);

	if(inst_str.c_str == NULL) return (InternalInst){.inst = 0, .str = NULL};

	if(compare_str(inst_str, MKSTR("#include"))){
		const size_t offset = inst_str.size + (inst_str.c_str - str_exp.c_str);
		const int64_t operand_begin = find_char(str_exp, '"', offset);
		const int64_t operand_size = find_char(str_exp, '"', 1 + operand_begin + offset);
		if(operand_begin < 0 || operand_size < 0){
			str_exp.c_str[str_exp.size];
			throw_error(ERROR_INVALID_SYNTAX, str_exp.c_str);
		}
		const String path__ = (String){.c_str = str_exp.c_str + operand_begin + offset + 1, .size = operand_size};
		return (InternalInst){.inst = INTERNAL_INSTRUCTION_INCLUDE, .str = path__};
	}
	if(compare_str(inst_str, MKSTR("#define"))){
		const String tkn1 = get_next_token(str_exp, inst_str.size + (inst_str.c_str - str_exp.c_str));
		if(tkn1.c_str == NULL){
			str_exp.c_str[str_exp.size] = '\0';
			throw_error(ERROR_INVALID_SYNTAX, str_exp.c_str);
		}
		const String tkn2 = get_next_token(str_exp, tkn1.size + (tkn1.c_str - str_exp.c_str));
		add_label((Label){.str = tkn1, .def = tkn2});
		return (InternalInst){.inst = 0, .str = NULL};
	}
	inst_str.c_str[inst_str.size] = '\0';
	throw_error(ERROR_UNKOWN_INSTRUCTION, inst_str.c_str);
	return (InternalInst){.inst = 0, .str = NULL};
}

static inline Exp resolve_inst(String str_exp, int required){

	String inst_str = get_next_token(str_exp, 0);

	if(inst_str.c_str == NULL){
		if(required){
			str_exp.c_str[str_exp.size] = '\0';
			throw_error(ERROR_UNKOWN_INSTRUCTION, str_exp.c_str);
		}
		return (Exp){.num_of_operands = 0, .instruction = 0, .operand.as_ptr = NULL, .operand_aux.as_ptr = NULL};
	}

	if(compare_str(inst_str, MKSTR("halt")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_HALT};
	if(compare_str(inst_str, MKSTR("dump")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_DUMP_STACK};
	if(compare_str(inst_str, MKSTR("gsp")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_GSP};
	if(compare_str(inst_str, MKSTR("ip")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_IP};

	if(compare_str(inst_str, MKSTR("push"))){
		const String operand_str = get_next_token(str_exp, inst_str.size + (inst_str.c_str - str_exp.c_str));
		if(operand_str.c_str != NULL && (get_int(operand_str.c_str[0]) >= 0 || operand_str.c_str[0] == '-')){
			return (Exp){.num_of_operands = 1, .instruction = INSTRUCTION_PUSH, .operand = get_operand(operand_str)};
		}
		return (Exp){.num_of_operands = 1, .instruction = INSTRUCTION_PUSH};
	}
	if(compare_str(inst_str, MKSTR("pop")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_POP};
	if(compare_str(inst_str, MKSTR("clean")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_CLEAN};
	if(compare_str(inst_str, MKSTR("dup")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_DUP};
	if(compare_str(inst_str, MKSTR("read")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_READ};
	if(compare_str(inst_str, MKSTR("set")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_SET};
	
	if(compare_str(inst_str, MKSTR("not")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_NOT};
	if(compare_str(inst_str, MKSTR("eq")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_EQUAL};
	if(compare_str(inst_str, MKSTR("jmp")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_JMP};
	if(compare_str(inst_str, MKSTR("jmpf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_JMP_IF};
	if(compare_str(inst_str, MKSTR("jmpnf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_JMP_IFNOT};
	
	if(compare_str(inst_str, MKSTR("plusi")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_PLUSI};
	if (compare_str(inst_str, MKSTR("minusi")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_MINUSI};
	if(compare_str(inst_str, MKSTR("muli")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_MULI};
	if(compare_str(inst_str, MKSTR("divi")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_DIVI};
	if(compare_str(inst_str, MKSTR("smalleri")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_SMALLERI};
	if(compare_str(inst_str, MKSTR("biggeri")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_BIGGERI};
	
	if(compare_str(inst_str, MKSTR("plusu")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_PLUSU};
	if (compare_str(inst_str, MKSTR("minusu")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_MINUSU};
	if(compare_str(inst_str, MKSTR("mulu")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_MULU};
	if(compare_str(inst_str, MKSTR("divu")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_DIVU};
	if(compare_str(inst_str, MKSTR("smalleru")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_SMALLERU};
	if(compare_str(inst_str, MKSTR("biggeru")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_BIGGERU};

	if(compare_str(inst_str, MKSTR("plusf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_PLUSF};
	if (compare_str(inst_str, MKSTR("minusf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_MINUSF};
	if(compare_str(inst_str, MKSTR("mulf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_MULF};
	if(compare_str(inst_str, MKSTR("divf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_DIVF};
	if(compare_str(inst_str, MKSTR("smallerf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_SMALLERF};
	if(compare_str(inst_str, MKSTR("biggerf")))
		return (Exp){.num_of_operands = 0, .instruction = INSTRUCTION_BIGGERF};
	

	if(required){
		inst_str.c_str[inst_str.size] = '\0';
		throw_error(ERROR_UNKOWN_INSTRUCTION, inst_str.c_str);
	}
	return (Exp){.num_of_operands = 0, .instruction = 0, .operand.as_ptr = NULL, .operand_aux.as_ptr = NULL};;	
}

void parse(Program* program, S_file_t _file){

	size_t last_expr_index = 0;

	const String file = _file.contents;

	int operands_expected = 0;

	for(size_t i = 0; i < file.size; ){
		if(file.c_str[i] == '#'){
			const int64_t exp_size = find_char(file, '\n', i);
			const String exp_str = (String){.c_str = file.c_str + i, .size = (exp_size < 0)? file.size - i : exp_size};
			InternalInst iinst =  parse_macro(exp_str);

			if(iinst.inst == INTERNAL_INSTRUCTION_INCLUDE){
				const char c = iinst.str.c_str[iinst.str.size];
				iinst.str.c_str[iinst.str.size] = '\0';
				S_file_t __file = read_file(_file.mother_dir, iinst.str.c_str);
				parse(program, __file);
				iinst.str.c_str[iinst.str.size] = c;
			}
			i += exp_size;
			continue;
		}
		if(file.c_str[i] == COMMENT_SYM){
			int64_t skp = find_char(file, '\n', i);
			if(skp < 0) break;
			i += skp;
			continue;
		}
		String token = get_next_token(file, i);
		if(token.c_str == NULL) break;
		//print_str(token); printf("\n");
		i = token.size + (token.c_str - file.c_str);
		if(operands_expected > 0){
			Var operand;
			if(token.c_str != NULL && get_int(token.c_str[0]) < 0 && token.c_str[0] != '-'){
				operand = get_operand(resolve_label(token, 1));
			}
			else operand = get_operand(token);
			if(program->size + sizeof(Var) >= PROGRAM_CAP) throw_error(ERROR_PROGRAM_SIZE_OVERFLOW, "");
			*(Var*)(program->data + program->size) = operand;
			program->size += sizeof(Var);
			operands_expected -= 1;
		} else{
			Exp expr = resolve_inst(token, 0);
			if(expr.instruction == 0) expr = resolve_inst(resolve_label(token, 1), 1);
			operands_expected += expr.num_of_operands;
			if(program->size + sizeof(Inst) >= PROGRAM_CAP) throw_error(ERROR_PROGRAM_SIZE_OVERFLOW, "");
			*(Inst*)(program->data + program->size) = expr.instruction;
			program->size += sizeof(Inst);
		}
	}
}

void write_program(const Program program, const char* output_path){
	FILE* file = fopen(output_path, "wb");

	if(!file){
		throw_error(ERROR_INVALID_FILE_PATH, output_path);
	}

	size_t written = 0;

	for(; written < program.size; written += program.size * fwrite(program.data, program.size, 1, file));

	if(written != program.size){
		printf("[ERROR] Expected To Write %zu bytes, Wrote %zu Instead\n", program.size, written);
		exit(INTERNAL_ERROR_INTERNAL);
	}

	fclose(file);
}


int main(int argc, char** argv){

	if(argc < 2){
		printf("[HELP] Usage: ./fictiler <path_to_input_file> <optional: path_to_output_executable>\n");
		printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
		return ERROR_INVALID_USAGE;
	}

	unsigned char program_data[PROGRAM_CAP];
	unsigned char stream_data[MAX_COMP_STC_STRM_MEMCAP];
	label_count = 0;

	stream = (Stream){.data = stream_data, .size = 0, .capacity = MAX_COMP_STC_STRM_MEMCAP};

	Program program = (Program){.data = program_data, .size = 0, .capacity = PROGRAM_CAP};

	S_file_t file = read_file((String){.c_str = NULL, .size = 0}, argv[1]);

	parse(&program, file);

	write_program(program, (argc == 3)? argv[2] : "out.virtual");

	return 0;
}
