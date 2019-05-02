# link with the library
CFLAGS += -I../lib
LIBS   += -L../lib -lmcp

# objects
CONF_OBJ := conftool.o

.PHONY: all
all: mcpconf

mcpconf: $(CONF_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.o mcpconf
