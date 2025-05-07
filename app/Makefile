QUIET = @echo '   CC' $@;
QUIETLINK = @echo 'LK' $@;
ifeq ($(Q)$(q), 0)
QUIET = 
QUIETLINK =
endif
MYCC = $(QUIET)gcc
MYLINK = $(QUIETLINK)gcc
MYLINKPP = $(QUIETLINK)g++
DEBUG = -g
#DEBUG = -g -fsanitize=address -fno-omit-frame-pointer
PRE = /tmp/_app/

OBJs := $(PRE)backend.o \
	$(PRE)ble.o \
	$(PRE)bluetoothAddr.o \
	$(PRE)btlib.o \
	$(PRE)common.o \
	$(PRE)language.o \
	$(PRE)logic.o \
	$(PRE)main.o \
	$(PRE)settings.o \
	$(PRE)ui.o

MYCFLAGS = $(CFLAGS) -Wno-deprecated-declarations
MYLDFLAGS = -pthread -lcrypto -lssl -lcurl -loath

OBJs_APP := $(OBJs) $(PRE)backend-spi.o
MYCFLAGS_APP = $(MYCFLAGS) -I ../build/lvgl/
MYLDFLAGS_APP = $(MYLDFLAGS) -L ../build/lvgl/build/lib/ -llvgl -llvgl_thorvg -linput

OBJs_DESKTOP := $(OBJs:%.o=%_.o) $(PRE)backend-gtk_.o
MYCFLAGS_DESKTOP = $(MYCFLAGS_APP) -DDESKTOP `pkg-config --cflags gtk+-3.0`
MYLDFLAGS_DESKTOP = $(MYLDFLAGS_APP) `pkg-config --libs gtk+-3.0`

.PHONY : clean

ifeq ($(shell uname -m), aarch64)
all: app_exec
else
all: app_exec desktop_exec
endif
clean: cleanup

app_exec: $(OBJs_APP)
	$(MYLINKPP) $(DEBUG) $(OBJs_APP) $(MYLDFLAGS_APP) -o app
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
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_APP)

$(PRE)%_.o : %.c
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_DESKTOP)

cleanup:
	@rm -f $(OBJs_APP) $(OBJs_DESKTOP) app appD
