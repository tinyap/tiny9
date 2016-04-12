/*

This file is distributred under the Lucent Public License, Version 1.02.
See ../../LUCENT 

Copyright © 2000-2009 Lucent Technologies.  All Rights Reserved.
Portions Copyright © 2001-2008 Russ Cox
Portions Copyright © 2008-2009 Google Inc.
*/
#include <u.h>
#include <libc.h>
#include <assert.h>

/*
 * Access local time entries of zoneinfo files.
 * Formats 0 and 2 are supported, and 4-byte timestamps
 * 
 * Copyright © 2008 M. Teichgräber
 * Contributed under the terms of the Lucent Public License 1.02.
 */
#include "zoneinfo.h"

static
struct Zoneinfo
{
	int	timecnt;		/* # of transition times */
	int	typecnt;		/* # of local time types */
	int	charcnt;		/* # of characters of time zone abbreviation strings */

	uchar *ptime;
	uchar *ptype;
	uchar *ptt;
	uchar *pzone;
} z;

/* Hardcoded PST. */
static uchar tzdata[] = {
	0x54, 0x5a, 0x69, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb9,
	0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x9e, 0xa6, 0x48, 0xa0,
	0x9f, 0xbb, 0x15, 0x90, 0xa0, 0x86, 0x2a, 0xa0, 0xa1, 0x9a, 0xf7, 0x90,
	0xcb, 0x89, 0x1a, 0xa0, 0xd2, 0x23, 0xf4, 0x70, 0xd2, 0x61, 0x26, 0x10,
	0xd6, 0xfe, 0x74, 0x20, 0xd8, 0x80, 0xad, 0x90, 0xda, 0xfe, 0xd1, 0xa0,
	0xdb, 0xc0, 0x90, 0x10, 0xdc, 0xde, 0xb3, 0xa0, 0xdd, 0xa9, 0xac, 0x90,
	0xde, 0xbe, 0x95, 0xa0, 0xdf, 0x89, 0x8e, 0x90, 0xe0, 0x9e, 0x77, 0xa0,
	0xe1, 0x69, 0x70, 0x90, 0xe2, 0x7e, 0x59, 0xa0, 0xe3, 0x49, 0x52, 0x90,
	0xe4, 0x5e, 0x3b, 0xa0, 0xe5, 0x29, 0x34, 0x90, 0xe6, 0x47, 0x58, 0x20,
	0xe7, 0x12, 0x51, 0x10, 0xe8, 0x27, 0x3a, 0x20, 0xe8, 0xf2, 0x33, 0x10,
	0xea, 0x07, 0x1c, 0x20, 0xea, 0xd2, 0x15, 0x10, 0xeb, 0xe6, 0xfe, 0x20,
	0xec, 0xb1, 0xf7, 0x10, 0xed, 0xc6, 0xe0, 0x20, 0xee, 0x91, 0xd9, 0x10,
	0xef, 0xaf, 0xfc, 0xa0, 0xf0, 0x71, 0xbb, 0x10, 0xf1, 0x8f, 0xde, 0xa0,
	0xf2, 0x7f, 0xc1, 0x90, 0xf3, 0x6f, 0xc0, 0xa0, 0xf4, 0x5f, 0xa3, 0x90,
	0xf5, 0x4f, 0xa2, 0xa0, 0xf6, 0x3f, 0x85, 0x90, 0xf7, 0x2f, 0x84, 0xa0,
	0xf8, 0x28, 0xa2, 0x10, 0xf9, 0x0f, 0x66, 0xa0, 0xfa, 0x08, 0x84, 0x10,
	0xfa, 0xf8, 0x83, 0x20, 0xfb, 0xe8, 0x66, 0x10, 0xfc, 0xd8, 0x65, 0x20,
	0xfd, 0xc8, 0x48, 0x10, 0xfe, 0xb8, 0x47, 0x20, 0xff, 0xa8, 0x2a, 0x10,
	0x00, 0x98, 0x29, 0x20, 0x01, 0x88, 0x0c, 0x10, 0x02, 0x78, 0x0b, 0x20,
	0x03, 0x71, 0x28, 0x90, 0x04, 0x61, 0x27, 0xa0, 0x05, 0x51, 0x0a, 0x90,
	0x06, 0x41, 0x09, 0xa0, 0x07, 0x30, 0xec, 0x90, 0x07, 0x8d, 0x43, 0xa0,
	0x09, 0x10, 0xce, 0x90, 0x09, 0xad, 0xbf, 0x20, 0x0a, 0xf0, 0xb0, 0x90,
	0x0b, 0xe0, 0xaf, 0xa0, 0x0c, 0xd9, 0xcd, 0x10, 0x0d, 0xc0, 0x91, 0xa0,
	0x0e, 0xb9, 0xaf, 0x10, 0x0f, 0xa9, 0xae, 0x20, 0x10, 0x99, 0x91, 0x10,
	0x11, 0x89, 0x90, 0x20, 0x12, 0x79, 0x73, 0x10, 0x13, 0x69, 0x72, 0x20,
	0x14, 0x59, 0x55, 0x10, 0x15, 0x49, 0x54, 0x20, 0x16, 0x39, 0x37, 0x10,
	0x17, 0x29, 0x36, 0x20, 0x18, 0x22, 0x53, 0x90, 0x19, 0x09, 0x18, 0x20,
	0x1a, 0x02, 0x35, 0x90, 0x1a, 0xf2, 0x34, 0xa0, 0x1b, 0xe2, 0x17, 0x90,
	0x1c, 0xd2, 0x16, 0xa0, 0x1d, 0xc1, 0xf9, 0x90, 0x1e, 0xb1, 0xf8, 0xa0,
	0x1f, 0xa1, 0xdb, 0x90, 0x20, 0x76, 0x2b, 0x20, 0x21, 0x81, 0xbd, 0x90,
	0x22, 0x56, 0x0d, 0x20, 0x23, 0x6a, 0xda, 0x10, 0x24, 0x35, 0xef, 0x20,
	0x25, 0x4a, 0xbc, 0x10, 0x26, 0x15, 0xd1, 0x20, 0x27, 0x2a, 0x9e, 0x10,
	0x27, 0xfe, 0xed, 0xa0, 0x29, 0x0a, 0x80, 0x10, 0x29, 0xde, 0xcf, 0xa0,
	0x2a, 0xea, 0x62, 0x10, 0x2b, 0xbe, 0xb1, 0xa0, 0x2c, 0xd3, 0x7e, 0x90,
	0x2d, 0x9e, 0x93, 0xa0, 0x2e, 0xb3, 0x60, 0x90, 0x2f, 0x7e, 0x75, 0xa0,
	0x30, 0x93, 0x42, 0x90, 0x31, 0x67, 0x92, 0x20, 0x32, 0x73, 0x24, 0x90,
	0x33, 0x47, 0x74, 0x20, 0x34, 0x53, 0x06, 0x90, 0x35, 0x27, 0x56, 0x20,
	0x36, 0x32, 0xe8, 0x90, 0x37, 0x07, 0x38, 0x20, 0x38, 0x1c, 0x05, 0x10,
	0x38, 0xe7, 0x1a, 0x20, 0x39, 0xfb, 0xe7, 0x10, 0x3a, 0xc6, 0xfc, 0x20,
	0x3b, 0xdb, 0xc9, 0x10, 0x3c, 0xb0, 0x18, 0xa0, 0x3d, 0xbb, 0xab, 0x10,
	0x3e, 0x8f, 0xfa, 0xa0, 0x3f, 0x9b, 0x8d, 0x10, 0x40, 0x6f, 0xdc, 0xa0,
	0x41, 0x84, 0xa9, 0x90, 0x42, 0x4f, 0xbe, 0xa0, 0x43, 0x64, 0x8b, 0x90,
	0x44, 0x2f, 0xa0, 0xa0, 0x45, 0x44, 0x6d, 0x90, 0x45, 0xf3, 0xd3, 0x20,
	0x47, 0x2d, 0x8a, 0x10, 0x47, 0xd3, 0xb5, 0x20, 0x49, 0x0d, 0x6c, 0x10,
	0x49, 0xb3, 0x97, 0x20, 0x4a, 0xed, 0x4e, 0x10, 0x4b, 0x9c, 0xb3, 0xa0,
	0x4c, 0xd6, 0x6a, 0x90, 0x4d, 0x7c, 0x95, 0xa0, 0x4e, 0xb6, 0x4c, 0x90,
	0x4f, 0x5c, 0x77, 0xa0, 0x50, 0x96, 0x2e, 0x90, 0x51, 0x3c, 0x59, 0xa0,
	0x52, 0x76, 0x10, 0x90, 0x53, 0x1c, 0x3b, 0xa0, 0x54, 0x55, 0xf2, 0x90,
	0x54, 0xfc, 0x1d, 0xa0, 0x56, 0x35, 0xd4, 0x90, 0x56, 0xe5, 0x3a, 0x20,
	0x58, 0x1e, 0xf1, 0x10, 0x58, 0xc5, 0x1c, 0x20, 0x59, 0xfe, 0xd3, 0x10,
	0x5a, 0xa4, 0xfe, 0x20, 0x5b, 0xde, 0xb5, 0x10, 0x5c, 0x84, 0xe0, 0x20,
	0x5d, 0xbe, 0x97, 0x10, 0x5e, 0x64, 0xc2, 0x20, 0x5f, 0x9e, 0x79, 0x10,
	0x60, 0x4d, 0xde, 0xa0, 0x61, 0x87, 0x95, 0x90, 0x62, 0x2d, 0xc0, 0xa0,
	0x63, 0x67, 0x77, 0x90, 0x64, 0x0d, 0xa2, 0xa0, 0x65, 0x47, 0x59, 0x90,
	0x65, 0xed, 0x84, 0xa0, 0x67, 0x27, 0x3b, 0x90, 0x67, 0xcd, 0x66, 0xa0,
	0x69, 0x07, 0x1d, 0x90, 0x69, 0xad, 0x48, 0xa0, 0x6a, 0xe6, 0xff, 0x90,
	0x6b, 0x96, 0x65, 0x20, 0x6c, 0xd0, 0x1c, 0x10, 0x6d, 0x76, 0x47, 0x20,
	0x6e, 0xaf, 0xfe, 0x10, 0x6f, 0x56, 0x29, 0x20, 0x70, 0x8f, 0xe0, 0x10,
	0x71, 0x36, 0x0b, 0x20, 0x72, 0x6f, 0xc2, 0x10, 0x73, 0x15, 0xed, 0x20,
	0x74, 0x4f, 0xa4, 0x10, 0x74, 0xff, 0x09, 0xa0, 0x76, 0x38, 0xc0, 0x90,
	0x76, 0xde, 0xeb, 0xa0, 0x78, 0x18, 0xa2, 0x90, 0x78, 0xbe, 0xcd, 0xa0,
	0x79, 0xf8, 0x84, 0x90, 0x7a, 0x9e, 0xaf, 0xa0, 0x7b, 0xd8, 0x66, 0x90,
	0x7c, 0x7e, 0x91, 0xa0, 0x7d, 0xb8, 0x48, 0x90, 0x7e, 0x5e, 0x73, 0xa0,
	0x7f, 0x98, 0x2a, 0x90, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0xff, 0xff, 0x9d,
	0x90, 0x01, 0x00, 0xff, 0xff, 0x8f, 0x80, 0x00, 0x04, 0xff, 0xff, 0x9d,
	0x90, 0x01, 0x08, 0xff, 0xff, 0x9d, 0x90, 0x01, 0x0c, 0x50, 0x44, 0x54,
	0x00, 0x50, 0x53, 0x54, 0x00, 0x50, 0x57, 0x54, 0x00, 0x50, 0x50, 0x54,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01
};

