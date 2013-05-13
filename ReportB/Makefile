all: alg.pdf

pdf: alg.pdf

alg.pdf: clean alg.tex
	for f in $$(ls doxygen/*.dot); do echo $${f%.*}; dot -Tpdf -o$${f%.*}.pdf $$f; done;
	texi2pdf alg.tex

clean:
	rm -f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl alg.pdf
