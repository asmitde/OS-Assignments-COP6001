# Link the binaries with pthread and libart library
LDFLAGS += -pthread -lrt

CFLAGS += -Wall

# Uncomment the following line to turn on debug statements
# CFLAGS += -D DEBUG

.PHONY: all clean

all:
	$(CC) $(CFLAGS) surfers.c -o surfers $(LDFLAGS)

clean:
	@- $(RM) $(programs)

