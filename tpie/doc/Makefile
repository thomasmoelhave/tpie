#
# $Id: Makefile,v 1.2 2002-06-26 23:25:54 tavi Exp $
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
  addexamples.tex applications.tex user_manual.tex sample_pgm.cpp 

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
