#include "vm_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define COMMENT_PREFIX '#'

item_t parse_order(int line, const char *code_name) {
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

void write_to(FILE *f, item_t t) {
	translate(&t);
	fwrite((void *)&t, 4, 1, f);
}

int main(int argc, const char **argv) {
	if(argc > 2) {
		const char *src = argv[1];
		const char *dst = argv[2];
		char line[1025];
		char code[10];
		char args[3][10];
		int line_idx = 0;

		FILE *src_f = fopen(src, "r");
		FILE *dst_f = fopen(dst, "wb");
		if(src_f && dst_f) {
			while(fgets(line, 1025, src_f)) {
				++line_idx;
				int arg_num = 0;
				if(strlen(line) != 0 && line[0] != COMMENT_PREFIX && (arg_num = sscanf(line, "%s %s %s %s", code, args[0], args[1], args[2]))) {
					item_t t = parse_order(line_idx, code);
					write_to(dst_f, t);
					for(int idx = 0; idx < arg_num - 1; ++idx) {
						item_t arg = translate_arg(args[idx]);
						write_to(dst_f, arg);
					}
				}	
			}
		}
		fclose(src_f);
		fclose(dst_f);
	}else{
		printf("command not match: src_file dst_file\n");
	}
}
