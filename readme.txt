Hardware accelerated n-body simulation on Zedboard.

1. how to compile
==========================
make to compile
make clean to delete object files

2. how to configure
==========================
edit config.h, make clean, make

3. how to use
=========================
compile geninput.c
./geninput world-size num-things  min-mass max-mass> file
./nbody -f file -y years -d days -s seconds [--anim] [-l]

4. how to get hardware acceleration going
==========================
switch to experimental or hwaccl
set up Xillinux
replace the bitstream in the Xillybus demo bundle with accelerator.bit
edit config.h, enable "HWACCL"
make
run
