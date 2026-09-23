# Build the report. Two passes so references resolve.
#   make        build the PDF
#   make check  build, then list what is still unwritten
TEX = project_report

all: $(TEX).pdf

$(TEX).pdf: $(TEX).tex $(wildcard figures/*)
	pdflatex -interaction=nonstopmode $(TEX).tex > /dev/null
	pdflatex -interaction=nonstopmode $(TEX).tex > /dev/null
	@echo "pages: $$(pdfinfo $(TEX).pdf | awk '/Pages/{print $$2}')"

check: $(TEX).pdf
	@n=$$(grep -c '\\todo{' $(TEX).tex); \
	 echo "$$n section(s) still unwritten:"; \
	 grep -n '\\todo{' $(TEX).tex | sed 's/^/  /' | cut -c1-90

clean:
	rm -f $(TEX).pdf $(TEX).aux $(TEX).log $(TEX).out

.PHONY: all check clean
