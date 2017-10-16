/* ======================================================================
 * IMPROC: Image Processing Software Package
 * Copyright (C) 2002 by George Wolberg
 *
 * UI.h - IMPROC user interface header file
 *
 * Written by: George Wolberg, Ming-Chao Chiang and Hadi Fadaifard, 2002
 * ======================================================================
 */



/* ----------------------------------------------------------------------
 * Standard Windows include files
 */

#include <windows.h>

/* ----------------------------------------------------------------------
 * Constants
 */
//#define TRUECOLOR	(VisClass==TrueColor || VisClass==DirectColor)
#define TRUECOLOR			1
#define MXWINDOW			64
#define MXPOPUP				6
#define BUFSZ				128
#define WM_BORDER_WIDTH		2
#define BORDER_WIDTH		1
#define SPACE				2
#define XTBAR_HEIGHT		21
#define ITBAR_HEIGHT		18



typedef		char	*String;
/*-----------------------------VIRTUAL KEYS------------------

*/

#define XK_0	'0'
#define XK_1	'1'
#define XK_2	'2'
#define XK_3	'3'
#define XK_4	'4'
#define XK_5	'5'
#define XK_6	'6'
#define XK_7	'7'
#define XK_8	'8'
#define XK_9	'9'

#define XK_a	'a'
#define XK_b	'b'
#define XK_c	'c'
#define XK_d	'd'
#define XK_e	'e'
#define XK_f	'f'
#define XK_g	'g'
#define XK_h	'h'
#define XK_i	'i'
#define XK_j	'j'
#define XK_k	'k'
#define XK_l	'l'
#define XK_m	'm'
#define XK_n	'n'
#define XK_o	'o'
#define XK_p	'p'
#define XK_q	'q'
#define XK_r	'r'
#define XK_s	's'
#define XK_t	't'
#define XK_u	'u'
#define XK_v	'v'
#define XK_w	'w'
#define XK_x	'x'
#define XK_y	'y'
#define XK_z	'z'

#define XK_A	'A'
#define XK_B	'B'
#define XK_C	'C'
#define XK_D	'D'
#define XK_E	'E'
#define XK_F	'F'
#define XK_G	'G'
#define XK_H	'H'
#define XK_I	'I'
#define XK_J	'J'
#define XK_K	'K'
#define XK_L	'L'
#define XK_M	'M'
#define XK_N	'N'
#define XK_O	'O'
#define XK_P	'P'
#define XK_Q	'Q'
#define XK_R	'R'
#define XK_S	'S'
#define XK_T	'T'
#define XK_U	'U'
#define XK_V	'V'
#define XK_W	'W'
#define XK_X	'X'
#define XK_Y	'Y'
#define XK_Z	'Z'

#define XK_plus	 '+'
#define XK_equal '='
#define XK_minus '-'
#define XK_space ' '

#define XK_Control_L	VK_CONTROL
#define XK_Control_R	VK_CONTROL
#define XK_Escape		VK_ESCAPE
#define XK_Return		VK_RETURN
#define XK_Delete		VK_DELETE
#define	XK_BackSpace	VK_BACK

#define ControlMask		1
#define ShiftMask		2
#define Mod1Mask		4



#define AllocAll		1
#define AllocNone		0
#define INTERACTIVE		TRUE
#define IMPROC			1


/*---------- The following definitions and Structures are added 
			 for compatibility with Unix programs	------------------------------------*/

#define Time			DWORD
#define KeySym			ushort
#define XComposeStatus	int

/*	Event Types	*/
enum xevent_types{
	NoMessage,
	Expose,
	EnterNotify,
	LeaveNotify,
	ButtonPress,
	ButtonRelease,
	KeyPress,
	KeyRelease,
	MotionNotify
};

enum button_types{
	Button1,
	Button2,
	Button3
};

typedef struct	{
	int type;
//	unsigned long serial;	
//	bool send_event;
	HWND window;
	Time time;
	int x,y;

	int x_root, y_root;
	unsigned int state;
	unsigned int button;
}XButtonEvent;
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct {
	
	int type;
//	unsigned long serial;	
//	bool send_event;
	HWND window;
	Time time;
	int x,y;

	int x_root, y_root;
	unsigned int state;
	unsigned int keycode;
	unsigned int nscan;
}XKeyEvent;
typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;


typedef struct {
	
	int type;
//	unsigned long serial;	
//	bool send_event;
	HWND window;
	Time time;
	int x,y;

	int x_root, y_root;
	unsigned int state;
	char is_hint;
}XMotionEvent;
typedef XMotionEvent XPointerMovedEvent;

