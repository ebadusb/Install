# set up offset directive based on wlink or $(LD) (cc)
# To use:
# set OFFSET, then include this file
#ifeq (${LD},wlink)
#Offsetmacro=op offset=$(OFFSET)k
#else
Offsetmacro=-@$(OFFSET)k
#endif
