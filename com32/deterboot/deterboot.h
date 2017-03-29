/* -------------------------------------------------------------------------- *
 *
 *   Copyright 2017 Deter-Project - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */


/*
 * deterboot.h
 *
 * Definitions and common structures for deterboot
 */

#include <netinet/in.h>

struct NetInfo
{
  struct in_addr myAddr,
                 bossAddr;
};
int getNetInfo(struct NetInfo *);

struct Question
{
  struct in_addr who;
  unsigned short port, response_port;
  void *what;
  size_t what_size, out_sz;

  struct in_addr me;
  void *response;
  size_t response_size, in_sz;

};
size_t ask(struct Question *q);

static const int QUESTION_OK           = 0x00;
static const int QUESTION_PARTIAL_SEND = 0x01;

#define MAX_BOOT_DATA  512
#define  MAX_BOOT_PATH 256
static const int BIVERSION_CURRENT = 1;


struct boot_info 
{
  short version;
  short	opcode;
  int	status;
  char	data[MAX_BOOT_DATA];
};

static const int BIOPCODE_BOOTWHAT_REQUEST       = 1;
static const int BIOPCODE_BOOTWHAT_REPLY         = 2; /* What to boot reply */
static const int BIOPCODE_BOOTWHAT_ACK           = 3; /* Ack to Reply */
static const int BIOPCODE_BOOTWHAT_ORDER         = 4; /* Unsolicited command */
static const int BIOPCODE_BOOTWHAT_INFO          = 5; /* Request for bootinfo */
static const int BIOPCODE_BOOTWHAT_KEYED_REQUEST = 6; /* Request for bootinfo, with key data stuffed into the data portion */

struct boot_what {
	short	flags;
	short	type;
	union {
		/*
		 * Type is BIBOOTWHAT_TYPE_PART
		 *
		 * Specifies the partition number.
		 */
		int			partition;
		
		/*
		 * Type is BIBOOTWHAT_TYPE_SYSID
		 *
		 * Specifies the PC BIOS filesystem type.
		 */
		int			sysid;
		
		/*
		 * Type is BIBOOTWHAT_TYPE_MB
		 *
		 * Specifies a multiboot kernel pathway suitable for TFTP.
		 */
		struct {
			struct in_addr	tftp_ip;
			char		filename[MAX_BOOT_PATH];
		} mb;

		/*
		 * Type is BIBOOTWHAT_TYPE_MFS
		 *
		 * Specifies network path to MFS (boss:/tftpboot/frisbee)
		 * With no host spec, defaults to bootinfo server IP.
		 */
		char			mfs[MAX_BOOT_PATH];

		/*
		 * Type is BIBOOTWHAT_TYPE_DISKPART
		 *
		 * Specifies the BIOS disk and partition numbers.
		 */
		struct {
			int disk;
			int partition;
		} dp;
	} what;
	/*
	 * Kernel and command line to pass to boot loader or multiboot kernel.
	 */
	char	cmdline[1];
};

static const int BIBOOTWHAT_TYPE_PART     = 1;	/* Boot a partition number */
static const int BIBOOTWHAT_TYPE_SYSID    = 2;	/* Boot a system ID */
static const int BIBOOTWHAT_TYPE_MB       = 3;	/* Boot a multiboot image */
static const int BIBOOTWHAT_TYPE_WAIT     = 4;	/* Wait, no boot until later */
static const int BIBOOTWHAT_TYPE_REBOOT   = 5;	/* Reboot */
static const int BIBOOTWHAT_TYPE_AUTO     = 6;	/* Do a bootinfo query */
static const int BIBOOTWHAT_TYPE_MFS      = 7;	/* Boot an MFS from server:/path */
static const int BIBOOTWHAT_TYPE_RESTART  = 8;	/* Restart ourselves without reset */
static const int BIBOOTWHAT_TYPE_DISKPART = 9; /* Boot a partition from a specific disk */
