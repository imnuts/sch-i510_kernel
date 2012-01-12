/****************************************************************************
**
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR       : Song Wei  			@LDK@
** DESCRIPTION: wtlfota_dpram_cmd.h: debugfs entry points
*/
#ifndef __WTLFOTA_DPRAM_CMD_H__
#define __WTLFOTA_DPRAM_CMD_H__

inline int WRITE_TO_WTLFOTA_DPRAM_VERIFY(u32 dest, void *src, int size);
inline int READ_FROM_WTLFOTA_DPRAM_VERIFY(void *dest, u32 src, int size);
#endif
