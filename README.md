# p4io-mdxfdrv

## What is this?
Uinput driver for DanceDanceRevolution White cabinet controls. This will make stock DDR white cab hardware (pads, front buttons and test/service buttons) be recognized as 3 gamepads on Linux systems.

Only tested with ADE-704A systems. If you have ADE-6291 system that you wish to test this on, please let me know how it went!

This driver is set to run in polling mode by default; which means it behaves slightly differently to the official game (for the better technically), and apparently it may stop the aftermarket pad light board from working. You can use `-DMDXF_AUTOGET=1` flag on CMake to build a driver compatible to it, though it may cause polling issues.

## How do I use this?
Run the `mdxfuinput` binary with a path to MDXF serial device (should be `/dev/ttyS1` on stock systems). Please ensure that you have the right permission for uinput and reading tty devices.

## TODO list
* Implement ddrio interface on windows
* Implement lighting
* Improve the yee-yee ass thread interface

## Known Issues
* ACIO receive failing after running for a long time under AUTOGET mode

## Special Thanks to
* The team behind [Bemanitools](https://github.com/djhackersdev/bemanitools): This project is LARGELY based on your work and research!
* [Zenith Arcade](https://zenitharcade.com/) for being my autism bouncy castle
* DinsFire64 for helpful pointers along the journey
* Amazing ITG players from Vancouver local scene for chiming in and testing the driver!

## Nerd section
### How does this work?
DDR White cab IO is split into 2 parts; p4io, which handles lightings and inputs present on the cab, and mdxf, which handles foot panels.

P4IO is basically a USB device that exposes 2-way bulk transport endpoint and 1-way interrupt transport endpoint. bulk transport is used to control lights, and interrupt transfer gives you 16 byte data of button status.
This internals were observed from p4io.sys, which basically wraps the usb transports nicely with IOCTLs.
Then, the protocol has been reimplemented in libusb to make it cross-platform on user-mode (because I'm too much of a coward to write a kernel driver yet!)

MDXF is where things get a bit different. It is an ACIO device, where ACIO is the proprietary serial protocol konami uses.
While bemanitools has a code for driving acio devices and even some of the commands for MDXF, it doesn't actually have the MDXF specific driver code.
The command scheme is very similar to other IOs that have been implemented though, so I was able to whip something out by referring to PANB driver codes and serial comm dumps.

Additionally, I had the extra constraint in the project; *that it must run on Linux.* While the P4IO code, implemented with libusb, is naturally cross-platform, bemanitools driver code for ACIO is windows-specific.
Thankfully it is very well structed and modularized (shoutouts to DJH for a beautiful codebase), and I was able to reimplemented just the OS-dependent part of it, making it to build and run on Linux natively with some (very many) hacks.

On the note of actually receiving the data using ACIO, the IO has an AUTO\_GET command which instructs the board to start sending the poll data automatically.
The game uses this command by default, so I originally wrote the code utilizing it.
Unfortunately, that means Konami has for some reason decided that it is a good idea to make the IO board spam the pad status at 115200 baudrate, and bemanitools' ACIO implementation being
not as efficient with just having an absurd amount of data in general, it will easily fill up the serial buffer and aciodrv will start to receive corrupt packets.
This can be relieved a bit by running aciodrv code on a separate thread, but under load (remember, 704a is a potato) and prolonged usage it happens regardless.

Thankfully, it is apparently still possible to just send the poll command to the board without AUTO\_GET, which allows you to poll at your own speed and solves the problem at a cost of (apparently) not being able to use third-party pad lights.

### How do I build it?
If building for Linux, just using CMake and your favourite compiler should do the job. Be sure to install `libevdev-devel` on fedora or any equivalent dev package on your system of chice.  
Additionally, you can add `-DDEBUG_BUILD=1` for including debug symbols and adding some debug messages, in case you need troubleshooting.

If you do need to use AUTO\_GET command for some reason, building with `-DMDXF_AUTOGET=1` should enable the old autoget codes. It is playable, but be aware that the buffer overflow still happens and you may experience subtle polling issues or input latency drifting.

This actually can build for Windows as well! Though it only compiles the test application. add `-DTARGET_WIN=1` and compile with Mingw64.
