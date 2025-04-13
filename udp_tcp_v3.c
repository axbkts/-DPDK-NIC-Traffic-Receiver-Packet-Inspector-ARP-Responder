#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <rte_eal.h>
#include <rte_mempool.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_arp.h>
#include <inttypes.h> 

#define RX_RING_SIZE 128
#define TX_RING_SIZE 256
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static const struct rte_eth_conf port_conf_default = { 
	.rxmode = { 
		.max_lro_pkt_size = RTE_ETHER_MAX_LEN,
		.offloads = RTE_ETH_RX_OFFLOAD_CHECKSUM, 
		}, 
};		

static void print_ip(uint32_t ip) {
    printf("%u.%u.%u.%u\n",
           (ip >> 24) & 0xFF,
           (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF,
           ip & 0xFF);
}

static void send_arp_reply(struct rte_mbuf *m, struct rte_ether_hdr *eth_hdr, 
                           struct rte_arp_hdr *arp_hdr, uint16_t port) {
    struct rte_mbuf *tx_mbuf = rte_pktmbuf_alloc(m->pool);
    if (!tx_mbuf) {
        printf("Failed to allocate tx_mbuf\n");
        return;
    }

    struct rte_ether_hdr *tx_eth_hdr = rte_pktmbuf_mtod(tx_mbuf, struct rte_ether_hdr *);
    struct rte_arp_hdr *tx_arp_hdr = (struct rte_arp_hdr *)(tx_eth_hdr + 1);

    rte_memcpy(&tx_eth_hdr->dst_addr, &eth_hdr->src_addr, sizeof(struct rte_ether_addr));
    rte_eth_macaddr_get(port, &tx_eth_hdr->src_addr);
    tx_eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_ARP);

    tx_arp_hdr->arp_hardware = rte_cpu_to_be_16(RTE_ARP_HRD_ETHER);
    tx_arp_hdr->arp_protocol = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
    tx_arp_hdr->arp_hlen = RTE_ETHER_ADDR_LEN;
    tx_arp_hdr->arp_plen = sizeof(uint32_t);
    tx_arp_hdr->arp_opcode = rte_cpu_to_be_16(RTE_ARP_OP_REPLY);

    rte_memcpy(&tx_arp_hdr->arp_data.arp_sha, &tx_eth_hdr->src_addr, sizeof(struct rte_ether_addr));
    rte_memcpy(&tx_arp_hdr->arp_data.arp_tha, &arp_hdr->arp_data.arp_sha, sizeof(struct rte_ether_addr));

    tx_arp_hdr->arp_data.arp_sip = arp_hdr->arp_data.arp_tip;
    tx_arp_hdr->arp_data.arp_tip = arp_hdr->arp_data.arp_sip;

    tx_mbuf->pkt_len = tx_mbuf->data_len = sizeof(struct rte_ether_hdr) + sizeof(struct rte_arp_hdr);
    tx_mbuf->nb_segs = 1;
    tx_mbuf->next = NULL;

    const uint16_t nb_tx = rte_eth_tx_burst(port, 0, &tx_mbuf, 1);
    if (nb_tx != 1) {
        rte_pktmbuf_free(tx_mbuf);
        printf("Failed to send ARP reply\n");
    } else {
        printf("ARP reply sent\n");
    }
}


