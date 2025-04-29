MCU = attiny202
MYPATH = /home/gregoire/tmp-inout/avr-gcc-13.2.0-x64-linux/bin/
MYCC = $(MYPATH)avr-gcc
MYCFLAGS = -Os -I $(MYPATH)/../avr/include

#DEBUG = -g
PRE = /tmp/_screenAvr/

OBJs := $(PRE)main.o

.PHONY : clean

all: main_exec
clean: cleanup

main_exec: $(OBJs)
	$(MYCC) -mmcu=$(MCU) $(DEBUG) -Wl,-Map=main.map $(OBJs) -o main.elf
	$(MYPATH)avr-objcopy -O ihex -j.text -j.data main.elf main.hex
	$(MYPATH)avr-objdump -h -S main.elf > main.lss
	$(MYPATH)avr-size main.elf

$(PRE)%.o : %.c
	@mkdir -p $(PRE)
	$(MYCC) -mmcu=$(MCU) $(DEBUG) $(MYCFLAGS) $< -o $@ -c

cleanup:
	@rm -f $(OBJs)
	@rm -f main.elf main.hex main.lss main.map
