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
PRE = ../build/_app/
MDC_VERSION=\"$(shell cat version.txt)\"

OBJs := $(PRE)apache2.o \
	$(PRE)backend.o \
	$(PRE)ble.o \
	$(PRE)bluetoothAddr.o \
	$(PRE)btlib.o \
	$(PRE)certificate.o \
	$(PRE)cJSON.o \
	$(PRE)common.o \
	$(PRE)communication.o \
	$(PRE)json.o \
	$(PRE)language.o \
	$(PRE)logic.o \
	$(PRE)main.o \
	$(PRE)modules.o \
	$(PRE)password.o \
	$(PRE)sbl.o \
	$(PRE)sbl2.o \
	$(PRE)settings.o \
	$(PRE)space.o \
	$(PRE)ui.o

MYCFLAGS = $(CFLAGS) -Wno-deprecated-declarations -DMDC_VERSION=$(MDC_VERSION)
MYLDFLAGS = -pthread -lcrypto -lssl -lcurl -loath

OBJs_APP := $(OBJs) $(PRE)backend-spi.o
MYCFLAGS_APP = $(MYCFLAGS) -I ../build/lvgl/
MYLDFLAGS_APP = $(MYLDFLAGS) -L ../build/lvgl/build/lib/ -llvgl -llvgl_thorvg -linput -lpam

OBJs_DESKTOP := $(OBJs:%.o=%_.o) $(PRE)backend-gtk_.o
MYCFLAGS_DESKTOP = $(MYCFLAGS_APP) -DDESKTOP `pkg-config --cflags gtk+-3.0`
MYLDFLAGS_DESKTOP = $(MYLDFLAGS_APP) `pkg-config --libs gtk+-3.0`

OBJs_WEB := $(PRE)backend__.o \
	$(PRE)backend-web__.o \
	$(PRE)cJSON__.o \
	$(PRE)communication__.o \
	$(PRE)language__.o \
	$(PRE)logic__.o \
	$(PRE)main__.o \
	$(PRE)settings__.o \
	$(PRE)ui__.o
#OBJs_WEB := $(PRE)demo-web__.o

OBJs_ZIGBEE := $(PRE)sbl.o \
	$(PRE)sbl2.o \
	$(PRE)main_zigbee.o

MYCFLAGS_WEB = $(MYCFLAGS) -I ../build/lvgl-web/ -DWEB -Wno-pointer-sign -s USE_SDL=2
MYLDFLAGS_WEB = -L ../build/lvgl-web/cmbuild/lib/ -llvgl -llvgl_thorvg -llvgl_demos -lSDL2 --shell-file /tmp/a.js -s SINGLE_FILE=1 -s INITIAL_MEMORY=33554432 -s EXPORTED_FUNCTIONS='["_main", "_button", "_requestPasscode", "_serverReceive", "_communicationStatus", "_malloc", "_free"]'

.PHONY : clean

ifeq ($(shell [ "$$(uname -m)" = "aarch64" ] || ischroot ; echo $$?), 0)
all: pre_build app_exec zigbee_exec
else
all: pre_build app_exec desktop_exec zigbee_exec web_exec
endif
clean: cleanup

pre_build:
	@mkdir -p $(PRE)
ifeq ($(shell [ "$$(uname -m)" = "aarch64" ] || ischroot ; echo $$?), 0)
	@ln -sf /usr/local/modules/MyDongleCloud/version.txt version.txt
endif

app_exec: $(OBJs_APP)
	$(MYLINKPP) $(DEBUG) $(OBJs_APP) $(MYLDFLAGS_APP) -o app
ifeq ($(DEBUG),)
	@strip app
endif
ifeq ($(shell [ "$$(uname -m)" = "aarch64" ] || ischroot ; echo $$?), 0)
	@setcap 'cap_net_admin+eip' app
	@cp app /usr/local/modules/MyDongleCloud/app
	@setcap 'cap_net_admin+eip' /usr/local/modules/MyDongleCloud/app
endif

desktop_exec: $(OBJs_DESKTOP)
	$(MYLINKPP) $(DEBUG) $(OBJs_DESKTOP) $(MYLDFLAGS_DESKTOP) -o appD
ifeq ($(DEBUG),)
	@strip appD
endif

zigbee_exec: $(OBJs_ZIGBEE)
	$(MYLINKPP) $(DEBUG) $(OBJs_ZIGBEE) $(MYLDFLAGS) -o zigbee
ifeq ($(DEBUG),)
	@strip zigbee
endif

web_exec: $(OBJs_WEB)
	@echo "{{{ SCRIPT }}}" > /tmp/a.js
	$(MYLINKPP_WEB) $(DEBUG) $(OBJs_WEB) $(MYLDFLAGS_WEB) -o app.js
ifeq ($(DEBUG),)
	@cp app.js ../client/src/assets/
endif

$(PRE)%.o : %.c
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_APP)

$(PRE)%_.o : %.c
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS_DESKTOP)

$(PRE)%__.o : %.c
	$(MYCC_WEB) $(DEBUG) $< -o $@ -c $(MYCFLAGS_WEB)

cleanup:
	@rm -f $(OBJs_APP) $(OBJs_DESKTOP) $(OBJs_ZIGBEE) $(OBJs_WEB) app appD app.js zigbee
