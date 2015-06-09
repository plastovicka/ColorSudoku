/*
	(C) 2005-2009  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "sudoku.h"

const TCHAR *subkey=_T("Software\\Petr Lastovicka\\sudoku");
const TCHAR *upkey=_T("Software\\Petr Lastovicka");
const TCHAR *hiscoreFile=_T("sudoku.sco");

struct Treg { char *s; int *i; } regVal[]={
	{"top", &mainTop},
	{"left", &mainLeft},
	{"width", &mainW},
	{"height", &mainH},
	{"diag", &diag},
	{"symetric", &symetric},
	{"size", &size},
	{"showErrTime", &showErrTime},
	{"toolOn", &toolBarVisible},
	{"symbol", &symbol0},
	{"sums", &killer},
	{"level", &level},
	{"greater", &greater},
	{"consecutive", &consecutive},
	{"oddeven", &oddeven},
	{"pdfPageWidth", &pdfObject.pageWidth},
	{"pdfPageHeight", &pdfObject.pageHeight},
	{"pdfCount", &pdfObject.count},
	{"pdfCountPerPage", &pdfObject.countPerPage},
	{"gameType", &gameType},
};
struct Tregs { char *s; TCHAR *i; DWORD n; } regValS[]={
	{"language", lang, sizeof(lang)},
	{"file", gameFn, sizeof(gameFn)},
	{"player", playerName, sizeof(playerName)},
	{"pdfFile", pdfObject.fn, sizeof(pdfObject.fn)},
};
//---------------------------------------------------------------------------
TCHAR num2char(int v)
{
	return (TCHAR)((v>=9) ? ('A'-9+v) : ((v<0) ? '.' : ('1'+v)));
}

int char2num(TCHAR c)
{
	if(c=='.') return -1;
	if(c>='A' && c<='Z') return c-'A'+9;
	if(c>='a' && c<='z') return c-'a'+9;
	if(c>='1' && c<='9') return c-'1';
	return -2;
}
//---------------------------------------------------------------------------
const int sizeTab[]={6, 9, 12, 6, 9, 12, 6, 9, 6, 9, 6, 9, 6, 9};
const int diagTab[]={0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1};
const int killerTab[]={0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0};
const int greaterTab[]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1};

TscoreTab *findScoreTab(BYTE _gameType, BYTE _size, BYTE _diag, BYTE _killer, BYTE _greater, BYTE _cons, BYTE _oddeven, BYTE flags)
{
	for(TscoreTab *t=score; t; t=t->next){
		if(t->gameType==_gameType && t->size==_size && t->diag==_diag &&
			t->killer==_killer && t->greater==_greater && t->cons==_cons &&
			t->oddeven==_oddeven && t->flags==flags)
			return t;
	}
	return 0;
}

TscoreTab *getScoreTab(BYTE _gameType, BYTE _size, BYTE _diag, BYTE _killer, BYTE _greater, BYTE _cons, BYTE _oddeven, BYTE flags)
{
	TscoreTab *t= findScoreTab(_gameType, _size, _diag, _killer, _greater, _cons, _oddeven, flags);
	if(!t){
		t=new TscoreTab;
		t->next=score;
		score=t;
		t->lastScore=-1;
		t->gameType=_gameType;
		t->size=_size;
		t->diag=_diag;
		t->killer=_killer;
		t->greater=_greater;
		t->cons=_cons;
		t->oddeven=_oddeven;
		t->flags=flags;
		memset(t->score, 0, sizeof(t->score));
	}
	return t;
}

void addScore()
{
	int i, dlg;
	TScore s;
	SYSTEMTIME t;

	if(playtime<=0 || playtime>65535 || symetric || level ||
		(size!=6 && size!=9 && size!=12)) return;
	readScore();
	//fill in a score record
	GetLocalTime(&t);
	s.date.wDay=(BYTE)t.wDay;
	s.date.wMonth=(BYTE)t.wMonth;
	s.date.wYear=t.wYear;
	s.date.wMinute=(BYTE)t.wMinute;
	s.date.wHour=(BYTE)t.wHour;
	s.playtime=(WORD)playtime;
	//get table
	TscoreTab *a= getScoreTab((BYTE)gameType, (BYTE)size, (BYTE)diag, (BYTE)killer, (BYTE)greater, (BYTE)consecutive, (BYTE)oddeven);
	TScore *scoreTable= a->score;
	//InsertSort
	for(i=0; i<Dscore; i++){
		if(s.playtime < scoreTable[i].playtime || !scoreTable[i].playtime) break;
	}
	if(i<Dscore){
		//ask for player name
		dlg= (int)DialogBoxParam(inst, MAKEINTRESOURCE(IDD_NAME),
			hWin, (DLGPROC)NameProc, 0);
		convertT2W(playerName, w);
		wcscpy(s.name, w);
		if(dlg!=IDCANCEL){
			//add new record to the score table
			a->lastScore=i;
			for(int j=Dscore-1; j>i; j--)  scoreTable[j]=scoreTable[j-1];
			scoreTable[i]=s;
			//save to disk
			writeScore();
			//show table
			DialogBoxParam(inst, MAKEINTRESOURCE(IDD_HISCORE), hWin, (DLGPROC)ScoreProc, 0);
		}
		else{
			//delete empty table
			if(!scoreTable[0].playtime){
				assert(a==::score);
				::score=a->next;
				delete a;
			}
		}
	}
}
//---------------------------------------------------------------------------
void parseScore(BYTE *data, int len)
{
	int i, n, Nname;
	WCHAR *w, **nameA=0;
	BYTE b;
	WORD d;
	TscoreTab *t;
	TScore *r;
	BYTE *p=data;

	if(!strncmp((char*)data, "Color Sudoku score 2", 20)){
		try
		{
			p+=20;
			//names
			Nname= *(WORD*)p;
			p+=2;
			nameA= new WCHAR*[Nname];
			w=(WCHAR*)p;
			for(i=0; i<Nname; i++){
				nameA[i]=w;
				w=wcschr(w, 0)+1;
			}
			p=(BYTE*)w;
			//score tables
			for(; p<data+len;){
				b=p[2];
				t=getScoreTab(p[0], p[1], (BYTE)(b&1), (BYTE)((b>>1)&1), (BYTE)((b>>2)&1),
					(BYTE)((b>>3)&1), (BYTE)((b>>4)&1), (BYTE)(b&0xe0));
				memset(t->score, 0, sizeof(t->score));
				p+=3;
				n=*p++;
				for(i=0; i<n; i++){
					r=&t->score[i];
					if(r<endA(t->score)){
						//name
						r->name[0]=0;
						d= *(WORD*)p;
						if(d<Nname) wcsncpy(r->name, nameA[d], sizeA(r->name));
						r->name[sizeA(r->name)-1]=0;
						//time and date
						r->playtime= *(WORD*)(p+2);
						memcpy(&r->date, p+4, 6);
						//if(r->date.wYear<1990 || r->date.wYear>3000) r->playtime=0;
					}
					p+=10;
				}
			}
		} catch(...)
		{
			msg("Score table is damaged");
		}
		delete[] nameA;
	}
	else if(len%(Dscore*sizeof(TScore))==0){
		//old version, fixed size file
		for(i=0; i<14; i++){
			if(p==data+len) break; //file is too short
			t=getScoreTab(0, (BYTE)sizeTab[i], (BYTE)diagTab[i], (BYTE)killerTab[i], (BYTE)greaterTab[i], 0, 0);
			//copy score table
			memcpy(t->score, p, Dscore*sizeof(TScore));
			p+=Dscore*sizeof(TScore);
		}
	}
}

void writeScore(BYTE *&data, int &len)
{
	TscoreTab *t;
	TScore *r;
	WCHAR **nameA;
	BYTE b, *p, *p1;
	int i, n, Nname;

	//estimate data length
	n=22;
	Nname=0;
	for(t=score; t; t=t->next){
		n+= Dscore*(sizeA(t->score->name)+10) + 4;
		Nname+= Dscore;
	}
	nameA= new WCHAR*[Nname];
	//write header
	p= data= new BYTE[n];
	strcpy((char*)p, "Color Sudoku score 2");
	p+=22;
	//write player names
	Nname=0;
	for(t=score; t; t=t->next){
		for(r=t->score; r<endA(t->score); r++){
			for(i=0;; i++){
				if(i==Nname){
					//write new name
					wcscpy(nameA[Nname++]= (WCHAR*)p, r->name);
					p+= 2*(wcslen(r->name)+1);
					break;
				}
				//compare with previous names
				if(!wcscmp(nameA[i], r->name)) break;
			}
		}
	}
	assert(Nname<0x10000);
	*(WORD*)(data+20)= (WORD)Nname;

	//write tables
	for(t=score; t; t=t->next){
		*p++= t->gameType;
		*p++= t->size;
		*p++= (BYTE)((t->diag) | (t->killer<<1) | (t->greater<<2) | (t->cons<<3) | (t->oddeven<<4) | t->flags);
		p1=p;
		p++;
		for(b=0, r=t->score; r<endA(t->score); b++, r++){
			if(!r->playtime) break;
			for(i=0;; i++){
				assert(i<Nname);
				if(!wcscmp(nameA[i], r->name)) break;
			}
			*(WORD*)p= (WORD)i;
			*(WORD*)(p+2)= r->playtime;
			memcpy(p+4, &r->date, 6);
			p+=10;
		}
		*p1=b;
	}
	len= int(p-data);
	delete[] nameA;
}

//hiscore file name
void getHiscoreFile(TCHAR *buf)
{
	HKEY key;
	DWORD d;

	getExeDir(buf, _T(""));
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
		0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
		d=2*(MAX_PATH-32);
		if(RegQueryValueEx(key, _T("Common AppData"), 0, 0, (BYTE *)buf, &d)==ERROR_SUCCESS){
			_tcscat(buf, _T("\\Petr Lastovicka\\"));
		}
		RegCloseKey(key);
	}
	_tcscat(buf, hiscoreFile);
}

void writeScore()
{
	HKEY key;
	FILE *f;
	TCHAR *t;
	BYTE *data=0;
	int len;
	TCHAR *fn=(TCHAR*)_alloca(2*MAX_PATH);
	getHiscoreFile(fn);

	if(score){
		writeScore(data, len);
		if(data && len){
			t=cutPath(fn)-1;
			*t=0;
			CreateDirectory(fn, 0);
			*t='\\';
			if((f=_tfopen(fn, _T("wb")))!=0){
				//write to file
				fwrite(data, len, 1, f);
				if(!fclose(f)){
					deleteini(HKEY_LOCAL_MACHINE);
				}
			}
			else if(RegCreateKey(HKEY_LOCAL_MACHINE, subkey, &key)==ERROR_SUCCESS){
				//write to registry
				RegSetValueEx(key, _T("BEST"), 0, REG_BINARY, data, len);
				RegCloseKey(key);
			}
		}
	}
	else{
		//table is empty => delete it
		DeleteFile(fn);
		deleteini(HKEY_LOCAL_MACHINE);
	}
}

void readScore()
{
	HKEY key;
	FILE *f;
	int len=0;
	BYTE *data=0;
	TCHAR *fn=(TCHAR*)_alloca(2*MAX_PATH);
	getHiscoreFile(fn);

	//read file
	if((f=_tfopen(fn, _T("rb")))!=0){
		fseek(f, 0, SEEK_END);
		len=ftell(f);
		rewind(f);
		if(len>0 && len<10000000){
			data=new BYTE[len];
			if(fread(data, len, 1, f)>0){
				parseScore(data, len);
			}
		}
		fclose(f);
	}
	else if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
		//read registry
		RegQueryValueEx(key, _T("BEST"), 0, 0, 0, (DWORD*)&len);
		if(len>0){
			data=new BYTE[len];
			RegQueryValueEx(key, _T("BEST"), 0, 0, data, (DWORD*)&len);
			parseScore(data, len);
		}
		RegCloseKey(key);
	}
	delete[] data;
}
//------------------------------------------------------------------
void open(TCHAR *fn)
{
	int i, x, y, v, err, n, pass, size1=0, extentx1, extenty1, diag1=0, undoPos1, gameType1, multi1, sizex1, sizey1, bufLen;
	char *p;
	char *buf;
	Tsquare *grp[Mboard], *s;

	FILE *f= _tfopen(fn, _T("rt"));
	if(!f){
		msglng(730, "Cannot open file %s", fn);
	}
	else{
		fseek(f, 0, SEEK_END);
		bufLen=ftell(f);
		aminmax(bufLen, 10000, 1000000);
		buf = new char[bufLen];
		err=0;
		//1.pass - check format, 2.pass - read data
		for(pass=0; pass<2 && !err; pass++){
			rewind(f);
			//header
			fgets(buf, bufLen, f);
			p=strchr(buf, 0)-1;
			if(strncmp(buf, "Sudoku", 6) || *p!='\n'){ err=1; break; }
			*p=0;
			gameType1=0;
			if(buf[6]){
				//size
				size1= strtol(buf+6, &p, 10);
				if(size1<=0 || *p!=' '){ err=1; break; }
				p++;
				//gameType
				gameType1=-1;
				for(i=0; gameTypeA[i].name; i++){
					if(!strcmp(gameTypeA[i].name, p)) gameType1=i;
				}
				if(gameType1<0){ err=12; break; }
			}
			//extent
			fgets(buf, bufLen, f);
			i = strlen(buf)-1;
			if(gameType1==0) size1 = i;
			getExtent(gameType1, size1, extentx1, extenty1, multi1, sizex1, sizey1);
			if(size1<4 || size1>Msize || i!=extentx1){ err=2; break; }

			if(pass){
				stop();
				size=size1;
				diag=diag1;
				gameType=gameType1;
				playtime=killer=greater=consecutive=oddeven=symetric=level=0;
				initSquare(true);
				noScore=true;
			}
			//board
			for(y=0; y<extenty1; y++){
				for(x=0; x<extentx1; x++){
					s=&board[x*extenty+y];
					if(buf[x]==' '){
						if(pass && s->val<Msymbol) err=3;
					}
					else{
						if(buf[x]==0){ err=3; goto le; }
						v=char2num(buf[x]);
						if(v<-1 || v>=size1) err=3;
						else if(pass){
							if(s->val>=Msymbol) err=3;
							else{
								put(v, s);
								if(v>=0) s->lock=1;
							}
						}
					}
				}
				fgets(buf, bufLen-4, f);
			}
			undoPos1=0;
			while(!feof(f)){
				if(!strncmp(buf, "Groups:", 7)){
					for(p=buf+7; *p!='\n';){
						i=strtol(p, &p, 10);
						if(i<=0 || i>9999 || *p!='{'){ err=7; break; }
						n=0;
						do{
							p++;
							x=char2num(*p++);
							y=char2num(*p++);
							if(x<0 || x>=extentx1 || y<0 || y>=extenty1 || n>=multi1*size1*size1+Mline){ err=7; break; }
							grp[n++]= &board[x*extenty1+y];
						} while(*p==',');
						if(*p!='}'){ err=7; break; }
						p++;
						if(pass){
							killer=1;
							addGroup(i, n, grp);
						}
					}
				}
				if(!strncmp(buf, "GreaterThan:", 12)){
					for(p=buf+12, i=0; *p!='\n'; p++, i++){
						v=*p-'0';
						if(v<0 || v>8 || i>=extentx1*extenty1){ err=11; break; }
						if(pass){
							s=&board[i];
							if(s->val>=Msymbol) p--;
							else{
								greater=1;
								putSign(v/3, s, 0);
								putSign(v%3, s, 1);
							}
						}
					}
				}
				if(!strncmp(buf, "Consecutive:", 12)){
					for(p=buf+12, i=0; *p!='\n'; p++, i++){
						v=*p-'0';
						if(v<0 || v>3 || i>=extentx1*extenty1){ err=13; break; }
						if(pass){
							s=&board[i];
							if(s->val>=Msymbol) p--;
							else{
								consecutive=1;
								if(v&2) putCons(true, s, 0);
								if(v&1) putCons(true, s, 1);
							}
						}
					}
				}
				if(!strncmp(buf, "OddEven:", 8)){
					for(p=buf+8, i=0; *p!='\n'; p++, i++){
						v=*p-'0';
						if(v<0 || v>2 || i>=extentx1*extenty1){ err=14; break; }
						if(pass){
							s=&board[i];
							if(s->val>=Msymbol) p--;
							else{
								oddeven=1;
								putOddEven(v, s);
							}
						}
					}
				}
				if(!strncmp(buf, "Mark:", 5)){
					x=y=0;
					for(p=buf+5; *p!='\n';){
						v=char2num(*p++);
						if(v<-1 || v>=size1 || y>=extenty1){ err=10; break; }
						if(v<0){
							x++;
							if(x==extentx1){ x=0; y++; }
						}
						else if(pass){
							s=&board[x*extenty+y];
							if(s->val>=Msymbol) err=10;
							else s->mark[v]=true;
						}
					}
				}
				if(!strncmp(buf, "Undo:", 5)){
					for(p=buf+5; *p!='\n';){
						x=char2num(*p++);
						y=char2num(*p++);
						v=char2num(*p++);
						if(x<0 || y<0 || v<-1 ||
							x>=extentx1 || y>=extenty1 || v>=size1){
							err=6; break;
						}
						if(pass){
							Tundo *u= addUndoRec1(undoPos1);
							u->action=0;
							u->s= &board[x*extenty+y];
							u->prev=(char)v;
							u->s->lock=0;
						}
						undoPos1++;
					}
				}
				if(!strncmp(buf, "UndoMark:", 9)){
					for(p=buf+9; *p!='\n';){
						x=char2num(*p++);
						y=char2num(*p++);
						i=2;//mark
						if(*p=='-'){ i=0; p++; } //symbol
						v=char2num(*p++);
						if(v<0) i=0;
						if(x<0 || y<0 || v<-1 ||
							x>=extentx1 || y>=extenty1 || v>=size1){
							err=15; break;
						}
						if(pass){
							Tundo *u= addUndoRec1(undoPos1);
							u->action=(char)i;
							u->s= &board[x*extenty+y];
							if(i==2){
								u->num=(char)v;
								u->s->mark[v] = !u->s->mark[v];
							}
							else{
								u->prev=(char)v;
								u->s->lock=0;
							}
						}
						undoPos1++;
					}
				}
				if(!strncmp(buf, "Diagonal:", 9)){
					if(!strncmp(buf+9, "yes", 3)) diag1=1;
					else if(!strncmp(buf+9, "no", 2)) diag1=0;
					else err=5;
				}
				if(!strncmp(buf, "Symmetrical:", 12)){
					if(!strncmp(buf+12, "yes", 3)){ if(pass) symetric=1; }
					else if(!strncmp(buf+12, "no", 2)){ if(pass) symetric=0; }
					else err=8;
				}
				if(!strncmp(buf, "Level:", 6)){
					i=sscanf(buf+6, "%d", &x);
					if(!i || unsigned(x)>100){
						err=9;
					}
					else if(pass){
						level=x;
					}
				}
				if(!strncmp(buf, "Time:", 5)){
					int h, m, sec;
					i=sscanf(buf+5, "%d:%d:%d", &h, &m, &sec);
					if(i<=1){
						err=4;
					}
					else if(pass && !editor){
						playtime=h*60+m;
						if(i==3) playtime=playtime*60+sec;
					}
				}
				fgets(buf, bufLen-4, f);
			}
			if(pass) undoPos=undoPos1;
		}
	le:
		fclose(f);
		delete[] buf;
		if(err){
			msglng(731, "Invalid format of %s", fn);
		}
		else{
			checkMenus();
			numButtons();
			invalidate();
			startTimer();
		}
	}
}

void save(TCHAR *fn)
{
	int i, j, x, y;
	char c;
	Tsquare *s;

	FILE *f= _tfopen(fn, _T("wt"));
	if(!f){
		msglng(733, "Cannot create file %s", fn);
	}
	else{
		fputs("Sudoku", f);
		if(gameType>0){
			fprintf(f, " %d %s", size, gameTypeA[gameType].name);
		}
		fputc('\n', f);
		for(y=0; y<extenty; y++){
			for(x=0; x<extentx; x++){
				s=&board[x*extenty+y];
				if(s->val>=Msymbol)
					fputc(' ', f);
				else{
					fputc(num2char(s->val), f);
				}
			}
			fputc('\n', f);
		}
		fprintf(f, "Diagonal:%s\nSymmetrical:%s\n",
			diag ? "yes" : "no", symetric ? "yes" : "no");
		if(level) fprintf(f, "Level:%d\n", level);
		if(Ngroup){
			fputs("Groups:", f);
			for(i=0; i<Ngroup; i++){
				Tgroup *g= &group[i];
				if(g->len){
					fprintf(f, "%d", g->sum);
					c='{';
					for(j=0; j<g->len; j++){
						fprintf(f, "%c%c%c", c,
							num2char(s2x(g->square[j])), num2char(s2y(g->square[j])));
						c=',';
					}
					fputc('}', f);
				}
			}
			fputc('\n', f);
		}
		if(Nsign>0){
			fputs("GreaterThan:", f);
			for(i=0; i<Nboard; i++){
				s=&board[i];
				if(s->val<Msymbol)
					fputc(s->sign[0]*3 + s->sign[1] + '0', f);
			}
			fputc('\n', f);
		}
		if(consecutive>0){  //Ncons can be 0
			fputs("Consecutive:", f);
			for(i=0; i<Nboard; i++){
				s=&board[i];
				if(s->val<Msymbol)
					fputc(int(s->cons[0])*2 + int(s->cons[1]) + '0', f);
			}
			fputc('\n', f);
		}
		if(oddeven){
			fputs("OddEven:", f);
			for(i=0; i<Nboard; i++){
				s=&board[i];
				if(s->val<Msymbol)
					fputc(s->oddEven + '0', f);
			}
			fputc('\n', f);
		}
		if(undoPos>0){
			fputs("UndoMark:", f);
			for(i=0; i<undoPos; i++){
				Tundo *u= &undoRec[i];
				if(u->action<=2){
					fprintf(f, "%c%c", num2char(s2x(u->s)), num2char(s2y(u->s)));
					if(u->action!=2){
						if(u->prev!=-1) fputc('-', f);
						fputc(num2char(u->prev), f);
					}
					else{
						fputc(num2char(u->num), f);
					}
				}
			}
			fputc('\n', f);
		}
		/*if(marks){
			fputs("Mark:",f);
			for(y=0; y<extenty; y++){
			for(x=0; x<extentx; x++){
			s=&board[x*extenty+y];
			if(s->val<Msymbol){
			for(i=0; i<Nsymbol; i++){
			if(s->mark[i]) fputc(num2char(i),f);
			}
			}
			fputc('.',f);
			}
			}
			fputc('\n',f);
			}*/
		TCHAR buf[12];
		printTime(buf, playtime);
		_ftprintf(f, _T("Time:%s\n"), buf);
		fclose(f);
	}
}
//---------------------------------------------------------------------------

