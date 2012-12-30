PAGES=index.md

GEN=$(PAGES:md=html) header.html footer.html
all: $(GEN)

header.html: template.html
	sed -e '/PAGE/,$$ d' < $< > $@

footer.html: template.html
	sed -e '0,/PAGE/ d' < $< > $@

%.html: %.md header.html footer.html
	(cat header.html; markdown $<; cat footer.html) > $@

clean:
	$(RM) $(GEN)
