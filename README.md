Intel(R) Dynamic Device Personalization Tool
============================================
October 21, 2020

Contents
========
- OVERVIEW
- COMMAND LINE PARAMETERS
- EXAMPLES
- EXIT CODES
- INSTALLATION
- CUSTOMER SUPPORT
- LEGAL


OVERVIEW
========
The Intel(R) Dynamic Device Personalization Tool is a utility for detecting the Dynamic Device Personalization (DDP)
profiles loaded on the following devices:
- Intel(R) Ethernet 800 Series
- Intel(R) Ethernet X710 Series

Dynamic Device Personalization (DDP) allows you to change the packet processing pipeline of a device by applying a profile
package to the device at runtime. Profiles can be used to, for example, add support for new protocols, change existing
protocols, or change default settings. See the Linux ice or i40e driver README for more information.

NOTE: The Intel Dynamic Device Personalization Tool requires the following:
- ice devices: NVM version 2.2 or later
- i40e devices: Linux i40e base driver version 2.7.27 or later

The output of this tool can be sent to the console or to an XML or JSON file.


COMMAND LINE PARAMETERS
=======================
When run with no parameters, the Intel Dynamic Device Personalization Tool displays information about all supported devices.

Usage: ddptool [parameters] [argument]

-a
Display information about all functions for all supported devices. This parameter cannot be used with -f.

-h, --help, -?
Display command line help.

-f FILENAME
Displays information about the profile contained in the specified package file. This parameter cannot be used with -a, -i,
or -s.

-i DEVNAME
Display information for the specified network interface name. Running ddptool without any parameters will provide network
interface names. This parameter cannot be used with -f.

-j [FILENAME]
Output in JSON format to a file. If [FILENAME] is not specified, output is sent to standard output.

-l
Silent mode

-s dddd:bb:ss.f
Display information about the device located at the specified PCI location (all numbers in hex). This parameter cannot be
used with -f.
Where:
  d - domain
  b - bus
  s - slot
  f - function

-v
Display the version of the DDP tool.

-x [FILENAME]
Output in XML format to a file. If [FILENAME] is not specified, output is sent to standard output.


EXAMPLES
========
````
# ddptool

Intel(R) Dynamic Device Personalization Tool
DDPTool version 1.0.1.4
Copyright (C) 2019 - 2020 Intel Corporation.

NIC  DevId D:B:S.F      DevName         TrackId  Version      Name
==== ===== ============ =============== ======== ============ ==============================
01)  1588  0000:01:00.0 enp1s0f0        80000008 1.0.3.0      GTPv1-C/U IPv4/IPv6 payload
02)  1588  0000:01:00.1 enp1s0f1        80000008 1.0.3.0      GTPv1-C/U IPv4/IPv6 payload
03)  1584  0000:03:00.0 enp3s0f2        -        -            Unsupported FW version
04)  1584  0000:05:00.0 enp5s0f3        DE010001 1.0.0.0      MPLSoUDP/GRE tunnels demo
05)  1584  0000:06:00.0 N/A             80000008 1.0.3.0      GTPv1-C/U IPv4/IPv6 payload
06)  1584  0000:07:00.0 enp8s0f4        -        -            No profile loaded
07)  1593  0000:60:00.0 ens261f0        C0000002 1.3.17.0     ICE COMMS Package
08)  1593  0000:60:00.1 ens261f1        C0000002 1.3.17.0     ICE COMMS Package
````

To send the output to the console in JSON format:
````
# ddptool -j

Intel(R) Dynamic Device Personalization Tool
DDPTool version 1.0.1.4
Copyright (C) 2019 - 2020 Intel Corporation.
{
  "DDPInventory": [
    {
      "device": "158B",
      "address": "0000:02:00.0",
      "name": "enp1s0f0",
      "display": "Intel(R) Ethernet Network Adapter XXV710-2",
      "DDPpackage": {
        "track_id": "80000008",
        "version": "1.0.3.0",
        "name": "GTPv1-C/U IPv4/IPv6 payload"
      }
    },
    {
      "device": "1584",
      "address": "0000:86:00.0",
      "display": "Intel(R) Ethernet Network Adapter XL710-Q1",
      "DDPpackage": [
        {
          "track_id": "DE010001",
          "version": "1.0.0.0",
          "name": "MPLSoUDP/GRE tunnels demo"
        },
        {
          "track_id": "DE010002",
          "version": "1.0.0.0",
          "name": "MPLSoUDP/GRE PTYPE demo"
        },
        {
          "track_id": "DE010003",
          "version": "1.0.0.0",
          "name": "MPLSoUDP/GRE decap demo"
        }
      ]
    }
  ]
}
````

