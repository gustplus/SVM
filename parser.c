#include "vm_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define COMMENT_PREFIX '#'

static FILE *src_f = NULL;
static FILE *dst_f = NULL;

item_t parse_instruction(int line, const char *code_name) {
	for(item_t i = 0; i < INSTRUCTION_COUNT; ++i) {
		const char *it = INSTRUCTION_STRS[i];
		if(0 == strcmp(it, code_name)) {
			return i;
		}
	}
	printf("unknown code: %s line %d\n", code_name, line);
	assert(0);
}

item_t translate_arg(const char *arg) {
	for(item_t i = 0; i < REG_REGISTER_COUNT; ++i) {
		const char *it = REGISTER_STRS[i];
		if(0 == strcmp(it, arg)) {
			return i;
		}
	}
	item_t it = (item_t)atoi(arg);
	return it;
}

static FILE *get_dst_file(const char *src_path) {
	char dst_path[1024] = {0};
	const char *src_name = strrchr(src_path, '/');
	if(!src_name) {
		src_name = src_path;
	}
	char *p = strrchr(src_name, '.');
	if(p && p > src_name) {
		int name_len = p - src_path;
		strncpy(dst_path, src_path, name_len);
		strcpy(dst_path + name_len, ".o");
		printf("%s->%s\n", src_path, dst_path);
		return fopen(dst_path, "wb");
	}
	return NULL;
}

static bool check_line(const char *line) {
	int len = strlen(line);
	return len != 0 && line[0] != COMMENT_PREFIX && line[0] != '\n';
}

//use stack to record the position of while/if pos
#define STACK_SIZE 10
static long positions[STACK_SIZE];
static const int item_s = sizeof(item_t);
static int sp = -1;

#define PUSH(val) (positions[++sp] = val)
#define POP() (positions[sp--])
#define EMPTY() (sp == -1)
#define FULL() (sp == (STACK_SIZE - 1))

static item_t *instructions;
static int ip = -1;
static int instruction_space = 4;

static bool fail_flg = FALSE;

static void check_space(int ip) {
	if(ip == instruction_space) {
		instruction_space *= 2;
		instructions = (item_t *)realloc(instructions, instruction_space * sizeof(item_t));
		assert(instructions);
	}
}

static void push_instruction(item_t t) {
	check_space(ip);
	translate(&t);
	instructions[ip] = t;
}

static void set_instruction(int o_ip, item_t t) {
	check_space(o_ip);
	translate(&t);
	instructions[o_ip] = t;
}

static item_t translate_instruction(item_t instruction) {
	++ip;
	if(IF == instruction) {
		instruction = IFN;
	}else if (IFN == instruction) {
		instruction = IF;
	}else if(WL == instruction) {	//if is WL/WLN, record the ip of the WL/WLN, because we use JMP to implement the loop statement
		PUSH(ip);
		instruction = IFN;
	}else if(WLN == instruction){
		PUSH(ip);
		instruction = IF;
	}
	push_instruction(instruction);
	return instruction;
}

static void handle_statement_end(item_t instruction) {
	assert(!EMPTY());
	int spaceholder_ip = POP();
	if(LW == instruction) {
		++ip;
		push_instruction(JMP);
		assert(!EMPTY());
		int jmp_ip = POP();
		++ip;
		push_instruction(jmp_ip);
	}
	set_instruction(spaceholder_ip, ip + 1);
}

static void handle_line(item_t instruction, item_t *args, size_t count) {
	if(FI == instruction || LW == instruction) {
		handle_statement_end(instruction);
		return;
	}

	instruction = translate_instruction(instruction);

	for(int i = 0; i < count; ++i) {
		++ip;
		push_instruction(args[i]);
	}
	if(IF == instruction || IFN == instruction) {
		PUSH(++ip);
		push_instruction(0);	//as a spaceholder, will be modified when reach FI/LW
	}
}

static void write_to_file() {
	fwrite(instructions, (ip + 1) * sizeof(item_t), 1, dst_f);
}

int main(int argc, const char **argv) {
	if(argc > 1) {
		const char *src = argv[1];
		src_f = fopen(src, "r");
		dst_f = get_dst_file(src);
		char line[1025];
		char str_instruct[10];
		char str_args[3][10];
		item_t args[3];
		int line_idx = 0;

		instructions = calloc(1, instruction_space * sizeof(item_t));

		if(src_f && dst_f) {
			while(fgets(line, 1025, src_f)) {
				++line_idx;
				int arg_num = 0;
				if(check_line(line) && (arg_num = sscanf(line, "%s %s %s %s", str_instruct, str_args[0], str_args[1], str_args[2])) && arg_num) {
					arg_num -= 1;	//sub str_instruct
					item_t instruct = parse_instruction(line_idx, str_instruct);
					for(int idx = 0; idx < arg_num; ++idx) {
						args[idx] = translate_arg(str_args[idx]);
					}
					handle_line(instruct, args, arg_num);
				}	
			}
		}
		fclose(src_f);
		assert(EMPTY());
		write_to_file();
		fclose(dst_f);
	}else{
		printf("command not match: src_file\n");
	}
}
