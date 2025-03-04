// GPIO-WinCmd.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SLABHIDDevice.h"
#pragma comment (lib, "SLABHIDDevice.lib")

#define VID								0x10C4
#define PID								0xEAC9

// HID Report IDs
#define ID_IN_CONTROL					0xFE
#define ID_OUT_CONTROL					0xFD
#define ID_IN_DATA						0x01
#define ID_OUT_DATA						0x02

#define MAX_GPIO_NUM	20
#define SYNC_TIMER		0
#define SYNC_TIME		100

typedef enum PIN_INPUT_OUTPUT
{
	PIN_INPUT =	0,
	PIN_OUTPUT = 1
}PIN_INOUT;

typedef enum PIN_OUTPUT_MODE
{
	PIN_OPEN_DRAIN = 0,
	PIN_PUSH_PULL = 1
}PIN_OUT_MODE;

typedef enum PIN_OUT_VALUE
{
	PIN_LOW	= 0,
	PIN_HIGH = 1
}PIN_VALUE;

HANDLE m_hid;
BYTE report[512], getreport[512];

int default_pull_disable = 1;
int default_inport = 0x0;
int default_portmode = 0x0;
int default_portpin = 0xffffff;

int SetPinMode( int pin, PIN_INOUT output, PIN_OUT_MODE pushpull, PIN_VALUE pinout )
{
	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  5;   // Number
	report[ 3 ] =  0x52;// Command
	report[ 4 ] =  pin;  // pin
	report[ 5 ] =  output;   // 0-input or 1-output mode
	report[ 6 ] =  pushpull;   // 0-Open drain or 1-pushpull
	report[ 7 ] =  pinout;   // 0-low or 1-high
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
	}
	return 0;
}

int SetPin( int pin, PIN_VALUE pinout )
{
	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  3;   // Number
	report[ 3 ] =  0x53;// Command
	report[ 4 ] =  pin;  // pin
	report[ 5 ] =  pinout?1:0;   // 0-low, 1-high
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
	}
	return 0;
}

int GetPin( int pin )
{
	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  2;   // Number
	report[ 3 ] =  0x54;// Command
	report[ 4 ] =  pin;  // pin
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		return( getreport[2] ); // 0 low,1 high
	}
	return -1;
}

int SetAllPins( int inout, int mask )
{
	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  7;   // Number
	report[ 3 ] =  0x55;// Command
	report[ 4 ] =  inout>>12;
	report[ 5 ] =  inout>>4;
	report[ 6 ] =  inout&0x0f;
	report[ 7 ] =  mask>>12;
	report[ 8 ] =  mask>>4;
	report[ 9 ] =  mask&0x0f;
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		return( (getreport[2]<<12) | (getreport[3]<<4) | (getreport[4]&0x0f) ); // 0 low,1 high
	}
	return -1;
}

int GetAllPins( void )
{
	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  1;   // Number
	report[ 3 ] =  0x56;// Command
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		return( (getreport[2]<<12) | (getreport[3]<<4) | (getreport[4]&0x0f) ); // 0 low,1 high
	}
	return -1;
}

