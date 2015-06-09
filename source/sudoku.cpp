/*
	(C) 2005-2011  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "sudoku.h"

#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"version.lib")

#ifdef USE_PNG
// png header and library can be found at  http://www.libpng.org
#include "png.h"
#pragma comment(lib,"libpng.lib")
#endif

//------------------------------------------------------------------

int
mainLeft=250, mainTop=20,
 mainW=500, mainH=500,
 size=9,
 diag=1,
 symetric=0,
 killer=0,
 greater=0,
 consecutive=0,
 oddeven=0,
 gameType=0,
 showErr,
 showErrTime=50,
 level=0,
 symbol0=2,
 groupPad=5,
 signPad=4,
 consPad=1,
 scrW, scrH,
 width, height,
 gridx, gridy,
 sizex, sizey,
 extentx, extenty,
 multi,
 selectedNum=-1,
 toolBarVisible=1,
 statusBarVisible=1,
 symbol,
 playtime,
 errTime,
 toolNumIndex,
 undoAllPos,
 inHint=0,
 toolH,
 toolSize,
 Naccel,
 insLen,
 curSolution;

bool
noScore,
 inserting,
 editor,
 delreg=false;

BYTE undoGroupTag;

TCHAR playerName[Dname]; //last winner
TscoreTab *score;

Tsquare *insSquares[Msymbol];

ACCEL accel[Maccel];
HINSTANCE inst;
HWND hWin, statusbar, toolbar;
HACCEL haccel;
HBITMAP toolBmp;
TCHAR gameFn[256], bmpFn[256];

COLORREF colors[Ncl]={
	0xffffff, 0x00f0f0, 0x0000f0, 0xf000f0, 0xf00000, 0xf0f000,
	0x00f000, 0x808030, 0x353535,
	0x808080, 0x308080, 0x61acf8,
	0x8b348b, 0x684ffb, 0xffac31, 0x4a5ca4,
	0xc18060, 0x8857ca, 0x97ee4f, 0x8ac193,

	//0x007b00,0x59a2c8,0x82ffc7,0xd14991,0xa786f2,

	0x7000c0, 0xd0d0d0, 0xc0a0a0, 0x000000,
	0x000000, 0x0000ff, 0x005000, 0xff00ff, 0x0000f0,
	0x800040, 0xaf00ff, 0x40f040, 0xaf00ff, 0xb0b0b0/**/
};

Tcoord coordSimple[]={{0, 0, 0, 0}};
Tcoord coordDouble[]={{0, 0, 0, 0}, {0, 0, 1, 1}};
Tcoord coordTwodoku[]={{0, 0, 0, 0}, {0, 1, 1, -1}};
Tcoord coordTriple[]={{0, 0, 0, 0}, {0, 0, 1, 1}, {0, 0, 2, 2}};
Tcoord coordButterfly[]={{0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 1, 1}};
Tcoord coordSohei[]={{1, 0, -1, 0}, {0, 1, 0, -1}, {2, 1, -2, -1}, {1, 2, -1, -2}};
Tcoord coordFlower[]={{0, 0, 1, 1}, {0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 2, 1}, {0, 0, 1, 2}};
Tcoord coordWindmill[]={{1, 1, -1, -1}, {1, 0, -2, 0}, {0, 1, 0, 0}, {2, 1, -2, -2}, {1, 2, 0, -2}};
Tcoord coordSamurai[]={{0, 0, 0, 0}, {2, 0, -2, 0}, {0, 2, 0, -2}, {2, 2, -2, -2}, {1, 1, -1, -1}};
//order of gameTypeA items must not be changed, array index is written in score tables
TgameType gameTypeA[]={{1, coordSimple, "Simple"}, {2, coordDouble, "Double"}, {2, coordTwodoku, "Twodoku"}, {4, coordButterfly, "Butterfly"}, {4, coordSohei, "Sohei"}, {5, coordFlower, "Flower"}, {5, coordWindmill, "Windmill"}, {5, coordSamurai, "Samurai"}, {3, coordTriple, "Triple"}, {0, 0, 0}};
int gameTypeComboOrder[]={0, 1, 2, 8, 3, 4, 5, 6, 7};

static int tabDiag, tabSize, tabKiller, tabGreater, tabCons, tabOddEven, tabGameType;

struct Toption {
	int *value;
	int dialog, menu;
	int *scoreTabValue;
} optionA[]={
	{&diag, IDC_DIAG, ID_DIAGONAL, &tabDiag},
	{&symetric, 309, ID_SYMETRIC, 0},
	{&killer, IDC_KILLER, ID_KILLER, &tabKiller},
	{&greater, IDC_GREATER, ID_GREATER, &tabGreater},
	{&consecutive, IDC_CONS, ID_CONSECUTIVE, &tabCons},
	{&oddeven, IDC_ODDEVEN, ID_ODDEVEN, &tabOddEven}};

TCHAR *title=_T("Color Sudoku");
TCHAR *titleEdit=_T("Color Sudoku - editor");
#define CLASSNAME _T("sudoku")