typedef struct {

	int				type;
//	unsigned long	serial;
//	bool			send_event;
	HWND window;
}XAnyEvent;

typedef union _XEvent{

	int				type;
	XAnyEvent		xany;
	XButtonEvent	xbutton;
	XKeyEvent		xkey;
	XMotionEvent	xmotion;
}XEvent;

typedef struct _COLOR{
	uchar red;
	uchar green;
	uchar blue;
}XColor;


/* -------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
 * Menu data structure.
 */
typedef struct menu {
	int		 type;		/* see below for the details	*/
	void		*name;		/* PFCP or label/cmd/menu name	*/
	void		*vptr;		/* PFV, submenu, or slider ptr	*/
	int		 key;		/* accelerator key: XK_*	*/

	/******** the following fields are for internal use only ********/
	HWND		pane;		/* label/command window (widget) handle */
	HWND		box;		/* parent window handle (box widget id)	*/
	struct menu	*self;		/* ptr to top of the menu array	*/
	struct menu	*parent;	/* ptr to parent		*/
	int		 runtime;	/* TRUE if run-time menu	*/
} menuS, *menuP;



/* ----------------------------------------------------------------------
 * Slider and sampler data structure.
 */
typedef struct slider {
	double	 min;			/* min value			*/
	double	 max;			/* max value			*/
	double	*val;			/* ptr to current value		*/
	PFV	 func;			/* ptr to callback func		*/
} sliderS, *sliderP;



/* ---------*---------*---------*---------*---------*---------*---------*
 * type		name			vptr		    
 *
 * M_NULL	to specify a null entry	NULL
 * M_NAME	menu    name (optional)	NULL
 * M_FUNC	command name		ptr to function returning void (PFV)
 * M_MENU	submenu name		ptr to submenu 
 * M_DRAG	slider  name		ptr to slider/sampler structure
 * M_SMPL	sampler name		ptr to slider/sampler structure
 * M_EXIT	to signal the end of the menu
 *
 * The following types having the name D_* indicate that a fct is executed
 * at run-time to determine the label for the null, menu, command, and
 * submenu entries.
 * The name field is a ptr to a function returning char*.
 *
 * D_NULL	null    name (PFCP)	NULL
 * D_NAME	menu    name (PFCP)	NULL
 * D_FUNC	command name (PFCP)	ptr to function returning void (PFV)
 * D_MENU	submenu name (PFCP)	ptr to submenu
 * D_DRAG	slider  name (PFCP)	ptr to slider/sampler structure
 * D_SMPL	sampler name (PFCP)	ptr to slider/sampler structure
 */

enum menu_types {
	M_NULL,
	M_NAME,
	M_FUNC,
	M_MENU,
	M_DRAG,
	M_SMPL,
	M_EXIT,

	D_NULL,
	D_NAME,
	D_FUNC,
	D_MENU,
	D_DRAG,
	D_SMPL
};



/* ----------------------------------------------------------------------
 * Copyright and Version
 */
extern char Copyright[];
extern char Version[];
extern char PortToWin[];


/*	Titlebar struct for the titlebar on wach ImagwWindows	*/
typedef struct {

	int		x;
	int		y;
	int		width;			/*	titlebar width	*/
	int		height;			/*	titlebar height	*/
//	char	caption[BUFSZ];	/*	caption	*/
} TitleBarS, *TitleBarP;


/*	---------------------------------------------------------------------
 *	MyWin window structure (I used MyWin instead of part and full windows
 *	in the Unix version of this program.)
 *
 *	Window manager in WindowsXP allows changes to the border width
 *  and height of window title bars. i used dialog boxes (with no titlebar)
 *  to keep the size of image windows constant across all windows versions.
 */
typedef struct _MYWIN{

	HWND window;				/*	Handle to the Dialog Box (Image Window) */
	HBITMAP pixmap;//hBitmap;	/*	Handle to the Bitmap in the Dialog Box  */
	HDC gc;//hdc;			/* gc   associated with this window	*/
	BOOL WinMaxed;			//Is the Dlg Maximized?
	int x;					/*	Current Dialog (window) x-coord	*/
	int y;					/*	Current Dialog (window) y-coord	*/
	int Org_x;				/*	The original x-coord of the imagewin.	*/
	int Org_y;				/*	The original y-coord of the imagewin.	*/
	int width;				/*	Dialog (window) width	*/
	int height;				/*	Dialog (window) height	*/
	int bw, bh;				/*	border width and height	*/

	HPEN hPen;				/* Handle to the pen associated with the window	*/
	HBRUSH hBrush;			/* Handle to the brush associated with the window */
	UINT PenStyle;			/* current style of the pen (used for dashes) */
	int PenWidth;			/* current width of the pen	*/
	COLORREF PenColor;		/* color of the pen	*/
	BOOL XOR;				/* current raster operation mode (xor is enabled or not)	*/

} MyWinS, *MyWinP;


