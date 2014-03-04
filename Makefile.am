ACLOCAL_AMFLAGS = -I m4
SUBDIRS = deps src tests

check-valgrind:
	$(MAKE) -C tests $@
	-sleep 0.1s #Let valgrind's output hit the screen
