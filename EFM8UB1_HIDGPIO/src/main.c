/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "efm8_device.h"
#include "boot.h"
#include "flash.h"
#include "usb_main.h"

// Holds the current command opcode
static uint8_t opcode;

// Holds reply to the current command
static uint8_t reply;

// ----------------------------------------------------------------------------
// Perform the bootloader verify command.
// ----------------------------------------------------------------------------
void doVerifyCmd(void)
{
}

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup (void)
{
  // Disable the watchdog here
}

SI_SBIT(LED, SFR_P3, 0); // LED

sbit P00 = P0^0;
sbit P01 = P0^1;
sbit P02 = P0^2;
sbit P03 = P0^3;
sbit P04 = P0^4;
sbit P05 = P0^5;
sbit P06 = P0^6;
sbit P07 = P0^7;

sbit P10 = P1^0;
sbit P11 = P1^1;
sbit P12 = P1^2;
sbit P13 = P1^3;
sbit P14 = P1^4;
sbit P15 = P1^5;
sbit P16 = P1^6;
sbit P17 = P1^7;

sbit P20 = P2^0;
sbit P21 = P2^1;
sbit P22 = P2^2;
sbit P23 = P2^3;
sbit P24 = P2^4;
sbit P25 = P2^5;
sbit P26 = P2^6;
sbit P27 = P2^7;

// ----------------------------------------------------------------------------
// Bootloader Mainloop
// ----------------------------------------------------------------------------
void setpin( char pin, char out )
{
  switch( pin )
  {
    case 0:
      P20 = out; break;
    case 1:
      P21 = out; break;
    case 2:
      P22 = out; break;
    case 3:
      P23 = out; break;

    case 4:
      P10 = out; break;
    case 5:
      P11 = out; break;
    case 6:
      P12 = out; break;
    case 7:
      P13 = out; break;
    case 8:
      P14 = out; break;
    case 9:
      P15 = out; break;
    case 10:
      P16 = out; break;
    case 11:
      P17 = out; break;

    case 12:
      P00 = out; break;
    case 13:
      P01 = out; break;
    case 14:
      P02 = out; break;
    case 15:
      P03 = out; break;
    case 16:
      P04 = out; break;
    case 17:
      P05 = out; break;
    case 18:
      P06 = out; break;
    case 19:
      P07 = out; break;
  }
}

unsigned char P0_out = 0, P1_out = 0, P2_out = 0;
char IsInput( char pin )
{
  if( pin < 4 )
  {
      if( P2_out & (1<<pin))
      {
          return 0;
      }
  }
  else if( pin < 12 )
  {
      if( P1_out & (1<<(pin-4)))
      {
          return 0;
      }
  }
  else if( pin < 20 )
  {
      if( P0_out & (1<<(pin-12)))
      {
          return 0;
      }
  }

  return 1;
}

xdata unsigned char pin, inout, outmode, out;
xdata unsigned char P0dat, P1dat, P2dat;
xdata unsigned char P0msk, P1msk, P2msk;
xdata char i;