/* ----------------------------------------------------------------------
 * Window data structure
 */
typedef struct {
	HWND window;	/* handle to the window	*/
	int			x;
	int			y;
	int	  width;	/* window width				*/
	int	  height;	/* window height			*/
	int	  bw;		/* window border width			*/

} windowS, *windowP;



/* ----------------------------------------------------------------------
 * Movie data structure
 */
typedef struct {
	int	  movie;	/* TRUE if a movie window		*/
	int	  play;		/* TRUE if playing			*/
	int	  nframe;	/* number of frames			*/
	int	  sframe;	/* starting frame number		*/
	int	  eframe;	/* ending   frame number		*/
	int	  cframe;	/* current  frame number		*/
	int	  dir;		/* 1: forward; -1: backward		*/
	int	  mode;		/* 1: yoyo; 0: wrap  around		*/
	int	  ticks;	/* # of ticks to pause			*/
	int	  count;	/* counter				*/
	HBITMAP	 *frames;	/* array of frames			*/
} movieS, *movieP;



/* ----------------------------------------------------------------------
 * Render (3D) data structure
 */
typedef struct {
	MyWinS mywin;		/* handle to the rendering window	*/
	void	  *vars;	/* handler function vars		*/
	menuP	  menuPtr;	/* pointer to rendering menu		*/ 
	PFV	  animate;	/* animate  handler function		*/
	PFV	  display;	/* display  handler function		*/
	PFV	  reshape;	/* reshape  handler function		*/
	PFI	  event;	/* event    handler function		*/
	PFV	  menu;		/* menu     handler function		*/
	
} renderS, *renderP;


#define MOVIE(n)		ImageWin[n].movie.movie
#define PLAY(n)			ImageWin[n].movie.play
#define NFRAME(n)		ImageWin[n].movie.nframe
#define SFRAME(n)		ImageWin[n].movie.sframe
#define EFRAME(n)		ImageWin[n].movie.eframe
#define CFRAME(n)		ImageWin[n].movie.cframe
#define DIR(n)			ImageWin[n].movie.dir
#define MODE(n)			ImageWin[n].movie.mode
#define TICKS(n)		ImageWin[n].movie.ticks
#define COUNT(n)		ImageWin[n].movie.count
#define FRAMES(n)		ImageWin[n].movie.frames

//#define RENDER_FULL		ImageWin[DrawImage].render.full
//#define RENDER_PART		ImageWin[DrawImage].render.part
#define RENDER_MYWIN	ImageWin[DrawImage].render.mywin.window
#define RENDER_VARS		ImageWin[DrawImage].render.vars
#define RENDER_MENU		ImageWin[DrawImage].render.menuPtr
#define ANIMATE_HANDLER		ImageWin[DrawImage].render.animate
#define DISPLAY_HANDLER		ImageWin[DrawImage].render.display
#define RESHAPE_HANDLER		ImageWin[DrawImage].render.reshape
#define   EVENT_HANDLER		ImageWin[DrawImage].render.event
#define    MENU_HANDLER		ImageWin[DrawImage].render.menu




/* ----------------------------------------------------------------------
 * Image window data structure
 */
typedef struct {
	TitleBarS tbar;			/*	ImageWindow titlebar	*/	
	MyWinS	mywin;			/* mywin contains handle to the window and bitmap for the imagewin*/
	int      width;	        /* image width				*/
	int      height;	/* image height				*/
	int			xoffset;
	int			yoffset;
	int      imagetype;	/* image type				*/
	int	 xscroll;	/* image x offset for scrolling		*/
	int	 yscroll;	/* image y offset for scrolling		*/
	int	 xtrack;	/* mouse x offset for tracking		*/
	int	 ytrack;	/* mouse y offset for tracking		*/
	int	 imagenum;	/* image number for UI_selectImage	*/
	int      expose;	/* non-zero if need to fully expose	*/
	int	 alloc;		/* AllocAll or AllocNone		*/
	movieS	 movie;		/* movie data structure			*/
	renderS	 render;	/* 3D rendering data structure		*/

} imagewinS, *imagewinP;

enum window_types {
	TBAR_WIN,
	PART_WIN,
	FULL_WIN
};


