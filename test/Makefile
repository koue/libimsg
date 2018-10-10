#
PROG=		imsg_test

CFLAGS=		-Wall \
		-I../src/lib/libutil
LDFLAGS=	-L../src/lib/libutil
LDADD=		-limsg

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

install:

test:
	./${PROG}

.include <bsd.prog.mk>
