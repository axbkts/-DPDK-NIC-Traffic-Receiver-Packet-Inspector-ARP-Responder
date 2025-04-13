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
   - $ git clone https://github.com/axbkts/DPDK-NIC-Traffic-Receiver-Packet-Inspector-ARP-Responder
   - $ cd DPDK-NIC-Traffic-Receiver-Packet-Inspector-ARP-Responder

4. Configure the Build:
   -  Open the Makefile and update the DPDK_DIR variable to point to your local DPDK installation :
      DPDK_DIR ?= /path/to/your/dpdk
      
   -  Ensure that the target configuration is correct by verifying the output of :
      cat $(DPDK_DIR)/build/.config | grep CONFIG_RTE_ARCH

## Building the Project
Use the Makefile provided in the repository:

 - $ make

This command compiles source file into an executable. The Makefile sets the compiler flags and links the necessary DPDK libraries.

## Running the Application
1. Run the Executable:
  $ sudo ./udp_tcp_v3 <dpdk EAL arguments>
  Replace <dpdk EAL arguments> with the necessary EAL parameters (such as core and memory channels) based on your system and network configuration.
2. Monitor Output:
The application will display detailed information about the Ethernet headers, IP addresses, and UDP packet contents.
ARP requests are automatically detected and responded to with an appropriate ARP reply.

## Additional Information
DPDK Documentation: Refer to the official DPDK documentation for in-depth details on API usage and performance tuning.
Customizations: The source code is structured to allow easy extension. You can add additional packet handling logic or integrate support for other protocols as needed.
Troubleshooting: Ensure that your NIC is supported by DPDK and that you have the necessary permissions (e.g., root access) to run the application.

