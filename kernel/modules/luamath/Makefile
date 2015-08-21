#	$NetBSD: Makefile,v 1.3 2015/02/02 14:03:05 lneto Exp $

#.include "../Makefile.inc"

KMOD=		luamath
SRCS=		luamath.c

CPPFLAGS+=	-I${S}/../external/mit/lua/dist/src \
		-I${S}/sys

.if make(tests)
CPPFLAGS+=  -DLUA_USER_H='"ltests.h"'
.endif

tests:		all

.include <bsd.kmodule.mk>
