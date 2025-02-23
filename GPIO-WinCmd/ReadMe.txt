USBHID GPIO extender.

This is a demo test code of use Pins simulate SPI timing and read a Nor flash MID/CID. This test is just an example of showing how to control Pins in Window programs. It's a start point of using USBHID GPIO extender.
1. The hardware can be ordered from www.taobao.com.
2. The .sls file can be imported to Simplicity Studio to compile or debug. A free Keil C51 license can be ordered from Simplicity Studio. Simplicity Studio can be downloaded from www.silabs.com. (Source code is uploaded too for easy to view.)
3. It can be changed freely if want.
4. It can be upgraded. You can use Silicon Labs AN945 tool upgrade.
5. If bootloader was damaged, need use C2 interface re-program. A hardware debugger will be used, like Silicon Labs Toolstick, UDA-8(UDA-32) or J-Link from www.segger.com. Those debugger can be used to debug EFM8UB10 software too.
6. Anything need help, you can mail to gr_li@163.com, or guanrong.li@silabs.com if it reachable which means I'm still working in Silicon Labs. (^_^)
7. Uploaded Date: 2025-02-23

Connect a SPI Nor flash, pins definition is below.
#define SPI_SCK		8
#define SPI_MISO	9
#define SPI_MOSI	10
#define SPI_CS		11

When you run in Windows command line, you can see the result.
The first Pins information is the default settings when power on.
The second Pins information is the settings after Nor pins config.
SPI Nor Infor is display flash manufacture ID and chip ID. In this test, a GD25Q16 is used.

D:\>GPIO-WinCmd.exe
0x80, 0x00000, 0x00000, 0xffff6 (Pull up Disabled)
   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I
   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x
   L   H   H   L   H   H   H   H   H   H   H   H   H   H   H   H   H   H   H   H
0x80, 0x00d00, 0x00d00, 0xffef6 (Pull up Disabled)
   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
   I   I   I   I   I   I   I   I   O   I   O   O   I   I   I   I   I   I   I   I
   x   x   x   x   x   x   x   x   P   x   P   P   x   x   x   x   x   x   x   x
   L   H   H   L   H   H   H   H   L   H   H   H   H   H   H   H   H   H   H   H
SPI infor MID 0xc8, CID 0x40 0x15
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
D:\>
