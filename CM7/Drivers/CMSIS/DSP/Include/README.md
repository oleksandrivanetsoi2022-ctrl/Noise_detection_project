This folder is a placeholder for the official CMSIS-DSP package.

To replace the lightweight shim with the real CMSIS-DSP:

1. Download CMSIS (arm-software/CMSIS_5) or install via your package manager.
2. Copy the DSP include files (arm_math.h and other headers) into this folder.
3. Add the corresponding library for Cortex-M7 (libarm_cortexM7lf_math.a) into
   Drivers/CMSIS/DSP/Lib and update the linker search path in Debug/makefile
   or your project settings to include `-L$(PROJECT_DIR)/Drivers/CMSIS/DSP/Lib`
   and `-l:libarm_cortexM7lf_math.a`.
4. Remove or disable `CM7/Core/Src/arm_math_impl.c` if you want to use the
   optimized library implementation.

Note: the project currently uses a lightweight shim to allow compiling
without the full CMSIS-DSP package. Replacing with the official package
will improve performance and accuracy of DSP operations.