# rmtdos

Remotely control DOS-based systems from Linux

Tested on FreeDOS-1.3 in QEMU and on a real Pentium-MMX 166 MHz.

"rmtdos" contains a DOS (FreeDOS or MSDOS or equivalent) TSR that allows a
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

## Building

1. Install ["dev86"](https://github.com/lkundrak/dev86), which provides a
   16-bit x86 compiler, assembler, and linker.
1. `make`
1. `sudo make setcap` (optional, see https://stackoverflow.com/a/46466642).
1. Copy `out/rmtdos.com` and `out/vga_demo.com` (optional) to a DOS system.

There are some non-traditional makefile targets that i find handy during
development:

1. `make format` - Reformats the C code w/ clang-format.
1. `make setcap` - Runs the `setcap` command on the client, so enable a
   non-root user to run it without requiring sudo.
1. `make typos` - Runs [typos](https://crates.io/crates/typos) on the source
   files, finding spelling mistakes.
1. `make bcclint` - Horrible hack that attempts to compile the DOS binaries
   with actual GCC (it will fail to produce a binary).  This is useful because
   dev86's compiler, `bcc` is a traditional K&R compiler, and will unhelpfully
   assume function prototypes if you forget to include the proper header.
   Modern `gcc -Wall` can be much more strict and issue all kinds of warnings
   for bad coding practices.

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
"ARP" (0x0806) and "IPV4" (0x0800).  If `rmtdos.com` registers for the same,
then the packet driver won't let any mTCP program successfully run.  Since
`rmtdos.com` uses its own EtherType, both stacks can co-exist on the same
packet driver and not require a packet multiplexor.

Q: Can I build it on DOS?

A: `rmtdos.com` and `vga_demo.com` might build there, but I've not bothered
trying.  Currently, all 3 programs are built on Linux.  The DOS programs
are built using ["dev86"](https://github.com/lkundrak/dev86).  FreeDOS ships
with "dev86" (if it is not already installed, you can install it with the
FreeDOS utility "fdimples").

Q: How to I exit the client?

A: `ALT-ESCAPE`.  The client sets ncurses to "raw" mode, so that the typical
termios control characters (ex: `CTRL-C`) do NOT generate signals, but
instead are presented to the client, so that the client can send them to
the DOS end.

Q: How can I view the raw Ethernet traffic (tcpdump filter expression)?

A: `tcpdump 'ether proto 0x80ab'`

A: (wireshark): `"eth.type == 0x80ab"`

Q: My DOS system does not have a NIC, can I use a serial port?

A: Yes (in theory, I've not tested this).  Connect the DOS system to your
Linux system via "null-modem" cable.  On the DOS side, run the "CSLIP" packet
driver.  On the Linux end, configure "cslip" on the serial TTY device.  This
guide contains a step-by-step walkthrough:

- https://mcmackins.org/stories/dos-slip.html