//delete registry settings
void deleteini(HKEY root)
{
	HKEY key;
	DWORD i;

	if(RegDeleteKey(root, subkey)==ERROR_SUCCESS){
		if(RegOpenKeyEx(root, upkey, 0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
			i=1;
			RegQueryInfoKey(key, 0, 0, 0, &i, 0, 0, 0, 0, 0, 0, 0);
			RegCloseKey(key);
			if(!i){
				RegDeleteKey(root, upkey);
			}
		}
	}
}

//save settings to the registry 
void writeini()
{
	HKEY key;

	if(RegCreateKey(HKEY_CURRENT_USER, subkey, &key)!=ERROR_SUCCESS)
		msglng(735, "Cannot write to Windows registry");
	else{
		for(Treg *u=regVal; u<endA(regVal); u++){
			RegSetValueExA(key, u->s, 0, REG_DWORD,
				(BYTE *)u->i, sizeof(int));
		}
		for(Tregs *v=regValS; v<endA(regValS); v++){
			convertA2T(v->s, t);
			RegSetValueEx(key, t, 0, REG_SZ,
				(BYTE *)v->i, (DWORD) sizeof(TCHAR)*(_tcslen(v->i)+1));
		}
		RegSetValueEx(key, _T("color"), 0, REG_BINARY,
			(BYTE *)colors, sizeof(colors));
		RegCloseKey(key);
	}
}

//read settings from the registry
void readini()
{
	HKEY key;
	DWORD d;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
		for(Treg *u=regVal; u<endA(regVal); u++){
			d=sizeof(int);
			RegQueryValueExA(key, u->s, 0, 0, (BYTE *)u->i, &d);
		}
		for(Tregs *v=regValS; v<endA(regValS); v++){
			convertA2T(v->s, t);
			d=v->n;
			RegQueryValueEx(key, t, 0, 0, (BYTE *)v->i, &d);
		}
		d=sizeof(colors);
		RegQueryValueEx(key, _T("color"), 0, 0,
			(BYTE *)colors, &d);
		RegCloseKey(key);
	}
	readScore();
}
//---------------------------------------------------------------------------
