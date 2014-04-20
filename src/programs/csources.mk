
csources: build
	for file in $(CSOURCES); do \
		gcc $(CFLAGS) $(OPTIMIZATION) -c $$file $(INCLUDES) -m64 -o build/`basename $$file .c`.o; \
	done
