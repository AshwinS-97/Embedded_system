TARGET := MICROBIT
BSPDIR = lib
BSPLIB = libbsp.a

GCCARM=/opt/arm-gcc
CC=arm-none-eabi-gcc
AS=arm-none-eabi-as
AR=arm-none-eabi-ar
LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

LINKER_SCRIPT  := $(BSPDIR)/bare.ld

# Source files common to all targets
OBJS += main.o \

# C flags common to all targets
CFLAGS += -I$(BSPDIR)           # include path
CFLAGS += -O3 -g3               # optimization
CFLAGS += -Wall -Werror         # treat all warnings as errors
CFLAGS += -mcpu=cortex-m4       # CPU architecture
CFLAGS += -mthumb -mabi=aapcs   # code generation options
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16    # floating point unit

# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin

# Linker flags
LDFLAGS := -T $(BSPDIR)/bare.ld # linker command file
LDFLAGS += -Map bare.map        # linker map file
LDFLAGS += --entry ResetISR     # executable entry point
LDFLAGS += --gc-sections        # let linker dump unused sections

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIBS := -L$(BSPDIR) -lbsp \
        $(GCCARM)/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libm.a \
        $(GCCARM)/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc.a \
        $(GCCARM)/lib/gcc/arm-none-eabi/12.2.1/thumb/v7e-m+fp/hard/libgcc.a \

.SUFFIXES: .c .i .S .o .out .hex

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.i:
	$(CC) -E $(CFLAGS) $< > $@

.S.o:
	$(CC) -c $(ASMFLAGS) $<

.out.hex:
	$(OBJCOPY) -O ihex $< $@

all: flash

flash: $(TARGET).hex
	@cmd.exe /c copy $(TARGET).hex E:

$(TARGET).out: $(OBJS) $(BSPDIR)/$(BSPLIB)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(SIZE) $@

$(BSPDIR)/$(BSPLIB):
	cd $(BSPDIR) && make $(BSPLIB)

clean:
	/bin/rm -f $(OBJS) $(TARGET).out
	cd $(BSPDIR) && make bspclean

# Dependencies

$(TARGET).hex: $(TARGET).out
$(TARGET).out: $(OBJS)
