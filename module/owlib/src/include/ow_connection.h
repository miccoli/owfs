/*
$Id$
    OW -- One-Wire filesystem
    version 0.4 7/2/2003

    Function naming scheme:
    OW -- Generic call to interaface
    LI -- LINK commands
    L1 -- 2480B commands
    FS -- filesystem commands
    UT -- utility functions

    Written 2003 Paul H Alfille
        Fuse code based on "fusexmp" {GPL} by Miklos Szeredi, mszeredi@inf.bme.hu
        Serial code based on "xt" {GPL} by David Querbach, www.realtime.bc.ca
        in turn based on "miniterm" by Sven Goldt, goldt@math.tu.berlin.de
    GPL license
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    Other portions based on Dallas Semiconductor Public Domain Kit,
    ---------------------------------------------------------------------------
    Copyright (C) 2000 Dallas Semiconductor Corporation, All Rights Reserved.
        Permission is hereby granted, free of charge, to any person obtaining a
        copy of this software and associated documentation files (the "Software"),
        to deal in the Software without restriction, including without limitation
        the rights to use, copy, modify, merge, publish, distribute, sublicense,
        and/or sell copies of the Software, and to permit persons to whom the
        Software is furnished to do so, subject to the following conditions:
        The above copyright notice and this permission notice shall be included
        in all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
    OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
        Except as contained in this notice, the name of Dallas Semiconductor
        shall not be used except as stated in the Dallas Semiconductor
        Branding Policy.
    ---------------------------------------------------------------------------
    Implementation:
    25-05-2003 iButtonLink device
*/

#ifndef OW_CONNECTION_H			/* tedious wrapper */
#define OW_CONNECTION_H

/*
Jan 21, 2007 from the IANA:
owserver        4304/tcp   One-Wire Filesystem Server
owserver        4304/udp   One-Wire Filesystem Server
#                          Paul Alfille <paul.alfille@gmail.com> January 2007


See: http://www.iana.org/assignments/port-numbers
*/
#define DEFAULT_SERVER_PORT       "4304"
#define DEFAULT_FTP_PORT          "21"
#define DEFAULT_HA7_PORT          "80"
#define DEFAULT_ENET_PORT         "80"
#define DEFAULT_LINK_PORT         "10001"
#define DEFAULT_XPORT_PORT        "10001"
#define DEFAULT_ETHERWEATHER_POST "15862"


#include "ow.h"
#include "ow_counters.h"
#include <sys/ioctl.h>
#include "ow_transaction.h"

/* reset types */
#include "ow_reset.h"

/* bus serach */
#include "ow_search.h"

/* connect to a bus master */
#include "ow_detect.h"

/* address for i2c or usb */
#include "ow_parse_address.h"

/* w1 sequence defines */
#include "ow_w1_seq.h"

#if OW_USB						/* conditional inclusion of USB */
/* Special libusb 0.1x search */
#include "ow_usb_cycle.h"
#endif /* OW_USB */

/* large enough for arrays of 2048 elements of ~49 bytes each */
#define MAX_OWSERVER_PROTOCOL_PACKET_SIZE  100050

/* com port fifo info */
/* The UART_FIFO_SIZE defines the amount of bytes that are written before
 * reading the reply. Any positive value should work and 16 is probably low
 * enough to avoid losing bytes in even most extreme situations on all modern
 * UARTs, and values bigger than that will probably work on almost any
 * system... The value affects readout performance asymptotically, value of 1
 * should give equivalent performance with the old implementation.
 *   -- Jari Kirma
 *
 * Note: Each bit sent to the 1-wire net requeires 1 byte to be sent to
 *       the uart.
 */
#define UART_FIFO_SIZE 160

/** USB bulk endpoint FIFO size
  Need one for each for read and write
  This is for alt setting "3" -- 64 bytes, 1msec polling
*/
#define USB_FIFO_EACH 64
#define USB_FIFO_READ 0
#define USB_FIFO_WRITE USB_FIFO_EACH
#define USB_FIFO_SIZE ( USB_FIFO_EACH + USB_FIFO_EACH )
#define HA7_FIFO_SIZE 128
#define W1_FIFO_SIZE 128
#define LINK_FIFO_SIZE UART_FIFO_SIZE
#define HA7E_FIFO_SIZE UART_FIFO_SIZE
#define HA5_FIFO_SIZE UART_FIFO_SIZE
#define LINKE_FIFO_SIZE 1500
#define I2C_FIFO_SIZE 1

#if USB_FIFO_SIZE > UART_FIFO_SIZE
#define MAX_FIFO_SIZE USB_FIFO_SIZE
#else
#define MAX_FIFO_SIZE UART_FIFO_SIZE
#endif

// For forward references
struct connection_in;

