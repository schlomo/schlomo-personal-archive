# The following three lines should be modified to appropriate values for your
# system.  Most of the configurable stuff is in config.h
#
#   CC=       an ansi-C compiler.  If "cc" doesn't work, try "gcc".
#   LIB=      libraries to link in.  -lcrypt, -lshadow, -lpam sometimes needed.
#   LOCALFLAGS=   compiler flags.  Usually -g, -O, and stuff like that.

# Settings for author's system (Redhat 6.1)
CC=gcc
LIB= -lcrypt
LOCALFLAGS= -g 

# For PAM on Redhat Linux
# LIB=-lpam -ldl

# For PAM on Solaris
# LIB=-lpam

# -------------------- No User Servicable Parts Below -----------------------

CFLAGS= $(LOCALFLAGS)

pwauth: pwauth.o
	$(CC) -o pwauth $(CFLAGS) pwauth.o $(LIB)

pwauth.o: pwauth.c config.h

clean:
	rm -f *.o pwauth
