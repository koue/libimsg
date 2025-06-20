#	$OpenBSD: Makefile,v 1.2 2023/06/13 10:38:45 tb Exp $

PROG=	ibuf_test
CFLAGS=	-I.${CURDIR}/../../../../lib/libutil/
LDADD=	.${CURDIR}/../../../../lib/libutil/libimsg.a
#DPADD=	${LIBUTIL}
MAN=

.include <bsd.regress.mk>
