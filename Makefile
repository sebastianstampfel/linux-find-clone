##
## @file Makefile
## Betriebssysteme Myfind Makefile - Group 11
## Beispiel 1
## Based on Hello-Makefile
##
## @author Sebastian Stampfel <ic19b084@technikum-wien.at>
## @author Benjamin Wiesbauer <>
## @author Milan Kollmann <>
## @date 2020/02/20
##

##
## ------------------------------------------------------------- variables --
##

CC=gcc
CFLAGS=-DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -Wformat=2 -fno-common -ftrapv -g -O3 -std=gnu11
CFLAGS-DEBUG=-DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -Wformat=2 -fno-common -ftrapv -g -std=gnu11
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECTS=main.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

.PHONY: all
all: myfind-grp11

myfind-grp11: $(OBJECTS)
	$(CC) $(CFLAGS) -o "$@" "$^"

debug: $(OBJECTS)
	$(CC) $(CFLAGS-DEBUG) -o "$@" "$^"

.PRECIOUS: %.tex

.PHONY: clean
clean:
	$(RM) *.o *~ myfind-grp11

.PHONY: distclean
distclean: clean
	$(RM) -r doc

doc: html ## pdf

.PHONY: html
html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
