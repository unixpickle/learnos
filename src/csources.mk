
csources: build
	for file in $(CSOURCES); do \
		gcc -c $$file $(OPTIMIZATION) $(INCLUDES_64) -m64 $(DISABLE_OPTIONS) -o build/`basename $$file .c`.o; \
	done
