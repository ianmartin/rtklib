/*------------------------------------------------------------------------------
* simslip.c : simulator slip
*
*          Copyright (C) 2008 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 22:04:02 $
* history : 2008/08/25 1.0 new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"
#include "ins.h"

static const char rcsid[]="$Id: sbspos.c,v 1.1 2008/07/17 22:04:02 ttaka Exp $";

#define PROGNAME    "simslip ver.0.1"
#define NTOBS		8

static char tobs[NTOBS][3]={"C1","L1","D1","S1","P2","L2","D2","S2"};
static rnxopt_t rnxopt={"simslip"};

/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" usage: inspos [option ...] file file [...]",
"",
" -t file   output trace to file [off]",
};
/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<sizeof(help)/sizeof(*help);i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* search next observation data index ----------------------------------------*/
static int nextobsf(const obs_t *obs, int *i, int rcv)
{
    double tt;
    int n;
    for (;*i<obs->n;(*i)++) if (obs->data[*i].rcv==rcv) break;
    for (n=0;*i+n<obs->n;n++) {
        tt=timediff(obs->data[*i+n].time,obs->data[*i].time);
        if (obs->data[*i+n].rcv!=rcv||tt>DTTOL) break;
    }
    return n;
}
/* randam number ------------------------------------------------------------*/
static double randd(void)
{
	return (double)rand()/RAND_MAX;
}
/* data gap -----------------------------------------------------------------*/
static int datagap(void)
{
	double n=randd(),gap;
//	if      (n<0.68) gap=1.0*randd();
//	else if (n<0.82) gap=1.0+2.0*randd();
//	else if (n<0.88) gap=3.0+2.0*randd();
//	else gap=5.0+30.0*randd();
	if      (n<0.40) gap=1.0*randd();
	else if (n<0.50) gap=1.0+2.0*randd();
	else if (n<0.60) gap=3.0+2.0*randd();
	else gap=5.0+30.0*randd();
	return (int)(gap/0.25);
}
/* simulate slips ------------------------------------------------------------*/
static void simslip(FILE *fp, gtime_t ts, gtime_t te, obs_t *obs, nav_t *nav,
                    int rate, int opt)
{
	gtime_t time;
	int i,j,n,m,outh=0,index,sat,nslip=0,vsat[MAXOBS];
	int slips[32][2]={{0}},gaps[32]={0},halfc[32]={0};
	double sliprate=1.0/rate,rr[3]={0},dtr,Qr[9],azel[2*32]={0},sinp,offset;
	double res[MAXOBS];
	char s[64];
	
	for (index=m=0;;index+=n) {
		if ((n=nextobsf(obs,&index,1))<=0) break;
		
		time=obs->data[index].time;
		if (ts.time!=0&&timediff(time,ts)<-0.05) continue;
		if (te.time!=0&&timediff(time,te)> 0.05) break;
		
		time2str(time,s,3);
		if (!outh) {
			outrnxobsh(fp,&rnxopt,tobs,NTOBS,time);
			outh=1;
		}
		pntpos(obs->data+index,n,nav,15*D2R,0,rr,Qr,&dtr,azel,vsat,res);
		
		if (++m%50==0) {
			sliprate=1.0/rate*randd()*2.0;
		}
		for (i=index;i<index+n;i++) {
			sat=obs->data[i].sat; if (sat>32) continue;
			
			sinp=sin(azel[1+(i-index)*2]); /* randomly generated weighted by el */
			if (randd()<sliprate/sinp/sinp) {
				/* add gaps */
				gaps[sat-1]=sinp>sin(75.0*D2R)?1:datagap()+1;
				trace(1,"add gap : time=%s sat=%2d gaps=%d\n",s,sat,gaps[sat-1]);
			}
			if (gaps[sat-1]>1) { /* in data gap */
				obs->data[i].sat=0;
				trace(2,"no data : time=%s sat=%2d gap=%d\n",s,sat,gaps[sat-1]);
			}
			else if (gaps[sat-1]==1) { /* end of data gap */
				for (j=0;j<2;j++) {
					/* add slips */
					slips[sat-1][j]+=(j==0?100:200);
					obs->data[i].LLI[j]|=1;
					trace(1,"add slip: time=%s PRN%2d L%d\n",s,sat,j+1);
				}
				if (opt&1) halfc[sat-1]=randd()<0.5?1:2;
				nslip++;
			}
			if ((opt&1)&&fmod(time2gpst(time,NULL)-0.25,6.0)<0.05) { /* subframe start */
				if (halfc[sat-1]>0) {
					if (halfc[sat-1]==2) {
						for (j=0;j<2;j++) obs->data[i].LLI[j]|=1;
					}
					halfc[sat-1]=0;
				}
			}
			for (j=0;j<2;j++) {
				offset=slips[sat-1][j]+((opt&1)&&halfc[sat-1]==2?0.5:0.0);
				obs->data[i].L[j]+=offset;
				if (opt&1) obs->data[i].LLI[j]|=halfc[sat-1]>0?2:0;
				trace(2,"offset : PRN%2d L%d offset=%.1f\n",sat,j+1,offset);
			}
		}
		/* output rinex obs data */
		outrnxobsb(fp,obs->data+index,n,0,tobs,NTOBS);
		
		for (i=0;i<32;i++) gaps[i]--;
	}
	printf("nslip=%d\n",nslip);
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=stdout;
    obs_t obs={0};
    nav_t nav={0};
    gtime_t ts={0},te={0};
    double elmin=15.0*D2R,es[6]={2000,1,1},ee[6]={2000,1,1};
    int i,n=0,rate=100,opt=0,trcout=0;
    char *files[32]={0},*ofile="",*sfile="",tfile[1024],*p;
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-o")&&i+1<argc) ofile=argv[++i];
        else if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",es,es+1,es+2);
            sscanf(argv[++i],"%lf:%lf:%lf",es+3,es+4,es+5);
            ts=epoch2time(es);
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",ee,ee+1,ee+2);
            sscanf(argv[++i],"%lf:%lf:%lf",ee+3,ee+4,ee+5);
            te=epoch2time(ee);
        }
        else if (!strcmp(argv[i],"-rate")&&i+1<argc) rate=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-opt")&&i+1<argc) opt=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-t")&&i+1<argc) trcout=atoi(argv[++i]);
        else if (!strncmp(argv[i],"-",1)) printhelp();
        else files[n++]=argv[i];
    }
    if (readrnx(files,n,&obs,&nav)<=0) {
        fprintf(stderr,"no observation data\n");
        return -1;
    }
    if (nav.n<=0) {
        fprintf(stderr,"no navigation message\n");
        return -1;
    }
    if (*ofile&&!(fp=fopen(ofile,"w"))) {
        fprintf(stderr,"output file open error : %s\n",ofile);
        return -1;
    }
    if (*ofile&&trcout>0) {
        strcpy(tfile,ofile);
        if (!(p=strrchr(tfile,'.'))) p=tfile+strlen(tfile);
        strcpy(p,".trc");
        traceopen(tfile);
        tracelevel(trcout);
    }
    /* output simulated obs */
    simslip(fp,ts,te,&obs,&nav,rate,opt);
    
    fclose(fp);
    return 0;
}
