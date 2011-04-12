/*------------------------------------------------------------------------------
* diffeph.c : make difference of ephemerides
*
* 2010/06/17  0.1 new
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "rtklib.h"

static const char *usage=
    "diffeph [-t0 y/m/d h:m:s][-ts ts][-ti ti][-s s][-1 r][-2 b] file1 file2 ...";

/* update rtcm struct --------------------------------------------------------*/
static void updatertcm(gtime_t time, rtcm_t *rtcm, FILE *fp)
{
    char s1[32],s2[32];
    
    while (input_rtcm3f(rtcm,fp)>=0) {
        time2str(time      ,s1,0);
        time2str(rtcm->time,s2,0);
        trace(2,"rtcm.time=%s time=%s\n",s1,s2);
        
        if (timediff(rtcm->time,time)>=5.0) break;
    }
}
/* update rtcm struct --------------------------------------------------------*/
static void updatepcv(gtime_t time, nav_t *nav, pcvs_t *pcvs)
{
    pcv_t *pcv;
    int i;
    
    for (i=0;i<MAXSAT;i++) {
        if (!(pcv=searchpcv(i+1,"",time,pcvs))) {
            fprintf(stderr,"no pcv: sat=%d\n",i+1);
            continue;
        }
        nav->pcv[i]=*pcv;
    }
}
/* print difference ----------------------------------------------------------*/
static void printephdiff(gtime_t time, int sat, int eph1, int eph2,
                         const nav_t *nav, int topt, int mopt)
{
    double tow,rs1[6],rs2[6],dts1[2],dts2[2];
    double drs[3],drss[3],rc[3],er[3],ea[3],ec[3];
    int i,week;
    char tstr[32],id[16];
    
    if (!satpos(time,time,sat,eph1,nav,rs1,dts1)) return;
    if (!satpos(time,time,sat,eph2,nav,rs2,dts2)) return;
    
    for (i=0;i<3;i++) drs[i]=rs1[i]-rs2[i];
    if (!normv3(rs2+3,ea)) return;
    cross3(rs2,rs2+3,rc);
    if (!normv3(rc,ec)) return;
    cross3(ea,ec,er);
    drss[0]=dot(drs,er,3); /* radial/along-trk/cross-trk */
    drss[1]=dot(drs,ea,3);
    drss[2]=dot(drs,ec,3);
    
    if (topt) {
        time2str(time,tstr,0);
        printf("%s ",tstr);
    }
    else {
        tow=time2gpst(time,&week);
        printf("%4d %6.0f",week,tow);
    }
    satno2id(sat,id);
    printf("%4s ",id);
    
    if (mopt&1) {
        printf("%8.3f %8.3f %8.3f ",drss[0],drss[1],drss[2]);
    }
    if (mopt&2) {
        printf("%8.3f ",(dts1[0]-dts2[0])*CLIGHT);
    }
    if (mopt&4) {
        printf(" %13.3f %13.3f %13.3f %12.3f",rs1[0],rs1[1],rs1[2],dts1[0]*CLIGHT);
    }
    printf("\n");
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=NULL;
    nav_t nav={0};
    pcvs_t pcvs={0};
    rtcm_t rtcm;
    gtime_t t0,time;
    double ep0[]={2000,1,1,0,0,0},tspan=24.0,tint=300;
    int i,s,n=0,nx=0,sat=0,topt=0,mopt=0,trl=0;
    int eph1=EPHOPT_BRDC,eph2=EPHOPT_PREC;
    char *files[32],*ext;
    
    t0=epoch2time(ep0);
    
    init_rtcm(&rtcm);
    rtcm.ssr=nav.ssr;
    rtcm.time=t0;
        
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-s" )&&i+1<argc) sat  =atoi(argv[++i]);
        else if (!strcmp(argv[i],"-1" )&&i+1<argc) eph1 =atoi(argv[++i]);
        else if (!strcmp(argv[i],"-2" )&&i+1<argc) eph2 =atoi(argv[++i]);
        else if (!strcmp(argv[i],"-x" )&&i+1<argc) trl  =atoi(argv[++i]);
        else if (!strcmp(argv[i],"-t0")&&i+2<argc) {
            if (sscanf(argv[++i],"%lf/%lf/%lf",ep0  ,ep0+1,ep0+2)<3||
                sscanf(argv[++i],"%lf:%lf:%lf",ep0+3,ep0+4,ep0+5)<1) {
                fprintf(stderr,"invalid time\n");
                return -1;
            }
        }
        else if (!strcmp(argv[i],"-ts")&&i+1<argc) tspan=atof(argv[++i]);
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) tint =atof(argv[++i]);
        else if (!strcmp(argv[i],"-h")) {
            fprintf(stderr,"usage: %s\n",usage);
            return 0;
        }
        else files[n++]=argv[i];
    }
    if (trl>0) {
        traceopen("diffeph.trace");
        tracelevel(trl);
    }
    if (!mopt) mopt=0xFF;
    
    t0=epoch2time(ep0);
    
    init_rtcm(&rtcm);
    rtcm.ssr=nav.ssr;
    rtcm.time=t0;
    
    for (i=0;i<n;i++) {
        if (!(ext=strrchr(files[i],'.'))) ext="";
        if (!strcmp(ext,".sp3")||!strcmp(ext,".clk")) {
            readpephs(files[i],&nav);
        }
        else if (!strcmp(ext,".atx")) {
            if (!readpcv(files[i],&pcvs)) {
                fprintf(stderr,"pcv read error: %s\n",files[i]);
                continue;
            }
            updatepcv(t0,&nav,&pcvs);
        }
        else if (!strcmp(ext,".rtcm3")||!strcmp(ext,".log")) {
            if (!(fp=fopen(files[i],"rb"))) {
                fprintf(stderr,"file open error: %s\n",files[i]);
                return -1;
            }
        }
        else files[nx++]=files[i]; /* rinex */
    }
    readrnx(files,nx,NULL,&nav,NULL);
    
    for (i=0;i<(int)(tspan*3600.0/tint);i++) {
        time=timeadd(t0,tint*i);
        
        fprintf(stderr,"time=%s\r",time_str(time,0));
        
        /* update rtcm struct */
        if (fp) updatertcm(time,&rtcm,fp);
            
        for (s=1;s<=32;s++) {
            if (sat&&s!=sat) continue;
            
            printephdiff(time,s,eph1,eph2,&nav,topt,mopt);
        }
    }
    if (fp) fclose(fp);
    
    return 0;
}
