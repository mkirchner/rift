#
# Makefile
#

.PHONY: tests

tests:
	$(MAKE) -C tests

test: tests
	tests/test_rift

# vim:ft=make
#
