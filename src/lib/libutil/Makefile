#
LIB=		imsg
CFLAGS+=	-Werror -Wstrict-prototypes -I${.CURDIR}
SRCS=		imsg-buffer.c imsg.c imsg.h getdtablecount.c getdtablecount.h \
		../libc/stdlib/recallocarray.c compat.h
INCS=		imsg.h
NO_OBJ=		yes

.include <bsd.lib.mk>
