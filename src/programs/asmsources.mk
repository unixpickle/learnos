asmsources: build
	for file in $(ASMSOURCES); do\
		nasm -f elf64 $(ASMFLAGS) $$file -o build/`basename $$file .s`.o;\
	done