TBBUTTON tbb[]={
	{4, ID_NEWGAME, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
	{0, 0, 0, TBSTYLE_SEP, {0}, 0},
	{3, ID_UNDO_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
	{1, ID_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
	{0, ID_REDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
	{2, ID_REDO_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
	{5, ID_DEL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
	{6, ID_INS, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_BUTTON, {0}, 0},
	{7, ID_SIGN, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_BUTTON, {0}, 0},
	{8, ID_CONS, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_BUTTON, {0}, 0},
	{9, ID_EVEN, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_BUTTON, {0}, 0},
	{0, 0, 0, TBSTYLE_SEP, {0}, 0},
};

OPENFILENAME gameOfn={
	sizeof(OPENFILENAME), 0, 0,
	_T("Sudoku (*.sud)\0*.sud\0(*.*)\0*.*\0"),
	0, 0, 1,
	gameFn, sizeA(gameFn),
	0, 0, 0, 0, 0, 0, 0, _T("SUD"), 0, 0, 0
};
OPENFILENAME bmpOfn={
	sizeof(OPENFILENAME), 0, 0,
#ifdef USE_PNG
	_T("Portable Network Graphics (*.png)\0*.png\0Bitmap (*.bmp)\0*.bmp\0"),
#else
	_T("Bitmap (*.bmp)\0*.bmp\0"),
#endif
	0, 0, 1,
	bmpFn, sizeA(bmpFn),
	0, 0, 0, 0, 0, 0, 0,
#ifdef USE_PNG
	_T("PNG"),
#else
	_T("BMP"),
#endif
	0, 0, 0
};
OPENFILENAME pdfOfn={
	sizeof(OPENFILENAME), 0, 0,
	_T("PDF\0*.pdf\0(*.*)\0*.*\0"),
	0, 0, 1,
	pdfObject.fn, sizeA(pdfObject.fn),
	0, 0, 0, 0, 0, 0, 0, _T("pdf"), 0, 0, 0
};

//------------------------------------------------------------------
int vmsg(int id, char *text, int btn, va_list v)
{
	static const int Mbuf=4096;
	TCHAR *buf=(TCHAR*)_alloca(2*Mbuf);
	_vsntprintf(buf, Mbuf, lng(id, text), v);
	buf[Mbuf-1]=0;
	int result=MessageBox(hWin, buf, title, btn);
	return result;
}

void msglng(int id, char *text, ...)
{
	va_list ap;
	va_start(ap, text);
	vmsg(id, text, MB_OK, ap);
	va_end(ap);
}

void msg(char *text, ...)
{
	va_list ap;
	va_start(ap, text);
	vmsg(-1, text, MB_OK, ap);
	va_end(ap);
}

void waitOn()
{
	SetCursor(LoadCursor(0, IDC_WAIT));
}

void waitOff()
{
	SetCursor(LoadCursor(0, IDC_ARROW));
}

void status(int i, TCHAR *txt, ...)
{
	TCHAR buf[256];

	va_list ap;
	va_start(ap, txt);
	_vstprintf(buf, txt, ap);
	SendMessage(statusbar, SB_SETTEXT, i, (LPARAM)buf);
	va_end(ap);
}

void printTime(TCHAR *buf, int t)
{
	div_t d = div(t, 60);
	if(t>=3600){
		_stprintf(buf, _T("%d:%02d:%02d"), d.quot/60, d.quot%60, d.rem);
	}
	else{
		_stprintf(buf, _T("%d:%02d"), d.quot, d.rem);
	}
}

void statusTime()
{
	TCHAR buf[10];
	printTime(buf, playtime);
	status(2, buf);
}

int getRadioButton(HWND hWnd, int item1, int item2)
{
	for(int i=item1; i<=item2; i++){
		if(IsDlgButtonChecked(hWnd, i)){
			return i-item1;
		}
	}
	return 0;
}

int getGameType(HWND hWnd)
{
	return gameTypeComboOrder[SendDlgItemMessage(hWnd, 170, CB_GETCURSEL, 0, 0)];
}

void initGameTypeCombo(HWND hWnd)
{
	int i, j, sel=0;

	for(i=0; i<sizeA(gameTypeComboOrder); i++){
		j=gameTypeComboOrder[i];
		if(j==gameType) sel=i;
		SendDlgItemMessageA(hWnd, 170, CB_ADDSTRING, 0, (LPARAM)gameTypeA[j].name);
	}
	SendDlgItemMessage(hWnd, 170, CB_SETCURSEL, sel, 0);
}

int openFileDlg(OPENFILENAME *o, HWND hWnd, DWORD flags)
{
	for(;;){
		o->hwndOwner= hWnd;
		o->Flags= flags;
		if(GetOpenFileName(o)) return 1; //ok
		if(CommDlgExtendedError()!=FNERR_INVALIDFILENAME
			|| !*o->lpstrFile) return 0; //cancel
		*o->lpstrFile=0;
	}
}

int saveFileDlg(OPENFILENAME *o, HWND hWnd, DWORD flags)
{
	for(;;){
		o->hwndOwner= hWnd;
		o->Flags= flags | OFN_HIDEREADONLY;
		if(GetSaveFileName(o)) return 1; //ok
		if(CommDlgExtendedError()!=FNERR_INVALIDFILENAME
			|| !*o->lpstrFile) return 0; //cancel
		*o->lpstrFile=0;
	}
}
//------------------------------------------------------------------
void toolBitmap()
{
	int i;
	TBADDBITMAP a;
	HDC dc, wdc;
	HGDIOBJ oldBmp, oldF;
	TCHAR c;
	HBRUSH br;
	RECT rc;
	HBITMAP b;
	LOGFONT font;
	const int Nbut1=35, Nbut=Nbut1+Msymbol+1;

	wdc=GetDC(toolbar);
	dc=CreateCompatibleDC(wdc);
	b=CreateCompatibleBitmap(wdc, Nbut*16, 15);
	ReleaseDC(toolbar, wdc);
	oldBmp=SelectObject(dc, b);
	rc.left=0; rc.right=16*Nbut1; rc.bottom=15; rc.top=0;
	FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
	SetBkMode(dc, TRANSPARENT);
	SetTextAlign(dc, TA_CENTER);
	memset(&font, 0, sizeof(LOGFONT));
	font.lfHeight=-15;
	_tcscpy(font.lfFaceName, _T("Tahoma"));
	oldF=SelectObject(dc, CreateFontIndirect(&font));
	for(i=0; i<Nbut1; i++){
		c=num2char(i);
		TextOut(dc, 16*i+8, -1, &c, 1);
	}
	DeleteObject(SelectObject(dc, oldF));
	for(; i<Nbut; i++){
		rc.left=i*16;
		rc.right=rc.left+16;
		FillRect(dc, &rc, br=CreateSolidBrush(colors[i-Nbut1]));
		DeleteObject(br);
	}
	SelectObject(dc, oldBmp);
	DeleteDC(dc);

	if(toolBmp){
		TBREPLACEBITMAP rb;
		rb.hInstOld=0;
		rb.nIDOld=(UINT_PTR)toolBmp;
		rb.hInstNew=0;
		rb.nIDNew=(UINT_PTR)b;
		rb.nButtons=Nbut;
		SendMessage(toolbar, TB_REPLACEBITMAP, 0, (LPARAM)&rb);
		DeleteObject(toolBmp);
	}
	else{
		a.hInst=(HINSTANCE)0;
		a.nID=(UINT_PTR)b;
		toolNumIndex= SendMessage(toolbar, TB_ADDBITMAP, Nbut, (LPARAM)&a);
	}
	toolBmp=b;
	InvalidateRect(toolbar, 0, TRUE);
}

void numButtons()
{
	int i;

	for(i=0; i<toolSize; i++){
		SendMessage(toolbar, TB_DELETEBUTTON, sizeA(tbb), 0);
	}
	toolSize=size;
	static TBBUTTON but[Msymbol];
	for(i=0; i<size; i++){
		TBBUTTON *b= but+i;
		b->fsState=TBSTATE_ENABLED;
		b->fsStyle=TBSTYLE_CHECK;
		static int O[]={0, 9, 35};
		b->iBitmap=i+toolNumIndex+O[symbol];
		b->idCommand=ID_SYMBOL+i;
	}
	SendMessage(toolbar, TB_ADDBUTTONS, size, (LPARAM)but);
	SendMessage(toolbar, TB_CHECKBUTTON, ID_SYMBOL+selectedNum, MAKELONG(TRUE, 0));
}
//---------------------------------------------------------------------------
BOOL CALLBACK SumProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM sum)
{
	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 24);
			if(sum) SetDlgItemInt(hWnd, 101, sum, FALSE);
			return TRUE;

		case WM_COMMAND:
			wP=LOWORD(wP);
			if(wP==IDOK){
				EndDialog(hWnd, GetDlgItemInt(hWnd, 101, 0, FALSE));
			}
			if(wP==IDCANCEL) EndDialog(hWnd, 10000);
			break;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
#ifdef USE_PNG
void wrPng(BYTE *bits, int w, int h, int line, FILE *f)
{
	png_structp png_ptr;
	png_infop info_ptr;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if(png_ptr){
		info_ptr = png_create_info_struct(png_ptr);
		if(info_ptr){
			if(!setjmp(png_jmpbuf(png_ptr))){
				png_init_io(png_ptr, f);
				png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
				BYTE **row_pointers = (BYTE**)png_malloc(png_ptr, h*png_sizeof(png_bytep));
				for(int i=0; i<h; i++){
					row_pointers[i]= bits+(h-i-1)*line;
				}
				png_set_rows(png_ptr, info_ptr, row_pointers);
				png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, 0);
				png_free(png_ptr, row_pointers);
			}
		}
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
}
#endif

void wrBmp(TCHAR *fn, int type)
{
	BITMAPINFOHEADER bmi;
	BITMAPFILEHEADER hdr;
	HBITMAP hBmp;
	BYTE *bits;
	FILE *f;
	int w, h, oldtool, oldstatus, oldW, oldH;
#ifdef USE_PNG
	bool isPng= (type==1);
#else
	const bool isPng=false; type;
#endif

	//create bitmap
	HDC winDC= GetDC(hWin);
	HDC dcb= CreateCompatibleDC(winDC);
	hBmp= CreateCompatibleBitmap(winDC, w=gridx*extentx, h=gridy*extenty);
	HGDIOBJ oldB= SelectObject(dcb, hBmp);
	ReleaseDC(hWin, winDC);
	//paint
	oldtool=toolBarVisible; oldstatus=statusBarVisible;
	oldW=width; oldH=height;
	toolBarVisible=statusBarVisible=0;
	width=w; height=h;
	paint(dcb, 0);
	toolBarVisible=oldtool; statusBarVisible=oldstatus;
	width=oldW; height=oldH;
	SelectObject(dcb, oldB);
	//initialize bitmap structure
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = w;
	bmi.biHeight = h;
	bmi.biPlanes = 1;
	bmi.biBitCount = (WORD)(isPng ? 24 : 16);
	bmi.biCompression = BI_RGB;
	bmi.biXPelsPerMeter=bmi.biYPelsPerMeter=0;
	bmi.biClrUsed = bmi.biClrImportant = 0; //needed for Linux wine
	//get pixels
	bits = new BYTE[h*(w*3+4)];
	if(!GetDIBits(dcb, hBmp, 0, h, bits, (BITMAPINFO*)&bmi, DIB_RGB_COLORS)){
		msg("GetDIBits failed");
	}
	else{
		//save file
		f=_tfopen(fn, _T("wb"));
		if(!f){
			msglng(733, "Cannot create file %s", fn);
		}
		else{
#ifdef USE_PNG
			if(isPng) wrPng(bits, w, h, bmi.biSizeImage/h, f);
			else
#endif
			{
				hdr.bfType = 0x4d42;  //'BM'
				hdr.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bmi.biSizeImage;
				hdr.bfReserved1 = hdr.bfReserved2 = 0;
				hdr.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
				fwrite(&hdr, sizeof(BITMAPFILEHEADER), 1, f);
				fwrite(&bmi, sizeof(BITMAPINFOHEADER), 1, f);
				fwrite(bits, bmi.biSizeImage, 1, f);
			}
			fclose(f);
		}
	}
	delete[] bits;
	DeleteObject(hBmp);
	DeleteDC(dcb);
}

//---------------------------------------------------------------------------
//level dialog box
BOOL CALLBACK LevelProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 25);
			SetDlgItemInt(hWnd, 101, level, FALSE);
			return TRUE;

		case WM_COMMAND:
			wP=LOWORD(wP);
			if(wP==IDOK){
				level=GetDlgItemInt(hWnd, 101, 0, FALSE);
				EndDialog(hWnd, 1);
			}
			if(wP==IDCANCEL) EndDialog(hWnd, 0);
			break;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
//show errors dialog box
BOOL CALLBACK ShowErrProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 27);
			SetDlgItemInt(hWnd, 101, showErrTime, FALSE);
			return TRUE;

		case WM_COMMAND:
			wP=LOWORD(wP);
			if(wP==IDOK){
				showErrTime=GetDlgItemInt(hWnd, 101, 0, FALSE);
				EndDialog(hWnd, 1);
			}
			if(wP==IDCANCEL) EndDialog(hWnd, 0);
			break;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
//player name dialog
BOOL CALLBACK NameProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 12);
			SendDlgItemMessage(hWnd, 101, EM_LIMITTEXT, Dname-1, 0);
			SetDlgItemText(hWnd, 101, playerName);
			return TRUE;

		case WM_COMMAND:
			wP=LOWORD(wP);
			if(wP==IDOK){
				GetDlgItemText(hWnd, 101, playerName, Dname);
				EndDialog(hWnd, wP);
			}
			if(wP==IDCANCEL) EndDialog(hWnd, wP);
			break;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
BOOL CALLBACK PdfProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
	struct PageSize {
		int w, h;
		TCHAR *s;
	};
	static const PageSize P[]=
	{{595, 842, _T("A4")}, {421, 595, _T("A5")}, {842, 1190, _T("A3")}};
	static const int L[]={1, 2, 3, 4, 6, 9, 12};
	int i;
	HWND combo;
	Toption *to;
	TCHAR buf[4];

	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 26);
			SetDlgItemInt(hWnd, 160, pdfObject.count, FALSE);
			SetDlgItemInt(hWnd, 163, pdfObject.pageWidth, FALSE);
			SetDlgItemInt(hWnd, 164, pdfObject.pageHeight, FALSE);
			combo= GetDlgItem(hWnd, 162);
			for(i=0; i<sizeA(P); i++){
				const PageSize *p = &P[i];
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)p->s);
				if(pdfObject.pageWidth==p->w && pdfObject.pageHeight==p->h){
					SendMessage(combo, CB_SETCURSEL, i, 0);
				}
			}
			combo= GetDlgItem(hWnd, 161);
			for(i=0; i<sizeA(L); i++){
				_stprintf(buf, _T("%d"), L[i]);
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)buf);
			}
			initGameTypeCombo(hWnd);
			SetDlgItemInt(hWnd, 161, pdfObject.countPerPage, FALSE);
			SetDlgItemInt(hWnd, 168, pdfObject.border, FALSE);
			SetDlgItemInt(hWnd, 169, pdfObject.spacing, FALSE);
			CheckRadioButton(hWnd, 350, 352, 350+symbol0);
			for(to=optionA; to<endA(optionA); to++){
				CheckDlgButton(hWnd, to->dialog, *to->value ? BST_CHECKED : BST_UNCHECKED);
			}
			SetDlgItemInt(hWnd, 165, level, FALSE);
			SetDlgItemText(hWnd, 166, pdfObject.fn);
			return TRUE;

		case WM_COMMAND:
			wP=LOWORD(wP);
			if(wP==IDOK){
				pdfObject.count = GetDlgItemInt(hWnd, 160, 0, FALSE);
				//page properties
				pdfObject.pageWidth = GetDlgItemInt(hWnd, 163, 0, FALSE);
				pdfObject.pageHeight = GetDlgItemInt(hWnd, 164, 0, FALSE);
				pdfObject.countPerPage = GetDlgItemInt(hWnd, 161, 0, FALSE);
				pdfObject.border = GetDlgItemInt(hWnd, 168, 0, FALSE);
				pdfObject.spacing = GetDlgItemInt(hWnd, 169, 0, FALSE);
				//game options
				for(to=optionA; to<endA(optionA); to++){
					*to->value= IsDlgButtonChecked(hWnd, to->dialog);
				}
				level= GetDlgItemInt(hWnd, 165, 0, FALSE);
				symbol0= getRadioButton(hWnd, 350, 352);
				gameType=getGameType(hWnd);

				GetDlgItemText(hWnd, 166, pdfObject.fn, sizeA(pdfObject.fn));
				EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
				pdfObject.print(hWnd);
				EndDialog(hWnd, wP);
			}
			if(wP==IDCANCEL) EndDialog(hWnd, wP);
			if(wP==162){
				i=SendDlgItemMessage(hWnd, 162, CB_GETCURSEL, 0, 0);
				if(i>=0 && i<sizeA(P)){
					SetDlgItemInt(hWnd, 163, P[i].w, FALSE);
					SetDlgItemInt(hWnd, 164, P[i].h, FALSE);
				}
			}
			if(wP==167){
				if(saveFileDlg(&pdfOfn, hWnd, 0)){
					SetDlgItemText(hWnd, 166, pdfObject.fn);
				}
			}
			break;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
