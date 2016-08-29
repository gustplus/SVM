#include "vm_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STACK_SIZE 64

//#define DEBUG_MODE

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
		printf("can't load %s, please check if the file is exist\n", file_name);
		running = FALSE;
	}
}

#define FETCH() program[IP]

static bool eval(item_t order) {
	bool no_jmp = TRUE;
	switch(order) {
		case PSH: {
#ifdef DEBUG_MODE
			printf("PSH %d\n", program[IP + 1]);
#endif
			stack[++SP] = program[++IP];
			break;
		}
		case POP: {
#ifdef DEBUG_MODE
			printf("POP\n");
#endif
			--SP;
			break;
		}
		case LOG: {
			printf("%d\n", registers[program[++IP]]);
			break;
		}
		case ADD: {
			registers[EXA] = stack[SP--];
			registers[EXB] = stack[SP];
			registers[EXC] = registers[EXA] + registers[EXB];
			stack[SP] = registers[EXC];
#ifdef DEBUG_MODE
			printf("ADD %d\n", stack[SP]);
#endif
			break;
		}
		case SUB: {
			registers[EXA] = stack[SP--];
			registers[EXB] = stack[SP];
			registers[EXC] = registers[EXB] - registers[EXA];
			stack[SP] = registers[EXC];
#ifdef DEBUG_MODE
			printf("SUB %d\n", stack[SP]);
#endif
			break;
		}
		case MUL: {
			registers[EXA] = stack[SP--];
			registers[EXB] = stack[SP];
			registers[EXC] = registers[EXA] * registers[EXB];
			stack[SP] = registers[EXC];
#ifdef DEBUG_MODE
			printf("MUL %d\n", stack[SP]);
#endif
			break;
		}
		case DIV: {
			registers[EXA] = stack[SP--];
			registers[EXB] = stack[SP];
			registers[EXC] = registers[EXB] / registers[EXA];
			stack[SP] = registers[EXC];
#ifdef DEBUG_MODE
			printf("DIV %d\n", stack[SP]);
#endif
			break;
		}
		case MOV: {
#ifdef DEBUG_MODE
			printf("MOV\n");
#endif
			registers[program[IP + 2]] = registers[program[IP + 1]];
			IP += 2;
			break;
		}
		case SET: {
			registers[program[IP + 1]] = program[IP + 2];
#ifdef DEBUG_MODE
			printf("SET %d\n", program[IP + 2]);
#endif
			IP += 2;
			break;
		}
		case IF: {
#ifdef DEBUG_MODE
			printf("IF\n");
			printf("%d == %d\n", registers[program[IP + 1]], program[IP + 2]);
#endif
			if(registers[program[IP + 1]] == program[IP + 2]) {
				IP = program[IP + 3];
#ifdef DEBUG_MODE
				printf("JMP %d\n", IP);
#endif
				no_jmp = FALSE;
			}else{
				IP += 3;
			}
			break;
		}
		case IFN: {
#ifdef DEBUG_MODE
			printf("IFN\n");
			printf("%d != %d\n", registers[program[IP + 1]], program[IP + 2]);
#endif
			if(registers[program[IP + 1]] != program[IP + 2]) {
				IP = program[IP + 3];
#ifdef DEBUG_MODE
				printf("JMP %d\n", IP);
#endif
				no_jmp = FALSE;
			}else{
				IP += 3;
			}
			break;
		}
		case JMP: {
			IP = program[IP + 1];
#ifdef DEBUG_MODE
			printf("JMP %d\n", IP);
#endif
			no_jmp = FALSE;
			break;
		}
		case STR: {
#ifdef DEBUG_MODE
			printf("STR %d\n", stack[SP]);
#endif
			registers[program[++IP]] = stack[SP];
			break;
		}
		case RTS: {
#ifdef DEBUG_MODE
			printf("RTS %d\n", registers[program[IP + 1]]);
#endif
			stack[++SP] = registers[program[++IP]];
			break;
		}
		case EXT: {
#ifdef DEBUG_MODE
			printf("EXT\n");
#endif
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
#ifdef DEBUG_MODE
		print_program();
#endif
		run_program();
	}
	return 0;
}