/*	The Main (Parent) Window's and Global Attributes	*/
/*------------------------------------------------------*/
extern int ScreenWidth;		/*	Width of the screen	*/
extern int ScreenHeight;	/*	Height of the screen	*/
extern HINSTANCE hInst;		/*	Handle to the instance of the running process	*/
extern char *szAppName;		/*	Name of the running process	*/
extern HWND MainHwnd;		/*	Handle to the main window	*/
extern int Client_Cx;		/*	Width of the MainWindow	*/
extern int Client_Cy;		/*	Height of the MainWindow	*/
extern HWND hLeftMenu;		/*	Handle to the LeftMenu	*/
extern int LeftMenuWidth;	/*	Width of the LeftMenu	*/
extern int LeftMenuHeight;	/*	Height of the LeftMenu	*/
extern int DlgWidth;		/* current default width of each imagewin	*/
extern int DlgHeight;		/* current default height of each imagewin	*/
extern int TBarHeight;		/* Height of the titlebars on imagewindows */
extern int TBarWidth;		/* Width of the titlebars on imagewindows 
							 * (the same as width of the imagewindows; not used)*/
extern HFONT hDefaultFont;
extern int FontSize;

/* ----------------------------------------------------------------------
 * Run-time settable global variables
 */
extern int		PopupMax;
extern int		MovieBW;
extern double		MovieScale;
extern double		MovieGamma;


/* ----------------------------------------------------------------------
 * Default directory and the ImageMagick directory;
 */
extern char MagickDir[];
extern char DefaultDir[];


/* ----------------------------------------------------------------------
 * Command line options
 */
//extern String		 VisName;
extern int		 Dither;


/* ----------------------------------------------------------------------
 * Visual info, visual, visual class, and depth
 */
//extern XVisualInfo	*VisInfo;
//extern Visual		*Vis;
extern uint		 VisClass;
extern uint		 VisDepth;
extern uint		 VisId;


/* ----------------------------------------------------------------------
 * Application-specific externs
 */
//extern Widget		 TopLevel;
//extern Widget		 TopBox;
extern XColor		*TopCmap;
//extern Colormap		 TopCmapID;
extern int		 TopCmapSize;
//extern Dimension	 TopLevelW;
//extern Dimension	 TopLevelH;
extern int		 ImageWinW;
extern int		 ImageWinH;
extern int		 PaneC;
extern jmp_buf		 EnvAbort;
//extern String		 ResName;
extern ulong		 LabelPix;
extern ulong		 BorderPix;
extern ulong		 Foreground;
extern ulong		 Background;
extern ulong		 Highlight;
extern ulong		 Unhighlight;
extern XEvent		*Event;



/* ----------------------------------------------------------------------
 * Font externs
 */
//extern String		 MenuFontN;
//extern XFontStruct	*MenuFontS;
//extern int		 MenuFontW;
//extern int		 MenuFontH;

//extern String		 TermFontN;
//extern XFontStruct	*TermFontS;
//extern int		 TermFontW;
//extern int		 TermFontH;



/* ----------------------------------------------------------------------
 * Miscellaneous externs
 *
 */
//extern XtAppContext	 AppContext;
extern windowS		 ImageBox;
extern imagewinS	 ImageWin[];



/* ----------------------------------------------------------------------
 * Pseudo xterm externs
 */
extern windowS		 Term;
extern int		 TermGets;
extern String		 TermBuf;
extern int		 TermCursorY;



/* ----------------------------------------------------------------------
 * File browser externs
 */
extern char		 FBrowserDir [];
extern char		 FBrowserFile[];
//extern char**		 FBrowserList;



/* ----------------------------------------------------------------------
 * Image and point selection, sampling, and tracking externs
 */
extern int		 SelectImage;
extern int		 SelectPoint;
extern int		 TrackPoint;
extern int		 TrackRepeat;
extern int		 Echo;


/* ----------------------------------------------------------------------
 * Function externs
 */
/*	UIbrowser.c	*/
extern void		UI_fileBrowser	(int, PFV);

/*	UIcolormap.c	*/
/*extern Colormap		UI_createTopLevelColormap(Window);
extern void		UI_setColors	 (Window, menuS*);
extern Colormap		UI_createColormap(Window, int, int);
extern void		UI_resetColormap(int, int);
extern XColor*		UI_getColormap	(Colormap);
*/

/*	UIevents.c	*/
//extern void		UI_appMainLoop	(XtAppContext);
extern int		UI_appMainLoop	();
extern void		UI_grabPointer	(HWND);
extern void		UI_ungrabPointer(void  );
extern void		UI_grabKeyboard	(HWND);
extern void		UI_ungrabKeyboard(void );
extern int XLookupString(XKeyEvent *ev,char* buf,int nMax, WORD *ks,int *status);

