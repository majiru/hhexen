/* i_main.c */

#include "h2stdinc.h"
#include "h2def.h"
#include "soundst.h"
#include "st_start.h"

void main(int argc, char **argv)
{
	myargc = argc; 
	myargv = argv; 
	rfork(RFNOTEG);
	H2_Main ();
} 
