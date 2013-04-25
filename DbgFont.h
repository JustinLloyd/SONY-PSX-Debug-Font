/*
 * Debug print routines for PSX
 *
 *                        DO WHATEVER PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 
 *   0. You can do whatever you want to with the work.
 *   1. You cannot stop anybody from doing whatever they want to with the work.
 *   2. You cannot revoke anybody elses DO WHATEVER PUBLIC LICENSE in the work.
 * 
 *  This program is free software. It comes without any warranty, to the extent
 *  permitted by applicable law. You can redistribute it and/or modify it under
 *  the terms of the DO WHATEVER PUBLIC LICENSE
 *  
 *  Software originally created by Justin Lloyd @ http://otakunozoku.com/
 *
 * File:
 *	DbgFont.h
 * Summary:
 *	Display debug output on PlayStation console.
 *
 * This set of routines prints debugging output on the Playstation
 * screen in a stroke font. It's useful for spitting out assertion information
 * or when you have a crash bug on boot-up and your VRAM contents have not yet
 * set up correctly. It works irrespective of the graphics you have loaded on
 * the VRAM, and does not require the system font.
 *
 * Other notes:
 *
 * Author      	Date		Notes
 * Otaku		97/11/27	Initial implementation.
 */


#ifndef __DBGFONT_H__
#define __DBGFONT_H__
#endif

	#include <stdarg.h>


#ifdef DEBUG

	extern void DbgPSXPrintf(char *string, ...) ;
	extern void DbgFontInit(void) ;
	extern void DbgFontFlush(void) ;
	extern void DbgCycleFontColour(void) ;

#else

	#define DbgFontInit()

	#ifdef CODE_WARRIOR
		#define DbgPSXPrintf()
	#else
		#define DbgPSXPrintf(expr, args...)
	#endif

	#define DbgFontFlush()
	#define DbgCycleFontColour()

#endif