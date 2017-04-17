# Makefile #
#
all: ppa ppb ppc ppd ppe

ppa: ppa.c checker.o
	cc -Wall -D_REENTRANT ppa.c util.c -pthread -o ppa

ppb: ppb.c checker.o
	cc -Wall -D_REENTRANT ppb.c util.c -o ppb -pthread

ppc: ppc.c checker.o
	cc -Wall -D_REENTRANT ppc.c util.c -o ppc -pthread

ppd: ppd.c checker.o
	cc -Wall -D_REENTRANT ppd.c util.c -o ppd -pthread

ppe: ppe.c checker.o
	cc -Wall -D_REENTRANT ppe.c util.c -o ppe -pthread


clean:
	rm -f ppa ppb ppc ppd ppe

