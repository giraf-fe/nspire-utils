# nspire-utils

Simple util library for ndless with more niche stuff that
aren't in the official Ndless SDK.

work in progress, some things may be implemented incorrectly

need help implementing:
 - devices:
   - gpio
   - uart
   - spi
   - i2c
   - keypad controller
   - dma controller
   - lcd
   - sha256
   - 3des
   - adc
   - alladin pmu
   - and probably others...
 - math
   - gemm, gemv, dotprod
   - convolutions
   - fixed point
   - could benefit from armv5te dsp instructions

 - higher level utilities
   - stuff on top of the low level device interfaces
     - stopwatch for the timers
     - sram relocator using mmu
     - faster allocator implementations
     - etc.
