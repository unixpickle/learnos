
cppsources: build
	for file in $(CPPSOURCES); do \
		g++ $(CFLAGS) $(CPPFLAGS) $(OPTIMIZATION) -c $$file $(INCLUDES) -m64 -o build/`basename $$file .cpp`.o; \
	done
