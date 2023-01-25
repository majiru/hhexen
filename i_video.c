/* i_video.c */

#include "h2stdinc.h"
#include "h2def.h"

#include "i_system.h"

extern byte *screens;
int DisplayTicker = 0;
int UpdateState = 0;

#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

static int resized;
static int mouseactive;
extern int usemouse;

static Rectangle grabout;
static Point center;

static void kbdproc(void);
static void mouseproc(void);

static uchar cmap[3*256];

static Image *backtile = nil;
static uchar tiledata[4096*3];

static void
catch(void *, char *msg)
{
	/* in case we crash, disable mouse grab */
	if(strncmp(msg, "sys:", 4) == 0)
		mouseactive = 0;
	noted(NDFLT);
}

static void I_UpdateTile(void)
{
	uchar *s, *d, *m;
	uchar *e;

	if(backtile == nil){
		s = W_CacheLumpName("F_022", PU_CACHE);
		d = tiledata;
		e = s + 4096;
		for(; s < e; s++){
			m = &cmap[*s * 3];
			*d++ = m[2];
			*d++ = m[1];
			*d++ = m[0];
		}
		backtile = allocimage(display, Rect(0, 0, 64, 64), RGB24, 1, DNofill);
		loadimage(backtile, backtile->r, tiledata, sizeof tiledata);
		if(backtile == nil)
			sysfatal("allocimage: %r");
	}
	draw(screen, screen->r, backtile, nil, ZP);
}

void I_InitGraphics(void)
{

	notify(catch);

	I_SetPalette ((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE));

	if(initdraw(nil, nil, "hexen") < 0)
		I_Error("I_InitGraphics failed");

	I_UpdateTile();

	center = addpt(screen->r.min, Pt(Dx(screen->r)/2, Dy(screen->r)/2));
	grabout = insetrect(screen->r, Dx(screen->r)/8);

	if(rfork(RFPROC|RFMEM) == 0){
		kbdproc();
		exits(nil);
	}

	if(rfork(RFPROC|RFMEM) == 0){
		mouseproc();
		exits(nil);
	}
}

void I_ShutdownGraphics(void)
{
	closedisplay(display);
}

void I_SetPalette(byte *palette)
{
	uchar *c;

	c = cmap;
	while(c < cmap+3*256)
		*c++ = gammatable[usegamma][*palette++];
}

void I_UpdateNoBlit(void)
{
	// DELETEME?
}

void I_StartFrame(void)
{
}

void I_Update(void)
{
	Image *rowimg;
	Rectangle r;
	int y, scale;
	uchar *s, *e, *d, *m;
	uchar buf[SCREENWIDTH*3*12];

	if(UpdateState == I_NOUPDATE)
		return;

	if(resized){
		resized = 0;
		if(getwindow(display, Refnone) < 0)
			sysfatal("getwindow: %r");

		I_UpdateTile();
		center = addpt(screen->r.min, Pt(Dx(screen->r)/2, Dy(screen->r)/2));
		grabout = insetrect(screen->r, Dx(screen->r)/8);
	}

	scale = Dx(screen->r)/SCREENWIDTH;
	if(scale <= 0)
		scale = 1;
	else if(scale > 12)
		scale = 12;
	if (UpdateState & I_FULLSCRN){
		UpdateState = I_NOUPDATE;
	}

	/* where to draw the scaled row */
	r = rectsubpt(rectaddpt(Rect(0, 0, scale*SCREENWIDTH, scale), center),
		Pt(scale*SCREENWIDTH/2, scale*SCREENHEIGHT/2));

	/* the row image, y-axis gets scaled with repl flag */
	rowimg = allocimage(display, Rect(0, 0, scale*SCREENWIDTH, 1), RGB24, scale > 1, DNofill);
	if(rowimg == nil)
		sysfatal("allocimage: %r");

	s = screens;
	for(y = 0; y < SCREENHEIGHT; y++){
		d = buf;
		e = s + SCREENWIDTH;
		for(; s < e; s++){
			m = &cmap[*s * 3];
			switch(scale){
			case 12:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 11:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 10:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 9:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 8:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 7:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 6:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 5:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 4:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 3:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 2:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			case 1:
				*d++ = m[2];
				*d++ = m[1];
				*d++ = m[0];
			}
		}
		loadimage(rowimg, rowimg->r, buf, d - buf);
		draw(screen, r, rowimg, nil, ZP);
		r.min.y += scale;
		r.max.y += scale;
	}
	freeimage(rowimg);

	flushimage(display, 1);
}

