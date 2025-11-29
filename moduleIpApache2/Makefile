QUIET = @echo '   CC' $@;
QUIETLINK = @echo 'LK' $@;
ifeq ($(Q)$(q), 0)
QUIET = 
QUIETLINK =
endif
MYCC = $(QUIET)gcc
MYLINKPP = $(QUIETLINK)gcc
#DEBUG = -g -fsanitize=address -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
PRE = ../build/_moduleipapache/

OBJs := $(PRE)module.o

MYCFLAGS = $(CFLAGS) -O2 -flto=auto -ffat-lto-objects -I/usr/include/apache2 -fPIC -DPIC `pkg-config --cflags apr-1`
MYLDFLAGS = -shared -fPIC -DPIC -flto=auto -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-soname -Wl,mod_app_ip.so

.PHONY : clean

all: pre_build module
clean: cleanup

pre_build:
	@mkdir -p $(PRE)

module: $(OBJs)
	$(MYLINKPP) $(DEBUG) $(OBJs) $(MYLDFLAGS) -o mod_app_ip.so
ifeq ($(DEBUG),)
	@strip mod_app_ip.so
endif
ifeq ($(shell [ "$$(uname -m)" = "aarch64" ] || ischroot ; echo $$?), 0)
	@cp mod_app_ip.so /usr/local/modules/apache2/
endif

$(PRE)%.o : %.c
	$(MYCC) $(DEBUG) $< -o $@ -c $(MYCFLAGS)

cleanup:
	@rm -f $(OBJs) mod_app_ip.so
