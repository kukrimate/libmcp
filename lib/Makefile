# common objects
LIBMCP_OBJ := mcp2210.o

# backends
ifeq ($(USE_LIBUSB),1)
LIBMCP_OBJ += hid_libusb.o
else ifeq ($(USE_HIDRAW),1)
LIBMCP_OBJ += hid_linux.o
else
$(error No backend selected)
endif

.PHONY: all
all: libmcp.a

libmcp.a: $(LIBMCP_OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.a *.o