BOOL CALLBACK ScoreProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
	const int colX[]={105, 127, 186};
	int cmd;
	RECT rc;
	POINT p;
	Toption *to;

	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 14);

			for(to=optionA; to<endA(optionA); to++){
				if(to->scoreTabValue){
					CheckDlgButton(hWnd, to->dialog, (*to->scoreTabValue=*to->value)!=0 ? BST_CHECKED : BST_UNCHECKED);
				}
			}
			tabSize=size;
			CheckRadioButton(hWnd, 200, 200+Msize, size+200);
			tabGameType=gameType;
			initGameTypeCombo(hWnd);
			return TRUE;

		case WM_COMMAND:
			cmd=LOWORD(wP);
			if(cmd==IDOK) EndDialog(hWnd, cmd);
			if(cmd==IDCANCEL) EndDialog(hWnd, cmd);
			if(cmd>=200 && cmd<300 ||
				cmd==IDC_DIAG || cmd==IDC_KILLER || cmd==IDC_GREATER ||
				cmd==IDC_CONS || cmd==IDC_ODDEVEN ||
				cmd==170 && HIWORD(wP)==CBN_SELCHANGE){
				if(cmd>=200 && cmd<300) tabSize=cmd-200;
				for(to=optionA; to<endA(optionA); to++){
					if(to->scoreTabValue){
						*to->scoreTabValue=IsDlgButtonChecked(hWnd, to->dialog);
					}
				}
				tabGameType=getGameType(hWnd);
				p.x=p.y=0;
				MapWindowPoints(GetDlgItem(hWnd, 206), hWnd, &p, 1);
				GetClientRect(hWnd, &rc);
				rc.bottom=p.y;
				InvalidateRect(hWnd, &rc, TRUE);
				SetFocus(GetDlgItem(hWnd, tabSize+200));
			}
			break;

		case WM_PAINT:
		{
			static const int Mbuf=64;
			TCHAR *buf=(TCHAR*)_alloca(sizeof(TCHAR)*Mbuf);
			static PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			TscoreTab *curScoreTab= findScoreTab((BYTE)tabGameType, (BYTE)tabSize, (BYTE)tabDiag, (BYTE)tabKiller, (BYTE)tabGreater, (BYTE)tabCons, (BYTE)tabOddEven);
			if(!curScoreTab){
				/*GetClientRect(hWnd,&rc);
				rc.bottom-=120;
				MoveToEx(ps.hdc,0,50,0);
				LineTo(ps.hdc,rc.right,rc.bottom);
				MoveToEx(ps.hdc,rc.right,50,0);
				LineTo(ps.hdc,0,rc.bottom);*/
			}
			else{
				TScore *s= curScoreTab->score;
				SetBkMode(ps.hdc, TRANSPARENT);
				int dpi=GetDeviceCaps(ps.hdc, LOGPIXELSX);

				for(int i=0; i<Dscore; i++, s++){
					if(!s->playtime) continue;
					int y=(24*i+35)*dpi/96;
					SetTextColor(ps.hdc, i==curScoreTab->lastScore ? colors[clLastScore] : 0);
					TextOutW(ps.hdc, 10, y, s->name, (int)wcslen(s->name)); //player name
					SetTextAlign(ps.hdc, TA_RIGHT);
					for(int j=0; j<3; j++){
						rc.right=colX[j];
						switch(j){
							case 0: //date
								SYSTEMTIME t;
								t.wYear=s->date.wYear;
								t.wMonth=s->date.wMonth;
								t.wDay=s->date.wDay;
								t.wHour=s->date.wHour;
								t.wMinute=s->date.wMinute;
								t.wSecond=t.wMilliseconds=0;
								GetDateFormat(0, 0, &t, 0, buf, Mbuf);
								break;
							case 1: //time
								GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS,
									&t, 0, buf, Mbuf);
								if((int)_tcslen(buf) > 5) rc.right += 15;
								break;
							case 2: //game time
								printTime(buf, s->playtime);
								break;
						}
						rc.left=rc.top=rc.bottom=0;
						MapDialogRect(hWnd, &rc);
						TextOut(ps.hdc, rc.right, y, buf, (int)_tcslen(buf));
					}
					SetTextAlign(ps.hdc, TA_LEFT);
				}
			}
			EndPaint(hWnd, &ps);
			break;
		}
	}
	return FALSE;
}
//------------------------------------------------------------------
void checkMenus()
{
	symbol= symbol0;
	if(killer) symbol=0;
	HMENU m=GetMenu(hWin);
	CheckMenuRadioItem(m, ID_SIZE+4, ID_SIZE+Msize, ID_SIZE+size, MF_BYCOMMAND);
	CheckMenuRadioItem(m, 350, 352, 350+symbol0, MF_BYCOMMAND);
	CheckMenuRadioItem(m, ID_MULTI, ID_MULTI+sizeA(gameTypeA)-1, ID_MULTI+gameType, MF_BYCOMMAND);

	for(Toption *to=optionA; to<endA(optionA); to++){
		CheckMenuItem(m, to->menu, *to->value ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
	}
	//CheckMenuItem(m,ID_SHOWERR,showErr ? MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
	HMENU m1=GetSubMenu(m, 2);
	EnableMenuItem(m1, GetMenuItemCount(m1)-2, killer ? MF_BYPOSITION|MF_GRAYED : MF_BYPOSITION|MF_ENABLED);
}

bool askNew()
{
	if(undoPos>0 && done<Nsquare && MessageBox(hWin, lng(801, "Start a new game ?"),
		title, MB_YESNO|MB_ICONQUESTION)!=IDYES) return true;
	return false;
}

void onMoved()
{
	if(IsIconic(hWin) || IsZoomed(hWin)) return;
	RECT rc;
	GetWindowRect(hWin, &rc);
	mainTop= rc.top;
	mainLeft= rc.left;
	mainW= rc.right-rc.left;
	mainH= rc.bottom-rc.top;
}

static int subId[]={404, 407, 401, 402, 403, 406, 405, 400};
static int editorItems[]={ID_EDITOR_END, ID_CLEAR, ID_CLEAR_ALL, ID_CLEAR_CONS, ID_CLEAR_GRP, ID_CLEAR_SGN, 0};
static int gameItems[]={ID_EDITOR, ID_NEWGAME, 0};

void reloadMenu()
{
	loadMenu(hWin, MAKEINTRESOURCE(IDR_MENU), subId);

	HMENU m=GetMenu(hWin);
	for(int *p= (editor ? gameItems : editorItems); *p; p++){
		DeleteMenu(m, *p, MF_BYCOMMAND);
	}

	checkMenus();
	DrawMenuBar(hWin);
}

void langChanged()
{
	reloadMenu();
}

static int toolEditItems[]={ID_INS, ID_SIGN, ID_CONS, ID_EVEN};

void editorChanged()
{
	reloadMenu();
	for(int *p=toolEditItems; p<endA(toolEditItems); p++){
		SendMessage(toolbar, TB_HIDEBUTTON, *p, MAKELONG(editor ? FALSE : TRUE, 0));
	}
	SetWindowText(hWin, editor ? titleEdit : title);
}

//------------------------------------------------------------------
void colorChanged()
{
	InvalidateRect(hWin, 0, TRUE);
	toolBitmap();
}

static COLORREF custom[16];

BOOL CALLBACK ColorProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP)
{
	static bool chng;
	static CHOOSECOLOR chc;
	static COLORREF clold[Ncl];
	DRAWITEMSTRUCT *di;
	HBRUSH br;
	int cmd;
	RECT rc;

	switch(msg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 23);
			memcpy(clold, colors, sizeof(clold));
			chng=false;
			return TRUE;

		case WM_DRAWITEM:
			di = (DRAWITEMSTRUCT*)lP;
			DrawFrameControl(di->hDC, &di->rcItem, DFC_BUTTON,
				DFCS_BUTTONPUSH|(di->itemState&ODS_SELECTED ? DFCS_PUSHED : 0));
			CopyRect(&rc, &di->rcItem);
			InflateRect(&rc, -3, -3);
			br= CreateSolidBrush(colors[di->CtlID-100]);
			FillRect(di->hDC, &rc, br);
			DeleteObject(br);
			break;

		case WM_COMMAND:
			cmd=LOWORD(wP);
			switch(cmd){
				default: //color square
					chc.lStructSize= sizeof(CHOOSECOLOR);
					chc.hwndOwner= hWnd;
					chc.hInstance= 0;
					chc.rgbResult= colors[cmd-100];
					chc.lpCustColors= custom;
					chc.Flags= CC_RGBINIT|CC_FULLOPEN;
					if(ChooseColor(&chc)){
						colors[cmd-100]=chc.rgbResult;
						InvalidateRect(GetDlgItem(hWnd, cmd), 0, TRUE);
						colorChanged();
						chng=true;
					}
					break;
				case IDCANCEL:
					if(chng){
						memcpy(colors, clold, sizeof(clold));
						colorChanged();
					}
					//!
				case IDOK:
					EndDialog(hWnd, cmd);
			}
			break;
	}
	return FALSE;
}
//------------------------------------------------------------------
DWORD getVer()
{
	HRSRC r;
	HGLOBAL h;
	void *s;
	VS_FIXEDFILEINFO *v;
	UINT i;

	r=FindResource(0, (TCHAR*)VS_VERSION_INFO, RT_VERSION);
	h=LoadResource(0, r);
	s=LockResource(h);
	if(!s || !VerQueryValueA(s, "\\", (void**)&v, &i)) return 0;
	return v->dwFileVersionMS;
}

