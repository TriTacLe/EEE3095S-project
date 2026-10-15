TEX = main

all: $(TEX).pdf

$(TEX).pdf: $(TEX).tex $(wildcard sections/*.tex) refs.bib $(wildcard figures/*)
	pdflatex -interaction=nonstopmode $(TEX).tex > /dev/null
	-bibtex $(TEX) > /dev/null
	pdflatex -interaction=nonstopmode $(TEX).tex > /dev/null
	pdflatex -interaction=nonstopmode $(TEX).tex > /dev/null
	@echo "pages: $$(pdfinfo $(TEX).pdf | awk '/Pages/{print $$2}') (limit 10)"

# The report is ready only when this list is empty. Citations also render as
# [n], so the template's "search the PDF for [" needs an eye; this does not.
check: $(TEX).pdf
	@n=$$(grep -rc '\\ph{' $(TEX).tex sections/ | awk -F: '{s+=$$2} END {print s+0}'); \
	 echo "$$n placeholder(s) left:"; \
	 grep -rn '\\ph{' $(TEX).tex sections/ | sed 's/^/  /' | cut -c1-100 || true
	@echo "overfull boxes: $$(grep -c Overfull $(TEX).log || true)"

clean:
	rm -f $(TEX).pdf $(TEX).aux $(TEX).log $(TEX).out $(TEX).bbl $(TEX).blg

.PHONY: all check clean
