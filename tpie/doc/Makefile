# Copyright (c) 1999 Octavian Procopiuc
#
# The Makefile for the TPIE documentation.
#
# $Id: Makefile,v 1.1 2001-12-19 21:22:29 tavi Exp $
#

LATEX = latex
BIBTEX = bibtex
MAKEINDEX = makeindex
DVIPS = dvips
LATEX2HTML = latex2html
PS2PDF = ps2pdf

all: tpie.ps

MAIN_SRC = tpie.tex

OTHER_SRC = GNUlicence.tex  ami_reference.tex  errorcodes.tex  \
  acknowledgements.tex appendices.tex implementation.tex tuning.tex  \
  addexamples.tex applications.tex user_manual.tex sample_pgm.cpp \
  block.tex btree.tex cache.tex

tpie.dvi: $(MAIN_SRC) $(OTHER_SRC) tpie.ind tpie.bbl
	$(LATEX) $<
	$(LATEX) $< > /dev/null

html:	tpie.aux tpie.bbl tpie.dvi
	$(LATEX2HTML) tpie

tpie.ps: tpie.dvi
	$(DVIPS) -o $@ $<

tpie.pdf: tpie.ps
	$(PS2PDF) $<

tpie.idx: $(MAIN_SRC) $(OTHER_SRC)
	-rm -f $@
	$(LATEX) $<

tpie.ind: tpie.idx
	$(MAKEINDEX) -o $@ $< > /dev/null

tpie.aux: $(MAIN_SRC) $(OTHER_SRC)
	$(LATEX) $<

tpie.bbl: tpie.aux tpie.bib
	$(BIBTEX) $(<:.aux=)

clean:
	-rm -f tpie.log tpie.ilg tpie.blg tpie.bbl tpie.ind tpie.dvi 
	-rm -f tpie.idx tpie.aux tpie.toc
	-rm -f *~
	-rm -rf tpie
