# Makefile for Danbo

NAME=danbo

CPPSOURCES=danbo danbotest
ASOURCES=

LIBS=feta

# ----

-include ../make-base/make-base.mk
-include ../make-base/make-lib.mk

CFLAGS+=-nostdlib -fno-rtti -fno-exceptions -fPIC
