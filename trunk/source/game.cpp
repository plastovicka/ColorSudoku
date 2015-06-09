/*
	(C) 2005-2011  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "sudoku.h"

int fontH[2];
HFONT hFont[2];

//---------------------------------------------------------------------------
int X(int x)
{
	return x*gridx;
}

int Y(int y)
{
	int r= y*gridy;
	if(toolBarVisible) r+=toolH;
	return r;
}

int s2x(Tsquare *s)
{
	return int(s-board)/extenty;
}

int s2y(Tsquare *s)
{
	return int(s-board)%extenty;
}

//---------------------------------------------------------------------------
bool test(Tsquare *s)
{
	int k, j, num;
	Tline *e;
	Tsquare *s1;
	Tgroup *g;

	num=s->val;
	if(num>=0 && num<Msymbol){
		//compare with solution
		if(s->origVal>=0) return num!=s->origVal;
		//solution is not available in editor or custom game
		if(s->oddEven==1 && (s->val&1)==0
			|| s->oddEven==2 && (s->val&1)!=0) return true;
		//there must not be two same symbols in one line
		for(j=0; j<s->lines; j++){
			e=s->ptr[j];
			for(k=0; k<e->len; k++){
				s1=e->ptr[k];
				if(s1->val==num && s!=s1) return true;
			}
		}
		//there must not be two same symbols inside one group
		g=s->group;
		if(g){
			for(j=0; j<g->len; j++){
				s1=g->square[j];
				if(s1->val==num && s!=s1) return true;
			}
		}
	}
	return false;
}

bool testSum(Tsquare *t)
{
	int j, S;
	Tsquare *s, *m;
	Tgroup *g;

	g=t->group;
	if(!g) return false;
	m=g->square[0];
	S=g->len;
	for(j=0; j<g->len; j++){
		s=g->square[j];
		if(s<m) m=s;
		if(s->val<0) return false;
		S+=s->val;
	}
	return t==m && S!=g->sum;
}

int testGreater(Tsquare *t)
{
	int j, q, result;
	Tsquare *s;

	result=0;
	if(t->val>=0){
		for(j=0; j<4; j++){
			q= t->sign[j];
			if(q){
				s= t->neighbour[j];
				if(s->val>=0 && (q==1 ? s->val<=t->val : s->val>=t->val)){
					result|= 1<<j;
				}
			}
		}
	}
	return result;
}

int testConsecutive(Tsquare *t)
{
	int j, result;
	Tsquare *s;

	result=0;
	if(consecutive && t->val>=0){
		for(j=0; j<4; j++){
			s= t->neighbour[j];
			if(s && s->val>=0 &&
				(s->val==t->val+1 || s->val==t->val-1) != t->cons[j]){
				result|= 1<<j;
			}
		}
	}
	return result;
}

bool testTotal()
{
	int i, S, S0, n;
	Tgroup *g;

	S=n=0;
	for(i=0; i<Ngroup; i++){
		g=&group[i];
		if(g->len>0){
			n+=g->len;
			S+=g->sum;
		}
	}
	S0=Nsquare*(Nsymbol+1)/2;
	if(n<Nsquare || S==S0) return false;
	msglng(800, "Total sum of all groups is %d, but it should be %d", S, S0);
	return true;
}

void checkErr()
{
	int i, eg, e, es, ec, et;
	Tsquare *t;

	et=0;
	for(i=0; i<Nboard; i++){
		t=&board[i];
		e=test(t);
		es=testSum(t);
		eg=testGreater(t);
		ec=testConsecutive(t);
		et|=e|es|eg|ec;
		if(e!=t->err || es!=t->errSum || eg!=t->errGreater || ec!=t->errCons){
			if(showErr || t->err==2) invalidateS(t);
			t->err=e;
			t->errSum=es;
			t->errGreater=eg;
			t->errCons=ec;
		}
	}
	if(!et) errTime=-1;
	else if(errTime<0) errTime=playtime;
	checkShowErr(false);
}

void checkShowErr(bool instantly)
{
	int i;
	Tsquare *t;

	if(!showErr){
		if(errTime>=0 && (instantly || editor || playtime>=errTime+showErrTime && showErrTime>=0)){
			showErr=1;
			noScore=true;
			for(i=0; i<Nboard; i++){
				t=&board[i];
				if(t->err || t->errSum || t->errGreater || t->errCons) invalidateS(t);
			}
		}
	}
	else{
		if(errTime<0) showErr=0;
	}
}

bool isFinished()
{
	int i;
	Tsquare *s;

	if(done!=Nsquare) return false;
	for(i=0; i<Nboard; i++){
		s=&board[i];
		if(test(s) || testSum(s) || testGreater(s) || testConsecutive(s)) return false;
	}
	return true;
}
//---------------------------------------------------------------------------
void paintSign(HDC dc, int x, int y, int o, int err)
{
	int d, dx, dy;
	HGDIOBJ oldP=0;
	static const int VX[]={0, 0, -1, 1}, VY[]={-1, 1, 0, 0};

	if(err && showErr){
		oldP=SelectObject(dc, CreatePen(PS_SOLID, 0, colors[clGreaterError]));
	}
	d=max(gridy/9, 2);
	dx=VX[o]*d; dy=VY[o]*d;
	MoveToEx(dc, x-dx, y-dy, 0);
	LineTo(dc, x+dx+dy, y+dy+dx);
	MoveToEx(dc, x-dx, y-dy, 0);
	LineTo(dc, x+dx-dy, y+dy-dx);
	if(err && showErr){
		DeleteObject(SelectObject(dc, oldP));
	}
}

void paintCons(HDC dc, int x, int y, int o, bool q, int err)
{
	int dx, dy, d;
	HGDIOBJ oldP=0;

	if(!showErr) err=0;
	if(!q && !err) return;
	if(err){
		oldP=SelectObject(dc, CreatePen(PS_SOLID, 2, colors[clConsError]));
	}
	dx=dy=0;
	d= q ? 4 : 8;
	if(!o) dx=max(gridx/d, 3);
	else dy=max(gridy/d, 3);
	MoveToEx(dc, x-dx, y-dy, 0);
	LineTo(dc, x+dx, y+dy);
	if(err){
		DeleteObject(SelectObject(dc, oldP));
	}
}

// dir: 0=top left, 1=bottom left, 2=bottom right, 3=top right
void addVertex(void *ptr, int i, int dir, void *)
{
	static int gx[]={0, 0, 1, 1};
	static int gy[]={0, 1, 1, 0};
	static int bx[]={1, 1, -1, -1};
	static int by[]={1, -1, -1, 1};

	dir&=3;
	POINT *&p = *(POINT **)ptr;
	p->x=X(i/extenty+gx[dir])+groupPad*bx[dir]-(bx[dir]<0);
	p->y=Y(i%extenty+gy[dir])+groupPad*by[dir]-(by[dir]<0);
	p++;
}

void paintGroup(HDC dc, Tgroup *g)
{
	int j, k;
	Tsquare *s, *m;
	POINT pt[Mboard*2+2], *p;
	TCHAR buf[10];

	if(!g->len) return;

	SetBkMode(dc, TRANSPARENT);
	for(k=0; k<g->len; k++){
		g->square[k]->tmp=0; //initialize visited edges mask
	}
	for(k=0; k<g->len; k++){
		p=pt;
		borderGroup(g, g->square[k]-board, &p, &addVertex, 0);
		assert(p<=endA(pt));
		Polygon(dc, pt, p-pt);
	}
	//find top left corner
	m=g->square[0];
	for(j=0; j<g->len; j++){
		s=g->square[j];
		if(s<m) m=s;
	}
	//print sum
	SetTextColor(dc, colors[m->errSum && showErr ? clSumError : clSum]);
	SetTextAlign(dc, TA_LEFT|TA_TOP);
	SetBkMode(dc, OPAQUE);
	SetBkColor(dc, colors[m->oddEven==1 ? clEven : clBkgnd]);
	_itot(g->sum, buf, 10);
	HGDIOBJ oldF= SelectObject(dc, GetStockObject(SYSTEM_FONT));
	TextOut(dc, X(s2x(m))+2, Y(s2y(m))+2, buf, _tcslen(buf));
	SelectObject(dc, oldF);
}

void paint(HDC dc, RECT *clip)
{
	int x, y, i, x1, y1, v, h[2], x0, y0, ng, gridx1, gridy1, oldCl, cl;
	TCHAR c;
	Tsquare *s;
	Tgroup *g;
	HGDIOBJ oldP, oldB, newB;
	static LOGFONT font;
	RECT rc;
	static int oldDone, oldNg;
	const int kdx=15;
	HBRUSH clbr[Msymbol], brEven, brBkgnd;
	HBRUSH brSysBkgnd= GetSysColorBrush(COLOR_BTNFACE);

	if(extentx==0) return;
	gridx=width/extentx;
	y0= toolBarVisible ? toolH : 0;
	GetClientRect(statusbar, &rc);
	gridy=(height-(statusBarVisible ? rc.bottom : 0)-y0)/extenty;
	gridx1=gridx-2; gridy1=gridy-2;
	if(killer){ gridx1-=groupPad*2+kdx; gridy1-=groupPad*2; }
	if(greater){ gridx1-=signPad*2; gridy1-=signPad*2; }
	else if(consecutive){ gridx1-=consPad*2; gridy1-=consPad*2; }
	//create fonts
	for(i=0; i<2; i++){
		h[i]= int((i ? gridy1/sizey*0.85 : gridy*0.8));
		if(h[i]!=fontH[i]){
			fontH[i]=h[i];
			memset(&font, 0, sizeof(LOGFONT));
			font.lfHeight=-h[i];
			_tcscpy(font.lfFaceName, _T("Arial"));
			DeleteObject(hFont[i]);
			hFont[i]=CreateFontIndirect(&font);
		}
	}
	//create brushes
	if(symbol==2){
		for(i=0; i<Nsymbol; i++){
			clbr[i]=CreateSolidBrush(colors[i]);
		}
	}
	brEven=CreateSolidBrush(colors[clEven]);
	brBkgnd=CreateSolidBrush(colors[clBkgnd]);
	oldB=0;
	oldCl=-1;
	oldP=SelectObject(dc, CreatePen(PS_SOLID, 0, colors[clGrid]));
	SetTextAlign(dc, TA_CENTER|TA_TOP);
	SetBkMode(dc, TRANSPARENT);
	i=0;
	for(x=0; x<extentx; x++){
		for(y=0; y<extenty; y++){
			s=&board[i++];
			x0=X(x); y0=Y(y);
			x1=X(x+1); y1=Y(y+1);
			if(clip && (x0>clip->right || y0>clip->bottom || x1<clip->left || y1<clip->top)) continue;
			v=s->val;
			if(v>=Msymbol){
				//out of board (Samurai)
				SetRect(&rc, x0, y0, x1, y1);
				FillRect(dc, &rc, brSysBkgnd);
				continue;
			}
			cl= s->oddEven==1 ? clEven : clBkgnd;
			newB= s->oddEven==1 ? brEven : brBkgnd;
			c=0;
			if(v>=0){
				switch(symbol){
					default: //digits
						c= num2char(v);
						break;
					case 1: //letters
						c= (TCHAR)('A'+v);
						break;
					case 2: //colors
						if(showErr && s->err==1) c='X';
						newB=clbr[v];
						break;
				}
			}
			//paint square
			if(newB!=oldB) SelectObject(dc, oldB=newB);
			if(cl!=oldCl) SetBkColor(dc, colors[oldCl=cl]);
			Rectangle(dc, x0, y0, x1, y1);
			if(s->err==2) c='!';
			if(v<0 && symbol==2){
				SetPixel(dc, x0+gridx/2, y0+gridy/2, 0);
			}
			if(v<0){
				//draw pencil marks
				SelectObject(dc, hFont[1]);
				SetTextColor(dc, colors[clNumber]);
				v=0;
				for(y1=0; y1<sizey; y1++){
					for(x1=0; x1<sizex; x1++){
						if(s->mark[v]){
							rc.left=x0+x1*gridx1/sizex+1;
							rc.top=y0+y1*gridy1/sizey+1;
							rc.right=x0+(x1+1)*gridx1/sizex+1;
							rc.bottom=y0+(y1+1)*gridy1/sizey+1;
							if(killer) OffsetRect(&rc, groupPad+kdx, groupPad);
							if(greater) OffsetRect(&rc, signPad, signPad);
							else if(consecutive) OffsetRect(&rc, consPad, consPad);
							TCHAR ch=0;
							switch(symbol){
								default: //digits
									ch= num2char(v);
									break;
								case 1: //letters
									ch= (TCHAR)('A'+v);
									break;
								case 2: //colors
									FillRect(dc, &rc, clbr[v]);
									break;
							}
							if(ch) TextOut(dc, (rc.left+rc.right+1)>>1,
								rc.top+(rc.bottom-rc.top-fontH[1])/2, &ch, 1);
						}
						v++;
					}
				}
			}
			//print symbol
			if(c){
				SetTextColor(dc, colors[(showErr && s->err || s->err==2) ? clError : clNumber]);
				SelectObject(dc, hFont[0]);
				TextOut(dc, x0+((gridx+1)>>1), y0+(gridy-fontH[0])/2, &c, 1);
			}
		}
	}
	//delete brushes
	SelectObject(dc, GetStockObject(NULL_BRUSH));
	if(symbol==2){
		for(i=0; i<Nsymbol; i++){
			DeleteObject(clbr[i]);
		}
	}
	DeleteObject(brEven);
	DeleteObject(brBkgnd);
	SelectObject(dc, GetStockObject(SYSTEM_FONT));
	//border
	rc.left=X(extentx);
	rc.top=Y(0);
	rc.right=width;
	rc.bottom=height;
	FillRect(dc, &rc, brSysBkgnd);
	rc.left=0;
	rc.top=Y(extenty);
	FillRect(dc, &rc, brSysBkgnd);
	//bold grid
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 0, colors[clBoldGrid])));
	if(sizex){
		for(i=0; i<multi; i++){
			x1=coord[i].x;
			y1=coord[i].y;
			for(x=x1; x<x1+size; x+=sizex){
				for(y=y1; y<y1+size; y+=sizey){
					Rectangle(dc, X(x), Y(y), X(x+sizex), Y(y+sizey));
				}
			}
		}
	}
	//groups
	DeleteObject(SelectObject(dc, CreatePen(PS_DASH, 0, colors[clGroup])));
	ng=0;
	for(i=0; i<Ngroup; i++){
		g=&group[i];
		if(g->len){
			paintGroup(dc, g);
			ng++;
		}
	}
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 2, colors[clCons])));
	//consecutive signs
	i=0;
	for(x=0; x<extentx; x++){
		for(y=0; y<extenty; y++){
			s=&board[i++];
			paintCons(dc, (X(x)+X(x+1))>>1, Y(y), 0, s->cons[0], s->errCons&1);
			paintCons(dc, X(x), (Y(y)+Y(y+1))>>1, 1, s->cons[1], s->errCons&2);
		}
	}
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 0, colors[clGreater])));
	//greater than signs
	i=0;
	for(x=0; x<extentx; x++){
		for(y=0; y<extenty; y++){
			s=&board[i++];
			if(s->sign[0]){
				paintSign(dc, (X(x)+X(x+1))>>1, Y(y), s->sign[0]-1, s->errGreater&1);
			}
			if(s->sign[1]){
				paintSign(dc, X(x), (Y(y)+Y(y+1))>>1, s->sign[1]+1, s->errGreater&2);
			}
		}
	}
	DeleteObject(SelectObject(dc, oldP));
	//status bar
	if(Nboard && (oldDone!=done || oldNg!=ng)){
		oldDone=done;
		oldNg=ng;
		if(!done && ng){
			status(0, _T("%d %s"), ng, lng(660, "groups"));
		}
		else{
			status(0, _T("%d  (%d%%)"), done, done*100/Nsquare);
		}
	}
}

void getSquareRect(Tsquare *s, RECT *rc)
{
	int x, y;

	x=s2x(s); y=s2y(s);
	rc->left=X(x);
	rc->right=X(x+1);
	rc->top=Y(y);
	rc->bottom=Y(y+1);
}

void invalidateS(Tsquare *s)
{
	RECT rc;
	getSquareRect(s, &rc);
	InvalidateRect(hWin, &rc, TRUE);
}

void invalidate()
{
	InvalidateRect(hWin, 0, TRUE);
}
//---------------------------------------------------------------------------
Tsquare *hitTest(LPARAM p, int *side)
{
	int x, y, k, d, m, rx[4], ry[4];
	Tsquare *s;
	RECT rc;

	x=LOWORD(p)/gridx;
	y=(HIWORD(p)-(toolBarVisible ? toolH : 0))/gridy;
	if(!(x>=0 && x<extentx && y>=0 && y<extenty)) return 0;
	s=&board[x*extenty+y];
	if(side){
		getSquareRect(s, &rc);
		rx[0]=rx[2]=(rc.left+rc.right)>>1;
		rx[1]=rc.left;
		rx[3]=rc.right-1;
		ry[1]=ry[3]=(rc.top+rc.bottom)>>1;
		ry[0]=rc.top;
		ry[2]=rc.bottom-1;
		m=0x7fffffff;
		for(k=0; k<4; k++){
			d=abs(rx[k]-LOWORD(p))+abs(ry[k]-HIWORD(p));
			if(d<m){
				m=d;
				*side=k;
			}
		}
	}
	return s;
}

void startTimer()
{
	SetTimer(hWin, 1000, 1000, 0);
}

void stop()
{
	KillTimer(hWin, 1000);
}

static int toolButtonA[]={ID_DEL, ID_INS, ID_SIGN, ID_CONS, ID_EVEN};
static TCHAR *statusToolA[]={_T("Del"), _T("Sum"), _T(">"), _T(" |"), _T("I/II")};

void select(int n)
{
	if(n<-5 || n>=size || (n==-4 && !consecutive) || (n==-5 && !oddeven)) return;
	if(selectedNum>=0){
		SendMessage(toolbar, TB_CHECKBUTTON, ID_SYMBOL+selectedNum, MAKELONG(FALSE, 0));
	}
	else{
		for(int i=1; i<=sizeA(toolButtonA); i++){
			if(selectedNum==-i){
				SendMessage(toolbar, TB_CHECKBUTTON, toolButtonA[i-1], MAKELONG(FALSE, 0));
			}
		}
	}
	selectedNum=n;
	if(n>=0){
		SendMessage(toolbar, TB_CHECKBUTTON, ID_SYMBOL+n, MAKELONG(TRUE, 0));
		status(1, _T("%c"), (symbol==1) ? (TCHAR)('A'+n) : num2char(n));
	}
	else{
		for(int i=1; i<=sizeA(toolButtonA); i++){
			if(n==-i){
				status(1, statusToolA[i-1]);
				SendMessage(toolbar, TB_CHECKBUTTON, toolButtonA[i-1], MAKELONG(TRUE, 0));
			}
		}
	}
}

void key(int cmd)
{
	if(cmd>='A' && cmd<='Z'){
		select(cmd-(symbol==1 ? 'A' : 'A'-9));
	}
	if(cmd>='1' && cmd<='9'){
		select(cmd-'1');
	}
	if(cmd>=VK_NUMPAD1 && cmd<=VK_NUMPAD9){
		select(cmd-VK_NUMPAD1);
	}
	if(cmd==VK_SPACE || cmd==VK_DELETE){
		select(-1);
	}
	if(cmd==VK_INSERT){
		select(-2);
	}
	if(cmd==0xBC || cmd==0xBE){ // <, >
		select(-3);
	}
	if(cmd==0xDC){ // |
		select(-4);
	}
	if(cmd==0xBF){ // /
		select(-5);
	}
}

DWORD getTickCount()
{
	static LARGE_INTEGER freq;
	if(!freq.QuadPart){
		QueryPerformanceFrequency(&freq);
		if(!freq.QuadPart) return GetTickCount();
	}
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (DWORD)(c.QuadPart*1000/freq.QuadPart);
}

void newGame()
{
	checkMenus();
	waitOn();
	DWORD t=getTickCount();
	do{
		gener();
	} while(greater && Nsign<size/4 && getTickCount()-t < 10000);
#ifdef _DEBUG
	status(4, _T("%d ms"), getTickCount()-t);
#endif
	waitOff();
	noScore=false;
	playtime=0;
	startTimer();
	invalidate();
}

void newGameFormat()
{
	if(editor){
		checkMenus();
		initSquare(false);
	}
	else{
		newGame();
	}
}

bool isGenerated()
{
	int i;
	Tsquare *s;

	for(i=0; i<Nboard; i++){
		s=&board[i];
		if(s->origVal<0) return false;
	}
	return true;
}

void resetSolution()
{
	int i;
	Tsquare *s;

	for(i=0; i<Nboard; i++){
		s=&board[i];
		if(s->origVal>=0 && s->origVal<Msymbol) s->origVal=-1;
	}
	Nsolution=0;
	noScore=true;
	checkErr();
}

//---------------------------------------------------------------------------
void hint(Tsquare *s)
{
	if(s->val>=0) return;
	s->err=2;
	invalidateS(s);
}

void hint()
{
	int i, j, k, p;
	Tline *e;
	Tundo *u;
	Tsquare *s;

	//oneOnSquare
	for(i=0; i<Nboard; i++){
		s=&board[i];
		if(s->n==1) hint(s);
	}
	//oneInLine
	for(j=0; j<Nline; j++){
		e=&line[j];
		for(k=0; k<Nsymbol; k++){
			if(e->number[k]==1){
				for(i=0; i<e->len; i++){
					s=e->ptr[i];
					if(!s->mask[k]) hint(s);
				}
			}
		}
	}
	/*for(i=0; i<Nboard; i++){
		if(board[i].err==2) return; //some hints were found
		}*/
	p=undoPos;
	inHint=done+1;
	resolve1();
	inHint=0;
	redoPos=undoPos;
	undoTo(p);
	if(redoSymbol()){
		u=&undoRec[undoPos-1];
		assert(u->action==0 || u->action==1);
		s=u->s;
		undo();
		hint(s);
	}
	freeGroups();
}
//---------------------------------------------------------------------------
void undoAll()
{
	if(!redoPos) redoPos=undoPos;
	while(undoPos>0){
		undo1();
		Tundo *u=&undoRec[undoPos];
		if(u->action<=2) invalidateS(u->s);
		if(undoPos==undoAllPos) break;
	}
}

