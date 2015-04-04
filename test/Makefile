# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as

# UCSD OSTL (Linux)
# GCCDIR = /home/ostl/cs120f/public/mips-x86.linux-xgcc/
# UCSD uAPE (Sparc/Solaris)
# GCCDIR = /home/solaris/ieng9/cs120f/public/mips-sparc.solaris-xgcc/
#GCCDIR = $(CSE120_GCC)
GCCDIR = /classes/cs4411/code/mips-x86.linux-xgcc/
LDFLAGS = -T script -N
ASFLAGS = -mips2
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

PATH = $(GCCDIR):/lib:/usr/bin:/bin

CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = gcc -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt shell matmult sort test fork kid deepfork kid4 kid5 bogus1 fromcons hellofile argkid argtest multiprog child1 child2 fileio

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.c > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult
	
test.o: test.c
	$(CC) $(CFLAGS) -c test.c
test: test.o start.o
	$(LD) $(LDFLAGS) start.o test.o -o test.coff
	../bin/coff2noff test.coff test
	
fork.o: fork.c
	$(CC) $(CFLAGS) -c fork.c
fork: fork.o start.o
	$(LD) $(LDFLAGS) start.o fork.o -o fork.coff
	../bin/coff2noff fork.coff fork
	
kid.o: kid.c
	$(CC) $(CFLAGS) -c kid.c
kid: kid.o start.o
	$(LD) $(LDFLAGS) start.o kid.o -o kid.coff
	../bin/coff2noff kid.coff kid
		
deepfork.o: deepfork.c
	$(CC) $(CFLAGS) -c deepfork.c
deepfork: deepfork.o start.o
	$(LD) $(LDFLAGS) start.o deepfork.o -o deepfork.coff
	../bin/coff2noff deepfork.coff deepfork
	
kid4.o: kid4.c
	$(CC) $(CFLAGS) -c kid4.c
kid4: kid4.o start.o
	$(LD) $(LDFLAGS) start.o kid4.o -o kid4.coff
	../bin/coff2noff kid4.coff kid4

kid5.o: kid5.c
	$(CC) $(CFLAGS) -c kid5.c
kid5: kid5.o start.o
	$(LD) $(LDFLAGS) start.o kid5.o -o kid5.coff
	../bin/coff2noff kid5.coff kid5
	
bogus1.o: bogus1.c
	$(CC) $(CFLAGS) -c bogus1.c
bogus1: bogus1.o start.o
	$(LD) $(LDFLAGS) start.o bogus1.o -o bogus1.coff
	../bin/coff2noff bogus1.coff bogus1

fromcons.o: fromcons.c
	$(CC) $(CFLAGS) -c fromcons.c
fromcons: fromcons.o start.o
	$(LD) $(LDFLAGS) start.o fromcons.o -o fromcons.coff
	../bin/coff2noff fromcons.coff fromcons
	
hellofile.o: hellofile.c
	$(CC) $(CFLAGS) -c hellofile.c
hellofile: hellofile.o start.o
	$(LD) $(LDFLAGS) start.o hellofile.o -o hellofile.coff
	../bin/coff2noff hellofile.coff hellofile
	
argkid.o: argkid.c
	$(CC) $(CFLAGS) -c argkid.c
argkid: argkid.o start.o
	$(LD) $(LDFLAGS) start.o argkid.o -o argkid.coff
	../bin/coff2noff argkid.coff argkid
	
argtest.o: argtest.c
	$(CC) $(CFLAGS) -c argtest.c
argtest: argtest.o start.o
	$(LD) $(LDFLAGS) start.o argtest.o -o argtest.coff
	../bin/coff2noff argtest.coff argtest
	
multiprog.o: multiprog.c
	$(CC) $(CFLAGS) -c multiprog.c
multiprog: multiprog.o start.o
	$(LD) $(LDFLAGS) start.o multiprog.o -o multiprog.coff
	../bin/coff2noff multiprog.coff multiprog
	
child1.o: child1.c
	$(CC) $(CFLAGS) -c child1.c
child1: child1.o start.o
	$(LD) $(LDFLAGS) start.o child1.o -o child1.coff
	../bin/coff2noff child1.coff child1

child2.o: child2.c
	$(CC) $(CFLAGS) -c child2.c
child2: child2.o start.o
	$(LD) $(LDFLAGS) start.o child2.o -o child2.coff
	../bin/coff2noff child2.coff child2

fileio.o: fileio.c
	$(CC) $(CFLAGS) -c fileio.c
fileio: fileio.o start.o
	$(LD) $(LDFLAGS) start.o fileio.o -o fileio.coff
	../bin/coff2noff fileio.coff fileio
