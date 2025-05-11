QUIET = @echo '   CC' $@;
QUIETLINK = @echo 'LK' $@;
ifeq ($(Q)$(q), 0)
QUIET = 
QUIETLINK =
endif
MYCC = $(QUIET)gcc
MYCC_WEB = $(QUIET)/opt/emsdk/upstream/emscripten/emcc
MYLINKPP = $(QUIETLINK)g++
MYLINKPP_WEB = $(QUIETLINK)/opt/emsdk/upstream/emscripten/em++
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

OBJs_WEB := $(PRE)backend__.o \
	$(PRE)backend-web__.o \
	$(PRE)language__.o \
	$(PRE)logic__.o \
	$(PRE)main__.o \
	$(PRE)settings__.o \
	$(PRE)ui__.o
#OBJs_WEB := $(PRE)demo-web__.o

MYCFLAGS_WEB = $(MYCFLAGS) -I ../build/lvgl-web/ -DWEB -Wno-pointer-sign -s USE_SDL=2
MYLDFLAGS_WEB = -L ../build/lvgl-web/cmbuild/lib/ -llvgl -llvgl_thorvg -llvgl_demos -lSDL2 --shell-file /tmp/a.js -s SINGLE_FILE=1 -s INITIAL_MEMORY=33554432 -s EXPORTED_FUNCTIONS='["_main", "_injectInput", "_requestPasscode"]'

.PHONY : clean

ifeq ($(shell uname -m), aarch64)
all: app_exec
else
all: app_exec desktop_exec web_exec
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

web_exec: $(OBJs_WEB)
	@echo "{{{ SCRIPT }}}" > /tmp/a.js
	$(MYLINKPP_WEB) $(DEBUG) $(OBJs_WEB) $(MYLDFLAGS_WEB) -o app.js
ifeq ($(DEBUG),)
	@cp app.js ../www/js/
endif

$(PRE)%.o : %.c
	@mkdir -p $(PRE)
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_APP)

$(PRE)%_.o : %.c
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_DESKTOP)

$(PRE)%__.o : %.c
	$(MYCC_WEB) $(DEBUG) $< -o $@ -c $(MYCFLAGS_WEB)

cleanup:
	@rm -f $(OBJs_APP) $(OBJs_DESKTOP) $(OBJs_WEB) app appD app.js