bool isLegalValue(Tsquare *s, int val)
{
	int i, j;
	Tgroup *g;
	Tsquare *t2;

	if(s->mask[val]!=0) return false;

	//killer sudoku
	g=s->group;
	if(g){
		for(i=0; i<g->len; i++){
			if(g->square[i]->val==val) return false;
		}
	}
	//greater than sudoku
	if(Nsign>0){
		for(j=0; j<4; j++){ //up,left,down,right
			t2= s->neighbour[j];
			switch(s->sign[j]){
				case 1:
					if(t2->upper<=val) return false;
					break;
				case 2:
					if(t2->lower>=val) return false;
					break;
			}
		}
	}
	//consecutive sudoku
	if(consecutive){
		for(j=0; j<4; j++){ //up,left,down,right
			t2= s->neighbour[j];
			if(t2 && t2->val>=0){
				i=abs(t2->val - val);
				if(s->cons[j] != (i==1)) return false;
			}
		}
	}
	return true;
}

void beginMacro()
{
	static BYTE undoGroupTagCounter;
	undoGroupTagCounter++;
	if(undoGroupTagCounter==0) undoGroupTagCounter=1;
	undoGroupTag = undoGroupTagCounter;
}

void endMacro()
{
	undoGroupTag = 0;
}

void showMarks()
{
	int i, j;
	Tsquare *t;

	beginMacro();
	for(i=0; i<Nboard; i++){
		t=&board[i];
		if(t->val<0){
			for(j=0; j<Nsymbol; j++){
				insertMark(t, j, isLegalValue(t, j));
			}
		}
	}
	endMacro();
}