void I_MouseEnable(int on)
{
	static char nocurs[2*4+2*2*16];
	static int fd = -1;

	if(mouseactive == on || !usemouse)
		return;
	if(mouseactive = on){
		if((fd = open("/dev/cursor", ORDWR|OCEXEC)) < 0)
			return;
		write(fd, nocurs, sizeof(nocurs));
	}else if(fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void I_ReadScreen(byte *scr)
{
	memcpy (scr, screens, SCREENWIDTH*SCREENHEIGHT);
}

void I_BeginRead(void)
{
	I_Error("PORTME i_video.c I_BeginRead");
}

void I_EndRead(void)
{
	I_Error("PORTME i_video.c I_EndRead");
}

void I_StartTic(void)
{
}

void I_WaitVBL(int)
{
}


static int
runetokey(Rune r)
{
	switch(r){
	case Kleft:
		return KEY_LEFTARROW;
	case Kright:
		return KEY_RIGHTARROW;
	case Kup:
		return KEY_UPARROW;
	case Kdown:
		return KEY_DOWNARROW;

	case Kshift:
		return KEY_RSHIFT;
	case Kctl:
		return KEY_RCTRL;
	case Kalt:
		return KEY_LALT;
	case Kaltgr:
		return KEY_RALT;

	case Kbs:
		return KEY_BACKSPACE;
	case '\n':
		return KEY_ENTER;
	case Kprint:
		return KEY_PAUSE;

	case KF|1:
	case KF|2:
	case KF|3:
	case KF|4:
	case KF|5:
	case KF|6:
	case KF|7:
	case KF|8:
	case KF|9:
	case KF|10:
	case KF|11:
	case KF|12:
		return KEY_F1+(r-(KF|1));

	default:
		if(r < 0x80)
			return r;
	}
	return 0;
}

static int
keytorune(int key)
{

	switch(key){
	case KEY_LEFTARROW:
		return Kleft;
	case KEY_RIGHTARROW:
		return Kright;
	case KEY_UPARROW:
		return Kup;
	case KEY_DOWNARROW:
		return Kdown;

	case KEY_RSHIFT:
		return Kshift;
	case KEY_RCTRL:
		return Kctl;
	case KEY_LALT:
		return Kalt;

	case KEY_BACKSPACE:
		return Kbs;
	case KEY_ENTER:
		return '\n';
	case KEY_PAUSE:
		return Kprint;

	case KEY_F1:
	case KEY_F2:
	case KEY_F3:
	case KEY_F4:
	case KEY_F5:
	case KEY_F6:
	case KEY_F7:
	case KEY_F8:
	case KEY_F9:
	case KEY_F10:
	case KEY_F11:
	case KEY_F12:
		return (KF|1)+(key-KEY_F1);

	default:
		if(key < 0x80)
			return key;
	}
	return 0;
}

static void
kbdproc(void)
{
	static char keys[MAXKEYS];
	char buf[128], *s, *p;
	int kfd, n;
	Rune r;
	event_t e;
	int i;

	if((kfd = open("/dev/kbd", OREAD)) < 0)
		sysfatal("can't open kbd: %r");

	buf[0] = 0;
	memset(keys, 0, sizeof keys);
	procsetname("hexen kbdproc");
	for(;;){
		n = read(kfd, buf, sizeof(buf)-1);
		if(n <= 0)
			break;
		buf[n] = 0;

		e.data1 = -1;
		e.data2 = -1;
		e.data3 = -1;

		for(p = buf; p - buf < n; p += strlen(p)+1){
			switch(*p){
			case 'c':
				chartorune(&r, p+1);
				if(r){
					e.data1 = r;
					e.type = ev_char;
					H2_PostEvent(&e);
				}
				/* no break */
			default:
				continue;
			case 'k':
				s = p+1;
				while(*s){
					s += chartorune(&r, s);
					if((e.data1 = runetokey(r)) && keys[e.data1] == 0){
						keys[e.data1] = 1;
						e.type = ev_keydown;
						H2_PostEvent(&e);
					}
				}
				break;
			case 'K':
				s = p+1;
				for(i = 0; i < MAXKEYS; i++){
					if(keys[i] == 0)
						continue;
					r = keytorune(i);
					if(!utfrune(s, r)){
						e.data1 = i;
						keys[e.data1] = 0;
						e.type = ev_keyup;
						H2_PostEvent(&e);
					}
				}
				break;
			}
		}
	}
}

static void
mouseproc(void)
{
	int fd, n, nerr;
	Mouse m, om;
	char buf[1+5*12];
	event_t e;

	if((fd = open("/dev/mouse", ORDWR)) < 0)
		sysfatal("can't open mouse: %r");

	memset(&m, 0, sizeof m);
	memset(&om, 0, sizeof om);
	nerr = 0;
	procsetname("hexen mouseproc");
	for(;;){
		n = read(fd, buf, sizeof buf);
		if(n != 1+4*12){
			fprint(2, "mouse: bad count %d not 49: %r\n", n);
			if(n<0 || ++nerr>10)
				break;
			continue;
		}
		nerr = 0;
		switch(buf[0]){
		case 'r':
			resized = 1;
			/* fall through */
		case 'm':
			if(!mouseactive)
				break;

			m.xy.x = atoi(buf+1+0*12);
			m.xy.y = atoi(buf+1+1*12);
			m.buttons = atoi(buf+1+2*12);
			m.msec = atoi(buf+1+3*12);

			if(!ptinrect(m.xy, grabout)){
				fprint(fd, "m%d %d", center.x, center.y);

				m.xy = center;
				om.xy = center;
			}
			
			e.type = ev_mouse;
			e.data1 = m.buttons;
			e.data2 = m.xy.x - om.xy.x;
			e.data3 = om.xy.y - m.xy.y;
			H2_PostEvent(&e);
			om = m;

			break;
		}
	}
}