static uint tzinit = 0;

static
long
get4(uchar *p)
{
	return (p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
}

enum {
	TTinfosz	= 4+1+1,
};

static
int
parsehead(void)
{
	uchar *p;
	int	ver;

	ver = tzdata[4];
	if (ver!=0)
	if (ver!='2')
		return -1;

	p = tzdata + 4 + 1 + 15;

	z.timecnt = get4(p+3*4);
	z.typecnt = get4(p+4*4);
	if (z.typecnt==0)
		return -1;
	z.charcnt = get4(p+5*4);
	z.ptime = p+6*4;
	z.ptype = z.ptime + z.timecnt*4;
	z.ptt = z.ptype + z.timecnt;
	z.pzone = z.ptt + z.typecnt*TTinfosz;
	return 0;
}

static
void
ttinfo(Tinfo *ti, int tti)
{
	uchar *p;
	int	i;

	i = z.ptype[tti];
	assert(i<z.typecnt);
	p = z.ptt + i*TTinfosz;
	ti->tzoff = get4(p);
	ti->dlflag = p[4];
	assert(p[5]<z.charcnt);
	ti->zone = (char*)z.pzone + p[5];
}

static
void
readtimezone(void)
{
	if (parsehead()==-1)
		z.timecnt = 0;
	else
		tzinit = 1;
}

static
tlong
gett4(uchar *p)
{
	long l;

	l = get4(p);
	if (l<0)
		return 0;
	return l;
}
int
zonetinfo(Tinfo *ti, int i)
{
	if (!tzinit)
		readtimezone();
	if (i<0 || i>=z.timecnt)
		return -1;
	ti->t = gett4(z.ptime + 4*i);
	ttinfo(ti, i);
	return i;
}

int
zonelookuptinfo(Tinfo *ti, tlong t)
{
	uchar *p;
	int	i;
	tlong	oldtt, tt;

	if (!tzinit)
		readtimezone();
	oldtt = 0;
	p = z.ptime;
	for (i=0; i<z.timecnt; i++) {
		tt = gett4(p);
		if (t<tt)
			break;
		oldtt = tt;
		p += 4;
	}
	if (i>0) {
		ttinfo(ti, i-1);
		ti->t = oldtt;
//		fprint(2, "t:%ld off:%d dflag:%d %s\n", (long)ti->t, ti->tzoff, ti->dlflag, ti->zone);
		return i-1;
	}
	return -1;
}

void
zonedump(int fd)
{
	int	i;
	uchar *p;
	tlong t;
	Tinfo ti;

	if (!tzinit)
		readtimezone();
	p = z.ptime;
	for (i=0; i<z.timecnt; i++) {
		t = gett4(p);
		ttinfo(&ti, i);
		fprint(fd, "%ld\t%d\t%d\t%s\n", (long)t, ti.tzoff, ti.dlflag, ti.zone);
		p += 4;
	}
}
