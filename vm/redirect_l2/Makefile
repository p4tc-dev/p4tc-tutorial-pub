CC := gcc
HOSTCC ?= $(CC)
DEFINES += -D_GNU_SOURCE
CCOPTS = -O0 -pipe -ggdb
WFLAGS := -Wall -Wstrict-prototypes  -Wmissing-prototypes
WFLAGS += -Wmissing-declarations -Wold-style-definition -Wformat=2

INCLUDE_DIR=/usr/include/p4tc_ctrl_runt_api/

P4TC_RUNT_INCLUDES=-I$(INCLUDE_DIR)
CFLAGS := $(WFLAGS) $(CCOPTS) $(P4TC_RUNT_INCLUDES) $(DEFINES) -fPIC

LIB_DIR=/usr/lib
LIBP4CTRL=$(LIB_DIR)/libp4tctrl.so
INCLUDES =$(wildcard ${INCLUDE_DIR}/*.h)
REDIRECT_L2=redirect_l2
REDIRECT_L2_SRC=$(REDIRECT_L2).c

all: $(REDIRECT_L2)

$(REDIRECT_L2): $(REDIRECT_L2_SRC) $(LIBP4CTRL)
	$(HOSTCC) $(CFLAGS) -o $@ $(REDIRECT_L2_SRC) -L$(LIB_DIR) -lp4tctrl

.PHONY: clean

clean:
	rm -f $(REDIRECT_L2)
