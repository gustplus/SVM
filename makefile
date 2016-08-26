all: parser vm
	#build SVM finished#

parser: parser.c vm_define.h
	gcc -o parser parser.c

vm: tinyVM.c vm_define.h
	gcc -o vm tinyVM.c