static void lcore_main(void) {
    const uint16_t nb_ports = rte_eth_dev_count_avail();
    uint16_t port;
    printf("Packet capturing on the RX ring:\n");

    while (1) {
        RTE_ETH_FOREACH_DEV(port) {
            struct rte_mbuf *bufs[BURST_SIZE];
            const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);
            if (nb_rx == 0)
                continue;

            for (uint16_t i = 0; i < nb_rx; i++) {
                struct rte_mbuf *m = bufs[i];
                struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);

                printf("Ethernet Header:\n");
                printf("\tDestination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                        eth_hdr->dst_addr.addr_bytes[0], eth_hdr->dst_addr.addr_bytes[1],
                        eth_hdr->dst_addr.addr_bytes[2], eth_hdr->dst_addr.addr_bytes[3],
                        eth_hdr->dst_addr.addr_bytes[4], eth_hdr->dst_addr.addr_bytes[5]);
                printf("\tSource MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                        eth_hdr->src_addr.addr_bytes[0], eth_hdr->src_addr.addr_bytes[1],
                        eth_hdr->src_addr.addr_bytes[2], eth_hdr->src_addr.addr_bytes[3],
                        eth_hdr->src_addr.addr_bytes[4], eth_hdr->src_addr.addr_bytes[5]);

                uint16_t ether_type = rte_be_to_cpu_16(eth_hdr->ether_type);
                printf("EtherType: 0x%04x\n", ether_type);

                if (ether_type == RTE_ETHER_TYPE_IPV4) {
                    struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)((uint8_t *)eth_hdr + sizeof(struct rte_ether_hdr));

                    printf("IPv4 Header:\n");
                    print_ip(rte_be_to_cpu_32(ipv4_hdr->src_addr));
                    print_ip(rte_be_to_cpu_32(ipv4_hdr->dst_addr));

                    if (ipv4_hdr->next_proto_id == IPPROTO_UDP) {
                        struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *)((uint8_t *)ipv4_hdr + sizeof(struct rte_ipv4_hdr));
                        printf("\tUDP packet: Source Port: %u, Destination Port: %u\n",
                               rte_be_to_cpu_16(udp_hdr->src_port), rte_be_to_cpu_16(udp_hdr->dst_port));

                        uint8_t *udp_data = (uint8_t *)udp_hdr + sizeof(struct rte_udp_hdr);
                        uint16_t udp_data_len = rte_be_to_cpu_16(udp_hdr->dgram_len) - sizeof(struct rte_udp_hdr);
                        printf("\tUDP Data (first 16 bytes): ");
                        for (uint16_t j = 0; j < 16 && j < udp_data_len; j++) {
                            printf("%02x ", udp_data[j]);
                        }
                        printf("\n");

                        if (udp_data_len > 16) {
                            printf("\tRemaining UDP Data: ");
                            for (uint16_t j = 16; j < udp_data_len; j++) {
                                printf("%02x ", udp_data[j]);
                            }
                            printf("\n");
                        }
                        
                        char udp_string[udp_data_len + 1];
    			memcpy(udp_string, udp_data, udp_data_len);
    			udp_string[udp_data_len] = '\0';
    			printf("\tUDP Data (as string): %s\n", udp_string);
                    } else {
                        printf("\tOther protocol: %u\n", ipv4_hdr->next_proto_id);
                    }
                } else if (ether_type == RTE_ETHER_TYPE_ARP) {
                    struct rte_arp_hdr *arp_hdr = (struct rte_arp_hdr *)((uint8_t *)eth_hdr + sizeof(struct rte_ether_hdr));
                    printf("Received ARP packet:\n");
                    printf("\tSource IP: ");
                    print_ip(rte_be_to_cpu_32(arp_hdr->arp_data.arp_sip));
                    printf("\tDestination IP: ");
                    print_ip(rte_be_to_cpu_32(arp_hdr->arp_data.arp_tip));
                    
                    if (rte_be_to_cpu_16(arp_hdr->arp_opcode) == RTE_ARP_OP_REQUEST) { 
                    	send_arp_reply(m, eth_hdr, arp_hdr, port); 
                    	}
                    
                    
                } else {
                    printf("Unknown packet type: EtherType=0x%04x\n", ether_type);
                }

                rte_pktmbuf_free(m);
            }
        }
    }
}

int main(int argc, char **argv) {
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "EAL initialization failed\n");
    }

    uint16_t nb_ports = rte_eth_dev_count_avail();
    printf("Available Ethernet ports: %u\n", nb_ports);

    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (mbuf_pool == NULL) {
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
    }
    printf("Memory pool created successfully!\n");

    uint16_t portid;
    RTE_ETH_FOREACH_DEV(portid) {
        if (portid >= nb_ports)
            break;

        if (rte_eth_dev_configure(portid, 1, 1, &port_conf_default) != 0) {
            rte_exit(EXIT_FAILURE, "Cannot configure device: port=%hu\n", portid);
        }

        if (rte_eth_rx_queue_setup(portid, 0, RX_RING_SIZE, rte_eth_dev_socket_id(portid), NULL, mbuf_pool) < 0) {
            rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%d\n", portid, portid);
        }

        if (rte_eth_tx_queue_setup(portid, 0, TX_RING_SIZE, rte_eth_dev_socket_id(portid), NULL) < 0) {
            rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%d\n", portid, portid);
        }

        if (rte_eth_dev_start(portid) < 0) {
            rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%d\n", portid, portid);
        }

        rte_eth_promiscuous_enable(portid);
    }
    printf("Ethernet port(s) configured and started successfully.\n");

    lcore_main();

    rte_eal_cleanup();

    return 0;
}

