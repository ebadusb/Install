# set up stack size directive based on wlink or $(LD) (cc)
# To use:
# set STACKSIZE, then include this file
#ifeq (${LD},wlink)
#Stackmacro=op stack=$(STACKSIZE)k
#else
Stackmacro=-N$(STACKSIZE)k
#endif
