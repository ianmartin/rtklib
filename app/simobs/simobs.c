/*------------------------------------------------------------------------------
* simobs.c : observation data simulator
*
*          Copyright (C) 2009 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:55:16 $
* history : 2009/03/23  1.0 new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define PROGNAME    "SIMOBS"            /* program name */

/* simulation parameters -----------------------------------------------------*/

static double minel     =5.0;           /* minimum elevation angle (deg) */
static double slipthres =35.0;          /* slip threashold (dBHz) */
static double errion    =0.005;         /* ionosphere error (m/10km) */
static double erreph    =0.8;           /* ephemeris/clock error (m) */

static double errcp1    =0.002;         /* carrier-phase meas error (m) */
static double errcp2    =0.002;         /* carrier-phase meas error/sin(el) (m) */
static double errpr1    =0.2;           /* pseudorange error (m) */
static double errpr2    =0.2;           /* pseudorange error/sin(el) (m) */

static int gpsblock[]={                 /* gps block flag (1:block IIF) */
    1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0, 0,0,0,0,0, 0,0
};
/* read elevation mask file --------------------------------------------------*/
static int readelmask(const char *file, double *elmask)
{
    FILE *fp;
    double az0=0.0,el0=0.0,az1,el1;
    char buff[256];
    int i,a;
    
    for (i=0;i<=360;i++) elmask[i]=0.0;
    
    if (!(fp=fopen(file,"r"))) return 0;
    while (fgets(buff,sizeof(buff),fp)) {
        if (buff[0]=='%') continue;
        if (sscanf(buff,"%lf %lf",&az1,&el1)<2) continue;
        if (az0<az1&&az1<=360.0&&0.0<=el1&&el1<=90.0) {
            for (a=(int)az0;a<=(int)az1;a++) {
                elmask[a]=el0*D2R;
            }
        }
        az0=az1; el0=el1;
    }
    fclose(fp);
    return 1;
}
/* generate random number with normal distribution ---------------------------*/
static double randn(double myu, double sig)
{
    double a,b;
    a=((double)rand()+1.0)/((double)RAND_MAX+1.0);  /* 0<a<=1 */
    b=((double)rand()+1.0)/((double)RAND_MAX+1.0);  /* 0<b<=1 */
    return myu+sqrt(-2.0*log(a))*sin(2.0*PI*b)*sig;
}
/* generate snr --------------------------------------------------------------*/
static void snrmodel(const double *azel, double *snr)
{
    /* snr and snr deviation pattern (dbHz) by elevation (5 deg interval) */
    double snrs[]={40,42,44,45,46,47,48,49,49,50,50,51,51,51,51,51,51,51,51};
    double sdvs[]={ 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    double loss[]={ 0, 3, 0, 3};
    int i,j;
    for (i=0;i<NFREQ;i++) {
        j=(int)(azel[1]*R2D/5.0);
        if (snr[i]==0.0) {
            snr[i]=snrs[j]+randn(0.0,sdvs[j])-loss[i];
        }
        else {
            snr[i]=0.5*snr[i]+0.5*(snrs[j]+randn(0.0,sdvs[j])-loss[i]);
        }
    }
}
/* generate error ------------------------------------------------------------*/
static void errmodel(const double *azel, double *snr, double *ecp, double *epr)
{
    int i;
    double ec,ep;
    
    for (i=0;i<NFREQ;i++) {
        ec=randn(0.0,errcp1)+randn(0.0,errcp2)/sin(azel[1]);
        ep=randn(0.0,errpr1)+randn(0.0,errpr2)/sin(azel[1]);
        
        ecp[i]=0.5*ecp[i]+0.5*ec; /* filter */
        epr[i]=0.5*epr[i]+0.5*ep; /* filter */
    }
}
/* satellite positions -------------------------------------------------------*/
static void satposs(gtime_t time, obsd_t *obs, int n, nav_t *nav, double *rs,
                    double *dts, double ers[][4])
{
	int i,j;
	
	for (i=0;i<n;i++) {
		/* precise ephemeris */
		satposp(time,obs+i,1,nav,rs+i*3,dts+i);
		
		/* broadcast ephemeris + error */
		if (norm(rs+i*3,3)<=0.0) {
			
			satpos(time,obs+i,1,nav,rs+i*3,dts+i);
			
			/* add ephemeirs and clock error */
			for (j=0;j<3;j++) rs[j+i*3]+=ers[i][j];
			dts[i]+=ers[i][3]/CLIGHT;
		}
	}
}
/* generate simulated observation data ---------------------------------------*/
static int simobs(gtime_t ts, gtime_t te, double tint, const double *rr,
                  nav_t *nav, obs_t *obs, int opt, const double *elmask)
{
    gtime_t time;
    obsd_t data[MAXSAT]={{{0}}};
    double pos[3],rs[3*MAXSAT],dts[MAXSAT],r,e[3],azel[2];
    double ecp[MAXSAT][NFREQ]={{0}},epr[MAXSAT][NFREQ]={{0}};
    double snr[MAXSAT][NFREQ]={{0}},ers[MAXSAT][4]={{0}};
    double iono,trop,fact,cp,pr,dtr=0.0,rref[3],bl;
    int i,j,k,n,ns,amb[MAXSAT][NFREQ]={{0}},sys,prn;
    char s[64];
    
    double pref[]={36.106114294,140.087190410,70.3010}; /* ref station */
    
    trace(3,"simobs:nnav=%d ngnav=%d\n",nav->n,nav->ng);
    
    for (i=0;i<2;i++) pref[i]*=D2R;
    pos2ecef(pref,rref);
    for (i=0;i<3;i++) rref[i]-=rr[i];
    bl=norm(rref,3)/1E4; /* baseline (10km) */
    srand(0);
    
    /* ephemeris error */
    for (i=0;i<MAXSAT;i++) {
        data[i].sat=i+1;
        data[i].P[0]=2E7;
        for (j=0;j<4;j++) ers[i][j]=randn(0.0,erreph);
    }
    srand(tickget());
    
    ecef2pos(rr,pos);
    n=(int)(timediff(te,ts)/tint+1.0);
    
    for (i=0;i<n;i++) {
        time=timeadd(ts,tint*i);
        time2str(time,s,0);
        
        for (j=0;j<MAXSAT;j++) data[j].time=time;
        
        for (j=0;j<3;j++) { /* iteration for pseudorange */
            satposs(time,data,MAXSAT,nav,rs,dts,ers);
            for (k=0;k<MAXSAT;k++) {
                if ((r=geodist(rs+k*3,rr,e))<=0.0) continue;
                data[k].P[0]=r+CLIGHT*(dtr-dts[k]);
            }
        }
        satposs(time,data,MAXSAT,nav,rs,dts,ers);
        
        for (j=ns=0;j<MAXSAT;j++) {
            
            if ((r=geodist(rs+j*3,rr,e))<=0.0) continue;
            satazel(pos,e,azel);
            if (azel[1]<minel*D2R) continue;
            if (azel[1]<elmask[(int)(azel[0]*R2D+0.5)]) continue;
            
            iono=ionmodel(time,nav->ion,pos,azel);
            trop=tropmodel(pos,azel,0.3);
            
            /* add ionospheric error */
            iono+=errion*bl*ionmapf(pos,azel);
            
            snrmodel(azel,snr[j]);
            errmodel(azel,snr[j],ecp[j],epr[j]);
            sys=satsys(data[j].sat,&prn);
            
            for (k=0;k<NFREQ;k++) {
                data[j].L[k]=data[j].P[k]=0.0;
                data[j].SNR[k]=0;
                data[j].LLI[k]=0;
                
                if (sys==SYS_GPS) {
                    if (k>=3) continue; /* no L5a/L5b in gps */
                    if (k>=2&&!gpsblock[prn-1]) continue; /* no L5 in block II */
                }
                else if (sys==SYS_GLO) {
                    if (k>=3) continue;
                }
                else if (sys==SYS_GAL) {
                    if (k==1) continue; /* no L2 in galileo */
                }
                else if (sys==SYS_QZS) {
                    ;
                }
                else continue;
                
                /* generate observation data */
                fact=lam[k]*lam[k]/lam[0]/lam[0];
                cp=r+CLIGHT*(dtr-dts[j])-fact*iono+trop+ecp[j][k];
                pr=r+CLIGHT*(dtr-dts[j])+fact*iono+trop+epr[j][k];
                
                if (amb[j][k]==0) amb[j][k]=(int)(-cp/lam[k]);
                data[j].L[k]=cp/lam[k]+amb[j][k];
                data[j].P[k]=pr;
                data[j].SNR[k]=(unsigned char)snr[j][k];
                data[j].LLI[k]=data[j].SNR[k]<slipthres?1:0;
            }
            if (obs->nmax<=obs->n) {
                if (obs->nmax==0) obs->nmax=65532; else obs->nmax+=65532;
                if (!(obs->data=(obsd_t *)realloc(obs->data,sizeof(obsd_t)*obs->nmax))) {
                    fprintf(stderr,"malloc error\n");
                    return 0;
                }
            }
            obs->data[obs->n++]=data[j];
            ns++;
        }
        fprintf(stderr,"time=%s nsat=%2d\r",s,ns);
    }
    fprintf(stderr,"\n");
    return 1;
}
/* simgal main ---------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp;
    rnxopt_t rnxopt={{0}};
    obs_t obs={0};
    nav_t nav={0};
    gtime_t ts={0},te={0};
    double es[]={2000,1,1,0,0,0},ee[]={2000,1,1,0,0,0},tint=30.0;
    double pos[3]={0},rr[3],elmask[361]={0};
    char *infile[16]={0},*outfile="",*elmaskfile="";
    char *sp3file="",*antfile="";
    int i,j,n=0,opt=0;
    
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-o")&&i+1<argc) outfile=argv[++i];
        else if (!strcmp(argv[i],"-ts")&&i+1<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf %lf:%lf:%lf",es,es+1,es+2,es+3,es+4,es+5);
            ts=epoch2time(es);
        }
        else if (!strcmp(argv[i],"-te")&&i+1<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf %lf:%lf:%lf",ee,ee+1,ee+2,ee+3,ee+4,ee+5);
            te=epoch2time(ee);
        }
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) tint=atof(argv[++i]);
        else if (!strcmp(argv[i],"-r")&&i+3<argc) {
            for (j=0;j<3;j++) pos[j]=atof(argv[++i]); /* lat,lon,hgt */
        }
        else if (!strcmp(argv[i],"-s")&&i+1<argc) {
            sp3file=argv[++i];
        }
        else if (!strcmp(argv[i],"-a")&&i+1<argc) {
            antfile=argv[++i];
        }
        else if (!strcmp(argv[i],"-m")&&i+1<argc) {
            elmaskfile=argv[++i];
        }
        else infile[n++]=argv[i];
    }
    if (n<=0) {
        fprintf(stderr,"no input file\n");
        return -1;
    }
    if (!*outfile) {
        fprintf(stderr,"no output file\n");
        return -1;
    }
    if (norm(pos,3)<=0.0) {
        fprintf(stderr,"no receiver pos\n");
        return -1;
    }
    pos[0]*=D2R; pos[1]*=D2R; pos2ecef(pos,rr);
    
    /* read simulated/real rinex nav files */
    readrnx(infile,n,&obs,&nav);
    
    if (nav.n<=0) {
        fprintf(stderr,"no nav data\n");
        return -1;
    }
    /* read sp3 precise ephemeris */
    if (*sp3file&&readsp3(sp3file,&nav.peph)<=0) {
        fprintf(stderr,"no sp3 precise ephemeris: %s\n",sp3file);
        return -1;
	}
    /* read antenna parameter file */
    if (*antfile&&!readsap(antfile,obs.data[0].time,nav.sap)) {
        fprintf(stderr,"antenna parameter file read error: %s\n",antfile);
        return -1;
	}
    /* read elev mask data */
    if (*elmaskfile&&!readelmask(elmaskfile,elmask)) {
        fprintf(stderr,"no elev mask file: %s\n",elmaskfile);
        return -1;
    }
    /* generate simulated observation data */
    if (!simobs(ts,te,tint,rr,&nav,&obs,opt,elmask)) return -1;
    
    /* output rinex obs file */
    if (!(fp=fopen(outfile,"w"))) {
        fprintf(stderr,"error : outfile open %s\n",outfile);
        return -1;
    }
    fprintf(stderr,"saving...: %s\n",outfile);
    strcpy(rnxopt.prog,PROGNAME);
    strcpy(rnxopt.comment[0],"SIMULATED OBS DATA");
    rnxopt.tstart=ts;
    rnxopt.tstart=te;
    rnxopt.navsys=SYS_ALL;
    rnxopt.obstype=OBSTYPE_PR|OBSTYPE_CP|OBSTYPE_SNR;
    rnxopt.freqtype=FREQTYPE_L1|FREQTYPE_L2|FREQTYPE_L5|FREQTYPE_L7;
    for (i=0;i<3;i++) rnxopt.apppos[i]=rr[i];
    
    outrnxobsh(fp,&rnxopt);
    
    for (i=0;i<obs.n;i=j) {
        for (j=i;j<obs.n;j++) {
            if (timediff(obs.data[j].time,obs.data[i].time)>0.001) break;
        }
        outrnxobsb(fp,&rnxopt,obs.data+i,j-i,0);
    }
    fclose(fp);
    return 0;
}
