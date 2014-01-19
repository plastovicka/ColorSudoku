/*
	(C) 2005-2009  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "sudoku.h"

//------------------------------------------------------------------
//clear board
void init(bool fromGener)
{
	int i, j;
	Tsquare *s;
	Tline *e;
	Tintersect *r;

	Nsquare=0;
	for(i=0; i<Nboard; i++){
		s=&board[i];
		if(s->lines==0){
			s->val=Msymbol;
			s->lock=1;
		}
		else{
			Nsquare++;
			s->val= -1;
			s->n=Nsymbol;
			s->lock=0;
			s->tmpInter=0;
			s->bit=0;
			memset(s->mask, 0, Nsymbol*sizeof(s->mask[0]));
			memset(s->mark, 0, Nsymbol*sizeof(s->mark[0]));
			s->lower=0;
			s->upper=Nsymbol-1;
		}
		if(!fromGener) s->origVal=s->val;
		s->err=s->errSum=false;
		s->errGreater=s->errCons=0;
	}
	for(i=0; i<Nline; i++){
		e=&line[i];
		for(j=0; j<Nsymbol; j++){
			e->number[j]=e->len;
			e->bit[j]=0;
		}
		e->n=e->len;
	}
	for(i=0; i<Nintersect; i++){
		r=&intersects[i];
		for(j=0; j<Nsymbol; j++){
			r->number[j]=r->len;
			r->b[j]=false;
		}
	}
	for(i=0; i<Ngroup; i++){
		group[i].n=0;
	}
	done=0;
	//init odd/even
	blocked=1;
	for(i=0; i<Nboard; i++){
		s=&board[i];
		if(s->oddEven>0){
			for(j=s->oddEven-1; j<Nsymbol; j+=2){
				no(j, s);
			}
		}
	}
	blocked=0;
	undoPos=redoPos=undoAllPos=0;

	//allocate solution array
	delete[] solution;
	solution=new BYTE[Msolution*(Nsquare*3+1)];
	Nsolution=0;

	if(!fromGener) status(4, _T(""));
}

static int SX[]={0, 0, 0, 0, 2, 0, 3, 0, 4, 3, 5, 0, 4, 0, 7, 5, 4, 0, 6, 0, 5, 7, 11, 0, 6, 5};
static int SY[]={0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 3, 0, 2, 3, 4, 0, 3, 0, 4, 3, 2, 0, 4, 5};

void getExtent(int gameType, int &size, int &extentx, int &extenty, int &multi, int &sizex, int &sizey)
{
	int i, extentx1, extenty1;
	Tcoord *o;

	aminmax(size, 4, sizeA(SX)-1);
	for(;;){
		sizex=SX[size];
		sizey=SY[size];
		if(sizex) break;
		size++;
	}
	extentx1=extenty1=0;
	multi=gameTypeA[gameType].multi;
	for(i=0, o=gameTypeA[gameType].coord; i<multi; i++, o++){
		amin(extentx1, coord[i].x = o->x * size + o->xd * sizex);
		amin(extenty1, coord[i].y = o->y * size + o->yd * sizey);
	}
	extentx = extentx1 + size;
	extenty = extenty1 + size;
}

struct Tie { int i; Tline *e; };

//create board
void initSquare(bool fromGener)
{
	int i, j, k, n, x, y, c, rlen, m, Elen;
	Tsquare *s, *s1, *board1;
	Tline *e, *e1;
	Tintersect *r;
	Tgroup *g;
	Tsquare *P[Mlen+1];
	Tie E[Mline1];

	getExtent(gameType, size, extentx, extenty, multi, sizex, sizey);
	Nsymbol=size;

	//allocate board
	delete[] board;
	board=new Tsquare[Nboard = extentx*extenty];
	assert(extentx<=Mextent && extenty<=Mextent && Nboard<=Mboard);

	for(s=board; s<board+Nboard; s++){
		s->lines= s->intersects= 0;
		s->neighbour[0]=s->neighbour[1]=s->neighbour[2]=s->neighbour[3]=0;
	}

	k=0; //line index
	for(m=0; m<multi; m++){
		board1= board + coord[m].x * extenty + coord[m].y;
		multiLinePtr[m] = &line[k];
		for(i=0; i<size; i++){
			//column
			e=&line[k++];
			e->len=size;
			e->intersects=0;
			e->type=L_COLUMN;
			e->multipart=m;
			for(j=0; j<size; j++){
				s=&board1[i*extenty+j];
				e->ptr[j]=s;
				s->ptr[s->lines++]=e;
				if(j>0) s->neighbour[0]= s-1;
				if(j<size-1) s->neighbour[2]= s+1;
			}
			//row
			e=&line[k++];
			e->len=size;
			e->intersects=0;
			e->type=L_ROW;
			e->multipart=m;
			for(j=0; j<size; j++){
				s=&board1[j*extenty+i];
				e->ptr[j]=s;
				s->ptr[s->lines++]=e;
				if(j>0) s->neighbour[1]= s-extenty;
				if(j<size-1) s->neighbour[3]= s+extenty;
			}
			//rectangle
			e=&line[k++];
			e->len=size;
			e->intersects=0;
			e->type=L_RECTANGLE;
			e->multipart=m;
			for(j=0; j<size; j++){
				x=(i%sizey)*sizex+(j%sizex);
				y=(i/sizey)*sizey+(j/sizex);
				s=&board1[x*extenty+y];
				e->ptr[j]=s;
				s->ptr[s->lines++]=e;
			}
		}
		if(diag){
			for(i=0; i<2; i++){
				e=&line[k++];
				e->len=size;
				e->intersects=0;
				e->type=L_DIAGONAL;
				e->multipart=m;
				for(j=0; j<size; j++){
					s=&board1[j*extenty + (i ? size-1-j : j)];
					e->ptr[j]=s;
					s->ptr[s->lines++]=e;
				}
			}
		}
	}
	Nline=k;
	assert(Nline==(3*size+diag*2)*multi);

	n=0; //intersect index
	for(i=0; i<Nline; i++){
		e=&line[i];
		if(e->type==L_RECTANGLE) continue;
		//copy pointers to P
		for(j=0; j<e->len; j++){
			P[j]= e->ptr[j];
		}
		P[j]=P[0];
		Elen=0;
		//go through line
		for(j=0; j<=e->len; j++){
			s=P[j];
			for(m=0; m<Elen; m++){
				e1=E[m].e;
				for(k=0;; k++){
					if(k==s->lines){
						//end of intersection, check intersection length
						c=1;
						if(e->multipart!=e1->multipart){
							if(e->type==L_ROW || e1->type==L_ROW) c=sizex;
							else if(e->type==L_COLUMN || e1->type==L_COLUMN) c=sizey;
							else c=3;
						}
						rlen=j-E[m].i;
						if(rlen > c && rlen < e->len){
							for(c=0;; c++){
								if(c==e->intersects){
									//create intersection e x e1
									assert(n<Mintersect);
									r=&intersects[n++];
									r->line[0]=e;
									assert(e->intersects<Minter1);
									e->inter[e->intersects++]=r;
									r->line[1]=e1;
									assert(e1->intersects<Minter1);
									e1->inter[e1->intersects++]=r;
									r->len=rlen;
									for(c=0; c<rlen; c++){
										r->square[c]= s1= P[E[m].i+c];
										assert(s1->intersects<sizeA(s1->inter));
										s1->inter[s1->intersects++]=r;
									}
									break;
								}
								r=e->inter[c];
								if(r->line[0]==e1 || r->line[1]==e1) break; //intersection already exists
							}
						}
						//remove from E
						E[m]=E[--Elen];
						m--;
						break;
					}
					if(e1==s->ptr[k]) break;
				}
			}
			for(k=0; k<s->lines; k++){
				e1=s->ptr[k];
				if(e1!=e){
					for(m=0;; m++){
						if(m==Elen){
							//add to E
							assert(Elen<sizeA(E));
							E[m].e=e1;
							E[m].i=j;
							Elen++;
							break;
						}
						if(E[m].e==e1) break; //do not add twice
					}
				}
			}
		}
	}
	Nintersect=n;
	assert(Nintersect>=multi*size*(sizex+sizey));

	//init. indexInLine[]
	for(i=0; i<Nline; i++){
		e=&line[i];
		assert(e->intersects<=Minter1);
		for(j=0; j<e->len; j++){
			s=e->ptr[j];
			for(k=0; s->ptr[k]!=e; k++) assert(k<s->lines);
			s->indexInLine[k]=j;
		}
	}
	for(i=0; i<Nboard; i++){
		s=&board[i];
		assert(s->intersects<=sizeA(s->inter));
		//no groups
		s->group=0;
		memset(s->group2, 0, sizeof(s->group2));
		//no greater signs
		s->sign[0]=s->sign[1]=s->sign[2]=s->sign[3]=0;
		//no consecutive sings
		s->cons[0]=s->cons[1]=s->cons[2]=s->cons[3]=0;
		//no odd/even
		s->oddEven=0;
	}
	Ngroup=Nsign=Ncons=0;
	//allocate group array
	freeGroups();
	delete[] group;
	group= new Tgroup[Mgroup = multi*size*size + multi*(3*size+2)];
	for(i=0; i<Mgroup; i++){
		g=&group[i];
		g->len=0;
		g->A=0;
	}
	//clear board
	init(false);
	if(!fromGener && oddeven){
		for(s=board+Nboard-1; s>=board; s--) putOddEven(2, s);
	}

	TCHAR *ch=_T("");
	if(diag) ch=_T("X");
	status(3, ch);
	errTime=-1;
	invalidate();
}

//------------------------------------------------------------------
void prepareGroup(Tgroup *g)
{
	int S, pos, j, m, *n, num, o;
	int(*gnumber)[Msymbol];
	char *p;
	Tline *e;
	Tsquare **gsquare;
	char a[MgroupLen];

	g->n=0;
	if(g->len<=0) return;
	//check if whole group is inside some line
	g->lines=0;
	for(o=0; o<g->square[0]->lines; o++){
		e=g->square[0]->ptr[o];
		for(j=0;; j++){
			if(j==g->len){
				assert(g->lines<Mline1);
				g->line[g->lines++]=e;
				break;
			}
			if(g->square[j]->lines<=o || g->square[j]->ptr[o]!=e) break;
		}
	}
	//ignore large group
	if(g->len>MgroupLen) return;
	S=0;
	for(j=0; j<g->len; j++){
		if(g->square[j]->val<0) S++;
	}
	if(S>MgroupLen2) return;

	m=0;
	memset(gnumber=g->number, 0, g->len*sizeof(g->number[0]));
	gsquare=g->square;
	//create variations
	S=g->sum-g->len;
	pos=0;
	a[0]=-1;
	for(;;){
		num=++a[pos];
		if(num==Nsymbol){
		lb:
			pos--;
			if(pos<0) break;
			S+=a[pos];
		}
		else if(!gsquare[pos]->mask[num]){
			for(j=0;; j++){
				if(j==pos){
					if(pos==g->len-1){
						if(S==num){
							if(g->n>=m){
								m+= g->len*16+(m>>1);
								g->A=(char*)realloc(g->A, m*g->len);
							}
							p=g->A+g->n*g->len;
							for(j=0; j<=pos; j++){
								n=&gnumber[j][p[j]=a[j]];
								(*n)++;
							}
							g->n++;
							goto lb;
						}
						break;
					}
					else{
						if(S<num) goto lb;
						S-=num;
						pos++;
						a[pos]=-1;
						break;
					}
				}
				if(a[j]==num) break;
			}
		}
	}
	if(!g->n) blocked++;
	amin(Mgroupn, g->n);
}

void saveGroup(Tgroup *g)
{
	g->n0=g->n;
	if(g->n) memcpy(g->number0, g->number, g->len*sizeof(g->number[0]));
}

void restoreGroup(Tgroup *g)
{
	g->n=g->n0;
	if(g->n) memcpy(g->number, g->number0, g->len*sizeof(g->number[0]));
}

void prepareGroups()
{
	for(int i=0; i<Ngroup; i++){
		prepareGroup(&group[i]);
	}
}

void saveGroups()
{
	for(int i=0; i<Ngroup; i++){
		saveGroup(&group[i]);
	}
}

void restoreGroups()
{
	for(int i=0; i<Ngroup; i++){
		restoreGroup(&group[i]);
	}
}

void freeGroups()
{
	int i, j, m;
	Tgroup *g;
	Tsquare *s;

	m=0;
	for(i=0; i<Ngroup; i++){
		g=&group[i];
		g->n=0;
		if(g->A){ free(g->A); g->A=0; }
		if(g->len){
			if(g->square[0]->group!=g){
				//delete temporary group
				g->len=0;
			}
			else{
				memset(g->number, 0, sizeof(g->number));
				m=i+1;
			}
		}
	}
	Ngroup=m;
	for(i=0; i<Nboard; i++){
		s=&board[i];
		for(j=0; j<Mgrp2; j++){
			s->group2[j]=0;
		}
	}
}

//------------------------------------------------------------------
struct TsavGrp
{
	char *A;
	int n, len, lines;
	Tline *line[Mline1];
	int number[MgroupLen][Msymbol];
};

void join(Tgroup *g, Tgroup *g2, TsavGrp *a)
{
	int j;

	//save variations
	a->len=g2->len;
	a->lines=g->lines;
	for(j=0; j<a->lines; j++) a->line[j]=g->line[j];
	char *w=a->A; a->A=g->A; g->A=w;
	a->n=g->n;
	assert(g->n==g->n0);
	memcpy(a->number, g->number, g->len*sizeof(g->number[0]));
	//append g2 to g
	for(j=0; j<g2->len; j++){
		g->square[g->len++]=g2->square[j];
		assert(g2->square[j]->group==g2);
		g2->square[j]->group=g;
	}
	g2->len=0;
	g->sum+=g2->sum;
	assert(g->sum>0);
	//create variations in new group
	prepareGroup(g);
	saveGroup(g);
}

void unJoin(Tgroup *g, Tgroup *g2, TsavGrp *a)
{
	int j;

	//split group g
	g2->len=a->len;
	g->lines=a->lines;
	for(j=0; j<a->lines; j++) g->line[j]=a->line[j];
	g->len-=a->len;
	g->sum-=g2->sum;
	for(j=0; j<a->len; j++){
		assert(g2->square[j]->group==g);
		g2->square[j]->group=g2;
	}
	//restore variations
	char *w=a->A; a->A=g->A; g->A=w;
	g->n0=a->n;
	memcpy(g->number0, a->number, g->len*sizeof(g->number[0]));
}

bool canJoin(Tgroup *g, Tgroup *g2)
{
	//there must not be symbol twice in one group
	if(g->len + g2->len > Nsymbol) return false;
	for(int j=0; j<g->len; j++){
		for(int o=0; o<g2->len; o++){
			if(g->square[j]->tmp==g2->square[o]->tmp) return false;
		}
	}
	return true;
}

bool canJoin(Tgroup *g, Tgroup *g2, Tgroup *g3)
{
	int j;
	int mask[Msymbol];

	if(g->len+g2->len+g3->len>Nsymbol) return false;
	memset(mask, 0, Nsymbol*sizeof(mask[0]));
	for(j=0; j<g->len; j++) mask[g->square[j]->tmp]++;
	for(j=0; j<g2->len; j++) mask[g2->square[j]->tmp]++;
	for(j=0; j<g3->len; j++) mask[g3->square[j]->tmp]++;
	for(j=0; j<Nsymbol; j++){
		if(mask[j]>1) return false;
	}
	return true;
}

//------------------------------------------------------------------
void gener()
{
	int i, i0, i2, k, k0, c, symetric1, levelN, Ngroup1;
	BYTE q, q2=0;
	bool b;
	Tsquare *s, *s2;
	Tundo *u;
	Tgroup *g, *g2, *gs=0, *gs2=0, *gw;
	int pos[Mboard];
	bool final[Mboard][2];
	static TsavGrp oldG[2];

	initSquare(true);
	//find some random solution
	i=consecutive;
	consecutive=0;
	resolve();
	consecutive=i;
	if(blocked){
		//Butterfly and Flower with diagonal
		assert(multi && diag && size==6);
		return;
	}
	//remember solution
	for(i=0; i<Nboard; i++){
		s=&board[i];
		s->origVal=s->val;
	}

	aminmax(level, 0, 100);
	levelN= (Nsquare*level+80)/100;

	if(consecutive){
		//put consecutive signs between squares which are consecutive
		for(i=0; i<Nboard; i++){
			s=&board[i];
			for(k=0; k<4; k++){
				s2= s->neighbour[k];
				if(s2 && s2->val>=0 && (s2->val==s->val+1 || s2->val==s->val-1))
					putCons(true, s, k);
			}
		}
	}

	if(oddeven){
		//put even squares on even symbols
		for(i=0; i<Nboard; i++){
			s=&board[i];
			if(s->val>=0) putOddEven(2-(s->val&1), s);
		}
	}

	if(greater){
		//put greater than signs between all squares
		for(i=0; i<Nboard; i++){
			s=&board[i];
			for(k=0; k<4; k++){
				s2= s->neighbour[k];
				if(s2) putSign((s2->val > s->val) ? 1 : 2, s, k);
			}
		}
	}

	if(killer){
		//make group from every square
		Ngroup=Nsquare;
		Ngroup1=0;
		for(i=0; i<Nboard; i++){
			s=&board[i];
			if(s->val<Msymbol){
				g=&group[Ngroup1++];
				g->len=1;
				g->sum=s->val+1;
				assert(g->sum>0);
				g->square[0]=s;
				s->group=g;
				final[i][0]=final[i][1]=false;
				s->tmp=s->val;
			}
		}
		assert(Ngroup1==Ngroup);
		//clear
		init(true);
		//initialize variations
		prepareGroups();
		saveGroups();

		for(c=2; c<=Nsymbol; c++){ //from small groups to larger groups
		lk:
			if(Ngroup1<=levelN) break;
			//choose two adjacent groups and try to join them
			i0=rand(Nboard);
			k0=rand(2); //down,right
			for(i=i0, k=k0;;){
				s=&board[i];
				s2=s->neighbour[k+2];
				if(s2){
					g=s->group;
					g2=s2->group;
					if(g==g2) goto ln; //both squares lie in the same group
					if(g->len+g2->len!=c || final[i][k]) goto ln;
					i2=Nboard-1-int(s2-board);
					if(!canJoin(g, g2)) goto lf;
					//symmetrical groups
					symetric1=symetric;
					if(symetric1){
						gs=board[i2].group;
						gs2=board[Nboard-1-i].group;
						if(g==gs || !gs || !gs2){
							assert(g2==gs2 || multi>1);
							symetric1=0;
						}
						else{
							assert(gs!=gs2 && gs->len+gs2->len==c && !final[i2][k]);
							if(gs==g2){
								//g g2=gs gs2
								if(!canJoin(g, g2, gs2)) goto lf;
								gw=g; g=g2; g2=gw;
							}
							else if(g==gs2){
								//gs gs2=g g2
								if(!canJoin(gs, g, g2)) goto lf;
								gw=gs; gs=gs2; gs2=gw;
							}
							else{
								assert(!(gs==g2 || gs2==g || gs==g || gs2==g2));
								if(!canJoin(gs, gs2)) goto lf;
							}
						}
					}
					restoreGroups();
					//join groups
					join(g, g2, &oldG[0]);
					Ngroup1--;
					if(symetric1){
						join(gs, gs2, &oldG[1]);
						Ngroup1--;
					}
					//resolve
					undoPos=0;
					blocked=0;
					resolve2();
					assert(!blocked);
					b= (done==Nsquare);
					init(true);
					if(b) goto lk; //OK, go to another random pair of groups
					//groups cannot be joined
					if(symetric1){
						unJoin(gs, gs2, &oldG[1]);
						Ngroup1++;
					}
					unJoin(g, g2, &oldG[0]);
					Ngroup1++;
				lf:
					final[i][k]=true;
					if(symetric) final[i2][k]=true;
				}
			ln:
				k=!k;  //next direction
				if(!k){
					i++; //next square
					if(i==Nboard) i=0;
				}
				if(i==i0 && k==k0) break;
			}
		}
	}
	else{ //not killer
		if(!symetric){
			//remove symbols which were not randomly put by backtracking
			k=0;
			for(i=0; i<undoPos; i++){
				if(undoRec[i].action==0) k++;
			}
			if(Nsquare-k>=levelN){
				undoing++;
				for(i=0; i<undoPos; i++){
					u=&undoRec[i];
					if(u->action==0) put(-1, u->s);
				}
				undoing--;
			}
		}
		//remove all symbols and insert them again (to clean undoRec)
		for(i=0; i<Nboard; i++){
			pos[i]=board[i].val;
		}
		init(true);
		for(i=0; i<Nboard; i++){
			if(pos[i]>=0 && pos[i]<Msymbol) put(pos[i], &board[i]);
			final[i][0]=false;
		}
		//remove some symbols from the board
	l:
		if(done>levelN){
			i0=rand(Nboard);
			for(i=i0;;){
				s=&board[i];
				if(s->val>=0 && !final[i][0] && s->val<Msymbol){
					i2=board+Nboard-1-s;
					s2=&board[i2];
					if(!symetric || s2->val>=0 && !final[i2][0] && s2->val<Msymbol){
						undoPos=0;
						put(-1, s);
						if(symetric) put(-1, s2);
						resolve1();
						assert(!blocked);
						b= (done==Nsquare);
						undoTo(b ? 1+symetric : 0);
						if(b) goto l;
						//square 'i' cannot be erased
						final[i][0]=true;
						if(symetric) final[i2][0]=true;
					}
				}
				i++; //next square
				if(i==Nboard) i=0;
				if(i==i0) break;
			}
		}
		//set all symbols as locked
		for(i=0; i<Nboard; i++){
			s=&board[i];
			if(s->val>=0) s->lock=1;
		}
	}
	if(greater){
		//remove unused greater than signs
		for(i=0; i<Nboard; i++){
			final[i][0]=final[i][1]=false;
		}
	lg:
		//choose some sign
		i0=rand(Nboard);
		k0=rand(2);
		for(i=i0, k=k0;;){
			s=&board[i];
			if(s->sign[k] && !final[i][k]){
				i2=board+Nboard-1-s+(k ? extenty : 1);
				s2=&board[i2];
				if(!symetric || s2->sign[k] && !final[i2][k]){
					//remove sign
					q=s->sign[k];
					putSign(0, s, k);
					if(symetric){
						q2=s2->sign[k];
						putSign(0, s2, k);
					}
					//resolve
					undoPos=0;
					blocked=0;
					restoreGroups();
					resolve2();
					assert(!blocked);
					b= (done==Nsquare);
					undoTo(0);
					if(b) goto lg;
					//sign cannot be removed, put it back
					putSign(q, s, k);
					final[i][k]=true;
					if(symetric){
						putSign(q2, s2, k);
						final[i2][k]=true;
					}
				}
			}
			k=!k;
			if(!k){
				i++;
				if(i==Nboard) i=0;
			}
			if(i==i0 && k==k0) break;
		}
	}
	freeGroups();
	undoPos=redoPos=0;
	Nsolution=0;
}
//------------------------------------------------------------------
