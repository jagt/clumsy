WinDivert 2.2: Windows Packet Divert
====================================

1. Introduction
---------------

Windows Packet Divert (WinDivert) is a user-mode packet interception library
for Windows 7, Windows 8 and Windows 10.

WinDivert enables user-mode capturing/modifying/dropping of network packets
sent to/from the Windows network stack.  In summary, WinDivert can:
    - capture network packets
    - filter/drop network packets
    - sniff network packets
    - (re)inject network packets
    - modify network packets
WinDivert can be used to implement user-mode packet filters, sniffers,
firewalls, NATs, VPNs, IDSs, tunneling applications, etc..

WinDivert supports the following features:
    - packet interception, sniffing, or dropping modes
    - support for loopback (localhost) traffic
    - full IPv6 support
    - network layer
    - simple yet powerful API
    - high-level filtering language
    - filter priorities
    - freely available under the terms of the GNU Lesser General Public
      License (LGPLv3)

For more information see doc/windivert.html

2. Architecture
---------------

The basic architecture of WinDivert is as follows:

                              +-----------------+
                              |                 |
                     +------->|    PROGRAM      |--------+
                     |        | (WinDivert.dll) |        |
                     |        +-----------------+        |
                     |                                   | (3) re-injected
                     | (2a) matching packet              |     packet
                     |                                   |
                     |                                   |
 [user mode]         |                                   |
 ....................|...................................|...................
 [kernel mode]       |                                   |
                     |                                   |
                     |                                   |
              +---------------+                          +----------------->
  (1) packet  |               | (2b) non-matching packet
 ------------>| WinDivert.sys |-------------------------------------------->
              |               |
              +---------------+

The WinDivert.sys driver is installed below the Windows network stack.  The
following actions occur:

(1) A new packet enters the network stack and is intercepted by WinDivert.sys
(2a) If the packet matches the PROGRAM-defined filter, it is diverted.  The
    PROGRAM can then read the packet using a call to WinDivertRecv().
(2b) If the packet does not match the filter, the packet continues as normal.
(3) PROGRAM either drops, modifies, or re-injects the packet.  PROGRAM can
    re-inject the (modified) using a call to WinDivertSend().

3. License
----------

WinDivert is dual-licensed under your choice of the GNU Lesser General Public
License (LGPL) Version 3 or the GNU General Public License (GPL) Version 2.
See the LICENSE file for more information.

4. About
--------

WinDivert was written by basil.

For further information, or bug reports, please contact:

    basil@reqrypt.org

The homepage for WinDivert is:

    https://reqrypt.org/windivert.html

The source code for WinDivert is hosted by GitHub at:

    https://github.com/basil00/Divert

