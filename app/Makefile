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

OBJs := $(PRE)main.o $(PRE)common.o

OBJs_ := $(OBJs)
OBJs_DESKTOP := $(OBJs)

MYCFLAGS = $(CFLAGS) -Wno-deprecated-declarations
MYLDFLAGS = -pthread -lcrypto -lssl -lcurl

MYCFLAGS_DESKTOP = $(MYCFLAGS) -DDESKTOP `pkg-config --cflags gtk+-3.0`
MYLDFLAGS_DESKTOP = $(MYLDFLAGS) `pkg-config --libs gtk+-3.0`

.PHONY : clean

ifeq ($(shell uname -m), aarch64)
all: app_exec
else
all: app_exec desktop_exec
endif
clean: cleanup

app_exec: $(OBJs_)
	$(MYLINKPP) $(DEBUG) $(OBJs_) $(MYLDFLAGS) -o app
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
	@rm -f $(OBJs_) $(OBJs_DESKTOP) app appD
