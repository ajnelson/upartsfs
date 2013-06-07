ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src

check-valgrind:
	$(MAKE) -C src $@
	-sleep 0.1s #Let valgrind's output hit the screen