/* Debugging interface to showing all bus traffic
 * You need to configure compile with
 * ./configure --enable-owtraffic
 * */
#include "ow_traffic.h"

/* -------------------------------------------- */
/* Interface-specific routines ---------------- */
#include "ow_bus_routines.h"

enum bus_speed {
	bus_speed_slow,
	bus_speed_overdrive,
};

enum bus_flex { bus_no_flex, bus_yes_flex };

/* -------------------------------------------- */
/* bUS-MASTER-specific routines ---------------- */
#include "ow_master.h"

#define CHANGED_USB_SPEED  0x001
#define CHANGED_USB_SLEW   0x002
#define CHANGED_USB_LOW    0x004
#define CHANGED_USB_OFFSET 0x008

//enum server_type { srv_unknown, srv_direct, srv_client, src_
/* Network connection structure */
enum bus_mode {
	bus_unknown = 0,
	bus_serial,
	bus_xport,
	bus_passive,
	bus_usb,
	bus_usb_monitor,
	bus_parallel,
	bus_server,
	bus_zero,
	bus_browse,
	bus_i2c,
	bus_ha7net,
	bus_ha5,
	bus_ha7e,
	bus_enet,
	bus_fake,
	bus_tester,
	bus_mock,
	bus_link,
	bus_elink,
	bus_etherweather,
	bus_bad,
	bus_w1,
	bus_w1_monitor,
};

enum adapter_type {
	adapter_DS9097 = 0,
	adapter_DS1410 = 1,
	adapter_DS9097U2 = 2,
	adapter_DS9097U = 3,
	adapter_LINK = 7,
	adapter_DS9490 = 8,
	adapter_tcp = 9,
	adapter_Bad = 10,
	adapter_LINK_10,
	adapter_LINK_11,
	adapter_LINK_12,
	adapter_LINK_13,
	adapter_LINK_14,
	adapter_LINK_other,
	adapter_LINK_E,
	adapter_DS2482_100,
	adapter_DS2482_800,
	adapter_HA7NET,
	adapter_HA5,
	adapter_HA7E,
	adapter_ENET,
	adapter_EtherWeather,
	adapter_fake,
	adapter_tester,
	adapter_mock,
	adapter_w1,
	adapter_w1_monitor,
	adapter_browse_monitor,
	adapter_xport,
	adapter_usb_monitor,
};

enum e_reconnect {
	reconnect_bad = -1,
	reconnect_ok = 0,
	reconnect_error = 2,
};

enum e_anydevices {
	anydevices_no = 0 ,
	anydevices_yes ,
	anydevices_unknown ,
};	

enum e_bus_stat {
	e_bus_reconnects,
	e_bus_reconnect_errors,
	e_bus_locks,
	e_bus_unlocks,
	e_bus_errors,
	e_bus_resets,
	e_bus_reset_errors,
	e_bus_short_errors,
	e_bus_program_errors,
	e_bus_pullup_errors,
	e_bus_timeouts,
	e_bus_read_errors,
	e_bus_write_errors,
	e_bus_detect_errors,
	e_bus_open_errors,
	e_bus_close_errors,
	e_bus_search_errors1,
	e_bus_search_errors2,
	e_bus_search_errors3,
	e_bus_status_errors,
	e_bus_select_errors,
	e_bus_try_overdrive,
	e_bus_failed_overdrive,
	e_bus_stat_last_marker
};

// flag to pn->selected_connection->branch.sn[0] to force select from root dir
#define BUSPATH_BAD	0xFF

struct connection_in {
	struct connection_in *next;
	INDEX_OR_ERROR index;
	char *name;
	FILE_DESCRIPTOR_OR_PERSISTENT file_descriptor;
	speed_t baud; // baud rate in the form of B9600
	struct termios oldSerialTio;    /*old serial port settings */
	// For adapters that maintain dir-at-once (or dirgulp):
	struct dirblob main;        /* main directory */
	struct dirblob alarm;       /* alarm directory */
#if OW_MT
	pthread_mutex_t bus_mutex;
	pthread_mutex_t dev_mutex;
	void *dev_db;				// dev-lock tree
#endif							/* OW_MT */
	enum e_reconnect reconnect_state;
	struct timeval last_lock;	/* statistics */
	struct timeval last_unlock;

	UINT bus_stat[e_bus_stat_last_marker];

	struct timeval bus_time;

	struct timeval bus_read_time;
	struct timeval bus_write_time;	/* for statistics */

	enum bus_mode busmode;
	struct interface_routines iroutines;
	enum adapter_type Adapter;
	char *adapter_name;
	enum e_anydevices AnyDevices;
	int ExtraReset;				// DS1994/DS2404 might need an extra reset
	size_t default_discard ; // linkhub-telnet escape chars
	enum bus_speed speed;
	enum bus_flex flex ;
	int changed_bus_settings;
	int ds2404_compliance;
	int ProgramAvailable;
	size_t last_root_devs;
	struct buspath branch;		// Branch currently selected