void delAllMarks()
{
	int i, j;
	Tsquare *t;

	beginMacro();
	for(i=0; i<Nboard; i++){
		t=&board[i];
		for(j=0; j<Nsymbol; j++){
			insertMark(t, j, false);
		}
	}
	endMacro();
}

void endEditor()
{
	int i;
	Tsquare *s;

	select(-1);
	for(i=0; i<Nboard; i++){
		s=&board[i];
		s->lock= (s->val!=-1);
	}
	undoPos=redoPos=undoAllPos=0;
	playtime=0;
	startTimer();
	noScore=true;
}
//---------------------------------------------------------------------------
void addGroup(int sum, int len, Tsquare **squares)
{
	int i;
	Tgroup *g;
	Tsquare *s;

	if(sum<=0 || len>Nsymbol) return;
	//find empty slot
	for(i=0;; i++){
		if(i==Ngroup){
			if(i==Mgroup) return;
			Ngroup++;
			break;
		}
		if(group[i].len==0) break;
	}
	//create group
	g=&group[i];
	g->sum=sum;
	g->len=0;
	g->n=0;
	for(i=0; i<len; i++){
		s=squares[i];
		if(!s->group){
			g->square[g->len++]=s;
			s->group=g;
		}
	}
}

void delGroup(Tgroup *g)
{
	if(!g) return;
	for(int i=0; i<g->len; i++){
		g->square[i]->group=0;
	}
	g->len=0;
}

