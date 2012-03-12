PROGS=mounted cpu
MNTSOURCE=mounted.c
CPUSOURCE=cpu.c
MNTOBJS=mounted.o
CPUOBJS=cpu.o
CFLAGS=-g 
OBJS=$(MNTOBJS) $(CPUOBJS)

all:	$(PROGS)

mounted: 	$(MNTOBJS)
	$(CC) -o mounted $(MNTOBJS)

cpu:		$(CPUOBJS)
	$(CC) -o cpu $(CPUOBJS) -lkstat

clean:
	-rm -f $(PROGS) $(OBJS) core *~