	size_t bundling_length;

	union master_union master;
};

extern struct inbound_control {
	int active ; // how many "bus" entries are currently in linked list
	int next_index ; // increasing sequence number
	struct connection_in * head ; // head of a linked list of "bus" entries
#if OW_MT
	my_rwlock_t monitor_lock; // allow monitor processes
	my_rwlock_t lock; // RW lock of linked list
#endif /* OW_MT */
	int next_fake ; // count of fake buses
	int next_tester ; // count tester buses
	int next_mock ; // count mock buses

	struct connection_in * w1_monitor ;
} Inbound_Control ; // Single global struct -- see ow_connect.c

/* Network connection structure */
struct connection_out {
	struct connection_out *next;
	void (*HandlerRoutine) (FILE_DESCRIPTOR_OR_ERROR file_descriptor);
	char *name;
	char *host;
	char *service;
	int index;
	struct addrinfo *ai;
	struct addrinfo *ai_ok;
	FILE_DESCRIPTOR_OR_ERROR file_descriptor;
	struct {
		char *type;					// for zeroconf
		char *domain;				// for zeroconf
		char *name;					// zeroconf name
	} zero ;
#if OW_MT
	pthread_t tid;
#endif							/* OW_MT */
#if OW_ZERO
	DNSServiceRef sref0;
	DNSServiceRef sref1;
#endif
};

extern struct outbound_control {
	int active ; // how many "bus" entries are currently in linked list
	int next_index ; // increasing sequence number
	struct connection_out * head ; // head of a linked list of "bus" entries
} Outbound_Control ; // Single global struct -- see ow_connect.c

/* This bug-fix/workaround function seem to be fixed now... At least on
 * the platforms I have tested it on... printf() in owserver/src/c/owserver.c
 * returned very strange result on c->busmode before... but not anymore */
enum bus_mode get_busmode(struct connection_in *c);
int BusIsServer(struct connection_in *in);

// mode bit flags for level
#define MODE_NORMAL                    0x00
#define MODE_STRONG5                   0x01
#define MODE_PROGRAM                   0x02
#define MODE_BREAK                     0x04

// 1Wire Bus Speed Setting Constants
#define ONEWIREBUSSPEED_REGULAR        0x00
#define ONEWIREBUSSPEED_FLEXIBLE       0x01	/* Only used for USB adapter */
#define ONEWIREBUSSPEED_OVERDRIVE      0x02

/* Serial port */
void COM_speed(speed_t new_baud, struct connection_in *in);
GOOD_OR_BAD COM_open(struct connection_in *in);
void COM_flush( const struct connection_in *in);
void COM_close(struct connection_in *in);
void COM_break(struct connection_in *in);
GOOD_OR_BAD COM_write( const BYTE * data, size_t length, struct connection_in *connection);
GOOD_OR_BAD COM_read( BYTE * data, size_t length, struct connection_in *connection);
void Slurp( FILE_DESCRIPTOR_OR_ERROR file_descriptor, unsigned long usec ) ;

GOOD_OR_BAD telnet_read(BYTE * buf, const size_t size, const struct parsedname *pn) ;

#define COM_slurp( file_descriptor ) Slurp( file_descriptor, 1000 )
#define TCP_slurp( file_descriptor ) Slurp( file_descriptor, 100000 )

void FreeInAll(void);
void RemoveIn( struct connection_in * conn ) ;

void FreeOutAll(void);
void DelIn(struct connection_in *in);

struct connection_in *AllocIn(const struct connection_in *in);
struct connection_in *LinkIn(struct connection_in *in);
struct connection_in *NewIn(const struct connection_in *in);

void Add_InFlight( GOOD_OR_BAD (*nomatch)(struct connection_in * trial,struct connection_in * existing), struct connection_in * new_in );
void Del_InFlight( GOOD_OR_BAD (*nomatch)(struct connection_in * trial,struct connection_in * existing), struct connection_in * new_in );

struct connection_in *find_connection_in(int nr);
int SetKnownBus( int bus_number, struct parsedname * pn) ;

struct connection_out *NewOut(void);

/* Bonjour registration */
void ZeroConf_Announce(struct connection_out *out);
void OW_Browse(struct connection_in *in);

GOOD_OR_BAD TestConnection(const struct parsedname *pn);

void ZeroAdd(const char * name, const char * type, const char * domain, const char * host, const char * service) ;
void ZeroDel(const char * name, const char * type, const char * domain ) ;

#define STAT_ADD1_BUS( err, in )     STATLOCK; ++((in)->bus_stat[err]) ; STATUNLOCK

#endif							/* OW_CONNECTION_H */
