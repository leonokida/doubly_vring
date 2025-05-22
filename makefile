all: doubly_vring

doubly_vring: doubly_vring.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic doubly_vring.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

doubly_vring.o: doubly_vring.c smpl.h
	$(COMPILE.c) -g  doubly_vring.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o doubly_vring

