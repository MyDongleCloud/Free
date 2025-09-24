MYCC = gcc
#DEBUG = -g
PRE = /tmp/_screendemo/

OBJs := $(PRE)screen.o $(PRE)screenBackend.o $(PRE)main.o

.PHONY : clean

all: main_exec
clean: cleanup

main_exec: $(OBJs)
	$(MYCC) $(DEBUG) $(OBJs) -o demo

$(PRE)%.o : %.c
	@mkdir -p $(PRE)
	$(MYCC) $(DEBUG) $< -o $@ -c

cleanup:
	@rm -f $(OBJs)
	@rm -f demo
