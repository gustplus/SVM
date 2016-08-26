#ifndef VM_DEFINE_H_
#define VM_DEFINE_H_

#define item_t int

typedef enum REGISTER_IDS {
	A, B, C, D, E, F, 
	EX,
	EXA,

	SP,
	IP,
	REGISTER_COUNT
} REGISTER_IDS;

char* REGISTER_STRS[] = {
	"A", "B", "C", "D", "E", "F",
	"EX", "EXA"
};

#define REG_REGISTER_COUNT SP

typedef enum ORDERS {
	PSH,	//push
	POP,	//pop
	ADD,	//add
	SUB,	//sub
	MUL,
	DIV,
	MOV,	//MOV register0 register1
	SET,	//SET register num
	IF,	//IF register statement jump_idx
	IFN,	//IFN register statement jump_idx
//	FI,	//end if
	STR,	//STR register (stack to register)
	RTS,	//RTS register (register to stack)
	LOG,	//print
	EXT,	//exit
	ORDER_COUNT
} ORDERS;

char* ORDER_STRS[] = {
	"PSH",
	"POP",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOV",
	"SET",
	"IF",
	"IFN",
//	"FI",
	"STR",
	"RTS",
	"LOG",
	"EXT"
};

typedef union int_32 {
	int i_val;
	char c_val;
} int_32;

typedef enum ENDIAN_TYPE {
	BIG_ENDIAN,
	LITTLE_ENDIAN
} ENDIAN_TYPE;

static ENDIAN_TYPE get_endian_type() {
	int_32 t;
	t.i_val = 1;
	return t.c_val == 1 ? LITTLE_ENDIAN : BIG_ENDIAN;
}

void swap_endian(item_t *endian) {
	char *p = (char *)endian;
	int i, j;
	int byte_num = sizeof(item_t);
	for(i = 0, j = (byte_num - 1); i < j; ++i, --j) {
		char tmp = p[i];
		p[i] = p[j];
		p[j] = tmp;
	}
}

void translate(item_t *endian) {
	if(BIG_ENDIAN == get_endian_type()) {
		swap_endian(endian);
	}	
}

#endif
