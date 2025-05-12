QUIET = @echo '   CC' $@;
QUIETLINK = @echo 'LK' $@;
ifeq ($(Q)$(q), 0)
QUIET = 
QUIETLINK =
endif
MYCC = $(QUIET)gcc
MYLINKPP = $(QUIETLINK)gcc
#DEBUG = -g -fsanitize=address -fno-omit-frame-pointer
PRE = /tmp/_init/

OBJs := $(PRE)init.o

.PHONY : clean

all: init_exec
clean: cleanup

init_exec: $(OBJs)
	$(MYCC) -static $(OBJs) -o init

$(PRE)%.o : %.c
	@mkdir -p $(PRE)
	$(MYCC) $(DEBUG) $< -o $@ -c

cleanup:
	@rm -f $(OBJs) init
