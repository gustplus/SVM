#include "vm_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define COMMENT_PREFIX '#'

static FILE *src_f = NULL;
static FILE *dst_f = NULL;

item_t parse_instruction(int line, const char *code_name) {
	for(item_t i = 0; i < ORDER_COUNT; ++i) {
		const char *it = ORDER_STRS[i];
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
		return fopen(dst_path, "w");
	}
	return NULL;
}

static bool check_line(const char *line) {
	int len = strlen(line);
	return len != 0 && line[0] != COMMENT_PREFIX && line[0] != '\n';
}

#define STACK_SIZE 10
static long positions[STACK_SIZE];
static const int item_s = sizeof(item_t);
static int sp = -1;
static int item_idx = -1;

#define PUSH(val) (positions[++sp] = val)
#define POP() (positions[sp--])
#define EMPTY() (sp == -1)
#define FULL() (sp == (STACK_SIZE - 1))

void write_to(FILE *f, item_t t) {
	translate(&t);
	fwrite((void *)&t, 4, 1, f);
}

static void handle_line(item_t instruction, item_t *args, size_t count) {
	if(FI == instruction || LW == instruction) {
		assert(!EMPTY());
		int ip = POP();
		int tmp_pos = ftell(dst_f);
		fseek(dst_f, ip, SEEK_SET);
		write_to(dst_f, LW == instruction ? item_idx + 3 : item_idx + 1);
		fseek(dst_f, tmp_pos, SEEK_SET);
		if(LW == instruction) {
			write_to(dst_f, JMP);
			assert(!EMPTY());
			int jmp_ip = POP();
			write_to(dst_f, jmp_ip);
			item_idx += 2;
		}
		return;
	}

	++item_idx;
	bool stat = TRUE;
	if(IF == instruction) {
		instruction = IFN;
	}else if (IFN == instruction) {
		instruction = IF;
	}else if(WL == instruction) {
		PUSH(item_idx);
		instruction = IFN;
	}else if(WLN == instruction){
		PUSH(item_idx);
		instruction = IF;
	}else{
		stat = FALSE;
	}

	
	write_to(dst_f, instruction);
	for(int i = 0; i < count; ++i) {
		++item_idx;
		write_to(dst_f, args[i]);
	}
	
	if(stat) {
		++item_idx;
		PUSH(ftell(dst_f));
		write_to(dst_f, 0);
	}
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
		fclose(dst_f);
		assert(EMPTY());
	}else{
		printf("command not match: src_file\n");
	}
}
