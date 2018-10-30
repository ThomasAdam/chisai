CC       ?= gcc
VPATH     = src
CFLAGS   += -std=c11 -Wall -Wextra -pedantic
LDFLAGS  += -lxcb

all: chisai maikuro
	
chisai: chisai.c config.h types.h
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

maikuro: maikuro.c
	$(CC) -o $@ $< $(CFLAGS) 
