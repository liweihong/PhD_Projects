##################################################################################
# COMPILERS, LINKERS, ARCHIVES, LIBRARIES, and FLAGS
##################################################################################

# for using glu
#MORE_CFLAGS = -DEXTERN=extern -DARGS\(x\)=x
MORE_CFLAGS=

CC = g++ -g -w

TOPSRC=.
PRODIR=$(TOPSRC)/Main
GLUDIR=$(TOPSRC)/Glu
DPSIMPPOLYDIR=$(TOPSRC)/DPSimpPoly
DOBJDIR=$(TOPSRC)/obj
DIR_OBJ = ./obj

OBJECTS = $(DIR_OBJ)/array.o $(DIR_OBJ)/hough.o $(DIR_OBJ)/avl.o $(DIR_OBJ)/st.o $(DIR_OBJ)/DP.o $(DIR_OBJ)/getopt.o $(DIR_OBJ)/nonrec.o $(DIR_OBJ)/seg_merge.o $(DIR_OBJ)/main_taper.o $(DIR_OBJ)/project.o
PROSRCS = $(GLUDIR)/array.c $(GLUDIR)/hough.c $(GLUDIR)/avl.c $(GLUDIR)/st.c $(DPSIMPPOLYDIR)/DP.c $(GLUDIR)/getopt.c $(DPSIMPPOLYDIR)/nonrec.c $(PRODIR)/seg_merge.c $(PRODIR)/main_taper.c $(PRODIR)/project.c $(GLUDIR)/hough.c
HEADERS = $(GLUDIR)/array.h $(GLUDIR)/hough.h $(GLUDIR)/avl.h $(GLUDIR)/st.h $(GLUDIR)/CImg.h $(DPSIMPPOLYDIR)/DP.h $(PRODIR)/project.h

ARR_SRC =  $(GLUDIR)/array.c
ARR_OBJ =  $(DIR_OBJ)/array.o
HGH_SRC =  $(GLUDIR)/hough.c
HGH_OBJ =  $(DIR_OBJ)/hough.o
AVL_SRC =  $(GLUDIR)/avl.c
AVL_OBJ =  $(DIR_OBJ)/avl.o
HAS_SRC =  $(GLUDIR)/st.c
HAS_OBJ =  $(DIR_OBJ)/st.o
DPP_SRC =  $(DPSIMPPOLYDIR)/DP.c
DPP_OBJ =  $(DIR_OBJ)/DP.o
OPT_SRC =  $(GLUDIR)/getopt.c
OPT_OBJ =  $(DIR_OBJ)/getopt.o
REC_SRC =  $(DPSIMPPOLYDIR)/nonrec.c
REC_OBJ =  $(DIR_OBJ)/nonrec.o
TAP_SRC =  $(PRODIR)/main_taper.c
TAP_OBJ =  $(DIR_OBJ)/main_taper.o
SEG_SRC =  $(PRODIR)/seg_merge.c
SEG_OBJ =  $(DIR_OBJ)/seg_merge.o
PRO_SRC =  $(PRODIR)/project.c
PRO_OBJ =  $(DIR_OBJ)/project.o

EXAMPLE = project
all: $(EXAMPLE)

##################################################################################
# EXAMPLES
##################################################################################

INCLUDEHDRS= -I$(GLUDIR) -I$(PRODIR) -I$(DPSIMPPOLYDIR) -DLINUX $(MORE_CFLAGS)
LINKHDRS= -Wall -W -ansi -pedantic -ffast-math -lm -lpthread -I/usr/X11R6/include -L/usr/X11R6/lib64 -lX11 -Dcimg_strict -Dcimg_use_xshm -lXext -Dcimg_use_xrandr -lXrandr

$(EXAMPLE): $(ARR_OBJ) $(HGH_OBJ) $(AVL_OBJ) $(HAS_OBJ) $(DPP_OBJ) $(OPT_OBJ) $(REC_OBJ) $(PRO_OBJ) $(TAP_OBJ) $(SEG_OBJ)
	$(CC) $(INCLUDEHDRS) -o $(EXAMPLE) $(OBJECTS) $(LINKHDRS)
	@etags $(PROSRCS) $(HEADERS)

$(AVL_OBJ): $(AVL_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(ARR_OBJ): $(ARR_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(HGH_OBJ): $(HGH_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(HAS_OBJ): $(HAS_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(DPP_OBJ): $(DPP_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(OPT_OBJ): $(OPT_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(REC_OBJ): $(REC_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(SEG_OBJ): $(SEG_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(TAP_OBJ): $(TAP_SRC)
	$(CC) $(INCLUDEHDRS) -c $< -o $@

$(PRO_OBJ): $(PRO_SRC)
	@rm -f $(EXAMPLE)
	$(CC) $(INCLUDEHDRS) -c $< -o $@


##################################################################################
# CLEANUP
##################################################################################

run: 
	$(CC) $(PRODIR)/run.c -o run
clean: 
	rm -f $(EXAMPLE) $(OBJECTS)

tags:
	@echo "create tags..."
	etags $(PROSRCS) $(HEADERS)
