/*
	(C) 2007-2011  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/

#include "hdr.h"
#include "sudoku.h"

Pdf pdfObject;


float Pdf::X(int x)
{
	return x*gridx + sudLeft;
}

float Pdf::Y(int y)
{
	return y*gridy + sudTop;
}

void Pdf::fprintf(FILE *f, const char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	for(char *s=buf; *s; s++){
		if(*s==',') *s='.';
	}
	fputs(buf, f);
	va_end(ap);
}

void Pdf::printRGB(FILE *f, int colorIndex, char *cmd)
{
	if(cmd[0]=='r'){
		if(colorIndex==oldBkColor) return;
		oldBkColor=colorIndex;
	}
	COLORREF c = colors[colorIndex];
	if(colorIndex==clBkgnd && symbol!=2) c=0xffffff;
	fprintf(f, "%.2f %.2f %.2f %s\n", (float)GetRValue(c)/255, (float)GetGValue(c)/255, (float)GetBValue(c)/255, cmd);
}

void Pdf::printSign(FILE *f, float x, float y, int o, int err)
{
	float d, dx, dy;
	static const int VX[]={0, 0, -1, 1}, VY[]={-1, 1, 0, 0};

	if(err && showErr){
		printRGB(f, clGreaterError, "RG");
	}
	d=gridy*0.13f;
	dx=VX[o]*d; dy=VY[o]*d;
	fprintf(f, "%.1f %.1f m\n%.1f %.1f l\n%.1f %.1f m\n%.1f %.1f l\nS\n",
		x-dx, y-dy, x+dx+dy, y+dy+dx,
		x-dx, y-dy, x+dx-dy, y+dy-dx);
	if(err && showErr){
		printRGB(f, clGreater, "RG");
	}
}

void Pdf::printCons(FILE *f, float x, float y, int o, int q, int err)
{
	float dx, dy, d;

	if(!showErr) err=0;
	if(!q && !err) return;
	if(err){
		printRGB(f, clConsError, "RG");
	}
	dx=dy=0;
	d= q ? 4.0f : 8.0f;
	if(!o) dx=gridx/d;
	else dy=gridy/d;
	fprintf(f, "%.1f %.1f m\n%.1f %.1f l\nS\n",
		x-dx, y-dy, x+dx, y+dy);
	if(err){
		printRGB(f, clCons, "RG");
	}
}

struct PointF
{
	float x, y;
};

void Pdf::addVertexF(void *ptr, int i, int dir, void *obj)
{
	static int gx[]={0, 0, 1, 1};
	static int gy[]={0, 1, 1, 0};
	static int bx[]={1, 1, -1, -1};
	static int by[]={1, -1, -1, 1};

	dir&=3;
	PointF *&p = *(PointF **)ptr;
	Pdf &o = *(Pdf*)obj;
	p->x= o.X(i/extenty+gx[dir]) + o.gridx*0.1f * bx[dir] - (bx[dir]<0);
	p->y= o.Y(i%extenty+gy[dir]) + o.gridy*0.1f * by[dir] - (by[dir]<0);
	p++;
}


void borderGroup(Tgroup *g, int i, void *p, TaddVertex addVertex, void *obj)
{
	int i0, d0, dir, dir0, dirL;
	Tsquare *s, *sn;

	s=&board[i];
	if(s->tmp & 1) return; //top border already drawed
	sn=s->neighbour[0];
	if(sn && sn->group==g) return; //this is inner square

	dir=3; //right
	//turn until next square is found
	for(;;){
		sn=s->neighbour[dir];
		if(sn && sn->group==g) break;
		dir=(dir-1)&3; //turn right
		if(dir==0){
			//only one square
			for(dir=0; dir<4; dir++) addVertex(p, i, dir, obj);
			return;
		}
	}
	s=sn;
	i=int(s-board);
	//remember start position and direction
	i0=i;
	d0=dir;
	//go clockwise
	do{
		//borderline is in bottom left corner of square s
		dir0=dir;
		dirL=(dir+1)&3; //turn left
		//turn right until next direction is found
		for(dir=dirL;; dir=(dir-1)&3){
			sn=s->neighbour[dir];
			if(sn && sn->group==g) break;
			s->tmp|= 1<<dir; //mark visited edges
		}
		//create vertex
		if(dir==dirL){
			addVertex(p, i, dirL, obj); //turned left
		}
		else if(dir!=dir0){
			addVertex(p, i, dir0, obj); //turned right
			if(dir==((dirL+1)&3)){
				addVertex(p, i, dir+1, obj); //backwards
			}
		}
		s=sn;
		i=int(s-board);
	} while(i!=i0 || dir!=d0);
}

void Pdf::printGroup(FILE *f, Tgroup *g)
{
	int j, k, xm, ym, x, y;
	Tsquare *s, *m;
	PointF pt[Mboard*2+2], *p, *q;

	if(!g->len) return;
	for(k=0; k<g->len; k++){
		g->square[k]->tmp=0; //initialize visited edges mask
	}
	for(k=0; k<g->len; k++){
		p=pt;
		borderGroup(g, g->square[k]-board, &p, &addVertexF, this);
		assert(p<=endA(pt));
		fprintf(f, "%.1f %.1f m\n", (p-1)->x, (p-1)->y);
		for(q= pt; q<p; q++){
			fprintf(f, "%.1f %.1f l\n", q->x, q->y);
		}
		fprintf(f, "S\n");
	}
	//find bottom left corner
	m=g->square[0];
	xm= s2x(m);
	ym= s2y(m);
	for(j=0; j<g->len; j++){
		s=g->square[j];
		x=s2x(s);
		y=s2y(s);
		if(x<xm || x==xm && y>ym){
			m=s;
			xm=x; ym=y;
		}
	}
	//print sum
	printRGB(f, m->errSum && showErr ? clSumError : clSum, "rg");
	fprintf(f, "BT\n%.1f %.1f Td\n(%d) Tj\nET\n",
		X(s2x(m))+gridx*0.16,
		Y(s2y(m))+gridy*(countPerPage > 6 ? 0.5 : 0.6),
		g->sum);
}

void Pdf::printGrid(FILE *f, bool bold)
{
	int i, x, y, x1, y1;
	float d;

	for(i=0; i<multi; i++){
		x1=coord[i].x;
		y1=coord[i].y;
		//vertical lines
		for(x=0; x<=size; x++){
			if((sizex && x%sizex==0) == bold){
				d= X(x+x1);
				fprintf(f, "%.1f %.1f m\n%.1f %.1f l\nS\n",
					d, Y(y1), d, Y(y1+size));
			}
		}
		//horizontal lines
		for(y=0; y<=size; y++){
			if((sizey && y%sizey==0) == bold){
				d= Y(y+y1);
				fprintf(f, "%.1f %.1f m\n%.1f %.1f l\nS\n",
					X(x1), d, X(x1+size), d);
			}
		}
	}
}

void Pdf::printSudoku(FILE *f)
{
	int x, y, i, v, ng, cl, newB;
	float x0, y0;
	TCHAR c;
	Tsquare *s;
	Tgroup *g;

	//background
	oldBkColor=-1;
	printRGB(f, clBkgnd, "rg");
	printRGB(f, clBoldGrid, "RG");
	for(i=0; i<multi; i++){
		fprintf(f, "%.1f %.1f %.1f %.1f re\nB\n",
			X(coord[i].x), Y(coord[i].y), size*gridx, size*gridy);
	}

	i=0;
	for(x=0; x<extentx; x++){
		for(y=0; y<extenty; y++){
			s=&board[i++];
			x0= X(x); y0= Y(y);
			v=s->val;
			if(v>=Msymbol) continue;
			newB=-1;
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
						newB=v;
						break;
				}
			}
			else if(s->oddEven==1){
				newB=clEven;
			}
			if(newB>=0){
				printRGB(f, newB, "rg");
				fprintf(f, "%.1f %.1f %.1f %.1f re\nf\n", x0, y0, gridx, gridy);
			}
			if(s->err==2) c='!';
			//print symbol
			if(c){
				fprintf(f, "/F0 %d Tf\n", (int)(gridy*0.8));
				cl= (showErr && s->err || s->err==2) ? clError : clNumber;
				printRGB(f, cl, "rg");
				fprintf(f, "BT\n%.1f %.1f Td\n(%c) Tj\nET\n",
					x0+gridx*0.32, y0+gridy*0.2, c);
			}
		}
	}
	//grid
	const float lineWidth=0.9f, lineWidthBold=1.8f;
	printRGB(f, clGrid, "RG");
	fprintf(f, "%f w\n", lineWidth);
	printGrid(f, false);
	//bold grid
	if(sizex){
		printRGB(f, clBoldGrid, "RG");
		fprintf(f, "%f w\n", lineWidthBold);
		printGrid(f, true);
	}
	fprintf(f, "1 w\n");

	//groups
	if(Ngroup){
		fprintf(f, "/F0 %d Tf\n", (int)(gridy*(countPerPage > 6 ? 0.37 : 0.3)));
		printRGB(f, clGroup, "RG");
		fprintf(f, "[4 3] 0 d\n"); //dash
		ng=0;
		for(i=0; i<Ngroup; i++){
			g=&group[i];
			if(g->len){
				printGroup(f, g);
				ng++;
			}
		}
		fprintf(f, "[] 0 d\n");
	}

	//consecutive signs
	if(consecutive){
		fprintf(f, "%f w\n", lineWidthBold);
		printRGB(f, clCons, "RG");
		i=0;
		for(x=0; x<extentx; x++){
			for(y=0; y<extenty; y++){
				s=&board[i++];
				x0= X(x); y0= Y(y);
				printCons(f, x0+gridx/2, y0, 0, s->cons[0], s->errCons&1);
				printCons(f, x0, y0+gridy/2, 1, s->cons[1], s->errCons&2);
			}
		}
		fprintf(f, "1 w\n");
	}

	//greater than signs
	if(greater){
		printRGB(f, clGreater, "RG");
		i=0;
		for(x=0; x<extentx; x++){
			for(y=0; y<extenty; y++){
				s=&board[i++];
				x0= X(x); y0= Y(y);
				if(s->sign[0]){
					printSign(f, x0+gridx/2, y0, s->sign[0]-1, s->errGreater&1);
				}
				if(s->sign[1]){
					printSign(f, x0, y0+gridy/2, s->sign[1]+1, s->errGreater&2);
				}
			}
		}
	}
}

void Pdf::print(HWND hDlg)
{
	int x, y, i, col, row, rows, cols, counter, sudWidth, sudHeight, borderX, borderY, pages, page, obj;
	SYSTEMTIME st;
	long *refA, ptrLen, ptrEndStream;
	static TCHAR buf[128];
	static const struct{ int rows, cols; } L[]={
		{1, 1}, {2, 1}, {3, 1}, {2, 2}, {3, 2}, {3, 3}, {4, 3}};

	//setlocale(LC_NUMERIC, "C");

	FILE *f = _tfopen(fn, _T("wb"));
	if(!f){
		msglng(733, "Cannot create file %s", fn);
	}
	else{
		//calculate number of pages
		if(countPerPage<4) cols=1;
		else if(countPerPage<7) cols=2;
		else if(countPerPage<13) cols=3;
		else cols=4;
		rows=countPerPage/cols;
		if(pageWidth>pageHeight) i=rows, rows=cols, cols=i;
		pages= (count-1)/(rows*cols)+1;

		//write headers and page objects
		refA= new long[2*pages+5];
		obj=4;
		refA[0]= 0;
		fprintf(f, "%%PDF-1.2\n");
		refA[1]= ftell(f);
		fprintf(f, "1 0 obj<</Type/Catalog/Pages 3 0 R>>endobj\n");
		refA[2]= ftell(f);
		GetLocalTime(&st);
		fprintf(f, "2 0 obj<</Producer (Sudoku)/CreationDate (D:%04d%02d%02d%02d%02d%02d)>>endobj\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		refA[3]= ftell(f);
		fprintf(f, "3 0 obj<</Type/Pages/Kids [");
		for(i=0; i<pages; i++){
			fprintf(f, "%d 0 R ", obj+1+2*i);
		}
		fprintf(f, "]/Count %d>>endobj\n", pages);
		refA[4]= ftell(f);
		fprintf(f, "4 0 obj<</Type/Font/Subtype/Type1/Encoding/WinAnsiEncoding/FirstChar 32/LastChar 255/BaseFont/%s/Name/F0>>endobj\n", "Arial");

		//calculate sudoku dimensions
		sudWidth =(pageWidth-2*border+spacing)/cols;
		sudHeight = (pageHeight-2*border+spacing)/rows;
		if(sudWidth*extenty < sudHeight*extentx)
			sudHeight = sudWidth*extenty/extentx;
		else
			sudWidth = sudHeight*extentx/extenty;
		borderX = (pageWidth+spacing-sudWidth*cols)>>1;
		borderY = (pageHeight+spacing-sudHeight*rows)>>1;

		for(page=0, counter=count; counter>0; page++){
			refA[++obj]= ftell(f);
			fprintf(f, "%d 0 obj<</Type/Page/Parent 3 0 R/MediaBox [0 0 %d %d]"
				"/Resources<</Font<</F0 4 0 R>>/ProcSet [/PDF /Text /ImageC ]>>"
				"/Contents %d 0 R>>endobj\n", obj, pageWidth, pageHeight, obj+1);
			refA[++obj]= ftell(f);
			fprintf(f, "%d 0 obj<</Length ", obj);
			ptrLen = ftell(f);
			fprintf(f, "      /Filter []>>stream\n");
			for(y=pageHeight-borderY+spacing, row=0; row<rows; row++){
				y-=sudHeight;
				for(x=borderX, col=0; col<cols && counter>0; x+=sudWidth, col++){
					//show progress
					_stprintf(buf, _T("%s (%d/%d)"), lng(26, "Create PDF"), count-counter, count);
					SetWindowText(hDlg, buf);
					UpdateWindow(hDlg);
					//generate one sudoku
					newGame();
					sudLeft = x;
					sudTop = y;
					gridx=(float)(sudWidth-spacing)/extentx;
					gridy=(float)(sudHeight-spacing)/extenty;
					printSudoku(f);
					counter--;
					//check ESC key
					MSG mesg;
					while(PeekMessage(&mesg, 0, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE)){
						if(mesg.wParam==VK_ESCAPE) counter=0;
					}
					//check Cancel button
					while(PeekMessage(&mesg, 0, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE)){
						RECT rc;
						GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rc);
						POINT pt;
						pt.x=LOWORD(mesg.lParam);
						pt.y=HIWORD(mesg.lParam);
						ClientToScreen(mesg.hwnd, &pt);
						if(PtInRect(&rc, pt)) counter=0;
					}
				}
			}
			ptrEndStream = ftell(f);
			fseek(f, ptrLen, SEEK_SET);
			fprintf(f, "%d", ptrEndStream - ptrLen-25);
			fseek(f, ptrEndStream, SEEK_SET);
			fprintf(f, "\nendstream endobj\n");
		}
		refA[++obj]= ftell(f);
		fprintf(f, "xref\n0 %d\n0000000000 65535 f\n", obj);
		for(i=1; i<obj; i++){
			fprintf(f, "%010d 00000 n\n", refA[i]);
		}
		fprintf(f, "trailer<</Size %d/Root 1 0 R/Info 2 0 R>>\nstartxref\n%d\n%%%%EOF",
			obj, refA[obj]);
		fclose(f);
	}
}
