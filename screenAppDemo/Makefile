QUIET = @echo '   CC' $@;
QUIETLINK = @echo 'LK' $@;
ifeq ($(Q)$(q), 0)
QUIET = 
QUIETLINK =
endif
MYCC = $(QUIET)gcc
MYPP = $(QUIET)g++
MYLINK = $(QUIETLINK)gcc
MYLINKPP = $(QUIETLINK)g++
DEBUG = -g
#DEBUG = -g -fsanitize=address -fno-omit-frame-pointer
PRE = /tmp/_screenapp/

OBJs := $(PRE)main.o $(PRE)ui.o $(PRE)backend.o $(PRE)backend-spi.o
OBJs_DESKTOP := $(PRE)main.o $(PRE)ui.o $(PRE)backend.o $(PRE)backend-gtk_.o

MYCFLAGS = $(CFLAGS) -I ../build/lvgl
MYLDFLAGS = -pthread -L ../build/lvgl/build/lib/ -llvgl -llvgl_thorvg -linput

MYCFLAGS_DESKTOP = $(MYCFLAGS) `pkg-config --cflags gtk+-3.0`
MYLDFLAGS_DESKTOP = $(MYLDFLAGS) `pkg-config --libs gtk+-3.0`

.PHONY : clean

ifeq ($(shell uname -m), aarch64)
all: app_exec
else
all: app_exec desktop_exec
endif
clean: cleanup

app_exec: $(OBJs)
	$(MYLINKPP) $(DEBUG) $(OBJs) $(MYLDFLAGS) -o app
ifeq ($(DEBUG),)
	@strip app
endif

desktop_exec: $(OBJs_DESKTOP)
	$(MYLINKPP) $(DEBUG) $(OBJs_DESKTOP) $(MYLDFLAGS_DESKTOP) -o appD
ifeq ($(DEBUG),)
	@strip appD
endif

$(PRE)%.o : %.c
	@mkdir -p $(PRE)
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS)

$(PRE)%_.o : %.c
	@mkdir -p $(PRE)
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_DESKTOP)

cleanup:
	@rm -f $(OBJs) $(OBJs_DESKTOP) app appD