void invalidateGroup(Tgroup *g)
{
	if(!g) return;
	for(int i=0; i<g->len; i++){
		invalidateS(g->square[i]);
	}
}

void insertGroup()
{
	int i, sum;
	bool z;
	Tgroup *g, *g1, *g2;
	Tsquare *s;

	g1=g2=0;
	z=false;
	for(i=0; i<insLen; i++){
		g=insSquares[i]->group;
		if(!g) z=true;
		else if(g!=g1 && g!=g2){
			if(!g1) g1=g;
			else if(!g2) g2=g;
			else return;
		}
	}
	if(g2){
		if(z) return;
		//join two groups
		for(i=0; i<g2->len; i++){
			(g1->square[g1->len++]=g2->square[i])->group=g1;
		}
		g1->sum+=g2->sum;
		invalidateGroup(g2);
		g2->len=0;
	}
	else if(!g1 || insLen==1){
		sum= DialogBoxParam(inst, MAKEINTRESOURCE(IDD_SUM), hWin, (DLGPROC)SumProc, g1 ? g1->sum : 0);
		if(sum>9999) return;
		if(sum==0){
			//delete group
			invalidateGroup(g1);
			delGroup(g1);
		}
		else if(!g1){
			//create new group
			addGroup(sum, insLen, insSquares);
			g1=insSquares[0]->group;
		}
		else{
			//change sum
			g1->sum=sum;
		}
	}
	else{
		//enlarge group
		for(i=0; i<insLen; i++){
			s=insSquares[i];
			if(!s->group && g1->len<Nsymbol){
				(g1->square[g1->len++]=s)->group=g1;
			}
		}
	}
	invalidateGroup(g1);
}

