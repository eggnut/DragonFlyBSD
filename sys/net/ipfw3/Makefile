# $FreeBSD: src/sys/modules/ipfw/Makefile,v 1.11.2.1 2003/02/14 14:09:21 maxim Exp $
# $DragonFly: src/sys/net/ipfw/Makefile,v 1.6 2008/09/16 12:16:08 sephe Exp $

KMOD=	ipfw3
SRCS=	ip_fw3.c
SRCS+=	ip_fw3_log.c ip_fw3_log.h
SRCS+=  ip_fw3_table.c ip_fw3_table.h
SRCS+=  ip_fw3_sync.c ip_fw3_sync.h
SRCS+=	opt_ipfw.h opt_inet.h

.ifndef BUILDING_WITH_KERNEL
opt_inet.h:
	echo '#define INET 1' > ${.OBJDIR}/${.TARGET}

opt_ipfw.h:
	echo '#define IPFIREWALL3 1' > ${.OBJDIR}/${.TARGET}
	# Debug prints
	#echo '#define IPFIREWALL_DEBUG 1' >> ${.OBJDIR}/${.TARGET}
	#
	# If you want it verbose
	#echo '#define IPFIREWALL_VERBOSE 1' >> ${.OBJDIR}/${.TARGET}
	#
	# If you want it to pass all packets by default
	#echo '#define IPFIREWALL_DEFAULT_TO_ACCEPT 1' >> ${.OBJDIR}/${.TARGET}
.endif

.include <bsd.kmod.mk>
