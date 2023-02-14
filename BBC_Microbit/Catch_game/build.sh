# Path to tool-chain
ARMGCC=/opt/arm-gcc
export PATH=$PATH:$ARMGCC/bin

# Path to BSP library
BSPDIR=lib

# ARM architecture specific compiler flags
ARCH="-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16"

# Keep every function in a separate section, this allows linker to discard unused ones
OPT="-ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin"

# Compiler flags
CFLAGS="-I$BSPDIR -O3 -g3 -Wall -Werror $ARCH $OPT"

# Compile the source files
echo Compiling main.c
arm-none-eabi-gcc -c $CFLAGS main.c

# Link the object code to form exectuable program
echo Linking MICROBIT.out
arm-none-eabi-ld -T $BSPDIR/bare.ld -Map bare.map \
        --entry ResetISR --gc-sections \
        main.o $BSPDIR/libbsp.a \
        $ARMGCC/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libm.a \
        $ARMGCC/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc.a \
        $ARMGCC/lib/gcc/arm-none-eabi/12.2.1/thumb/v7e-m+fp/hard/libgcc.a \
        -o MICROBIT.out

# Check sizes
arm-none-eabi-size MICROBIT.out

# Generate HEX file to load on the target
arm-none-eabi-objcopy -O ihex MICROBIT.out MICROBIT.hex

# Upload on the target
cmd.exe /c copy MICROBIT.hex E:

# The above command is applicable for a Windows PC. You may change it
# appropriately to on Linux or MacOS. It should be something like:
# cp MICROBIT.hex /media/MICROBIT       # linux
# cp MICROBIT.hex /Volumes/MICROBIT     # mac