void main(void)
{
  // Initialize the communication channel and clear the flash keys
  boot_initDevice();
  flash_setKeys(0, 0);
  flash_setBank(0);

  // Loop until a run application command is received
  while (true)
  {
    // Wait for a valid boot record to arrive
    boot_nextRecord();

    LED = 0;

    // Receive the command byte and convert to opcode
    opcode = boot_getByte();
    
    // Assume success - handlers will modify if there is an error
    reply = BOOT_ACK_REPLY;

    // Interpret the command opcode
    switch (opcode)
    {
      case 0x40: // Set channel
        break;

      case 0x41:
        // ADC results share the RAM same as USB RAM
        break;

      case 0x48: // Reboot to bootloader
          *((uint8_t SI_SEG_DATA *)0x00) = 0xA5;
          RSTSRC = RSTSRC_SWRSF__SET | RSTSRC_PORSF__SET;
          break;

      case 0x49: // Get version
          break;

      case 0x50: // Whole settings set
          if( boot_getByte() )
          {
              XBR2 = XBR2_WEAKPUD__PULL_UPS_ENABLED | XBR2_XBARE__ENABLED
                      | XBR2_URT1E__DISABLED | XBR2_URT1RTSE__DISABLED
                      | XBR2_URT1CTSE__DISABLED;
          }
          else
          {
              XBR2 = XBR2_WEAKPUD__PULL_UPS_DISABLED | XBR2_XBARE__ENABLED
                      | XBR2_URT1E__DISABLED | XBR2_URT1RTSE__DISABLED
                      | XBR2_URT1CTSE__DISABLED;
          }
          P0_out = boot_getByte();
          P1_out = boot_getByte();
          P2_out = boot_getByte();
          P0MDOUT = boot_getByte() & (P0_out);
          P1MDOUT = boot_getByte() & (P1_out);
          P2MDOUT = boot_getByte() & (P2_out);
          P0 = boot_getByte() | (~P0_out);
          P1 = boot_getByte() | (~P1_out);
          P2 = boot_getByte() | (~P2_out);
          // return settings immediately
      case 0x51: // Whole settings get
          usb_txBuf[1] = (XBR2&0x80);
          usb_txBuf[2] = P0_out;
          usb_txBuf[3] = P1_out;
          usb_txBuf[4] = P2_out;
          usb_txBuf[5] = P0MDOUT;
          usb_txBuf[6] = P1MDOUT;
          usb_txBuf[7] = P2MDOUT;
          usb_txBuf[8] = P0;
          usb_txBuf[9] = P1;
          usb_txBuf[10] = P2;
          break;

      case 0x52: // single pin mode set
          pin = boot_getByte();
          inout = boot_getByte();   // 0-input, 1-output
          outmode = boot_getByte(); // 0-open drain, 1-pushpull
          out = boot_getByte(); //0-low, 1=high
          if( out ) out = 1;
          if( inout ) // output
          {
            setpin( pin, out );
          }
          else
          {
            setpin( pin, 1 );
          }

          if( pin < 4 )
          {
              if( inout ) // output
              {
                  P2_out |= (1<<pin);
                  if( outmode ) // pushpull
                  {
                      P2MDOUT |= (1<<pin);
                  }
                  else
                  {
                      P2MDOUT &= (~(1<<pin));
                  }
              }
              else
              {
                  P2MDOUT &= (~(1<<pin));
                  P2_out &= (~(1<<pin));
              }
          }
          else if( pin < 12 )
          {
              if( inout ) // output
              {
                  P1_out |= (1<<(pin-4));
                  if( outmode ) // pushpull
                  {
                      P1MDOUT |= (1<<(pin-4));
                  }
                  else
                  {
                      P1MDOUT &= (~(1<<(pin-4)));
                  }
              }
              else
              {
                  P1_out &= (~(1<<(pin-4)));
                  P1MDOUT &= (~(1<<(pin-4)));
              }
          }
          else if( pin < 20 )
          {
              if( inout ) // output
              {
                  P0_out |= (1<<(pin-12));
                  if( outmode ) // pushpull
                  {
                      P0MDOUT |= (1<<(pin-12));
                  }
                  else
                  {
                      P0MDOUT &= (~(1<<(pin-12)));
                  }
              }
              else
              {
                  P0_out &= (~(1<<(pin-12)));
                  P0MDOUT &= (~(1<<(pin-12)));
              }
          }
          break;

      case 0x53: // single pin out set
          pin = boot_getByte();
          out = boot_getByte(); //0-low, 1=high
          if(out) out = 1;
          if( IsInput(pin) ) out = 1;
          setpin( pin, out );
          break;

      case 0x54: // single pin get
          pin = boot_getByte();
          if( pin < 4 )
          {
              usb_txBuf[1] = P2;
          }
          else if( pin < 12 )
          {
              usb_txBuf[1] = P1;
              pin -= 4;
          }
          else if( pin < 20 )
          {
              usb_txBuf[1] = P0;
              pin -= 12;
          }
          usb_txBuf[1] &= (1<<pin);
          if( usb_txBuf[1] ) usb_txBuf[1] = 1;
          break;

      case 0x55: // Pins set
        P0dat = boot_getByte();
        P1dat = boot_getByte();
        P2dat = boot_getByte();
        P0msk = boot_getByte();
        P1msk = boot_getByte();
        P2msk = boot_getByte();

        for( i = 0; i < 8; i++ )
        {
            if( P0msk & (1<<i) )
            {
                if( P0_out & (1<<i) )
                  { setpin( i+12, (P0dat & (1<<i))?1:0); }
                else
                  { setpin( i+12, 1); }
            }
            if( P1msk & (1<<i) )
            {
                if( P1_out & (1<<i) )
                  { setpin( i+4, (P1dat & (1<<i))?1:0); }
                else
                  { setpin( i+4, 1); }
            }
            if( P2msk & (1<<i) )
            {
                if( P2_out & (1<<i) )
                  { setpin( i, (P2dat & (1<<i))?1:0); }
                else
                  { setpin( i, 1); }
            }
        }
//        P0 = boot_getByte() | (~P0_out);
//        P1 = boot_getByte() | (~P1_out);
//        P2 = boot_getByte() | (~P2_out);
        // return status immediately
      case 0x56: // Pins get
        usb_txBuf[1] = P0;
        usb_txBuf[2] = P1;
        usb_txBuf[3] = P2;
        break;
      default:
        // Return bootloader revision for any unrecognized command
        //reply = BL_REVISION;
        break;
    }

    // Reply with the results of the command
    boot_sendReply(reply);
  }
}