LRESULT CALLBACK AboutProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM)
{
	char buf[48];
	DWORD d;

	switch(message){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 11);
			d=getVer();
			sprintf(buf, "%d.%d", HIWORD(d), LOWORD(d));
			SetDlgItemTextA(hWnd, 101, buf);
			return TRUE;

		case WM_COMMAND:
			switch(wParam){
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd, wParam);
					return TRUE;
				case 123:
					GetDlgItemTextA(hWnd, wParam, buf, sizeA(buf)-13);
					if(!_tcscmp(lang, _T("English"))) strcat(buf, "/indexEN.html");
					ShellExecuteA(0, 0, buf, 0, 0, SW_SHOWNORMAL);
					break;
			}
			break;
	}
	return FALSE;
}
//------------------------------------------------------------------
LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, notif;
	Tsquare *t;

	switch(message){
		case WM_PAINT:
		{
			static PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			paint(ps.hdc, &ps.rcPaint);
			EndPaint(hWnd, &ps);
		}
			break;
		case WM_LBUTTONDOWN:
			lbutton(lParam);
			break;
		case WM_RBUTTONDOWN:
			rbutton(lParam);
			break;
		case WM_LBUTTONUP:
			if(inserting){
				ReleaseCapture();
				inserting=false;
				insertGroup();
				resetSolution();
			}
			break;
		case WM_MOUSEMOVE:
			if(inserting){
				t= hitTest(lParam);
				if(t && t!=insSquares[insLen-1] && insLen<Nsymbol){
					insSquares[insLen++]=t;
				}
			}
#ifdef _DEBUGM
			mousemove(lParam);
#endif
			break;

		case WM_TIMER:
			if(!IsIconic(hWin)){
				playtime++;
				statusTime();
				checkShowErr(false);
			}
			break;
		case WM_KEYDOWN:
			key(wParam);
			break;
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmm = (LPMINMAXINFO)lParam;
			lpmm->ptMinTrackSize.x = 250;
			lpmm->ptMinTrackSize.y = 200+toolH;
			break;
		}
		case WM_SIZE:
			width=LOWORD(lParam);
			height=HIWORD(lParam);
			SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
			SendMessage(statusbar, WM_SIZE, 0, 0);
			onMoved();
			invalidate();
			break;
		case WM_MOVE:
			onMoved();
			break;
		case WM_CLOSE:
			SendMessage(hWin, WM_COMMAND, ID_EXIT, 0);
			break;
		case WM_QUERYENDSESSION:
			writeini();
			return TRUE;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_COMMAND:
			notif=HIWORD(wParam);
			wParam=LOWORD(wParam);
			if(setLang(wParam)) break;
			if(wParam>=ID_SYMBOL && wParam<unsigned(ID_SYMBOL+size)){
				select(wParam-ID_SYMBOL);
				break;
			}
			if(wParam>=ID_SIZE+4 && wParam<=ID_SIZE+Msize){
				if(askNew()) break;
				size=wParam-ID_SIZE;
				newGameFormat();
				numButtons();
				break;
			}
			if(wParam>=ID_MULTI && wParam<ID_MULTI+sizeA(gameTypeA)-1){
				if(askNew()) break;
				gameType=wParam-ID_MULTI;
				newGameFormat();
				break;
			}

			switch(wParam){
				case ID_CLEAR:
					noScore=true;
					init(false);
					invalidate();
					break;
				case ID_CLEAR_ALL:
					noScore=true;
					initSquare(false);
					invalidate();
					break;
				case ID_EDITOR:
					if(!editor){
						if((undoPos==0 || done==Nsquare) && isGenerated()){
							initSquare(false);
						}
						editor=true;
						playtime=0;
						noScore=true;
						editorChanged();
					}
					break;
				case ID_EDITOR_END:
					if(editor){
						endEditor();
						editor=false;
						editorChanged();
					}
					break;
				case ID_SOLVE:
				case ID_SOLVE1:
					if(testTotal()) break;
					noScore=true;
					if(done<Nsquare){
						waitOn();
#ifdef _DEBUG
						DWORD time=getTickCount();
#endif
						Nsolution=0;
						curSolution=-1; //find all solutions (up to Msolution)
						undoAllPos=undoPos;
						if(wParam==ID_SOLVE1) resolve1(); else resolve();
						freeGroups();
#ifdef _DEBUG
						status(4, _T("%d ms"), getTickCount()-time);
#endif
						waitOff();
					}
					if(Nsolution>0){
						i=curSolution;
						curSolution++;
						if(curSolution>=Nsolution) curSolution=0;
						if(Nsolution>1){
							if(i<0){
								status(4, _T("%d %s"), Nsolution, lng(662, "solutions"));
							}
							else{
								status(4, _T("%d/%d"), curSolution+1, Nsolution);
							}
						}
						rdSolution();
					}
					else{ //easy solution (without recurse) or not solvable
						curSolution=0;
						status(4, _T(""));
					}
					checkErr();
					invalidate();
					break;
				case ID_CHEAT:
					noScore=true;
					if(errTime<0){
						waitOn();
						hint();
						waitOff();
					}
					checkShowErr(true);
					break;
				case ID_UNDO:
					undo();
					checkErr();
					break;
				case ID_REDO:
					redo();
					checkErr();
					break;
				case ID_UNDO_SYMBOL:
					undoSymbol();
					checkErr();
					break;
				case ID_REDO_SYMBOL:
					redoSymbol();
					checkErr();
					break;
				case ID_UNDO_ALL:
					undoAll();
					checkErr();
					break;
				case ID_REDO_ALL:
					while(redo());
					checkErr();
					break;
				case ID_DEL:
					select(-1);
					break;
				case ID_INS:
					select(-2);
					break;
				case ID_SIGN:
					select(-3);
					break;
				case ID_CONS:
					select(-4);
					break;
				case ID_EVEN:
					select(-5);
					break;
				case ID_EXIT:
					writeini();
					DestroyWindow(hWin);
					break;
				case ID_DIAGONAL:
					if(askNew()) break;
					diag=!diag;
					newGameFormat();
					break;
				case ID_SYMETRIC:
					if(askNew()) break;
					symetric=!symetric;
					newGameFormat();
					break;
				case ID_LEVEL:
					if(DialogBox(inst, MAKEINTRESOURCE(IDD_LEVEL), hWnd, (DLGPROC)LevelProc)){
						if(!editor) newGame();
					}
					break;
				case ID_SHOWERR:
					DialogBox(inst, MAKEINTRESOURCE(IDD_ERRTIME), hWnd, (DLGPROC)ShowErrProc);
					break;
				case ID_KILLER:
					if(askNew()) break;
					killer=!killer;
					newGameFormat();
					numButtons();
					break;
				case ID_GREATER:
					if(askNew()) break;
					greater=!greater;
					newGameFormat();
					break;
				case ID_CONSECUTIVE:
					if(askNew()) break;
					consecutive=!consecutive;
					newGameFormat();
					if(selectedNum==-4 && !consecutive) select(-1);
					break;
				case ID_ODDEVEN:
					if(askNew()) break;
					oddeven=!oddeven;
					newGameFormat();
					if(selectedNum==-5 && !oddeven) select(-1);
					break;
				case ID_DIGITS:
				case ID_LETTERS:
				case ID_COLORS:
					symbol0=wParam-350;
					checkMenus();
					invalidate();
					numButtons();
					break;
				case ID_NEWGAME:
					if(editor) SendMessage(hWnd, WM_COMMAND, ID_EDITOR_END, 0);
					else newGame();
					break;
				case ID_DELINI:
					delreg=true;
					break;
				case ID_DELHISCORE:
					if(MessageBox(hWnd,
						lng(799, "Do you really want to delete all hiscores ?"), title,
						MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) == IDYES){
						for(TscoreTab *tab=score; tab;){
							TscoreTab *t1= tab->next;
							delete tab;
							tab=t1;
						}
						score=0;
						writeScore();
					}
					break;
				case ID_BEST_SCORES:
					DialogBox(inst, MAKEINTRESOURCE(IDD_HISCORE), hWnd, (DLGPROC)ScoreProc);
					break;
				case ID_COLORDLG:
					DialogBox(inst, MAKEINTRESOURCE(IDD_COLORS), hWin, (DLGPROC)ColorProc);
					break;
				case ID_ABOUT:
					DialogBox(inst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutProc);
					break;
				case ID_HELP_README:
				{
					TCHAR *buf=(TCHAR*)_alloca(2*MAX_PATH);
					getExeDir(buf, lng(13, "readme.txt"));
					if(ShellExecute(0, _T("open"), buf, 0, 0, SW_SHOWNORMAL)==(HINSTANCE)ERROR_FILE_NOT_FOUND){
						msglng(730, "Cannot open %s", buf);
					}
				}
					break;
				case ID_WRBMP:
					if(saveFileDlg(&bmpOfn, hWnd, 0)){
						wrBmp(bmpFn, bmpOfn.nFilterIndex);
					}
					break;
				case ID_SAVE:
					if(saveFileDlg(&gameOfn, hWnd, OFN_OVERWRITEPROMPT)){
						save(gameFn);
					}
					break;
				case ID_OPEN:
					if(openFileDlg(&gameOfn, hWnd, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY)){
						open(gameFn);
						checkErr();
					}
					break;
				case ID_CLEAR_GRP:
					resetSolution();
					for(i=0; i<Ngroup; i++){
						delGroup(&group[i]);
					}
					invalidate();
					break;
				case ID_CLEAR_SGN:
				case ID_CLEAR_CONS:
					resetSolution();
					for(i=0; i<Nboard; i++){
						if(wParam==ID_CLEAR_SGN){
							putSign(0, &board[i], 0);
							putSign(0, &board[i], 1);
						}
						if(wParam==ID_CLEAR_CONS){
							putCons(false, &board[i], 0);
							putCons(false, &board[i], 1);
						}
					}
					invalidate();
					break;
				case ID_MARKS:
					noScore=true;
					showMarks();
					break;
				case ID_DELMARKS:
					delAllMarks();
					break;
				case ID_PDF:
					if(askNew()) break;
					DialogBox(inst, MAKEINTRESOURCE(IDD_PDF), hWnd, (DLGPROC)PdfProc);
					numButtons();
					break;
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
//------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	int i;
	HDC dc;
	MSG mesg;
	RECT rc;

	inst = hInstance;

	//DPIAware
	typedef BOOL(WINAPI *TGetProcAddress)();
	TGetProcAddress getProcAddress = (TGetProcAddress)GetProcAddress(GetModuleHandle(_T("user32")), "SetProcessDPIAware");
	if(getProcAddress) getProcAddress();

	memset(custom, 200, sizeof(custom));
	_tcscpy(pdfObject.fn, _T("sudoku.pdf"));
	pdfObject.pageWidth=595;
	pdfObject.pageHeight=842;
	pdfObject.count=6;
	pdfObject.countPerPage=6;
	pdfObject.border=40;
	pdfObject.spacing=20;
	readini();
	//load common controls
#if _WIN32_IE >= 0x0300
	INITCOMMONCONTROLSEX iccs;
	iccs.dwSize= sizeof(INITCOMMONCONTROLSEX);
	iccs.dwICC= ICC_BAR_CLASSES;
	InitCommonControlsEx(&iccs);
#else
	InitCommonControls();
#endif
	// create the main window
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = WndMainProc;
	wc.hInstance = inst;
	wc.lpszClassName = CLASSNAME;
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = LoadIcon(inst, MAKEINTRESOURCE(IDI_MAINICON));
	if(!RegisterClass(&wc)){
#ifdef UNICODE
		msg("This version cannot run on Windows 95/98/ME.");
#else
		msg("RegisterClass failed");
#endif
		return 2;
	}
	scrW= GetSystemMetrics(SM_CXSCREEN);
	scrH= GetSystemMetrics(SM_CYSCREEN);
	aminmax(mainLeft, 0, scrW-50);
	aminmax(mainTop, 0, scrH-50);
	hWin = CreateWindow(CLASSNAME, title,
		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
		mainLeft, mainTop, mainW, mainH, 0, 0, inst, 0);
	if(!hWin){
		msg("CreateWindow failed");
		return 3;
	}

	haccel= LoadAccelerators(inst, MAKEINTRESOURCE(IDR_ACCELERATOR));
	Naccel= CopyAcceleratorTable(haccel, accel, sizeA(accel));
	initLang();
	//create status bar
	statusbar= CreateStatusWindow(WS_CHILD, 0, hWin, 1);
	static int parts[]={100, 140, 210, 230, -1};
	dc=GetDC(hWin);
	for(i=0; i<sizeA(parts)-1; i++){
		parts[i]=parts[i]*GetDeviceCaps(dc, LOGPIXELSX)/96;
	}
	ReleaseDC(hWin, dc);
	SendMessage(statusbar, SB_SETPARTS, sizeA(parts), (LPARAM)parts);
	ShowWindow(statusbar, SW_SHOW);
	//create tool bar
	i=sizeA(tbb);
	for(TBBUTTON *u=tbb; u<endA(tbb); u++){
		if(u->fsStyle==TBSTYLE_SEP) i--;
	}
	toolbar = CreateToolbarEx(hWin,
		WS_CHILD|TBSTYLE_TOOLTIPS, 2, i,
		inst, IDB_TOOLBAR, tbb, sizeA(tbb),
		16, 16, 16, 15, sizeof(TBBUTTON));
	GetClientRect(toolbar, &rc);
	MapWindowPoints(toolbar, hWin, (POINT*)&rc, 2);
	toolH= rc.bottom;
	if(toolBarVisible) ShowWindow(toolbar, SW_SHOW);

	langChanged();
	ShowWindow(hWin, cmdShow);
	initSquare(false);

	UpdateWindow(hWin);
	toolBitmap();
	numButtons();

	while(GetMessage(&mesg, NULL, 0, 0)==TRUE){
		if(!TranslateAccelerator(hWin, haccel, &mesg)){
			TranslateMessage(&mesg);
			DispatchMessage(&mesg);
		}
	}
	if(delreg) deleteini(HKEY_CURRENT_USER);
	return 0;
}
