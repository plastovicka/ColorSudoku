/*
	(C) 2005-2009  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "resource.h"
#include "lang.h"

#define Mmulti 8            //number of grids
#define Msymbol 20          //number of symbols
#define Mlen Msymbol        //line length
#define Msize Msymbol       //single grid size
#define Mextentx (Msize*5-8) //entire board width
#define Mextenty (Msize*3-4) //entire board height
#define Mboard (Mextentx*Mextenty) //number of squares
#define Mline (Mmulti*(3*Msize+2)) //number of lines
#define Mintersect (Mmulti*(Msize+2)*13) //number of intersections
#define MgroupLen 12  //group length
#define MgroupLen2 5  //variation size
#define Mline1 (Mmulti*5) //max. lines on one square
#define Minter1 Mlen  //max. intersections of one line
#define Mgrp2 6       //max. temp. groups on one square
#define Dscore 8      //number of lines in a score table
#define Dname 16      //player name length+1
#define Msolution 100 //max. solutions
#define ID_SYMBOL 3000
#define ID_SIZE 4000
#define ID_MULTI 450

struct Tline;
struct Tintersect;
struct Tgroup;

enum{
	clLastScore=20, clBkgnd, clGrid, clBoldGrid,
	clNumber, clError, clSum, clSumError, clGroup,
clGreater, clGreaterError, clCons, clConsError, clEven, Ncl
};

enum TlineType { L_COLUMN, L_ROW, L_RECTANGLE, L_DIAGONAL };

//------------------------------------------------------------------
struct Tsquare {
	int
		val,       //symbol written on this square, -1 if empty
		n,         //how many symbols can be on this square (count of zero values in mask array)
		lines,     //length of ptr array
		intersects,//length of inter array
		bit,       //which symbols cannot be on this square
		lock,      //the val has been generated and user can't change it
		tmp,
		origVal,
		err, errSum, errGreater, errCons;//player has made a mistake
	Tgroup *group;        //in which group is this square (killer sudoku)
	Tgroup *group2[Mgrp2];//helper groups
	BYTE sign[4];         //1=neighbour is greater, 2=neighbour is less, 0=no sign (greater than sudoku)
	bool cons[4];         //consecutive sign (symbols differ by one)
	int oddEven;          //1=val is odd and displayed number is even, 2=val is even and displayed number is odd
	Tsquare *neighbour[4];//pointers to adjacent up,left,down,right squares, 0 at board borders
	int upper, lower;      //max and min possible symbols (greater than sudoku)
	Tintersect *tmpInter; //temporary variable for allInInter()
	bool mark[Msymbol];   //user pencil marks
	int mask[Msymbol];    //which symbols cannot be on this square
	Tline *ptr[Mline1];   //in which lines is this square
	int indexInLine[Mline1];//position in lines
	Tintersect *inter[Mline1*(Mline1-1)/2];//in which intersections is this square
};

struct Tline {
	int
		len,  //length of ptr array
		n,    //count of empty squares in this line
		intersects;        //length of inter array
	bool changed;
	TlineType type;      //row,column,rectangle,diagonal
	int multipart;       //in which part of multi sudoku lies this line
	Tsquare *ptr[Mlen];  //squares that belong to this line
	int bit[Msymbol];    //on which squares cannot be each symbol
	int number[Msymbol]; //on how many squares can be each symbol
	Tintersect *inter[Minter1]; //intersections with other lines
};

struct Tintersect {
	int len;        //length of square array
	Tline *line[2]; //intersection of which lines
	Tsquare *square[Mlen]; //squares that lies in this intersection
	int number[Msymbol];   //on how many squares can be each symbol
	bool b[Msymbol];       //rule has been applied
};

struct Tgroup {
	int
		len, //length of square array
		sum, //sum of values on squares
		n,   //count of possible variations
		n0,  //count of all variations
		lines; //length of line array
	Tline *line[Mline1]; //whole group is inside these lines
	char *A;  //variations
	Tsquare *square[Msymbol]; //squares that belong to this group
	int number[MgroupLen][Msymbol]; //how many variations contain symbol
	int number0[MgroupLen][Msymbol];
};

struct Tundo {
	char action; //0=symbol,1=backtracking symbol, 2=mark, 3=allInInter, 5=removed candidate
	char num;    //value
	char prev;   //previous value, if action<=1
	BYTE grp;
	Tsquare *s;  //square
	Tintersect *r; //intersection, if action=3
};

struct Ttime
{
	WORD wYear;
	BYTE wMonth;
	BYTE wDay;
	BYTE wHour;
	BYTE wMinute;
};

struct TScore
{
	WCHAR name[Dname];
	WORD playtime;
	Ttime date;
};

struct TscoreTab
{
	TscoreTab *next;
	BYTE gameType, size, diag, killer, greater, cons, oddeven, flags;
	int lastScore;
	TScore score[Dscore];
};

struct Tcoord
{
	int x, y, xd, yd;
};

struct TgameType
{
	int multi;
	Tcoord *coord;
	char *name;
};

struct Txy
{
	int x, y;
};

class Pdf
{
public:
	int count, countPerPage, pageWidth, pageHeight, border, spacing;
	TCHAR fn[256];
	void print(HWND hDlg);
private:
	float gridx, gridy;
	int oldBkColor;
	int sudLeft, sudTop;

	void printSudoku(FILE *f);
	void printGrid(FILE *f, bool bold);
	void printGroup(FILE *f, Tgroup *g);
	void printSign(FILE *f, float x, float y, int o, int err);
	void printCons(FILE *f, float x, float y, int o, int q, int err);
	void printRGB(FILE *f, int colorIndex, char *cmd);
	static void addVertexF(void *ptr, int i, int dir, void *obj);
	static void fprintf(FILE *f, const char *fmt, ...);
	float X(int x);
	float Y(int y);
};

//------------------------------------------------------------------
template <class T> inline void aminmax(T &x, int l, int h){
	if(x<l) x=l;
	if(x>h) x=h;
}

template <class T> inline void amax(T &x, int h){
	if(x>h) x=h;
}

template <class T> inline void amin(T &x, int l){
	if(x<l) x=l;
}

//------------------------------------------------------------------
#ifdef UNICODE 
#define convertT2W(t,w) \
	WCHAR *w=t
#define convertW2T(w,t) \
	TCHAR *t=w
#define convertA2T(a,t) \
	int cnvlen##t=strlen(a)+1;\
	TCHAR *t=(TCHAR*)_alloca(2*cnvlen##t);\
	MultiByteToWideChar(CP_ACP, 0, a, -1, t, cnvlen##t);
#define convertT2A(t,a) \
	int cnvlen##a=wcslen(t)+1;\
	char *a=(char*)_alloca(cnvlen##a);\
	WideCharToMultiByte(CP_ACP, 0, t, -1, a, cnvlen##a, 0,0);
#else
#define convertT2A(t,a) \
	char *a=t
#define convertA2T(a,t) \
	TCHAR *t=a
#define convertW2T(w,t) \
	int cnvlen##t=wcslen(w)+1;\
	TCHAR *t=(TCHAR*)_alloca(cnvlen##t);\
	WideCharToMultiByte(CP_ACP, 0, w, -1, t, cnvlen##t, 0,0);
#define convertT2W(t,w) \
	int cnvlen##w=strlen(t)+1;\
	WCHAR *w=(WCHAR*)_alloca(2*cnvlen##w);\
	MultiByteToWideChar(CP_ACP, 0, t, -1, w, cnvlen##w);
#endif

//------------------------------------------------------------------
extern Tsquare *board; //from up left corner down
extern Tline line[Mline];
extern Tintersect intersects[Mintersect];
extern Tgroup *group;
extern Tundo *undoRec;
extern BYTE *solution;
extern TgameType gameTypeA[];
extern Txy coord[Mmulti];
extern Tline *multiLinePtr[Mmulti];
extern COLORREF colors[Ncl];
extern TscoreTab *score;

extern int size, sizex, sizey, extentx, extenty, gameType, blocked, Nboard, Nsquare, Nsymbol, Nline, Nintersect, Ngroup, Nsign, Ncons, done, diag, symetric, undoPos, redoPos, killer, greater, consecutive, oddeven, inHint, level, undoAllPos, Nsolution, curSolution, symbol0, symbol, showErr, groupPad, multi, Mgroupn, MinLine, MonSquare, Mwing, undoing, Mgroup, showErrTime;
extern int gridx, gridy, toolBarVisible, toolH, width, height, statusBarVisible, signPad, consPad, selectedNum, playtime, insLen, errTime, mainLeft, mainTop, mainW, mainH;
extern bool noScore, inserting, editor;
extern BYTE undoGroupTag;
extern Pdf pdfObject;
extern HWND hWin, statusbar, toolbar;
extern TCHAR playerName[Dname];
extern const TCHAR *subkey, *hiscoreFile;
extern Tsquare *insSquares[Msymbol];
extern TCHAR gameFn[256], bmpFn[256];

void init(bool fromGener);
void initSquare(bool fromGener);
void getExtent(int _gameType, int &_size, int &_extentx, int &_extenty, int &_multi, int &_sizex, int &_sizey);
void gener();
void freeGroups();
void prepareGroup(Tgroup *g);
void prepareGroups();
void put(int num, Tsquare *s);
void put1(int num, Tsquare *s);
void putSign(int q, Tsquare *s, int k);
void putCons(bool q, Tsquare *s, int k);
void putOddEven(int o, Tsquare *s);
void toggleMark(Tsquare *s, int num);
void insertMark(Tsquare *s, int num, bool visible);
void no(int num, Tsquare *s);
Tundo *addUndoRec1(int pos);
Tundo *addUndoRec();
bool undo();
bool undoSymbol();
void undo1();
bool redo();
bool redoSymbol();
void undoTo(int p);
void resolve();
void resolve1();
void resolve2();
bool isFinished();
unsigned rand(unsigned n);
void invalidate();
void invalidateS(Tsquare *s);
void status(int i, TCHAR *txt, ...);
void hint(Tsquare *s);
TCHAR num2char(int v);
int s2x(Tsquare *s);
int s2y(Tsquare *s);
void checkMenus();
void waitOn();
void waitOff();
void checkMenus();
void numButtons();
void printTime(TCHAR *buf, int t);
BOOL CALLBACK ScoreProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP);
BOOL CALLBACK NameProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM);
BOOL CALLBACK SumProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM sum);
typedef void TaddVertex(void*, int, int, void*);
void borderGroup(Tgroup *g, int i, void *p, TaddVertex addVertex, void *obj);
void printPdf();
void paint(HDC dc, RECT *clip);
void readScore();
void lbutton(LPARAM lParam);
void rbutton(LPARAM lParam);
void insertGroup();
void checkErr();
void checkShowErr(bool instantly);
Tsquare *hitTest(LPARAM p, int *side=0);
void key(int cmd);
void select(int n);
void stop();
void startTimer();
bool testTotal();
void undoAll();
void hint();
void save(TCHAR *fn);
void open(TCHAR *fn);
void delGroup(Tgroup *g);
void showMarks();
void delAllMarks();
void addScore();
void newGame();
void writeScore();
void addGroup(int sum, int len, Tsquare **squares);
DWORD getTickCount();
TscoreTab *getScoreTab(BYTE _gameType, BYTE _size, BYTE _diag, BYTE _killer, BYTE _greater, BYTE _cons, BYTE _oddeven, BYTE flags=0);
TscoreTab *findScoreTab(BYTE _gameType, BYTE _size, BYTE _diag, BYTE _killer, BYTE _greater, BYTE _cons, BYTE _oddeven, BYTE flags=0);
void readini();
void writeini();
void deleteini(HKEY root);
void endEditor();
void newGameFormat();
void rdSolution();
bool isGenerated();
void resetSolution();
