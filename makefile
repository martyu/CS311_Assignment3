TARGET=assignment3
HTML=main_html

default: pdf ${TARGET}

both: pdf html

${TARGET}: main.cpp
	g++ main.cpp -pthread -o ${TARGET}

dvi: ${TARGET}.tex 
	latex ${TARGET}.tex
	latex $(TARGET).tex

ps: dvi
	dvips -R -P outline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps


html:
	cp ${TARGET}.tex ${HTML}.tex
	latex ${HTML}.tex
	latex2html -split 0 -show_section_numbers -local_icons -no_navigation ${HTML}

	sed 's/<\/SUP><\/A>/<\/SUP><\/A> /g' < ${HTML}/index.html > ${HTML}/index2.html
	mv ${HTML}/index2.html ${HTML}/index.html
	rm ${HTML}.*


