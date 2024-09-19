#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
SHAREDIR?=	/usr/local
LIB=		imsg
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Werror -Wstrict-prototypes -I${.CURDIR}
SRCS=		imsg-buffer.c imsg.c imsg.h getdtablecount.c getdtablecount.h
INCS=		imsg.h
MAN=		imsg_init.3
NO_OBJ=		yes

.include <bsd.lib.mk>
