DPDK_DIR ?= /../dpdk-stable-23.11.2 # your dpdk directory

RTE_SDK = $(DPDK_DIR)
RTE_TARGET = $(shell cat $(RTE_SDK)/build/.config | grep CONFIG_RTE_ARCH | cut -d'=' -f2 | tr -d '"')

CC = gcc
CFLAGS = -O3 -I$(RTE_SDK)/$(RTE_TARGET)/include -march=native -mtune=native -funroll-loops
LDFLAGS = -L$(RTE_SDK)/$(RTE_TARGET)/lib -lrte_eal -lrte_ethdev -lrte_mbuf -lrte_mempool -lrte_ring -pthread

TARGET = udp_tcp_v3
SRCS = udp_tcp_v3.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

