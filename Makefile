LIB=libtask.a
TCPLIBS=

ASM=asm.o
OFILES=\
	$(ASM)\
	channel.o\
	context.o\
	print.o\
	qlock.o\
	rendez.o\
	task.o

ifdef ARM6M
all: $(LIB)
CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar
CFLAGS=-Wall -c -I. \
	-mcpu=cortex-m0 -mthumb -mabi=aapcs \
	-mfloat-abi=soft -ffunction-sections -fdata-sections \
	-fno-strict-aliasing -fno-builtin --short-enums
else
all: $(LIB) primes tcpproxy testdelay
OFILES+=\
	fd.o\
	net.o

CC=gcc
AR=ar
CFLAGS=-Wall -c -I. -ggdb
endif

AS=$(CC) -c


ifdef DEBUG
CFLAGS+=-ggdb
endif

$(OFILES): taskimpl.h task.h 386-ucontext.h power-ucontext.h arm-ucontext.h

%.o: %.s
	$(AS) -x assembler-with-cpp $*.s

%.o: %.c
	$(CC) $(CFLAGS) $*.c

$(LIB): $(OFILES)
	ar rvc $(LIB) $(OFILES)

primes: primes.o $(LIB)
	$(CC) -o primes primes.o $(LIB)

tcpproxy: tcpproxy.o $(LIB)
	$(CC) -o tcpproxy tcpproxy.o $(LIB) $(TCPLIBS)

httpload: httpload.o $(LIB)
	$(CC) -o httpload httpload.o $(LIB)

testdelay: testdelay.o $(LIB)
	$(CC) -o testdelay testdelay.o $(LIB)

testdelay1: testdelay1.o $(LIB)
	$(CC) -o testdelay1 testdelay1.o $(LIB)

clean:
	rm -f *.o primes tcpproxy testdelay testdelay1 httpload $(LIB)

install: $(LIB)
	cp $(LIB) /usr/local/lib
	cp task.h /usr/local/include