To send the output to the ddpout.xml file:
````
# ddptool -x ddpout.xml

The ddpout.xml file will look like the following:

<?xml version="1.0" encoding="UTF-8"?>
<DDPInventory lang="en">
  <Instance device="1589" location="0000:02:00.0" name="enp1s0f0" display="Intel(R) Ethernet Converged Network Adapter
X710-T">
    <DDPpackage track_id="80000008" version="1.0.3.0" name="GTPv1-C/U IPv4/IPv6 payload"></DDPpackage>
  </Instance>
  <Instance device="1589" location="0000:02:00.1" name="enp1s0f1" display="Intel(R) Ethernet Converged Network Adapter
X710-T">
    <DDPpackage track_id="80000008" version="1.0.3.0" name="GTPv1-C/U IPv4/IPv6 payload"></DDPpackage>
  </Instance>
  <Instance device="1589" location="0000:02:00.2" name="enp1s0f2" display="Intel(R) Ethernet Converged Network Adapter
X710-T">
    <DDPpackage track_id="80000008" version="1.0.3.0" name="GTPv1-C/U IPv4/IPv6 payload"></DDPpackage>
  </Instance>
  <Instance device="1589" location="0000:02:00.3" name="enp1s0f3" display="Intel(R) Ethernet Converged Network Adapter
X710-T">
    <DDPpackage track_id="80000008" version="1.0.3.0" name="GTPv1-C/U IPv4/IPv6 payload"></DDPpackage>
  </Instance>
  <Instance device="1589" location="0000:03:00.0" name="enp1s0f4" display="Intel(R) Ethernet Converged Network Adapter
X710-T">
    <DDPpackage track_id="80000008" version="1.0.3.0" name="GTPv1-C/U IPv4/IPv6 payload"></DDPpackage>
  </Instance>
</DDPInventory>
````

-f Examples
-----------
The following are examples of using the -f parameter.
````
# ddptool -f gtk.pkgo

File Name          TrackId      Version      Name
================== ============ ============ ==============================
gtk.pkgo           80000008     1.0.3.0      GTPv1-C/U IPv4/IPv6 payload

# ddptool -f gtk.pkgo -j
{
  "DDPInventory": {
    "file_name": "gtp.pkgo"
    "DDPpackage": {
      "track_id": "80000008",
      "version": "1.0.3.0",
      "name": "GTPv1-C/U IPv4/IPv6 payload"
    }
  }
}

# ddptool -f gtk.pkgo -x
<?xml version="1.0" encoding="UTF-8"?>
<DDPInventory lang="en">
  <Instance file="gtp.pkgo">
    <DDPpackage track_id="80000008" version="1.0.3.0" name="GTPv1-C/U IPv4/IPv6 payload"></DDPpackage>
  </Instance>
</DDPInventory>
````

EXIT CODES
==========
Upon exit, when possible, the Intel Dynamic Device Personalization Tool reports an overall status code to indicate the
results of the operation. In general, a non-zero return code indicates an error occurred during processing.

Value	Description
0		Success
1		Bad command line parameter
2		An Internal error has occurred
3		Insufficient privileges to run the tool
4		No supported adapter found
5		No driver available
6		Unsupported base driver version
7		Cannot communicate with one or more adapters
8		Lack of DDP profiles on all devices
9		Cannot read all information from one or more devices
10		Cannot create output file
11		Cannot find specific devices
12		Cannot parse the DDP package file


INSTALLATION
=============


Installing the tool on Linux*
----------------------------

In order to run this tool on Linux*, the base driver must be installed on the system.


CUSTOMER SUPPORT
================
- Main Intel web support site: http://support.intel.com

- Network products information: http://www.intel.com/network

Intel Wired Networking project hosted by Sourceforge: http://sourceforge.net/projects/e1000


LEGAL / DISCLAIMERS
===================
Copyright (C) 2019 - 2020, Intel Corporation. All rights reserved.

Intel Corporation assumes no responsibility for errors or omissions in this document. Nor does Intel make any commitment to
update the information contained herein.

Intel and the Intel logo are trademarks of Intel Corporation or its subsidiaries in the U.S. and/or other countries.

*Other names and brands may be claimed as the property of others.

This software is furnished under license and may only be used or copied in accordance with the terms of the license. The
information in this manual is furnished for informational use only, is subject to change without notice, and should not be
construed as a commitment by Intel Corporation. Intel Corporation assumes no responsibility or liability for any errors or
inaccuracies that may appear in this document or any software that may be provided in association with this document. Except
as permitted by such license, no part of this document may be reproduced, stored in a retrieval system, or transmitted in
any form or by any means without the express written consent of Intel Corporation.