int GetSetting( void )
{
	printf("\r\n");
	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  1;   // Number
	report[ 3 ] =  0x51;// Command
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
		HidDevice_Close(m_hid);
		return 1;
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		int tmpinport = (getreport[3]<<12) | (getreport[4] <<4) | getreport[5];
		int tmpportmode = (getreport[6]<<12) | (getreport[7] <<4) | getreport[8];
		int tmpportpin = (getreport[9]<<12) | (getreport[10] <<4) | getreport[11];

		printf("0x%02x, 0x%05x, 0x%05x, 0x%05x", (int)getreport[2], tmpinport, tmpportmode, tmpportpin );

		if( getreport[2] ) // 0-pull up Enable
		{
			printf( " (Pull up Disabled)\r\n" );
		}
		else
		{
			printf( " (Pull up Enabled)\r\n" );
		}

		for( int i = 0; i < MAX_GPIO_NUM; i++ )
		{
			printf("%4d", i);
		}

		printf("\r\n  ");
		for( int i = 0; i < MAX_GPIO_NUM; i++ )
		{
			if( tmpinport & (1<<i) )
			{
				printf(" O  ");
			}
			else
			{
				printf(" I  ");
			}
		}

		printf("\r\n  ");
		for( int i = 0; i < MAX_GPIO_NUM; i++ )
		{
			if( tmpportmode & (1<<i) )
			{
				printf(" P  ");
			}
			else
			{
				printf(" x  ");
			}
		}

		printf("\r\n  ");
		for( int i = 0; i < MAX_GPIO_NUM; i++ )
		{
			if( tmpportpin & (1<<i) )
			{
				printf(" H  ");
			}
			else
			{
				printf(" L  ");
			}
		}
	}

	return 0;
}

#define SPI_SCK		8
#define SPI_MISO	9
#define SPI_MOSI	10
#define SPI_CS		11

void spi_init(void)
{
	SetPinMode(SPI_SCK, PIN_OUTPUT, PIN_PUSH_PULL, PIN_LOW);
	SetPinMode(SPI_MOSI, PIN_OUTPUT, PIN_PUSH_PULL, PIN_HIGH);
	SetPinMode(SPI_MISO, PIN_INPUT, PIN_OPEN_DRAIN, PIN_HIGH);
	SetPinMode(SPI_CS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_HIGH);
}

BYTE spi_transfer_byte(BYTE byte)
{
	unsigned char received_byte = 0;

	for (int i = 0; i < 8; i++)
	{
		if (byte & (1 << (7 - i)))
		{
			SetPin(SPI_MOSI, PIN_HIGH);
		}
		else
		{
			SetPin(SPI_MOSI, PIN_LOW);
		}

		SetPin(SPI_SCK, PIN_HIGH);
		if (GetPin(SPI_MISO))
		{
			received_byte |= (1 << (7 - i));
		}
		SetPin(SPI_SCK, PIN_LOW);
	}

	return received_byte;
}

void main(int argc, char* argv[])
{
	int devnum = HidDevice_GetNumHidDevices(VID, PID);
	BYTE status = HidDevice_Open(&m_hid, 0, VID, PID, MAX_REPORT_REQUEST_XP);
	if( status != HID_DEVICE_SUCCESS )
	{
		printf( "Device open failed" );
		return;
	}
	HidDevice_SetTimeouts( m_hid, 100, 100 ); // get/set timeout

	GetSetting();
	spi_init();
	GetSetting();

	// Read SPI Nor Identification C8 40 16 GD25Q32
	SetPin(SPI_CS, PIN_LOW); // Chip select
	spi_transfer_byte( 0x9f ); // cmd
	BYTE data = spi_transfer_byte( 0x00 );
	printf("\r\nSPI infor MID 0x%02x, ", data );
	data = spi_transfer_byte( 0x00 );
	printf("CID 0x%02x ", data );
	data = spi_transfer_byte( 0x00 );
	printf("0x%02x\r\n", data );
	SetPin(SPI_CS, PIN_HIGH);	// Chip de-select

	//Read Data Bytes
	SetPin(SPI_CS, PIN_LOW);   // Chip select
	int addr = 0;
	spi_transfer_byte( 0x03 ); // cmd
	spi_transfer_byte( addr<<16 ); // 24 bits address
	spi_transfer_byte( addr<<8 );
	spi_transfer_byte( addr );
	for( int i = 0; i < 16; i++ )
	{
		data = spi_transfer_byte( 0x00 ); // read
		printf("0x%02x,", data);
	}

	SetPin(SPI_CS, PIN_HIGH);	// Chip de-select
	HidDevice_Close(m_hid);
	return;
}
