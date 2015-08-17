#	$NetBSD: Makefile,v 1.3 2015/02/02 14:03:05 lneto Exp $

#.include "../Makefile.inc"

.PATH:	../iolib/ \

KMOD=		luaio
SRCS=		luaio.c

SRCS+=		iolib.c

CPPFLAGS+=	-I${S}/../external/mit/lua/dist/src \
		-I${S}/sys

.include <bsd.kmodule.mk>
