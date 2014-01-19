/*
  (C) 2005-2009  Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/
#include "hdr.h"
#include "sudoku.h"
//------------------------------------------------------------------
//#define NO_allInInter
//#define DBG_VISIBLE
//#define NO_rand

#ifdef NO_rand
static int S=3;
#endif

int Nsymbol,Nboard,Nline,Nintersect,Ngroup,Nsign,Ncons,Nsolution,Nsquare;
int Mgroup;
int blocked,undoing,done,counter,maxCounter;
int undoPos,redoPos;
int Mgroupn,MinLine,MonSquare,Mwing;

Tundo *undoRec;
Tsquare *board;
Tline line[Mline];
Tintersect intersects[Mintersect];
Tgroup *group;
BYTE *solution;
Txy coord[Mmulti];
Tline *multiLinePtr[Mmulti];

//------------------------------------------------------------------
unsigned seed= (unsigned)time(0)+GetTickCount();

unsigned rand(unsigned n)
{
  seed=seed*1386674153+265620371;
  return (unsigned)(UInt32x32To64(n,seed)>>32);
}

//symbol 'num' in line 'e' can be only on one square
void oneInLine(int num, Tline *e)
{
  int i;
  Tsquare *s,*t;

  t=0;
  for(i=0; i<e->len; i++){
    s=e->ptr[i];
    if(s->val>=0){
      if(s->val==num) return;
    }else{
      if(!s->mask[num]) t=s;
    }
  }
  if(t) put1(num,t);
  else blocked++;
}

//only one symbol can be on square 's'
void oneOnSquare(Tsquare *s)
{
  int i;

  if(s->val<0){
    for(i=0; i<Nsymbol; i++){
      if(!s->mask[i]){
        put1(i,s);
        return;
      }
    }
    blocked++;
  }
}

//------------------------------------------------------------------
//symbol 'num' can be on square 's'
void yes(int num, Tsquare *s)
{
  int j;
  Tline *e;

  if(!--s->mask[num]){
    s->n++;
    s->bit&=~(1<<num);
    amax(s->lower,num);
    amin(s->upper,num);
    for(j=0; j<s->lines; j++){
      e=s->ptr[j];
      e->number[num]++;
      e->bit[num]&=~(1<<s->indexInLine[j]);
      e->changed=true;
    }
#ifndef NO_allInInter
    for(j=0; j<s->intersects; j++){
      s->inter[j]->number[num]++;
    }
#endif
  }
}

//symbol 'num' cannot be on square 's'
void no(int num, Tsquare *s)
{
  int j,pos,i;
  Tline *e;
  Tgroup *g;
  char *p,*pe,w;

  if(s->val==num) blocked++;

  if(!s->mask[num]++){
    s->n--;
    s->bit|=(1<<num);
    if(s->lower==num){
      for(j=num+1; j<Nsymbol && s->mask[j]; j++) ;
      s->lower=j;
    }
    if(s->upper==num){
      for(j=num-1; j>=0 && s->mask[j]; j--) ;
      s->upper=j;
    }
    for(j=0; j<s->lines; j++){
      e=s->ptr[j];
      e->number[num]--;
      e->bit[num]|=(1<<s->indexInLine[j]);
      e->changed=true;
    }
#ifndef NO_allInInter
    for(j=0; j<s->intersects; j++){
      s->inter[j]->number[num]--;
    }
#endif
    //lines
    if(s->n<=1) oneOnSquare(s);
    for(j=0; j<s->lines; j++){
      e=s->ptr[j];
      if(e->number[num]<=1) oneInLine(num,e);
    }
    //groups
    for(j=-1; j<Mgrp2; j++){
      g= (j>=0) ? s->group2[j] : s->group;
      if(g && g->n){
        for(pos=0; g->square[pos]!=s; pos++) ;
        assert(pos<g->len);
        if(g->number[pos][num]){
          assert(g->number[pos][num]>0);
          pe= g->A+g->n*g->len;
          for(p=g->A+pos; p<pe;  ){
            if(*p==num){
              //remove variation
              g->n--;
              pe-=g->len;
              p-=pos;
              for(i=0; i<g->len; i++){
                g->number[i][w=p[i]]--;
                p[i]=pe[i];
                pe[i]=w;
              }
              if(!g->number[pos][num]) break;
              p+=pos;
            }else{
              p+=g->len;
            }
          }
          assert(!g->number[pos][num]);
          if(!g->n) blocked++; //no more variations for this group
        }
      }
    }
  }
}

int nou(int num, Tsquare *s)
{
  if(s->mask[num]) return 0;
  no(num,s);
  Tundo *u=addUndoRec();
  u->action=5;
  u->s=s;
  u->num=(char)num;
  return blocked;
}
//------------------------------------------------------------------
void allInInter(Tintersect *r)
{
  int i,j,k;
  Tline *e;
  Tundo *u;
  Tsquare *s;
  
#ifdef NO_allInInter
  return;
#endif
  for(i=0; i<Nsymbol; i++){
    if(!r->b[i] && r->number[i]>0 && 
      ((r->line[0]->number[i]==r->number[i]) ^ 
      (r->line[1]->number[i]==r->number[i])) ){
      //symbol 'i' can be only in intersection 'r' in some line
      r->b[i]=true;
      for(j=0; j<r->len; j++){
        r->square[j]->tmpInter=r;
      }
      for(k=0; k<2; k++){
        e=r->line[k];
        for(j=0; j<e->len; j++){
          s=e->ptr[j];
          if(s->tmpInter!=r) no(i,s);
        }
      }
      //create undo record
      u=addUndoRec();
      u->action=3;
      u->r=r;
      u->num=(char)i;
    }
  }
}

void notAllInInter(int i, Tintersect *r)
{
  int j,k;
  Tline *e;
  Tsquare *s;
  
  blocked++;
  assert(r->b[i]);
  r->b[i]=false;
  for(j=0; j<r->len; j++){
    r->square[j]->tmpInter=r;
  }
  for(k=0; k<2; k++){
    e=r->line[k];
    for(j=0; j<e->len; j++){
      s=e->ptr[j];
      if(s->tmpInter!=r) yes(i,s);
    }
  }
  blocked--;
}

void doGroups()
{
  int i,j,k,n,o;
  Tgroup *g;
  Tsquare *s;
  Tline *e;

  for(i=0; i<Ngroup; i++){
    g=&group[i];
    if(!g->n) continue;
    for(j=0; j<g->len; j++){
      s=g->square[j];
      if(s->val<0){
        for(k=0; k<Nsymbol; k++){
          if(!g->number[j][k]){
            //symbol 'k' on square 's' is not in any variation 
            if(nou(k,s)) return;
          }
        }
      }
    }
    if(g->len>2 && g->lines){
      for(k=0; k<Nsymbol; k++){
        n=0;
        for(j=0; j<g->len; j++){
          if(g->square[j]->val==k) break;
          n+=g->number[j][k];
        }
        if(n==g->n){
          //symbol 'k' is in all variations of group 'g'
          for(o=0; o<g->lines; o++){
            e=g->line[o];
            for(j=0; j<e->len; j++){
              s=e->ptr[j];
              if(s->group!=g){
                for(n=0; ;n++){
                  if(n==Mgrp2){
                    if(nou(k,s)) return;
                    break;
                  }
                  if(s->group2[n]==g) break;
                }
              }
            }
          }
        }
      }
    }
  }
}

void createTempGroups()
{
  int i,j,k,n,m;
  Tline *e;
  Tsquare *s;
  Tgroup *g;
  Tsquare *P[Mlen];
  Tgroup *G[Mlen];

  if(!Ngroup) return;

  //delete old groups
  amax(Ngroup,Nsquare);
  for(i=0; i<Nboard; i++){
    s=&board[i];
    for(j=0; j<Mgrp2; j++){
      if(s->group2[j]){
        s->group2[j]->len=0;
        s->group2[j]=0;
      }
    }
  }

  for(i=0; i<Nline; i++){
    e=&line[i];
    if(e->len!=Nsymbol){ assert(0); continue; }
    n=m=0; //n=length of P, m=length of G

    //find all groups in line 'e'
    for(j=0; j<e->len; j++){
      s=e->ptr[j];
      g=s->group;
      if(g){
        for(k=0; k<g->lines; k++){
          if(g->line[k]==e){
            //whole group 'g' is inside line 'e'
            //add 'g' to array G, if it isn't there yet
            for(k=0; ;k++){
              if(k==m){ G[m++]=g; break; }
              if(G[k]==g) break;
            }
            goto lg;
          }
        }
      }
      P[n++]=s;
      if(s->group2[Mgrp2-1]){ n=0; break; } //group2 overflowed
     lg:;
    }
    //G contains groups, P contains squares outside those groups
    if(n>0 && m>0 && n<=4){
      //create new group from squares in P
      g=&group[Ngroup++];
      assert(Ngroup<=Mgroup);
      g->sum= Nsymbol*(Nsymbol+1)/2; // 1 + 2 + ... + Nsymbol
      for(k=0; k<m; k++){
        g->sum-= G[k]->sum;
      }
      g->len=n;
      for(k=0; k<n; k++){
        g->square[k]= s= P[k];
        //write pointer 'g' to some empty slot in s->group2
        for(j=0; ; j++){
          assert(j<Mgrp2);
          if(!s->group2[j]){ s->group2[j]=g; break; }
        }
      }
      prepareGroup(g);
    }
  }
}

//------------------------------------------------------------------
//write symbol 'num' on square 's'
void put2(int num, Tsquare *s)
{
  int i,j,k,prev;
  Tline *e;
  Tundo *u;
  Tsquare *t;

  assert(num<Msymbol);
  prev=s->val;
  if(!undoing){
    //create undo record
    u=addUndoRec();
    if(undoPos>redoPos || u->s!=s || u->num!=num || u->action>1){
      u->action=0;
      u->s=s;
      u->num=(char)num;
      u->prev=(char)prev;
      redoPos=0;
    }
  }
  s->val=num;
  //symbol 'num' cannot be on other squares in the same line
  for(j=0; j<s->lines; j++){
    e=s->ptr[j];
    if(prev<0 && num>=0) e->n--;
    else if(num<0 && prev>=0) e->n++;
    for(k=0; k<e->len; k++){
      t=e->ptr[k];
      if(t!=s){
        if(prev>=0) yes(prev,t);
        if(num>=0) no(num,t);
      }
    }
  }
  //there cannot be other symbols on square 's'
  if(prev>=0){
    done--;
    for(i=0; i<Nsymbol; i++){
      if(i!=prev) yes(i,s);
    }
  }
  if(num>=0){
    done++;
    for(i=0; i<Nsymbol; i++){
      if(i!=num) no(i,s);
    }
  }
}

//write symbol 'num' on square 's'
//called by algorithm when solving
void put1(int num, Tsquare *s)
{
  if(blocked) return;
  assert(!s->mask[num] && s->val<0); //symbol must be allowed and square must be empty
  put2(num,s);

  //create variations for group on this square
  for(int i=-1; i<Mgrp2; i++){
    Tgroup *g= (i>=0) ? s->group2[i] : s->group;
    if(g && !g->n){
      prepareGroup(g);
    }
  }
}

//write symbol 'num' on square 's'
//called by user or generator
void put(int num, Tsquare *s)
{
  blocked++; //don't solve other squares
  put2(num,s);
  blocked--;
}

//write greater than sign 'q' at square 's' in direction 'k'
//delete sign if q==0
void putSign(int q, Tsquare *s, int k)
{
  if(!s->neighbour[k]){
    assert(!q);
    return;
  }
  if(q){
    if(!s->sign[k]) Nsign++;
  }else{
    if(s->sign[k]) Nsign--;
  }
  s->sign[k]= BYTE(q);
  s->neighbour[k]->sign[k^2]= BYTE(q ? 3-q : 0);
}

//write or delete consecutive sign at square 's' in direction 'k'
void putCons(bool q, Tsquare *s, int k)
{
  if(!s->neighbour[k]){
    assert(!q);
    return;
  }
  if(q){
    if(!s->cons[k]) Ncons++;
  }else{
    if(s->cons[k]) Ncons--;
  }
  s->cons[k]= s->neighbour[k]->cons[k^2]= q;
}

void putOddEven(int o, Tsquare *s)
{
  int j;

  blocked++; //don't solve other squares
  if(s->oddEven>0){
    for(j=s->oddEven-1; j<Nsymbol; j+=2){
      yes(j,s);
    }
  }
  s->oddEven=o;
  if(o>0){
    for(j=o-1; j<Nsymbol; j+=2){
      no(j,s);
    }
  }
  blocked--;
}

//undo one move or action
void undo1()
{
  undoing++;
  Tundo *u= &undoRec[--undoPos];
  switch(u->action){
  case 3:
    notAllInInter(u->num,u->r);
    break;
  case 5:
    yes(u->num,u->s);
    break;
  case 2:
    u->s->mark[u->num] = !u->s->mark[u->num];
    break;
  default:
    u->num=(char)u->s->val;
    put(u->prev,u->s);
    break;
  }
  undoing--;
}

void undoTo(int p)
{
  while(undoPos>p) undo1();
}
//------------------------------------------------------------------
void consInSameLine(Tsquare *s, Tsquare *s1, Tsquare *s2)
{
  int i,j,o;

  for(i=0; i<s1->lines; i++){
    for(j=0; j<s2->lines; j++){
      if(s1->ptr[i]==s2->ptr[j]){
        //s1,s2 are in the same line
        nou(0,s);
        nou(Nsymbol-1,s);
        for(o=1; o<Nsymbol-1; o++){
          if((s1->mask[o-1] || s2->mask[o+1]) && (s1->mask[o+1] || s2->mask[o-1])){
            nou(o,s);
          }
        }
        return;
      }
    }
  }
}

void resolve3()
{
  int i,j,m,n,o,r,u,a,b,bm,k;
  Tline *e,*f;
  Tsquare *t,*t2,*C[4];

l1:       
  if(inHint && done>=inHint) return;
  u=undoPos;

  //intersections
  for(i=0; i<Nintersect; i++){
    allInInter(&intersects[i]);
    if(blocked) return;
  }
  if(undoPos>u) goto l1;
  
  //killer sudoku
  doGroups();
  if(blocked) return;
  if(undoPos>u) goto l1;

  //lines
  for(i=0; i<Nline; i++){
    e=&line[i];
    if(!e->changed) continue;
    e->changed=false;
    //n symbols in a line (n>=2)
    for(j=0; j<Nsymbol; j++){
      if(e->number[j]<=1) continue;
      m=e->bit[j]; //squares subset complement
      if(!m) continue;
      n=b=0;
      for(o=0; o<Nsymbol; o++){
        if((e->bit[o]&m)==m){ n++; b|=(1<<o); }
      }
      if(n>=e->number[j]){
        if(n>e->number[j]){ blocked++; return; } //too many symbols to fit these squares
        bm= ~b&((1<<Nsymbol)-1);
        for(o=0; o<e->len; o++){
          if(!(m&(1<<o))){
            t=e->ptr[o];
            if(~t->bit & bm){
              for(r=0; r<Nsymbol; r++){
                if(!(b&(1<<r))){
                  if(!t->mask[r]){ 
                    amin(MinLine,n);
                    if(nou(r,t)) return;
                  }
                }
              }
            }
          }
        }
      }
    }
    //n symbols on a square (n>=2)
    for(j=0; j<e->len; j++){
      t=e->ptr[j];
      if(t->n<=1) continue;
      m=t->bit; //which symbols will not be seeked
      if(!m) continue; //all symbols can be on this square
      //find other squares in this line with the same symbols
      n=b=0;
      for(o=0; o<e->len; o++){
        if((e->ptr[o]->bit&m)==m){ n++; b|=(1<<o); }
      }
      if(n>=t->n){
        if(n>t->n){ blocked++; return; } //too few symbols to fill these squares
        bm= ~b&((1<<e->len)-1);
        for(o=0; o<Nsymbol; o++){
          if(!(m&(1<<o))){
            if(~e->bit[o] & bm){
              for(r=0; r<e->len; r++){
                if(!(b&(1<<r))){
                  if(!e->ptr[r]->mask[o]){
                    amin(MonSquare,n);
                    if(nou(o,e->ptr[r])) return;
                  }
                }
              }
            }
          }
        }
      }
    }
  } 
  if(undoPos>u) goto l1;
  
  //X-Wing, Swordfish
  for(k=0; k<multi; k++){
    for(o=0; o<2; o++){ //columns,rows
      for(r=0; r<Nsymbol; r++){
        for(i=0; i<size; i++){
          e=&multiLinePtr[k][i*3+o];
          if(e->number[r]<=1) continue;
          m=e->bit[r];
          if(!m) continue;
          //find other lines with this bitmask
          n=b=0;
          for(j=0; j<size; j++){
            if((multiLinePtr[k][j*3+o].bit[r]&m)==m){ n++; b|=(1<<j); }
          }
          if(n>=e->number[r]){
            if(n>e->number[r]){ blocked++; return; } //too many lines for this symbol
            bm= ~b&((1<<size)-1); 
            for(j=0; j<size; j++){
              if(!(m&(1<<j))){
                f=&multiLinePtr[k][j*3+!o];
                if(~f->bit[r] & bm){
                  for(a=0; a<size; a++){
                    if(!(b&(1<<a))){
                      if(!f->ptr[a]->mask[r]){
                        amin(Mwing,n);
                        if(nou(r,f->ptr[a])) return;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  } 
  if(undoPos>u) goto l1;

  //greater than sudoku
  if(Nsign>0){
    for(i=0; i<Nboard; i++){
      t=&board[i];
      for(j=0; j<2; j++){ //up,left
        t2= t->neighbour[j];
        switch(t->sign[j]){
        case 1: //t2>t
          if(t2->lower <= t->lower){
            for(r=t->lower; r>=0; r--){
              if(nou(r,t2)) return;
            }
          }
          if(t2->upper <= t->upper){
            for(r=t2->upper; r<Nsymbol; r++){
              if(nou(r,t)) return;
            }
          }
          break;
        case 2: //t2<t
          if(t->lower <= t2->lower){
            for(r=t2->lower; r>=0; r--){
              if(nou(r,t)) return;
            }
          } 
          if(t->upper <= t2->upper){
            for(r=t->upper; r<Nsymbol; r++){
              if(nou(r,t2)) return;
            }
          } 
          break;
        }
      }
    }
  }
  if(undoPos>u) goto l1;

  //consecutive sudoku
  if(consecutive){
    for(i=0; i<Nboard; i++){
      t=&board[i];
      n=0;
      for(j=0; j<4; j++){ //up,left,down,right
        t2= t->neighbour[j];
        if(t2){
          if(t->cons[j]){
            C[n++]=t2;
            if(t2->mask[1]) nou(0,t);
            if(t2->mask[Nsymbol-2]) nou(Nsymbol-1,t);
            for(o=1; o<Nsymbol-1; o++){
              if(t2->mask[o-1] && t2->mask[o+1]) nou(o,t);
            }
          }else{//(!t->cons[j])
            o= t2->lower;
            k= t2->upper - o;
            if(k<=2){
              if(k>0) nou(o+1,t);
              if(k==1) nou(o,t);
              if(k==0){
                if(o>0) nou(o-1,t);
                if(o<Nsymbol-1) nou(o+1,t);
              }
            }
          }
        }
      }
      if(n>1){
        //square t has more consecutive neighbours
        consInSameLine(t,C[0],C[1]);
        if(n>2){
          consInSameLine(t,C[0],C[2]);
          consInSameLine(t,C[1],C[2]);
        }
      }
      if(blocked) return;
    }
  }
  if(undoPos>u) goto l1;
}

void resolve2()
{
  int i,k;
  Tsquare *s;
  Tline *e;

  createTempGroups();

  for(i=0; i<Nboard; i++){
    s=&board[i];
    if(s->val<0 && s->n<=1){
      oneOnSquare(s);
      if(blocked) return; 
    }
  }
  for(i=0; i<Nline; i++){
    e=&line[i];
    for(k=0; k<Nsymbol; k++){
      if(e->number[k]<=1){
        oneInLine(k,e);
        if(blocked) return;
      }
    }
  }
  resolve3();
}

//solve without backtracking
void resolve1()
{
  blocked=0;
  prepareGroups();
  resolve2();
}

BYTE *getSolutionPtr(int index)
{
  return &solution[index*(Nsquare*3+1)];
}

void wrSolution()
{
  int i,j;
  BYTE *p;

  p= getSolutionPtr(Nsolution);
  for(i=undoAllPos; i<undoPos; i++){
    Tundo *u= &undoRec[i];
    if(u->action<=1){
      j=int(u->s - board);
      *p++ = (BYTE)u->s->val;
      *p++ = LOBYTE(j);
      *p++ = HIBYTE(j);
    }
  }
  *p++=254;
  Nsolution++;
  //display the first solution
  if(Nsolution==1 && counter>100){
    RedrawWindow(hWin,0,0,RDW_INVALIDATE|RDW_UPDATENOW);
  }
}

void rdSolution()
{
  if(undoPos>undoAllPos) undoAll();
  for(BYTE *p= getSolutionPtr(curSolution); *p!=254; p+=3){
    put(*p, &board[p[1]+(p[2]<<8)]);
  }
}

void resolveRecurse()
{
  int i,i0,k,k0,u,j;
  unsigned oldSeed;
  Tsquare *s;

  if(blocked) return;
  
  #ifdef DBG_VISIBLE
  static int cnt;
  if(--cnt<0){
    cnt=10;
    invalidate(); 
    UpdateWindow(hWin);
  }
  #endif

  //choose random square
  i0=rand(Nboard); 
  if(counter>0 || diag){
    //choose square which has least possibilities
    j=Msymbol+1;
    k0=i0=0;
    for(s=board+Nboard-1; s>=board; s--){
      if(s->n<=j && s->val<0){
        k=rand(0xffffffff);
        if(s->n<j || k>k0){
          j=s->n;
          i0=int(s-board);
          k0=k;
        }
      }
    }
  }
  //choose random symbol
  k0=rand(Nsymbol);

  for(i=i0; ;){
    s=&board[i];
    if(s->val<0){ //find empty square
      for(k=k0; ; ){
        if(!s->mask[k]){
          oldSeed=seed;
          u=undoPos;
          put1(k,s);
          assert(undoRec[u].action==0);
          undoRec[u].action=1;
          //solve as many squares as possible without recursion
          resolve3();
          //recurse
          resolveRecurse();
          if(!blocked){
            if(!isFinished()) blocked++;
            else{
              //OK, solution has been found
              if(curSolution>=0) return; //one solution is enough
              if(Nsolution>=Msolution) return; //buffer is full
              wrSolution();
            }
          }
          //backtrack
          undoTo(u);
          seed=oldSeed;
          prepareGroups();
          counter++;
          if(counter>maxCounter && 
            (curSolution>=0 || Nsolution==0)) return; //time out
          blocked=0;
        }
        k++;
        if(k==Nsymbol) k=0;
        if(k==k0) break;
      }
      //there can be no symbol on square 's'
      blocked++;
      return;
    }
    i++;
    if(i==Nboard) i=0;
    if(i==i0) break; 
  }
  //OK, there is no empty square
}

//solve with backtracking
//find all solutions if curSolution<0
void resolve()
{
  #ifdef NO_rand
    seed=S++;
    maxCounter=50000;
  #else 
    maxCounter=500;       
  #endif

  resolve1();
  //increase depth until solution is found
  for(;;maxCounter=maxCounter*3/2){
    counter=0;
    resolveRecurse();
    #ifdef _DEBUG
      status(1,_T("%d"),counter);
    #endif
    if(counter<maxCounter ||   //all combinations have been processed
      curSolution<0 && Nsolution>0) break; //solution found
    blocked=0;
  }
}

//------------------------------------------------------------------
static int Nundo=0;

Tundo *addUndoRec1(int pos)
{
  if(pos>=Nundo)
  {
    assert(pos==Nundo);
    size_t newN = max(Nundo*2, 1024);
    Tundo *newA = new Tundo[newN];
    if(Nundo>0) memcpy(newA,undoRec,Nundo*sizeof(Tundo));
    delete[] undoRec;
    undoRec=newA;
    Nundo=newN;
  }
  Tundo *u=undoRec+pos;
  u->grp=undoGroupTag;
  return u;
}

Tundo *addUndoRec()
{
  return addUndoRec1(undoPos++);
}

bool undoToAction(int action)
{
  Tundo *u;

  for(;;){
    if(undoPos<=0) return false;
    if(!redoPos) redoPos=undoPos;
    undo1();
    u=&undoRec[undoPos];
    if(u->action<=2) invalidateS(u->s);
    if(u->action<=action &&
      (u->grp==0 || undoPos==0 || undoRec[undoPos-1].grp!=u->grp)) 
      return true;
  }
}

bool undoSymbol()
{
  return undoToAction(1);
}

bool undo()
{
  return undoToAction(2);
}

bool redoToAction(int action)
{
  Tundo *u;
  Tsquare *s;

  for(;;){
    if(!redoPos || redoPos==undoPos) return false;
    u=&undoRec[undoPos];
    s=u->s;
    if(u->action<=2){ 
      if(u->action==2) toggleMark(s,u->num); 
      else{ put(u->num,s); invalidateS(s); }
    }
    if(u->action<=action &&
      (u->grp==0 || redoPos==undoPos || undoRec[undoPos].grp!=u->grp)) 
      return true;
    if(u->action>=3) memmove(u,u+1,((--redoPos)-undoPos)*sizeof(Tundo));
  }
}

bool redoSymbol()
{
  return redoToAction(1);
}

bool redo()
{
  return redoToAction(2);
}
