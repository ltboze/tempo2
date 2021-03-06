#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
//  Copyright (C) 2006,2007,2008,2009, George Hobbs, Russell Edwards

/*
*    This file is part of TEMPO2. 
* 
*    TEMPO2 is free software: you can redistribute it and/or modify 
*    it under the terms of the GNU General Public License as published by 
*    the Free Software Foundation, either version 3 of the License, or 
*    (at your option) any later version. 
*    TEMPO2 is distributed in the hope that it will be useful, 
*    but WITHOUT ANY WARRANTY; without even the implied warranty of 
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
*    GNU General Public License for more details. 
*    You should have received a copy of the GNU General Public License 
*    along with TEMPO2.  If not, see <http://www.gnu.org/licenses/>. 
*/

/*
*    If you use TEMPO2 then please acknowledge it by citing 
*    Hobbs, Edwards & Manchester (2006) MNRAS, Vol 369, Issue 2, 
*    pp. 655-672 (bibtex: 2006MNRAS.369..655H)
*    or Edwards, Hobbs & Manchester (2006) MNRAS, VOl 372, Issue 4,
*    pp. 1549-1574 (bibtex: 2006MNRAS.372.1549E) when discussing the
*    timing model.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tempo2.h"
#include "read_fortran2.h"

double date2mjd(int idat);

void readJBO_bat(char *fname,pulsar *psr,int p)
{
  int itype,i,swap;
  int endit=0;
  int nobs=0;
  union Convert
  {
    char ch[25][sizeof(double)];
    double dbl[25];
    int ival[50];
  } buf;
  const char *CVS_verNum = "$Revision: 1.5 $";

  if (displayCVSversion == 1) CVSdisplayVersion("readJBO_bat.C","readJBO_bat()",CVS_verNum);

  open_file2(fname,&swap);
  printf("Attempting to read %s\n",fname);
  read_record_int2(); 
  do { 
    /*  for (j=0;j<40;j++)
	{*/
      itype = read_int2(); 
      /*      printf("Read itype = %d %d %d %d\n",itype,sizeof(int),sizeof(float),sizeof(double));*/
      for (i=0;i<25;i++)
	{ buf.dbl[i] = read_double2(); buf.ch[i][8]='\0';}
      read_record_int2(); read_record_int2(); 
      if (itype==3)
	{
	  /* Should swap bytes on linux here because we are using characters and not doubles */
	  if (swap==1)
	    {
	      char dummy[1000];
	      for (i=0;i<(int)strlen(buf.ch[1])+1;i++)
		dummy[i] = buf.ch[1][strlen(buf.ch[1])-i-1];
	      strcpy(buf.ch[1],dummy);
	    }
	  printf("Barycentric arrival times written by %s ver %s\n",buf.ch[1],buf.ch[2]); 
	}
      else if (itype==0) /* New observation */
	{
	}
      else if (itype==1) /* Observatory site */
	{
	}
      else if (itype==-1)
	endit=1;
      else if (itype==2) /* BAT record */
	{ 
	  /*	  psr->obsn[nobs].bat = (longdouble)buf.ival[0]; */
	  /*	  for (i=0;i<25;i++)
		  printf("buf[%d] = %f\n",i,buf.dbl[i]); */
	  psr->obsn[nobs].bat = (longdouble)(date2mjd((int)buf.dbl[0])+buf.dbl[1]/86400.0);
	  psr->obsn[nobs].sat = psr->obsn[nobs].bat;
	  psr->obsn[nobs].toaErr = buf.dbl[2]; /* What units for error? */
	  psr->obsn[nobs].freq = buf.dbl[3];
	  psr->obsn[nobs].deleted = 0;
	  psr->obsn[nobs].clockCorr = 0; /* Have barycentric arrival times */
	  psr->obsn[nobs].delayCorr = 0;
	  strcpy(psr->obsn[nobs].telID,"8");  /* SHOULD BE 'BAT' */
	  strcpy(psr->obsn[nobs].fname,"UNKNOWN"); /* HARDCODE FOR NOW */
	  nobs++;
	}
      else
	{
	  printf("Unknown itype = %d\n",itype);
	  exit(1);
	}
    }  while (endit==0); 
  psr->nobs=nobs;
  close_file2();
}

double date2mjd(int idat)
{
  int monthd[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
  int iyr,imn,idy,i;
  double nd;

  iyr = (int)(idat / 10000) + 1900;
  imn = (int)fortran_mod(idat / 100,100);
  idy = (int)fortran_mod(idat,100);
  nd = (int)(iyr * 365.25);
  if (fortran_mod(iyr,4) == 0) 
    {
      monthd[2] = 29;
      nd = nd - 1;
    }
  else
    monthd[2] = 28;

  for (i=1;i<imn;i++)
    nd = nd + monthd[i];
  return (double)((nd + idy) - 678956);
}
