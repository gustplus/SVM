#include "vm_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0
#define STACK_SIZE 64
typedef int bool;

#define SP (registers[SP])
#define IP (registers[IP])

static int registers[REGISTER_COUNT];

static bool running = FALSE;
static int *program = NULL;
static int stack[STACK_SIZE];

static int code_space = 4;
static int code_count = 0;

static void init_vm() {
	running = TRUE;
	SP = -1;
	IP = 0;
	program = calloc(1, sizeof(item_t) * code_space);
	if(!program) {
		running = FALSE;
	}
}

static void print_program() {
	printf("code: %d\n", code_count);
	for(int i = 0; i < code_count; ++i) {
		printf("%d\n", program[i]);
	}
	printf("code end\n");
}

static bool read_item(FILE *f) {
	item_t ret;
	int size = sizeof(item_t);
	if(!fread((void *)&ret, size, 1, f)) {
		return FALSE;
	}

	if(code_count + 1 >= code_space) {
		code_space *= 2;
		program = realloc(program, code_space * sizeof(item_t));
		assert(program);
	}

	translate(&ret);
	program[code_count++] = ret;
	return TRUE;
}

static void load_program(const char *file_name) {
	FILE *f = fopen(file_name, "r");
	if(f) {
		while(read_item(f)) {}
	}else{
		running = FALSE;
	}
}

#define FETCH() program[IP]

static bool eval(item_t order) {
	bool no_jmp = TRUE;
	switch(order) {
		case PSH: {
	//		printf("PSH %d\n", program[IP + 1]);
			stack[++SP] = program[++IP];
			break;
		}
		case POP: {
	//		printf("POP\n");
			--SP;
			break;
		}
		case LOG: {
			printf("log%d\n", registers[program[++IP]]);
			break;
		}
		case ADD: {
	//		printf("ADD\n");
			registers[A] = stack[SP--];
			registers[B] = stack[SP];
			registers[C] = registers[A] + registers[B];
			stack[SP] = registers[C];
			break;
		}
		case SUB: {
			registers[A] = stack[SP--];
			registers[B] = stack[SP];
			registers[C] = registers[A] - registers[B];
			stack[SP] = registers[C];
			break;
		}
		case MUL: {
			registers[A] = stack[SP--];
			registers[B] = stack[SP];
			registers[C] = registers[A] * registers[B];
			stack[SP] = registers[C];
			break;
		}
		case DIV: {
			registers[A] = stack[SP--];
			registers[B] = stack[SP];
			registers[C] = registers[A] / registers[B];
			stack[SP] = registers[C];
			break;
		}
		case MOV: {
			registers[program[IP + 2]] = registers[program[IP + 1]];
			IP += 2;
			break;
		}
		case SET: {
			registers[program[IP + 1]] = program[IP + 2];
			IP += 2;
			break;
		}
		case IF: {
			if(registers[program[IP + 1]] == program[IP + 2]) {
				IP = program[IP + 3];
	//			printf("ip = %d\n", IP);
				no_jmp = FALSE;
			}else{
				IP += 3;
			}
			break;
		}
		case IFN: {
			if(registers[program[IP + 1]] != program[IP + 2]) {
				IP = program[IP + 3];
	//			printf("ip = %d\n", IP);
				no_jmp = FALSE;
			}else{
				IP += 3;
	//			printf("ip = %d\n", IP);
			}
			break;
		}
		case STR: {
	//		printf("STR %d\n", stack[SP]);
			registers[program[++IP]] = stack[SP];
			break;
		}
		case RTS: {
	//		printf("RTS\n");
			stack[++SP] = registers[program[++IP]];
			break;
		}
		case EXT: {
	//		printf("EXT\n");
			running = FALSE;
			break;
		}
	}
	return no_jmp;
}

static void run_program() {
	while(running && IP < code_count) {
		item_t order = FETCH();
		if(eval(order)) {
			++IP;
		}
	}
}

int main(int argc, const char **argv) {
	if(argc > 1) {
		init_vm();
		load_program(argv[1]);
	//	print_program();
		run_program();
	}
	return 0;
}
