all: ReportB.pdf

pdf: ReportB.pdf

ReportB.pdf: clean ReportB.tex
	for f in $$(ls doxygen/*.dot); do echo $${f%.*}; dot -Tpdf -o$${f%.*}.pdf $$f; done;
	texi2pdf ReportB.tex

clean:
	rm -f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl ReportB.pdf
