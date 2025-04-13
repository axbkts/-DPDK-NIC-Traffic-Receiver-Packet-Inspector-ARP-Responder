# DPDK-NIC-Traffic-Receiver-Packet-Inspector-ARP-Responder

## Overview
This high-performance network application is built with the Data Plane Development Kit (DPDK) for real-time NIC traffic capture and packet processing. It inspects Ethernet/IP/UDP packets and automatically replies to ARP requests. The app employs burst-based reception and efficient memory pooling to ensure minimal latency even under heavy network traffic.

## Features
- **NIC Traffic Capture:** Continuously receives network packets directly from the NIC.
- **Packet Inspection:** Logs detailed information from Ethernet and IPv4 headers, including MAC and IP addresses. For UDP packets, it displays source/destination ports and payload content.
- **ARP Handling:** Detects ARP requests and generates appropriate ARP reply packets.
- **Performance Optimization:** Utilizes DPDK's burst-based packet processing and memory pooling for high-throughput, low-latency performance.

## Requirements
- **DPDK:** Version 23.11.2 or later. Adjust `DPDK_DIR` in the Makefile to match your installation path.
- **GCC:** GNU Compiler Collection installed on your system.
- **POSIX-compliant OS:** This project is designed for Unix-like operating systems.
- **Make:** For building the project via the provided Makefile.

## Installation & Setup

1. **Install DPDK:**
   - Follow the official DPDK installation guide for your OS.
   - Ensure that the DPDK environment is set up correctly and that you have built the DPDK libraries.

2. **Clone the Repository:**
   $ git clone https://github.com/axbkts/DPDK-NIC-Traffic-Receiver-Packet-Inspector-ARP-Responder
   $ cd DPDK-NIC-Traffic-Receiver-Packet-Inspector-ARP-Responder

3. Configure the Build:
