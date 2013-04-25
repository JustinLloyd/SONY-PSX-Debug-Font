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
 *	DbgFont.c
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
 *	To use this file you will need to change the lines that reference
 *  g_primitiveOffset, g_primitiveBuffer, and g_orderingTable to your own
 *	primitive table and ordering table variables.
 *
 * To do:
 *  Clean up stroke data so that it uses less lines
 *
 * Author      	Date		Notes
 * Otaku		97/11/27	Initial implementation.
 * Otaku		99/02/23	Changed to a nicer font
 * Otaku		99/03/15	Updated to use newer vsprintf routines
 */


// standard set of includes
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include "DbgFont.h"
#


#define k_DBG_CURSOR_START_X	16
#define k_DBG_CURSOR_START_Y	24
#define k_FONT_CHAR_WIDTH		4
#define k_FONT_CHAR_HEIGHT		6
#define k_FONT_INTERCHAR_SPACE	6
#define k_FONT_INTERLINE_SPACE	4
#define k_MAX_FONT_COLOURS	4


#define MC(sx,sy,ex,ey)	((int)(((sx&0x0F)<<12)|(sy&0x0F)<<8)|((ex&0x0F)<<4)|((ey&0x0F)))
#define GC_SX(n) ((int)((n>>12)&0x0F))
#define GC_SY(n) ((int)((n>>8)&0x0F))
#define GC_EX(n) ((int)((n>>4)&0x0F))
#define GC_EY(n) ((int)((n)&0x0F))

	static	char	s_outputBuffer[2048],
					*s_outputStream ;

	static	int	s_fontColour ;

	static	int	s_colourTable[k_MAX_FONT_COLOURS][3] = { { 0, 0, 0 }, { 255, 255, 255 }, { 40, 200, 40 }, { 200, 40, 40 } } ;

	static	int	chUnknown[]			= {  0 } ;
	static	int	chNum0[]			= {  9, MC(1,0,3,0), MC(3,0,4,2), MC(4,2,4,4), MC(4,4,3,6), MC(3,6,1,6), MC(1,6,0,4), MC(0,4,0,2), MC(0,2,1,0), MC(1,0,3,6) } ;
	static	int	chNum1[]			= {  3, MC(0,4,2,6), MC(2,6,2,0), MC(4,0,0,0) } ;
	static	int	chNum2[]			= {  9, MC(0,5,1,6), MC(1,6,3,6), MC(3,6,4,5), MC(4,5,4,3), MC(4,3,3,2), MC(3,2,1,2), MC(1,2,0,1), MC(0,1,0,0), MC(0,0,4,0) } ;
	static	int	chNum3[]			= { 11, MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,2), MC(4,2,3,3), MC(3,3,1,3), MC(3,3,4,4), MC(4,4,4,5), MC(4,5,3,6), MC(3,6,1,6), MC(1,6,0,5) } ;
	static	int	chNum4[]			= {  3, MC(3,0,3,6), MC(3,6,0,2), MC(0,2,4,2) } ;
	static	int	chNum5[]			= {  8, MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,3), MC(4,3,3,4), MC(3,4,0,4), MC(0,4,0,6), MC(0,6,4,6) } ;
	static	int	chNum6[]			= { 11, MC(4,5,3,6), MC(3,6,1,6), MC(1,6,0,4), MC(0,4,0,1), MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,3), MC(4,3,3,4), MC(3,4,1,4), MC(1,4,0,3) } ;
	static	int	chNum7[]			= {  3, MC(2,0,4,4), MC(4,4,4,6), MC(4,6,0,6) } ;
	static	int	chNum8[]			= { 15, MC(1,3,0,2), MC(0,2,0,1), MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,2), MC(4,2,3,3), MC(3,3,1,3), MC(1,3,0,4), MC(0,4,0,5), MC(0,5,1,6), MC(1,6,3,6), MC(3,6,4,5), MC(4,5,4,4), MC(4,4,3,3) } ;
	static	int	chNum9[]			= { 11, MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,5), MC(4,5,3,6), MC(3,6,1,6), MC(1,6,0,5), MC(0,5,0,3), MC(0,3,1,2), MC(1,2,3,2), MC(3,2,4,3) } ;
	static	int	chAlphaA[]			= {  5, MC(0,0,0,4), MC(0,4,2,6), MC(2,6,4,4), MC(4,4,4,0), MC(0,3,4,3) } ;
	static	int	chAlphaB[]			= { 10, MC(3,3,4,4), MC(4,4,4,5), MC(4,5,3,6), MC(3,6,0,6), MC(0,6,0,0), MC(0,0,3,0), MC(3,0,4,1), MC(4,1,4,2), MC(4,2,3,3), MC(3,3,0,3) } ;
	static	int	chAlphaC[]			= {  7, MC(4,1,3,0), MC(3,0,1,0), MC(1,0,0,2), MC(0,2,0,4), MC(0,4,1,6), MC(1,6,3,6), MC(3,6,4,5) } ;
	static	int	chAlphaD[]			= {  6, MC(0,0,0,6), MC(0,6,2,6), MC(2,6,4,4), MC(4,4,4,2), MC(4,2,2,0), MC(2,0,0,0) } ;
	static	int	chAlphaE[]			= {  6, MC(4,0,0,0), MC(0,0,0,3), MC(0,3,3,3), MC(3,3,0,3), MC(0,3,0,6), MC(0,6,4,6) } ;
	static	int	chAlphaF[]			= {  5, MC(0,0,0,3), MC(0,3,3,3), MC(3,3,0,3), MC(0,3,0,6), MC(0,6,4,6) } ;
	static	int	chAlphaG[]			= {  9, MC(2,3,4,3), MC(4,3,4,1), MC(4,1,3,0), MC(3,0,1,0), MC(1,0,0,1), MC(0,1,0,5), MC(0,5,1,6), MC(1,6,3,6), MC(3,6,4,5) } ;
	static	int	chAlphaH[]			= {  5, MC(0,0,0,6), MC(0,6,0,3), MC(0,3,4,3), MC(4,3,4,6), MC(4,6,4,0) } ;
	static	int	chAlphaI[]			= {  3, MC(0,0,4,0), MC(2,0,2,6), MC(4,6,0,6) } ;
	static	int	chAlphaJ[]			= {  7, MC(0,2,0,1), MC(0,1,1,0), MC(1,0,2,0), MC(2,0,3,1), MC(3,1,3,6), MC(3,6,4,6), MC(4,6,0,6) } ;
	static	int	chAlphaK[]			= {  5, MC(0,0,0,6), MC(0,6,0,3), MC(0,3,4,6), MC(4,6,0,3), MC(0,3,4,0) } ;
	static	int	chAlphaL[]			= {  2, MC(0,6,0,0), MC(0,0,4,0) } ;
	static	int	chAlphaM[]			= {  4, MC(0,0,0,6), MC(0,6,2,3), MC(2,3,4,6), MC(4,6,4,0) } ;
	static	int	chAlphaN[]			= {  3, MC(0,0,0,6), MC(0,6,4,0), MC(4,0,4,6) } ;
	static	int	chAlphaO[]			= {  8, MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,5), MC(4,5,3,6), MC(3,6,1,6), MC(1,6,0,5), MC(0,5,0,1) } ;
	static	int	chAlphaP[]			= {  6, MC(0,0,0,6), MC(0,6,3,6), MC(3,6,4,5), MC(4,5,4,4), MC(4,4,3,3), MC(3,3,0,3) } ;
	static	int	chAlphaQ[]			= { 11, MC(1,3,4,0), MC(4,0,3,1), MC(3,1,4,2), MC(4,2,4,5), MC(4,5,3,6), MC(3,6,1,6), MC(1,6,0,5), MC(0,5,0,1), MC(0,1,1,0), MC(1,0,2,0), MC(2,0,3,1) } ;
	static	int	chAlphaR[]			= {  8, MC(0,0,0,6), MC(0,6,3,6), MC(3,6,4,5), MC(4,5,4,4), MC(4,4,3,3), MC(3,3,0,3), MC(0,3,2,3), MC(2,3,4,0) } ;
	static	int	chAlphaS[]			= { 11, MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,2), MC(4,2,3,3), MC(3,3,1,3), MC(1,3,0,4), MC(0,4,0,5), MC(0,5,1,6), MC(1,6,3,6), MC(3,6,4,5) } ;
	static	int	chAlphaT[]			= {  3, MC(0,6,4,6), MC(4,6,2,6), MC(2,6,2,0) } ;
	static	int	chAlphaU[]			= {  5, MC(0,6,0,1), MC(0,1,1,0), MC(1,0,3,0), MC(3,0,4,1), MC(4,1,4,6) } ;
	static	int	chAlphaV[]			= {  2, MC(0,6,2,0), MC(2,0,4,6) } ;
	static	int	chAlphaW[]			= {  4, MC(0,6,1,0), MC(1,0,2,4), MC(2,4,3,0), MC(3,0,4,6) } ;
	static	int	chAlphaX[]			= {  4, MC(0,0,4,6), MC(4,6,2,3), MC(2,3,0,6), MC(0,6,4,0) } ;
	static	int	chAlphaY[]			= {  3, MC(0,6,2,3), MC(2,3,2,0), MC(2,3,4,6) } ;
	static	int	chAlphaZ[]			= {  3, MC(0,6,4,6), MC(4,6,0,0), MC(0,0,4,0) } ;
	static	int	chUnderline[]		= {  1, MC(0,0,4,0) } ;
	static	int	chBackSlash[]		= {  1, MC(0,6,4,0) } ;
	static	int	chExclamation[]		= {  2, MC(2,6,2,2), MC(2,0,2,0) } ;
	static	int	chDoubleQuote[]		= {  2, MC(1,4,1,6), MC(3,6,3,4) } ;
	static	int	chHash[]			= {  4, MC(1,0,1,6), MC(3,0,3,6), MC(0,2,4,2), MC(0,4,4,4) } ;
	static	int	chDollar[]			= {  8, MC(1,1,3,1), MC(3,1,4,2), MC(4,2,3,3), MC(3,3,1,3), MC(1,3,0,4), MC(0,4,1,5), MC(1,5,3,5), MC(2,0,2,6) } ;
	static	int	chPercentage[]		= {  3, MC(0,0,3,6), MC(0,6,0,6), MC(3,0,3,0) } ;
	static	int	chAmpersand[]		= {  9, MC(4,3,1,0), MC(1,0,0,1), MC(0,1,0,2), MC(0,2,2,4), MC(2,4,2,5), MC(2,5,1,6), MC(1,6,0,5), MC(0,5,0,4), MC(0,4,4,0) } ;
	static	int	chApostrophe[]		= {  2, MC(2,3,3,4), MC(3,4,3,6) } ;
	static	int	chOpenBracket[]		= {  3, MC(3,0,1,2), MC(1,2,1,4), MC(1,4,3,6) } ;
	static	int	chCloseBracket[]	= {  3, MC(1,0,3,2), MC(3,2,3,4), MC(3,4,1,6) } ;
	static	int	chAsterisk[]		= {  3, MC(2,0,2,6), MC(0,1,4,5), MC(0,5,4,1) } ;
	static	int	chPlus[]			= {  2, MC(0,3,4,3), MC(2,1,2,5) } ;
	static	int	chComma[]			= {  2, MC(1,0,2,1), MC(2,1,2,2) } ;
	static	int	chMinus[]			= {  1, MC(0,3,3,3) } ;
	static	int	chPeriod[]			= {  1, MC(2,1,3,1) } ;
	static	int	chForwardSlash[]	= {  1, MC(0,0,4,6) } ;
	static	int	chColon[]			= {  2, MC(2,2,2,2), MC(2,4,2,4) } ;
	static	int	chSemiColon[]		= {  3, MC(2,4,2,4), MC(2,2,2,1), MC(2,1,1,0) } ;
	static	int	chLessThan[]		= {  2, MC(0,0,4,3), MC(4,3,0,6) } ;
	static	int	chEqual[]			= {  2, MC(1,2,3,2), MC(1,4,3,4) } ;
	static	int	chGreaterThan[]		= {  2, MC(4,0,0,3), MC(0,3,4,6) } ;
	static	int	chQuestionMark[]	= {  7, MC(2,0,2,0), MC(2,2,2,2), MC(2,2,4,4), MC(4,4,4,5), MC(4,5,3,6), MC(3,6,1,6), MC(1,6,0,5) } ;
	static	int	chAt[]				= {  9, MC(4,0,1,0), MC(1,0,0,1), MC(0,1,0,5), MC(0,5,1,6), MC(1,6,3,6), MC(3,6,4,5), MC(4,5,4,2), MC(4,2,2,2), MC(2,2,2,4) } ;
	static	int	chOpenSquareBracket[]= { 3, MC(3,0,1,0), MC(1,0,1,6), MC(1,6,3,6) } ;
	static	int	chCloseSquareBracket[]={ 3, MC(1,0,3,0), MC(3,0,3,6), MC(3,6,1,6) } ;
	static	int	chHat[]				= {  2, MC(2,5,3,6), MC(3,6,4,5) } ;
	static	int	chReverseApos[]		= {  1, MC(2,6,3,5) } ;
	static	int	chOpenCurlyBrace[]	= {  6, MC(3,0,2,1), MC(2,1,2,2), MC(2,2,1,3), MC(1,3,2,4), MC(2,4,2,5), MC(2,5,3,6) } ;
	static	int	chVertBar[]			= {  2, MC(2,0,2,2), MC(2,4,2,6 ) } ;
	static	int	chCloseCurlyBrace[]	= {  6, MC(1,0,2,1), MC(2,1,2,2), MC(2,2,3,3), MC(3,3,2,4), MC(2,4,2,5), MC(2,5,1,6) } ;
	static	int	chTilde[]			= {  3, MC(0,5,1,6), MC(1,6,2,5), MC(2,5,3,6) } ;

	static	int	*characterSet[256]={
	chNum0,			chNum1,			chNum2,			chNum3,				chNum4,			chNum5,				chNum6,			chNum7,
	chNum8,			chNum9,			chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chExclamation,	chDoubleQuote,	chHash,				chDollar,		chPercentage,		chAmpersand,	chApostrophe,
	chOpenBracket,	chCloseBracket, chAsterisk,		chPlus,				chComma,		chMinus,			chPeriod,		chForwardSlash,
	chNum0,			chNum1,			chNum2,			chNum3,				chNum4,			chNum5,				chNum6,			chNum7,
	chNum8,			chNum9,			chColon,		chSemiColon,		chGreaterThan,	chEqual,			chLessThan,		chQuestionMark,
	chAt,			chAlphaA,		chAlphaB,		chAlphaC,			chAlphaD,		chAlphaE,			chAlphaF,		chAlphaG,
	chAlphaH,		chAlphaI,		chAlphaJ,		chAlphaK,			chAlphaL,		chAlphaM,			chAlphaN,		chAlphaO,
	chAlphaP,		chAlphaQ,		chAlphaR,		chAlphaS,			chAlphaT,		chAlphaU,			chAlphaV,		chAlphaW,
	chAlphaX,		chAlphaY,		chAlphaZ,		chOpenSquareBracket,chBackSlash,	chCloseSquareBracket,	chHat,		chUnderline,
	chReverseApos,	chAlphaA,		chAlphaB,		chAlphaC,			chAlphaD,		chAlphaE,			chAlphaF,		chAlphaG,
	chAlphaH,		chAlphaI,		chAlphaJ,		chAlphaK,			chAlphaL,		chAlphaM,			chAlphaN,		chAlphaO,
	chAlphaP,		chAlphaQ,		chAlphaR,		chAlphaS,			chAlphaT,		chAlphaU,			chAlphaV,		chAlphaW,
	chAlphaX,		chAlphaY,		chAlphaZ,		chOpenCurlyBrace,	chVertBar,		chCloseCurlyBrace,	chTilde,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown,		chUnknown,		chUnknown,			chUnknown,		chUnknown,			chUnknown,		chUnknown,
	chUnknown,		chUnknown
	} ;


#ifdef DEBUG
void DbgPSXPrintf(char *string, ...)
	{
#if 0
	return ;
#else
	char	s[k_MAX_STRING_LEN] ;

	char *ch ;

	va_list	args ;

	va_start(args, string) ;
	vsprintf(s, string, args) ;
	va_end(args) ;

	// pick up first character in supplied text string
	ch = s ;
	// while not end of text string
	while (*ch)
		{
		// copy character to debug output stream
		*s_outputStream++ = *ch++ ;
		}

	// terminate debug output stream
	*s_outputStream = NULL ;
#endif
	}


void DbgFontInit(void)
	{
	// set output stream to start of output buffer
	s_outputStream = s_outputBuffer ;
	// set initial font colour
	s_fontColour = 3 ;
	}



void DbgCycleFontColour(void)
	{
	s_fontColour++ ;
	if (s_fontColour >= k_MAX_FONT_COLOURS)
		s_fontColour = 0 ;

	}


void DbgFontFlush(void)
	{
	char	*pChar ;

	int		*strokeData ;

	LINE_F2	*strokePoly ;

	int	numLines,
		lineIndex,
		startX, startY, endX, endY,
		cx = k_DBG_CURSOR_START_X, cy = k_DBG_CURSOR_START_Y,
		r, g, b ;

	long charCode ;

	// retrieve font colour from colour table
	r = s_colourTable[s_fontColour][0] ;
	g = s_colourTable[s_fontColour][1] ;
	b = s_colourTable[s_fontColour][2] ;

	// for each character in output debug stream
	for (pChar = s_outputBuffer; *pChar; pChar++)
		{
		// convert current character to uppercase
		charCode = *pChar ;
		// if character is a new line
		if (charCode == '\n')
			{
			// position cursor at start of line
			cx = k_DBG_CURSOR_START_X ;
			// move cursor down one line
			cy += k_FONT_CHAR_HEIGHT + k_FONT_INTERLINE_SPACE ;
			// continue with next character
			continue ;
			}

		// set colour of unknown character

		// locate table that has stroke data for the current character
		strokeData = characterSet[charCode] ;
		// read number of line segments from stroke data table
		numLines = (int)(*strokeData) ;
		// step to next entry in stroke data
		strokeData++ ;
		for (lineIndex=0; lineIndex<numLines; lineIndex++)
			{
			// set start of line x coord to stroke code
			startX = GC_SX(*strokeData) ;
			// set start of line y coord to next stroke code
			startY = GC_SY(*strokeData) ;
			// set end of line x coord to current stroke code
			endX = GC_EX(*strokeData) ;
			// set end of line y coord to current stroke code
			endY = GC_EY(*strokeData) ;
			// step to next set of coords in stroke data
			strokeData += 1 ;
			// reserve space for polygon in primitive buffer
			strokePoly=(LINE_F2 *)&PrimBuffer[cdb][PrimOffset] ;
			// set flat shaded 2-sided polygon type (LINE_F2)
			setLineF2(strokePoly) ;
			// set colour of polygon to previous R, G, B values
			setRGB0(strokePoly, r, g, b) ;
			// set four corners of polygon
			setXY2(strokePoly, cx+startX, cy-startY, cx+endX, cy-endY) ;
			// set transparency of polygon to off
			SetSemiTrans(strokePoly, 0) ;
			// add polygon to PSX ordering table
			addPrim(db[cdb].ot+10, strokePoly) ;
			// increment pointer to primitive buffer
			PrimOffset += sizeof(LINE_F2) ;
			}

		// step cursor column to next character position
		cx += k_FONT_CHAR_WIDTH + k_FONT_INTERCHAR_SPACE ;
		// if cursor column > screen width
		if (cx >= SCREEN_WIDTH - k_FONT_CHAR_WIDTH)
			{
			// move cursor column to first screen column
			cx = k_DBG_CURSOR_START_X ;
			// move cursor row to next screen row
			cy += k_FONT_CHAR_HEIGHT + k_FONT_INTERLINE_SPACE ;
			}

		}

	s_outputStream = s_outputBuffer ;
	*s_outputStream = NULL ;
	}
#endif