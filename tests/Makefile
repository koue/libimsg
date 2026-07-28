#
PROG=		imsg_composev_test

CFLAGS=		-Werror \
       		-I../src/lib/libutil
LDFLAGS=	-L../src/lib/libutil
LDADD=		-limsg

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR=	.

install:

test:
	./${PROC}

.include <bsd.prog.mk>
