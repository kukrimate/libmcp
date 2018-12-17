OBJ = mcp2210.o

# Operating system dependent
ifeq ($(shell uname -s),Linux)
OBJ  += hid_linux.o
LIBS += -ludev
else
$(error Your platform is not supported at the moment!)
endif

APPOBJ = conftool.o

.PHONY: all
all: libmcp2210.so libmcp2210.a mcp2210_conf

# Shared library
libmcp2210.so: $(OBJ)
	$(CC) $(LDFLAGS) -shared $(OBJ) -o $@ $(LIBS)

# Static library
libmcp2210.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)

# Configuration tool
mcp2210_conf: $(APPOBJ) libmcp2210.a
	$(CC) $(LDFLAGS) $(APPOBJ) libmcp2210.a -o $@ $(LIBS)

# Compile C code
%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) libmcp2210.so libmcp2210.a $(APPOBJ) mcp2210_conf
