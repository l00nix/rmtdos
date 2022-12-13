# rmtdos

Remotely control DOS-based systems from Linux

Tested on FreeDOS-1.3 in QEMU and on a real Pentium-MMX 166 MHz.

"rmtdos" contains a DOS (FreeDOS or MSDOS or equivilent) TSR that allows a
Linux system on the same LAN to remotely control the DOS system.

## Theory of Operation

There are two main programs, one that runs on DOS (`rmtdos.com`), and the other
that runs on Linux (`rmtdos-client`).  There is also a VGA text mode demo
(`vga_demo.com`) program for testing VGA text video modes and displaying test
patterns.

`rmtdos.com`:

1. Probes for a local
   [PC/TCP packet driver](http://crynwr.com/packet_driver.html) (or you can
   specify the exact packet driver IRQ on the command line).
1. Registers to receive raw Ethernet frames with an
   [EtherType](https://en.wikipedia.org/wiki/EtherType) of **80ab**
   (also configurable on the command line).
1. Hooks interrupts **08** (BIOS clock) and **2f** (DOS multiplex).
1. Goes resident (TSR).
1. Responds to Ethernet packets with the correct EtherType and internal
   packet signatures.
1. Streams contents of VGA text framebuffer (**b800:0000**) to any connected
   clients.
1. Can receive control packets that include keystrokes to be shoved into
   the BIOS keyboard buffer.
1. Run with `-h` to see usage and comment line options.

`rmtdos-client`:

1. The client probes the local LAN by broadcasting a special packet for the same
   EtherType.
1. Presents a cheesy ncurses UI that lists discovered clients, and allows the
   user to select one (just press the key for the client ID, `0`-`9`).
1. Periodically sends a refresher packet to the client the user wants to
   "connect" to.
1. Receives VGA text memory dumps, and renders them via `ncurses`.
1. Run with `-h` to see usage and comment line options.
1. Run with `-u` to uninstall/unload the resident program.

## Usage

On the DOS system, install a PC/TCP packet driver, then run `rmtdos.com`.
It should find the packet driver, initialize, "go resident" and return control
to DOS.

![Installing rmtdos.com](/images/install.png)

On Linux, run the client.  You might need to override the default ethernet
device name.  Sadly, you'll need to run it as root since it needs
to create a raw socket.  `sudo ./rmtdos-client -i br0`.

![Client menu](/images/menu.png)

The displayed columns are:

1. Ordinal id for the DOS system (0 .. n-1)
1. MAC address
1. VGA mode (text modes are 0,1,2,3).
1. Width and Height of the screen.
1. Time since last packet received from the DOS system.

Then select the client that you want to connect.

![Client view](/images/live.png)
![Another demo](/images/defrag.png)

## Future Plans

1. Add mouse support.  DOS has
   [int 33h](https://stanislavs.org/helppc/int_33.html) with lots of support
   for setting the mouse's cursor position.

1. Possibly add remote file transfer capacilities (kind of like TFTP over raw
   Ethernet, maybe with a Linux-side FUSE driver).

## FAQ

Q: Why did you use raw Ethernet packets and not TCP/IP or UDP?

A: Memory overhead.  A primary design criteria was to keep the memory usage
of the resident `rmtdos.com` program as small as possible.  Basically,
there were four options for implementing full TCP/IP support, and each
of them would have added serious developmental delays and RAM costs:

1. [mTCP](https://www.brutman.com/mTCP/) - Would not work in TSR mode
   without significant code changes.  mTCP assumes that it can issue DOS
   file IO requests "whenever", and that is not compatible with a TSR that
   does not want to bother switching out the DOS PSPs and such.

1. Wattcp - Unmaintained, didn't even go there.

1. TSR TCP/IP implementation - It wants 73K of RAM at runtime.

1. Roll my own.  Um, no thanks.

A: Compatibility with other mTCP programs.  mTCP tells the packet driver
that it wants to register for ALL EtherTypes, but it only really uses
"ARP" (0x0806) and "IPV4" (0x0800).  If `rmtdos.com` regsiters for the same,
then the packet driver won't let any mTCP program successfuly run.  Since
`rmtdos.com` uses its own EtherType, both stacks can co-exist on the same
packet driver and not require a packet multiplexor.

Q: Can I build it on DOS?

A: `rmtdos.com` and `vga_demo.com` might build there, but I've not bothered
trying.  Currently, all 3 programs are built on Linux.  The DOS programs
are built using ["dev86"](https://github.com/lkundrak/dev86).

Q: How to I exit the client?

A: `ALT-ESCAPE`.  The client sets ncurses to "raw" mode, so that the typical
termios control characters (ex: `CTRL-C`) do NOT generate signals, but
instead are presented to the client, so that the client can send them to
the DOS end.

Q: How can I view the raw Ethernet traffic (tcpdump filter expression)?

A: `tcpdump 'ether proto 0x80ab'`
A: (wireshark): `"eth.type == 0x80ab"`
