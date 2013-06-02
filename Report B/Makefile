DOTDIR=doxygen
IMDIR=images

$(IMDIR)/%.pdf:
	for f in $$(ls doxygen/*.dot); do echo $${f%.*}; s=`basename $$f .dot`; dot -Tpdf -oimages/$$s.pdf $$f; done;


all : ReportB.pdf

pdf : ReportB.pdf

ReportB.pdf : ReportB.tex $(IMDIR)/%.pdf
	texi2pdf --tidy ReportB.tex
	cd ReportB.t2d/pdf/build && makeglossaries ReportB
	texi2pdf --tidy ReportB.tex

clean :
	rm -f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl ReportB.pdf
	rm -r *.t2d
