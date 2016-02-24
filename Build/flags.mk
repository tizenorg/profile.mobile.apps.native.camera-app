
DEBUG_OP = -g3 -D_DEBUG 
CPP_DEBUG_OP = -g3 -D_DEBUG 

OPTIMIZATION_OP = -O0 
CPP_OPTIMIZATION_OP = -O0 

COMPILE_FLAGS = $(DEBUG_OP) $(OPTIMIZATION_OP) -Wall -c -fmessage-length=0 

CPP_COMPILE_FLAGS = $(CPP_DEBUG_OP) $(CPP_OPTIMIZATION_OP) -Wall -c -fmessage-length=0 

LINK_FLAGS = -Xlinker --allow-shlib-undefined 

AR_FLAGS = 

EDC_COMPILE_FLAGS = 