#
CC=		cc -Wall -Wstrict-prototypes -g -O0 -I.

CFLAGS+=	-I../src/lib/libutil
LDFLAGS+=
LDADD=		../src/lib/libutil/libimsg.a

IMSG_DEP=	../src/lib/libutil/imsg.h \
		../src/lib/libutil/libimsg.a

all: imsg_test

imsg_test: imsg_test.c $(IMSG_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o imsg_test imsg_test.c $(LDADD)

clean:
	rm -f *_test *.core