void insertSign(LPARAM lParam, int what)
{
	int k;
	int q;
	Tsquare *s;

	s= hitTest(lParam, &k);
	if(s && s->neighbour[k]){
		if(k>1) what=BYTE(3-what);
		if(s->sign[k]==what) q=0;
		else q=what;
		putSign(q, s, k);
		invalidateS(s->neighbour[k]);
		invalidateS(s);
		resetSolution();
	}
}

void insertCons(LPARAM lParam)
{
	int k;
	Tsquare *s;

	s= hitTest(lParam, &k);
	if(s && s->neighbour[k]){
		putCons(!s->cons[k], s, k);
		invalidateS(s->neighbour[k]);
		invalidateS(s);
		resetSolution();
	}
}

void insertOddEven(LPARAM lParam)
{
	Tsquare *s;

	s= hitTest(lParam, 0);
	if(s){
		putOddEven(s->oddEven == 1 ? 2 : 1, s);
		invalidateS(s);
		resetSolution();
	}
}

void insertMark(Tsquare *s, int num, bool visible)
{
	if(s->mark[num]!=visible){
		Tundo *u = &undoRec[undoPos];
		if(undoPos>=redoPos || u->s!=s || u->num!=num || u->action!=2){
			u=addUndoRec();
			u->action=2;
			u->s=s;
			u->num=(char)num;
			redoPos=0;
		}
		else{
			undoPos++;
		}
		s->mark[num]=visible;
		invalidateS(s);
	}
}

