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


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "tempo2.h"

long double solarWindModel(pulsar psr);

void dm_delays(pulsar *psr,int npsr,int p,int i,double delt,double dt_SSB)
{
  double ctheta,freqf,pospos,r,voverc,dmval,dmval2;
  double rsa[3],pos[3],vobs[3],yrs;
  double dmDot;
  int j,k;
  const char *CVS_verNum = "$Revision: 1.19 $";

  if (displayCVSversion == 1) CVSdisplayVersion("dm_delays.C","dm_delays()",CVS_verNum);
  if (debugFlag==1) printf("In dm_delays with pulsar %d; number of obs = %d\n",p,psr[p].nobs);

  if (psr[p].obsn[i].delayCorr==0) /* No correction */
    {
      psr[p].obsn[i].tdis1 = 0.0;
      psr[p].obsn[i].tdis2 = 0.0;
    }
  else
    {
      for (k=0;k<3;k++)
	pos[k] = psr[p].posPulsar[k]+delt*psr[p].velPulsar[k];      
      pospos = sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]);
      for (k=0;k<3;k++)
	pos[k] /= pospos;
      if (debugFlag==1) printf("In dm_delays with pos = %f %f %f\n",pos[0],pos[1],pos[2]);      
      
      /* Calculate position of observatory from Sun */
      for (j=0;j<3;j++)
	{
	  /*	  rsa[j] = -psr[p].obsn[i].sun_ssb[j] + psr[p].obsn[i].earthMoonBary_ssb[j] -
		  psr[p].obsn[i].earthMoonBary_earth[j] + psr[p].obsn[i].observatory_earth[j]; */
	  if (strcmp(psr[p].obsn[i].telID,"STL_FBAT")==0)
	    rsa[j] = -psr[p].obsn[i].sun_ssb[j] + psr[p].obsn[i].observatory_earth[j];
	  else
	    rsa[j] = -psr[p].obsn[i].sun_ssb[j] + psr[p].obsn[i].earth_ssb[j] + psr[p].obsn[i].observatory_earth[j];
	}
      if (debugFlag==1) printf("In dm_delays with rsa = %f %f %f\n",rsa[0],rsa[1],rsa[2]);            
      //      printf("In dm_delays with rsa = %s %f %f %f\n",psr[p].obsn[i].fname,rsa[0],rsa[1],rsa[2]);            
      /* What about Sun from SSB? */
      for (j=0;j<3;j++)
	{
	  if (strcmp(psr[p].obsn[i].telID,"STL_FBAT")==0)
	    {
	      if (psr[p].param[param_telx].paramSet[1] == 1 &&
		  psr[p].param[param_tely].paramSet[1] == 1 &&
		  psr[p].param[param_telz].paramSet[1] == 1)
		{
		  if (j==0)      vobs[j] = (double)psr[p].param[param_telx].val[1];
		  else if (j==1) vobs[j] = (double)psr[p].param[param_tely].val[1];
		  else if (j==2) vobs[j] = (double)psr[p].param[param_telz].val[1];
		  //		  printf("Setting vobs = %g\n",vobs[j]);
		}
	      else
		{
		  if (debugFlag==1) printf("WARNING: setting vobs = 0, this can give a large error!\n");
		  vobs[j] = 0.0;
		}
	    }
	  else
	    vobs[j] = psr[p].obsn[i].earth_ssb[j+3] + psr[p].obsn[i].siteVel[j];
	}
	/*	vobs[j] = psr[p].obsn[i].earthMoonBary_ssb[j+3] - psr[p].obsn[i].earthMoonBary_earth[j+3] + 
		psr[p].obsn[i].siteVel[j];*/
      if (debugFlag==1) printf("In dm_delays with vobs = %f %f %f\n",vobs[0],vobs[1],vobs[2]);      
      // printf("In dm_delays with vobs = %s %g %g %g\n",psr[p].obsn[i].fname,vobs[0],vobs[1],vobs[2]);      
      r = sqrt(dotproduct(rsa,rsa));
      ctheta = dotproduct(pos,rsa)/r;
      voverc = dotproduct(pos,vobs);
      freqf = psr[p].obsn[i].freq*1.0e6*(1.0-voverc);   /* Converting frequency in to Hz */
      //      printf("freqf = %g %g %g %g\n",freqf,psr[p].obsn[i].freq,voverc,psr[p].obsn[i].einsteinRate);
      /* Transform freq due to Einstein delay ! */
      if (psr[p].dilateFreq && freqf > 0 && psr[p].obsn[i].einsteinRate != 0.0)
      	freqf /= psr[p].obsn[i].einsteinRate;

      if (debugFlag==1) printf("In dm_delays: Transforming frequency due to Einstein delay\n");      
      
      psr[p].obsn[i].freqSSB = freqf; /* Record observing frequency in barycentric frame */
      if (debugFlag==1) printf("In dm_delays: set freqSSB\n");      
      yrs = (psr[p].obsn[i].sat - psr[p].param[param_dmepoch].val[0])/365.25;
      dmDot=0.0;
      
      longdouble arg = 1.0;
      // redwards changed to avoid using slow calls to pow
      // Note this is not a Taylor Series - the Edwards paper says that it is!
      for (k=1;k<9;k++)
	{
	  arg *= yrs;
	  if (psr[p].param[param_dm].paramSet[k]==1)
	    dmDot+=(double)(psr[p].param[param_dm].val[k]*arg); 
	}
      if (debugFlag==1) printf("In dm_delays: calculated dmDot %Lg\n",psr[p].param[param_dm].val[0]);
      dmval = psr[p].param[param_dm].val[0]+dmDot;
      if (debugFlag==1) printf("In dm_delays: calculating dmval\n");      
      // NOT DONE ANYMORE:      dmval += psr[p].obsn[i].phaseOffset;  /* In completely the wrong place - phaseoffset is actually DM offset */


      /* Are we using DM values using DMMODEL parameter? */
      if (psr[p].param[param_dmmodel].paramSet[0] == 1)
	{
	  double m,c;
	  static int t=0xFFFF;
	  dmval=0;
	  //double meanDM=(double)psr[p].param[param_dm].val[0];
	  double meanDM=getParameterValue(&psr[p],param_dmmodel,0);

	  if (psr[p].param[param_dm].paramSet[0] == 1 && psr->param[param_dmmodel].linkTo[0] != param_dm)
	  {
		  if(t & 0x000F){
			  printf("WARNING: Using DMMODEL value for dispersion measure instead of the DM parameter\n");
			  printf("         Please set 'DMMODEL DM' in the par file rather than specifying a value\n");
			  t= t & 0xFFF0;
		  }
		  //	      psr[p].param[param_dmmodel].val[0]=psr[p].param[param_dm].val[0];
	  }

	  // set the mean DMOFF to zero.
	  if(t&0x00F0){
		  double mean_dmoff=0;
		  for (k=0;k<psr[p].dmoffsNum;k++){
			  mean_dmoff+=psr[p].dmoffsDM[k];
		  }
		  mean_dmoff /= (double)psr[p].dmoffsNum;
		  if(mean_dmoff > 1e-12){
			  printf("WARNING: Mean DMOFF = %lg\n",mean_dmoff);
			  for(k=0;i < psr[p].nconstraints; k++){
				  if (psr[p].constraints[k]==constraint_dmmodel_mean){
					  printf("         Removing mean to keep DMMODEL constraint correct.\n");
					  for (k=0;k<psr[p].dmoffsNum;k++){
						  psr[p].dmoffsDM[k] -= mean_dmoff;
					  }
					  break;
				  }
			  }
		  }
		  t = t & 0xFF0F;
	  }

	  if ((double)psr[p].obsn[i].sat < psr[p].dmoffsMJD[0])
	    dmval = meanDM + psr[p].dmoffsDM[0];
	  else if ((double)psr[p].obsn[i].sat > psr[p].dmoffsMJD[psr[p].dmoffsNum-1])
	    dmval = meanDM+psr[p].dmoffsDM[psr[p].dmoffsNum-1];
	  else
	    {
	      for (k=0;k<psr[p].dmoffsNum-1;k++)
		{
		  if ((double)psr[p].obsn[i].sat >= psr[p].dmoffsMJD[k] &&
		      (double)psr[p].obsn[i].sat < psr[p].dmoffsMJD[k+1])
		    {
		      // Do linear interpolation
		      // Note: this is also used at various points in the 
		      // code (e.g., textOutput.C - if any changes are 
		      // made here then these changes should be made throughout!
		      m = (psr[p].dmoffsDM[k]-psr[p].dmoffsDM[k+1])/(psr[p].dmoffsMJD[k]-psr[p].dmoffsMJD[k+1]);
		      c = psr[p].dmoffsDM[k]-m*psr[p].dmoffsMJD[k];
		      dmval = m*(double)psr[p].obsn[i].sat+c + meanDM;
		      break;
		    }
		}
	    }
	}
      else
	{
	  /* Set using DMX ranges */
	  for (k=0; k<psr[p].ndmx; k++) 
	    {
	      if ((psr[p].obsn[i].sat > psr[p].param[param_dmxr1].val[k])
                  && (psr[p].obsn[i].sat < psr[p].param[param_dmxr2].val[k]))
            dmval += psr[p].param[param_dmx].val[k];
	    }
	  
	  /* Check for flag to set dm to exact value */
	  for (k=0;k<psr[p].obsn[i].nFlags;k++)
	    {
	      if (strcmp(psr[p].obsn[i].flagID[k],"-dm")==0) /* Have DM correction */
		{
		  sscanf(psr[p].obsn[i].flagVal[k],"%lf",&dmval);
		  dmval+=psr[p].dmOffset;
		}
	      if (strcmp(psr[p].obsn[i].flagID[k],"-dmo")==0) /* Have DM correction */
		{
		  sscanf(psr[p].obsn[i].flagVal[k],"%lf",&dmval2);
		  dmval+=(dmval2+psr[p].dmOffset);
		}
	    }
	  if (debugFlag==1) printf("In dm_delays: Looked for flags\n");      
	}
      if (freqf<=1) /* Have infinitive frequency */
	psr[p].obsn[i].tdis1 = 0.0;
      else
	psr[p].obsn[i].tdis1 = dmval/DM_CONST/1.0e-12/freqf/freqf; 

      if (debugFlag==1) printf("In dm_delays: calculate tdis1\n");      
      /* Add frequency dependent delay term */
      if (psr[p].param[param_fddc].paramSet[0]==1 && freqf>1)
	psr[p].obsn[i].tdis1 += (double)(psr[p].param[param_fddc].val[0]/pow(freqf*1.0e-6,(double)psr[p].param[param_fddi].val[0]));
      if (freqf<=1.0 || psr[p].ipm==0){
	psr[p].obsn[i].tdis2 = 0.0;
      }
      else if (psr[p].swm==0) // Simple tempo2 solar wind model
	{
	  /* TEMPO1: Note: the following line simplifies to 2e14 theta/r/sin(theta)/2/freqf^2 */
	  /* psr[p].obsn[i].tdis2 = 2.0e14*acos(ctheta)/r/sqrt(1.0-ctheta*ctheta)/2.0/freqf/freqf; 
 printf("tdis %f %f %f %f %f %g %g\n",1.0e6,AU_DIST,SPEED_LIGHT,DM_CONST_SI,psr[p].ne_sw,1.0e6*AU_DIST*AU_DIST/SPEED_LIGHT/DM_CONST_SI*psr[p].ne_sw*
 acos(ctheta)/r/sqrt(1.0-ctheta*ctheta)/freqf/freqf,psr[p].obsn[i].tdis2); */
	  psr[p].obsn[i].tdis2 = 1.0e6*AU_DIST*AU_DIST/SPEED_LIGHT/DM_CONST_SI*psr[p].ne_sw*
		  acos(ctheta)/r/sqrt(1.0-ctheta*ctheta)/freqf/freqf; 
	}
      else if (psr[p].swm==1) // More complex solar wind model introduced by Xiaopeng You and William Coles
	{
	  //	  if(acos(ctheta)*180/3.14159265>120)
	    psr[p].obsn[i].tdis2 = (double)solarWindModel(psr[p],i)/DM_CONST/1.0e-12/freqf/freqf;
	    //	  else
	    //	    psr[p].obsn[i].tdis2 = 1.0e6*AU_DIST*AU_DIST/SPEED_LIGHT/DM_CONST_SI*psr[p].ne_sw*acos(ctheta)/r/sqrt(1.0-ctheta*ctheta)/freqf/freqf; 	    
	}

            if (debugFlag==1) 
	  printf("[%d/%d] In dm_delays with tdis2 = %g, freqf = %g %g %g %d %g\n",i,
		 psr[p].nobs,(double)psr[p].obsn[i].tdis2,(double)freqf,psr[p].obsn[i].freq,
		 (1.0-voverc),psr[p].dilateFreq,psr[p].obsn[i].einsteinRate);      

      /* psr[p].obsn[i].tdis = (psr[p].param[param_dm].val/2.41e-16 +
	 2.0e14*acos(ctheta)/r/sqrt(1.0-ctheta*ctheta)/2.0)/freqf/freqf; */
      /* printf("Dispersion delay = %g\n",(double)(psr[p].obsn[i].tdis1+psr[p].obsn[i].tdis2));  */

    }
  if (debugFlag==1) 
      printf("Exiting dm_delays with pulsar %d; number of obs = %d\n",p,psr[p].nobs);

}