/*	UIhistory.c	*/
extern void		UI_initHistory	(void);
extern char*		UI_historyptr	(void);
extern void		UI_displayHistory(void);
extern void		UI_showHistory	(void);
extern void		UI_saveHistory	(void);


/*	UIimagewin.c	*/
extern void		UI_createImageWindows	(void);
//extern void		UI_createFullImageWindow(int );
//extern void		UI_deleteFullImageWindow(int );
extern void		UI_setVisibleImages	(int );
extern BOOL CALLBACK DlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

/*	UIinit.c	*/
extern void		UI_init		(void);
//extern int		UI_makeCurrent	(Window);
extern int		UI_makeCurrent	(HDC);
extern HFONT	GetDefaultFont();

/*	UImenuwin.c	*/
extern void		UI_createMenuWindows(void);
extern void		UI_displayMenu	    (menuP);
extern void		UI_renderMenu	    (menuP);
extern void		UI_restoreRenderMenu(void);
//extern Widget		UI_mainBox	    (void);
extern void		UI_showWaitIcon	    (void);
extern void		UI_hideWaitIcon	    (void);


/*	UImovie.c	*/
extern void		UI_movieLoad	(int, imageP*);
extern void		UI_movieEject	(int );
extern void		UI_moviePlay	(int );
extern void		UI_movieStep	(int );
extern void		UI_movieStop	(int );
extern void		UI_movieEjectAll(void);
extern void		UI_moviePlayAll	(void);
extern void		UI_movieStepAll	(void);
extern void		UI_movieStopAll	(void);
extern void		UI_movieReverse	(int );
extern void		UI_movieFaster	(int );
extern void		UI_movieSlower	(int );
extern void		UI_movieAutoreverse(int );
extern void		UI_movieWraparound (int);
extern void		UI_playAll	(void);
extern int		UI_movieCount	(void);


/*	UIpopupwin.c	*/
extern int		UI_createPopupWindow(String);


/*	UIquery.c	*/
extern int		UI_askInt	(char*, int, int, int);
extern void		UI_askString	(char*, char*, char* );
extern double		UI_askDouble	(char*, double, double, double);
extern int		UI_askHex	(char*, char*);
extern int		UI_askYN	(char*, int  );
extern int		UI_getLine	(char*);
extern imageP		UI_selectImage	(int);
extern int		UI_selectImageN	(int);
extern imageP		UI_selectLut	(int);
extern int		UI_selectXY	(int*, int*);
extern int		UI_sampleXY	(int*, int*);
extern int		UI_trackXY	(int*, int*);
extern void		UI_printf	(char*, ...);

/*	UIstatus.c	*/
extern void		UI_printImageStatus (void);
extern void		UI_printWindowStatus(void);
//extern void		UI_printColormap    (void);


/*	UItermwin.c	*/
//extern void		UI_createTermWindow	(void);
extern void		UI_GainedFocus();
extern void		UI_moveResizeTermWindow	(HWND ButtonsBox);
extern void		UI_clearScreen	(void);
extern void		UI_refresh	(void);
extern void		UI_showCursor	(void);
extern void		UI_clearCursor	(void);
extern void		UI_puts		(String);
extern void		UI_putchar	(char);
extern int		UI_gets		(String);
extern void		UI_scrollup	(void);
extern void		UI_escape	(void);
extern void		UI_return	(void);
extern void		UI_home		(void);
extern void		UI_begin	(void);
extern void		UI_up		(void);
extern void		UI_delete	(void);
extern void		UI_kill		(void);
extern void		UI_collect	(String);
extern void		UI_destroyTermWindow(void);

/*	UIutil.c	*/
//extern void	UI_setWMColormapWindow	(Window);
//extern void	UI_resetWMColormapWindow(void  );
//extern void	UI_highlightBorder	(Window);
//extern void	UI_unhighlightBorder	(Window);
extern void	UI_drawTitlebar		(int);
extern void	UI_updateTitlebar	(int);
extern void	UI_getImage		(int, imageP);
extern void	UI_putImage		(imageP, int, int);
extern void	UI_drawImageToPixmap	(int);
extern int	UI_copyPixmapToWindow	(HBITMAP, HDC,
					int, int,
					int, int,
					int, int);
extern int	UI_copyWindowToPixmap	(HDC, HBITMAP,
					int, int,
					int, int,
					int, int);
//extern GC	UI_createGC		(Window, ulong, ulong, XFontStruct*);
extern void	UI_copy			(void);
extern void	UI_incNextImage		(void);
extern void	UI_decNextImage		(void);
extern imageP	UI_interleave		(imageP);
extern void	UI_renderAll		(void);
extern int	UI_renderCount		(void);