void toggleMark(Tsquare *s, int num)
{
	insertMark(s, num, !s->mark[num]);
}
//------------------------------------------------------------------
void lbutton(LPARAM lParam)
{
	Tsquare *s;

	s= hitTest(lParam);
	if(s){
		if(selectedNum==-2){
			inserting=true;
			SetCapture(hWin);
			insSquares[0]=s;
			insLen=1;
		}
		else if(selectedNum==-3){
			//insert greater than sign
			insertSign(lParam, 1);
		}
		else if(selectedNum==-4){
			//insert consecutive sign
			insertCons(lParam);
		}
		else if(selectedNum==-5){
			//toggle odd/even
			insertOddEven(lParam);
		}
		else if((!s->lock || editor) && s->val!=selectedNum){
			put(selectedNum, s);
			invalidateS(s);
			if(editor) resetSolution();
			checkErr();
			if(isFinished()){
				stop();
				status(4, lng(661, "GAME OVER"));
				if(!noScore) addScore();
			}
		}
	}
}

void rbutton(LPARAM lParam)
{
	Tsquare *s= hitTest(lParam);
	if(selectedNum==-3){
		//put greater than sign
		insertSign(lParam, 2);
	}
	else if(s && s->val<0 && selectedNum>=-1){
		if(selectedNum<0){
			//delete all marks on one square
			beginMacro();
			for(int i=0; i<Nsymbol; i++){
				insertMark(s, i, false);
			}
			endMacro();
		}
		else{
			toggleMark(s, selectedNum);
		}
	}
}

#ifdef _DEBUGM
void mousemove(LPARAM lParam)
{
	int i;
	TCHAR buf[Msymbol*5];

	Tsquare *s= hitTest(lParam);
	if(s){
		buf[0]=0;
		for(i=0; i<Nsymbol; i++){
			_stprintf(_tcschr(buf,0),_T(" %d,"),s->mask[i]);
		}
		_tcschr(buf,0)[-1]=0;
		status(4,buf);
	}
}
#endif
//------------------------------------------------------------------
