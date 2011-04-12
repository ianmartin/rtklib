/*------------------------------------------------------------------------------
* rinex.c : rinex functions
*
*          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 2.11, December 10, 2007
*     [2] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 3.00, November 28, 2007
*     [3] IS-GPS-200D, Navstar GPS Space Segment/Navigation User Interfaces,
*         7 March, 2006
*     [4] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 2.12, June 23, 2009
*
* version : $Revision:$
* history : 2006/01/16 1.0  new
*           2007/03/14 1.1  read P1 if no obstype of C1
*           2007/04/27 1.2  add readrnxt() function
*           2007/05/25 1.3  add support of file path with wild-card (*)
*                           add support of compressed files
*           2007/11/02 1.4  support sbas/geo satellite
*                           support doppler observables
*                           support rinex bug of week handover
*                           add rinex obs/nav output functions
*           2008/06/16 1.5  export readrnxf(), add compress()
*                           separate sortobs(), uniqeph(), screent()
*           2008/10/28 1.6  fix bug on reading rinex obs header types of observ
*           2009/04/09 1.7  support rinex 2.11
*                           change api of outrnxobsh(),outrnxobsb(),outrnxnavb()
*           2009/06/02 1.8  add api outrnxgnavb()
*           2009/08/15 1.9  support glonass
*                           add slip save/restore functions
*           2010/03/03 1.10 fix bug of array access by disabled satellite
*           2010/07/21 1.11 support rinex ver.2.12, 3.00
*                           support rinex extension for qzss
*                           support geo navigation messages
*                           added api:
*                               setrnxcodepri(),outrnxhnavh(),outrnxhnavb(),
*                           changed api:
*                               readrnx(),readrnxt(),outrnxnavh(),outrnxgnavh()
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* constants/macros ----------------------------------------------------------*/

#define SQR(x)      ((x)*(x))

#define NUMSYS      6                   /* number of systems */
#define MAXRNXLEN   1024                /* max rinex record length */
#define MAXPOSHEAD  16                  /* max head line position */
#define MINFREQ_GLO -7                  /* min frequency number glonass */
#define MAXFREQ_GLO 13                  /* max frequency number glonass */

static const int navsys[]={             /* satellite systems */
    SYS_GPS,SYS_GLO,SYS_GAL,SYS_QZS,SYS_SBS,SYS_CMP,0
};
static const char syscodes[]="GREJSC";  /* satellite system codes */

static const char obscodes[]="CLDS";    /* obs type codes */

static const char frqcodes[]="125768";  /* frequency codes */

static const char frqvalid[][6]={       /* frequency valid flags */
    {1,1,1,0,0,0},{1,1,0,0,0,0},{1,0,1,1,1,1},{1,1,1,0,1,0},{1,0,1,0,0,0},
    {0,0,0,0,0,0}
};
static const double ura_eph[]={         /* ura values (ref [3] 20.3.3.3.1.1) */
    2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
    3072.0,6144.0,0.0
};
/* global variables ----------------------------------------------------------*/

static char codepri[][16]={             /* priority of multiple codes in freq */
    "CPYWMNSLEABXZ",                    /* L1 */
    "CPYWMNDSLX",                       /* L2 */
    "IQX",                              /* L5/E5a */
    "IQX",                              /* E5b */
    "ABCXZSL",                          /* E6/LEX */
    "IQX",                              /* E5(a+b) */
};
/* set string without tail space ---------------------------------------------*/
static void setstr(char *dst, const char *src, int n)
{
    char *p=dst;
    const char *q=src;
    while (*q&&q<src+n) *p++=*q++;
    *p--='\0';
    while (p>=dst&&*p==' ') *p--='\0';
}
/* adjust time considering week handover -------------------------------------*/
static gtime_t adjweek(gtime_t t, gtime_t t0)
{
    double tt=timediff(t,t0);
    if (tt<-302400.0) return timeadd(t, 604800.0);
    if (tt> 302400.0) return timeadd(t,-604800.0);
    return t;
}
/* adjust time considering week handover -------------------------------------*/
static gtime_t adjday(gtime_t t, gtime_t t0)
{
    double tt=timediff(t,t0);
    if (tt<-43200.0) return timeadd(t, 86400.0);
    if (tt> 43200.0) return timeadd(t,-86400.0);
    return t;
}
/* time string for ver.3 (yyyymmdd hhmmss UTC) -------------------------------*/
static void timestr_rnx(char *str)
{
    gtime_t time;
    double ep[6];
    time=timeget();
    time.sec=0.0;
    time2epoch(time,ep);
    sprintf(str,"%04.0f%02.0f%02.0f %02.0f%02.0f%02.0f UTC",ep[0],ep[1],ep[2],
            ep[3],ep[4],ep[5]);
}
/* system code to system -----------------------------------------------------*/
static int code2sys(char code)
{
    char *p;
    if (!(p=strchr(syscodes,code))) return SYS_NONE;
    return navsys[(int)(p-syscodes)];
}
/* satellite to satellite code -----------------------------------------------*/
static int sat2code(int sat, char *code)
{
    int prn;
    switch (satsys(sat,&prn)) {
        case SYS_GPS: sprintf(code,"G%2d",prn); break;
        case SYS_GLO: sprintf(code,"R%2d",prn); break;
        case SYS_GAL: sprintf(code,"E%2d",prn); break;
        case SYS_SBS: sprintf(code,"S%2d",prn-100); break;
        case SYS_QZS: sprintf(code,"J%2d",prn); break; /* extention */
        default: return 0;
    }
    return 1;
}
/* ura index to ura value (m) ------------------------------------------------*/
static double uravalue(int sva)
{
    return 0<=sva&&sva<15?ura_eph[sva]:32767.0;
}
/* ura value (m) to ura index ------------------------------------------------*/
static int uraindex(double value)
{
    int i;
    for (i=0;i<15;i++) if (ura_eph[i]>=value) break;
    return i;
}
/* decode obs type -----------------------------------------------------------*/
static int decode_obstype(const char *tobs, int *freq)
{
    char *p,*q;
    if (!(p=strchr(obscodes,tobs[0]))||!(q=strchr(frqcodes,tobs[1]))) return -1;
    *freq=(int)(q-frqcodes);
    return (int)(p-obscodes);
}
/* initialize station parameter ----------------------------------------------*/
static void init_sta(sta_t *sta)
{
    int i;
    *sta->name   ='\0';
    *sta->marker ='\0';
    *sta->antdes ='\0';
    *sta->antsno ='\0';
    *sta->rectype='\0';
    *sta->recver ='\0';
    *sta->recsno ='\0';
    sta->antsetup=sta->itrf=sta->deltype=0;
    for (i=0;i<3;i++) sta->pos[i]=0.0;
    for (i=0;i<3;i++) sta->del[i]=0.0;
    sta->hgt=0.0;
}
/*------------------------------------------------------------------------------
* input rinex functions
*-----------------------------------------------------------------------------*/

/* set code priority -----------------------------------------------------------
* set code priority for multiple codes in a frequency
* args   : int    freq    I     frequency (0:L1,1:L2,2:L5,3:L7,4:L6,5:L8)
*          char   *pri    I     priority of codes (series of code characters)
*                               (higher priority precedes lower)
* return : none
*-----------------------------------------------------------------------------*/
extern void setrnxcodepri(int freq, const char *pri)
{
    trace(3,"setrnxcodepri:freq=%d pri=%s\n",freq,pri);
    
    if (0<=freq&&freq<NFREQ) strcpy(codepri[freq],pri);
}
/* check code priority -------------------------------------------------------*/
static int chkrnxcodepri(int freq, char code1, char code2)
{
    char *p,*q;
    
    /* no code */
    if (!code2) return 1;
    
    /* search code priority */
    if (!(p=strchr(codepri[freq],code1))) return 0;
    if (!(q=strchr(codepri[freq],code2))) return 1;
    
    return p<q; /* pri_code1>pri_code2 */
}
/* convert obs type code ver.2 -> ver.3 --------------------------------------*/
static void convcode(char *tobs)
{
    if      (!strcmp(tobs,"P1")) strcpy(tobs,"C1P"); /* ver.2.11 */
    else if (!strcmp(tobs,"P2")) strcpy(tobs,"C2P"); /* ver.2.11 */
    else if (!strcmp(tobs,"C1")) strcpy(tobs,"C1C"); /* ver.2.11 */
    else if (!strcmp(tobs,"C2")) strcpy(tobs,"C2C"); /* ver.2.11 */
    
    else if (!strcmp(tobs,"CA")) strcpy(tobs,"C1C"); /* ver.2.12 L1(C/A),GLO L1(SA) */
    else if (!strcmp(tobs,"LA")) strcpy(tobs,"L1C"); /* ver.2.12 L1(C/A),GLO L1(SA) */
    else if (!strcmp(tobs,"DA")) strcpy(tobs,"D1C"); /* ver.2.12 L1(C/A),GLO L1(SA) */
    else if (!strcmp(tobs,"SA")) strcpy(tobs,"S1C"); /* ver.2.12 L1(C/A),GLO L1(SA) */
    
    else if (!strcmp(tobs,"CB")) strcpy(tobs,"C1S"); /* ver.2.12 L1(L1C) */
    else if (!strcmp(tobs,"LB")) strcpy(tobs,"L1S"); /* ver.2.12 L1(L1C) */
    else if (!strcmp(tobs,"DB")) strcpy(tobs,"D1S"); /* ver.2.12 L1(L1C) */
    else if (!strcmp(tobs,"SB")) strcpy(tobs,"S1S"); /* ver.2.12 L1(L1C) */
    
    else if (!strcmp(tobs,"CC")) strcpy(tobs,"C2C"); /* ver.2.12 L2(L2C) */
    else if (!strcmp(tobs,"LC")) strcpy(tobs,"L2C"); /* ver.2.12 L2(L2C) */
    else if (!strcmp(tobs,"DC")) strcpy(tobs,"D2C"); /* ver.2.12 L2(L2C) */
    else if (!strcmp(tobs,"SC")) strcpy(tobs,"S2C"); /* ver.2.12 L2(L2C) */
    
    else if (!strcmp(tobs,"CD")) strcpy(tobs,"C2C"); /* ver.2.12 GLO L2(SA) */
    else if (!strcmp(tobs,"LD")) strcpy(tobs,"L2C"); /* ver.2.12 GLO L2(SA) */
    else if (!strcmp(tobs,"DD")) strcpy(tobs,"D2C"); /* ver.2.12 GLO L2(SA) */
    else if (!strcmp(tobs,"SD")) strcpy(tobs,"S2C"); /* ver.2.12 GLO L2(SA) */
}
/* decode obs header ---------------------------------------------------------*/
static void decode_obsh(FILE *fp, char *buff, int ver, int *tsys,
                        char tobs[][MAXOBSTYPE][4], nav_t *nav, sta_t *sta)
{
    double del[3];
    int i,j,k,n,nt;
    char *label=buff+60,*p;
    
    trace(3,"decode_obsh: ver=%d\n",ver);
    
    if      (strstr(label,"MARKER NAME"         )) {
        if (sta) setstr(sta->name,buff,60);
    }
    else if (strstr(label,"MARKER NUMBER"       )) { /* opt */
        if (sta) setstr(sta->marker,buff,20);
    }
    else if (strstr(label,"MARKER TYPE"         )) ; /* ver.3 */
    else if (strstr(label,"OBSERVER / AGENCY"   )) ;
    else if (strstr(label,"REC # / TYPE / VERS" )) {
        if (sta) {
            setstr(sta->recsno, buff,   20);
            setstr(sta->rectype,buff+20,20);
            setstr(sta->recver, buff+40,20);
        }
    }
    else if (strstr(label,"ANT # / TYPE"        )) {
        if (sta) {
            setstr(sta->antsno,buff   ,20);
            setstr(sta->antdes,buff+20,20);
        }
    }
    else if (strstr(label,"APPROX POSITION XYZ" )) {
        if (sta) {
            for (i=0,j=0;i<3;i++,j+=14) sta->pos[i]=str2num(buff,j,14);
        }
    }
    else if (strstr(label,"ANTENNA: DELTA H/E/N")) {
        if (sta) {
            for (i=0,j=0;i<3;i++,j+=14) del[i]=str2num(buff,j,14);
            sta->del[2]=del[0]; /* h */
            sta->del[0]=del[1]; /* e */
            sta->del[1]=del[2]; /* n */
        }
    }
    else if (strstr(label,"ANTENNA: DELTA X/Y/Z")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: PHASECENTER")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: B.SIGHT XYZ")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: ZERODIR AZI")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: ZERODIR XYZ")) ; /* opt ver.3 */
    else if (strstr(label,"CENTER OF MASS: XYZ" )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / # / OBS TYPES" )) { /* ver.3 */
        if (!(p=strchr(syscodes,buff[0]))) {
            trace(2,"invalid system code: sys=%c\n",buff[0]);
            return;
        }
        i=(int)(p-syscodes);
        n=(int)str2num(buff,3,3);
        for (j=nt=0,k=7;j<n;j++,k+=4) {
            if (k>58) {
                if (!fgets(buff,MAXRNXLEN,fp)) break;
                k=7;
            }
            if (nt<MAXOBSTYPE-1) setstr(tobs[i][nt++],buff+k,3);
        }
        *tobs[i][nt]='\0';
    }
    else if (strstr(label,"WAVELENGTH FACT L1/2")) ; /* opt ver.2 */
    else if (strstr(label,"# / TYPES OF OBSERV" )) { /* ver.2 */
        n=(int)str2num(buff,0,6);
        for (i=nt=0,j=10;i<n;i++,j+=6) {
            if (j>58) {
                if (!fgets(buff,MAXRNXLEN,fp)) break;
                j=10;
            }
            if (nt>=MAXOBSTYPE-1) continue;
            setstr(tobs[0][nt],buff+j,2);
            if (ver<=2) convcode(tobs[0][nt]);
            nt++;
        }
        *tobs[0][nt]='\0';
    }
    else if (strstr(label,"SIGNAL STRENGTH UNIT")) ; /* opt ver.3 */
    else if (strstr(label,"INTERVAL"            )) ; /* opt */
    else if (strstr(label,"TIME OF FIRST OBS"   )) {
        if      (!strncmp(buff+48,"GPS",3)) *tsys=TSYS_GPS;
        else if (!strncmp(buff+48,"GLO",3)) *tsys=TSYS_UTC;
        else if (!strncmp(buff+48,"GAL",3)) *tsys=TSYS_GAL;
        else if (!strncmp(buff+48,"QZS",3)) *tsys=TSYS_QZS; /* extension */
        else if (!strncmp(buff+48,"CMP",3)) *tsys=TSYS_CMP; /* extension */
    }
    else if (strstr(label,"TIME OF LAST OBS"    )) ; /* opt */
    else if (strstr(label,"RCV CLOCK OFFS APPL" )) ; /* opt */
    else if (strstr(label,"SYS / DCBS APPLIED"  )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / PCVS APPLIED"  )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / SCALE FACTOR"  )) ; /* opt ver.3 */
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
    else if (strstr(label,"# OF SALTELLITES"    )) ; /* opt */
    else if (strstr(label,"PRN / # OF OBS"      )) ; /* opt */
}
/* decode nav header ---------------------------------------------------------*/
static void decode_navh(char *buff, nav_t *nav)
{
    int i,j;
    char *label=buff+60;
    
    trace(3,"decode_navh:\n");
    
    if      (strstr(label,"ION ALPHA"           )) { /* opt ver.2 */
        if (nav) {
            for (i=0,j=2;i<4;i++,j+=12) nav->ion_gps[i]=str2num(buff,j,12);
        }
    }
    else if (strstr(label,"ION BETA"            )) { /* opt ver.2 */
        if (nav) {
            for (i=0,j=2;i<4;i++,j+=12) nav->ion_gps[i+4]=str2num(buff,j,12);
        }
    }
    else if (strstr(label,"DELTA-UTC: A0,A1,T,W")) { /* opt ver.2 */
        if (nav) {
            for (i=0,j=3;i<2;i++,j+=19) nav->utc_gps[i]=str2num(buff,j,19);
            for (;i<4;i++,j+=9) nav->utc_gps[i]=str2num(buff,j,9);
        }
    }
    else if (strstr(label,"IONOSPHERIC CORR"    )) { /* opt ver.3 */
        if (nav) {
            if (!strncmp(buff,"GPSA",4)) {
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_gps[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"GPSB",4)) {
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_gps[i+4]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"GAL",3)) {
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_gal[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"QZSA",4)) { /* extension */
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_qzs[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"QZSB",4)) { /* extension */
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_qzs[i+4]=str2num(buff,j,12);
            }
        }
    }
    else if (strstr(label,"TIME SYSTEM CORR"    )) { /* opt ver.3 */
        
        ; /* tbd for time system parameters in v.2.4.0 */
    }
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
}
/* decode gnav header --------------------------------------------------------*/
static void decode_gnavh(char *buff, nav_t *nav)
{
    char *label=buff+60;
    
    trace(3,"decode_gnavh:\n");
    
    if      (strstr(label,"CORR TO SYTEM TIME"  )) ; /* opt */
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
}
/* decode geo nav header -----------------------------------------------------*/
static void decode_hnavh(char *buff, nav_t *nav)
{
    char *label=buff+60;
    
    trace(3,"decode_hnavh:\n");
    
    if      (strstr(label,"CORR TO SYTEM TIME"  )) ; /* opt */
    else if (strstr(label,"D-UTC A0,A1,T,W,S,U" )) ; /* opt */
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
}
/* read rinex header ---------------------------------------------------------*/
static char readrnxh(FILE *fp, int *ver, int *sys, int *tsys,
                     char tobs[][MAXOBSTYPE][4], nav_t *nav, sta_t *sta)
{
    char buff[MAXRNXLEN],*label=buff+60,type= ' ';
    int i=0;
    
    trace(3,"readrnxh:\n");
    
    *ver=2; *sys=SYS_GPS; *tsys=TSYS_GPS;
    
    while (fgets(buff,MAXRNXLEN,fp)) {
        
        if (strlen(buff)<=60) continue;
        
        else if (strstr(label,"RINEX VERSION / TYPE")) {
            *ver=(int)str2num(buff,0,9);
            type=*(buff+20);
            
            switch (*(buff+40)) {
                case 'R': *sys=SYS_GLO;  *tsys=TSYS_UTC; break;
                case 'E': *sys=SYS_GAL;  *tsys=TSYS_GAL; break;
                case 'S': *sys=SYS_SBS;  *tsys=TSYS_GPS; break;
                case 'J': *sys=SYS_QZS;  *tsys=TSYS_QZS; break; /* extension */
                case 'C': *sys=SYS_CMP;  *tsys=TSYS_CMP; break; /* v.2.12 */
                case 'M': *sys=SYS_NONE; *tsys=TSYS_GPS; break; /* mixed */
            }
            continue;
        }
        else if (strstr(label,"PGM / RUN BY / DATE")) continue;
        else if (strstr(label,"COMMENT"            )) continue; /* opt */
        
        switch (type) {
            case 'O': decode_obsh(fp,buff,*ver,tsys,tobs,nav,sta); break;
            case 'N': decode_navh (buff,nav); break;
            case 'G': decode_gnavh(buff,nav); break;
            case 'H': decode_hnavh(buff,nav); break;
            case 'J': decode_navh (buff,nav); break; /* extension */
            case 'L': decode_navh (buff,nav); break; /* for javad rnx2rin */
        }
        if (strstr(label,"END OF HEADER")) return type;
        
        if (++i>=MAXPOSHEAD&&type==' ') break; /* no rinex file */
    }
    return ' ';
}
/* decode obs epoch ----------------------------------------------------------*/
static int decode_obsepoch(FILE *fp, char *buff, int ver, int tsys,
                           gtime_t *time, int *flag, int *sats)
{
    int i,j,n;
    char satid[8]="";
    
    trace(4,"decode_obsepoch: ver=%d tsys=%d\n",ver,tsys);
    
    if (ver<=2) { /* ver.2 */
        if ((n=(int)str2num(buff,29,3))<=0) return 0;
        
        /* epoch flag: 3:new site,4:header info,5:external event */
        *flag=(int)str2num(buff,28,1);
        
        if (3<=*flag&&*flag<=5) return n;
        
        if (str2time(buff,0,26,time)) {
            trace(2,"rinex obs invalid epoch: epoch=%26.26s\n",buff);
            return 0;
        }
        for (i=0,j=32;i<n;i++,j+=3) {
            if (j>=68) {
                if (!fgets(buff,MAXRNXLEN,fp)) break;
                j=32;
            }
            if (i<MAXOBS) {
                strncpy(satid,buff+j,3);
                sats[i]=satid2no(satid);
            }
        }
    }
    else { /* ver.3 */
        if ((n=(int)str2num(buff,32,3))<=0) return 0;
        
        *flag=(int)str2num(buff,31,1);
        
        if (3<=*flag&&*flag<=5) return n;
        
        if (buff[0]!='>'||str2time(buff,1,28,time)) {
            trace(2,"rinex obs invalid epoch: epoch=%29.29s\n",buff);
            return 0;
        }
    }
    if (tsys==TSYS_UTC) *time=utc2gpst(*time); /* utc -> gpst */
    
    trace(4,"decode_obsepoch: time=%s flag=%d\n",time_str(*time,3),*flag);
    return n;
}
/* decode obs data -----------------------------------------------------------*/
static int decode_obsdata(FILE *fp, char *buff, int ver,
                          char tobs[][MAXOBSTYPE][4], obsd_t *obs)
{
    double data;
    int i,j,k=0,type,freq;
    char satid[8]="",codes[4][NFREQ]={{0}},*p;
    unsigned char lli,code;
    
    trace(4,"decode_obsdata: ver=%d\n",ver);
    
    if (ver>=3) { /* ver.3 */
        strncpy(satid,buff,3);
        obs->sat=(unsigned char)satid2no(satid);
        if (!(p=strchr(syscodes,satid[0]))) {
            trace(2,"rinex obs invalid system: sat=%s\n",satid);
            return 0;
        }
        k=(int)(p-syscodes);
    }
    if (!obs->sat) {
        trace(4,"decode_obsdata: unsupported sat sat=%s\n",satid);
        return 0;
    }
    for (i=0;i<NFREQ;i++) {
        obs->P[i]=obs->L[i]=0.0; obs->D[i]=0.0f;
        obs->SNR[i]=obs->LLI[i]=obs->code[i]=0;
    }
    for (i=0,j=ver<=2?0:3;*tobs[k][i];i++,j+=16) {
        
        if (ver<=2&&j>=80) { /* ver.2 */
            if (!fgets(buff,MAXRNXLEN,fp)) break;
            j=0;
        }
        if ((data=str2num(buff,j,14))==0.0) continue;
        
        lli=(unsigned char)str2num(buff,j+14,1)&3;
        code=obs2code(tobs[k][i]+1);
        type=decode_obstype(tobs[k][i],&freq);
        
        /* check code priority */
        if (!chkrnxcodepri(freq,tobs[k][i][2],codes[type][freq])) continue;
        
        switch (type) {
            case 0: obs->P[freq]=data; obs->code[freq]=code; break; /* C */
            case 1: obs->L[freq]=data; obs->LLI[freq]=lli;   break; /* L */
            case 2: obs->D[freq]=(float)data;                break; /* D */
            case 3: obs->SNR[freq]=(unsigned char)data;      break; /* S */
        }
        codes[type][freq]=tobs[k][i][2];
    }
    trace(4,"decode_obsdata: time=%s sat=%2d\n",time_str(obs->time,0),obs->sat);
    return 1;
}
/* save slips ----------------------------------------------------------------*/
static void saveslips(unsigned char slips[][NFREQ], obsd_t *data)
{
    int i;
    for (i=0;i<NFREQ;i++) {
        if (data->LLI[i]&1) slips[data->sat-1][i]|=1;
    }
}
/* restore slips -------------------------------------------------------------*/
static void restslips(unsigned char slips[][NFREQ], obsd_t *data)
{
    int i;
    for (i=0;i<NFREQ;i++) {
        if (slips[data->sat-1][i]&1) data->LLI[i]|=1;
        slips[data->sat-1][i]=0;
    }
}
/* add obs data --------------------------------------------------------------*/
static int addobsdata(obs_t *obs, const obsd_t *data)
{
    if (obs->nmax<=obs->n) {
        obs->nmax+=65536;
        if (!(obs->data=(obsd_t *)realloc(obs->data,sizeof(obsd_t)*obs->nmax))) {
            trace(1,"addobs: memalloc error n=%d\n",obs->nmax);
            obs->n=obs->nmax=0;
            return 0;
        }
    }
    obs->data[obs->n++]=*data;
    return 1;
}
/* read rinex obs ------------------------------------------------------------*/
static void readrnxobs(FILE *fp, gtime_t ts, gtime_t te, double tint, int rcv,
                       int ver, int tsys, char tobs[][MAXOBSTYPE][4],
                       obs_t *obs)
{
    gtime_t time={0};
    obsd_t data={{0}};
    int i=0,n=0,flag=0,sats[MAXOBS]={0};
    unsigned char slips[MAXSAT][NFREQ]={{0}};
    char buff[MAXRNXLEN];
    
    trace(3,"readrnxobs: rcv=%d ver=%d tsys=%d\n",rcv,ver,tsys);
    
    if (!obs||rcv>MAXRCV) return;
    
    while (fgets(buff,MAXRNXLEN,fp)) {
        if (i==0) {
            /* decode obs epoch */
            if ((n=decode_obsepoch(fp,buff,ver,tsys,&time,&flag,sats))<=0) {
                continue;
            }
        }
        else if (flag<=2||flag==6) {
            data.time=time;
            data.sat=(unsigned char)sats[i-1];
            data.rcv=(unsigned char)rcv;
            
            /* decode obs data */
            if (decode_obsdata(fp,buff,ver,tobs,&data)) {
                
                saveslips(slips,&data);
                
                if (screent(time,ts,te,tint)) {
                    restslips(slips,&data);
                    
                    /* add obs data */
                    if (i<=MAXOBS&&!addobsdata(obs,&data)) break;
                }
            }
        }
        if (++i>n) i=0;
    }
    trace(4,"readrnxobs: nobs=%d\n",obs->n);
}
/* decode ephemeris ----------------------------------------------------------*/
static void decode_nav(nav_t *nav, int ver, int sat, gtime_t toc, double *data)
{
    eph_t eph={0};
    int sys=satsys(sat,NULL);
    
    trace(3,"decode_nav: ver=%d sat=%2d\n",ver,sat);
    
    if (!(sys&(SYS_GPS|SYS_GAL|SYS_QZS))) return;
    
    if (nav->nmax<=nav->n) {
        nav->nmax+=1024;
        if (!(nav->eph=(eph_t *)realloc(nav->eph,sizeof(eph_t)*nav->nmax))) {
            trace(1,"decode_eph malloc error: n=%d\n",nav->nmax);
            nav->n=nav->nmax=0;
            return;
        }
    }
    eph.sat=sat;
    eph.toc=toc;
    
    eph.f0=data[0];
    eph.f1=data[1];
    eph.f2=data[2];
    
    eph.A=SQR(data[10]); eph.e=data[ 8]; eph.i0  =data[15]; eph.OMG0=data[13];
    eph.omg =data[17]; eph.M0 =data[ 6]; eph.deln=data[ 5]; eph.OMGd=data[18];
    eph.idot=data[19]; eph.crc=data[16]; eph.crs =data[ 4]; eph.cuc =data[ 7];
    eph.cus =data[ 9]; eph.cic=data[12]; eph.cis =data[14];
    
    if (sys==SYS_GPS||sys==SYS_QZS) {
        eph.iode=(int)data[ 3];      /* IODE */
        eph.iodc=(int)data[26];      /* IODC */
        eph.toes=     data[11];      /* toe (s) in gps week */
        eph.week=(int)data[21];      /* gps week */
        eph.toe=adjweek(gpst2time(eph.week,data[11]),toc);
        eph.ttr=adjweek(gpst2time(eph.week,data[27]),toc);
        
        eph.code=(int)data[20];      /* GPS: codes on L2 ch */
        eph.svh =(int)data[24];      /* sv health */
        eph.sva=uraindex(data[23]);  /* ura (m->index) */
        eph.flag=(int)data[22];      /* GPS: L2 P data flag */
        
        eph.tgd[0]=   data[25];      /* TGD */
        eph.fit   =   data[28];      /* fit interval */
    }
    else if (sys==SYS_GAL) { /* GAL ver.3 */
        eph.iode=(int)data[ 3];      /* IODnav */
        eph.toes=     data[11];      /* toe (s) in galileo week */
        eph.week=(int)data[21];      /* galileo week */
        eph.toe=adjweek(gst2time(eph.week,data[11]),toc);
        eph.ttr=adjweek(gst2time(eph.week,data[27]),toc);
        
        eph.code=(int)data[20];      /* data sources */
                                     /* bit 0 set: I/NAV E1-B */
                                     /* bit 1 set: F/NAV E5a-I */
                                     /* bit 2 set: F/NAV E5b-I */
                                     /* bit 8 set: af0-af2 toc are for E5a.E1 */
                                     /* bit 9 set: af0-af2 toc are for E5b.E1 */
        eph.svh =(int)data[24];      /* sv health */
                                     /* bit     0: E1B DVS */
                                     /* bit   1-2: E1B HS */
                                     /* bit     3: E5a DVS */
                                     /* bit   4-5: E5a HS */
                                     /* bit     6: E5b DVS */
                                     /* bit   7-8: E5b HS */
        eph.sva =uraindex(data[23]); /* ura (m->index) */
        
        eph.tgd[0]=   data[25];      /* BGD E5a/E1 */
        eph.tgd[1]=   data[26];      /* BGD E5b/E1 */
    }
    if (eph.iode<0||255<eph.iode) {
        trace(2,"rinex nav invalid iode: iode=%d\n",eph.iode);
        return;
    }
    if (eph.iodc<0||1023<eph.iodc) {
        trace(2,"rinex nav invalid iodc: iodc=%d\n",eph.iodc);
        return;
    }
    nav->eph[nav->n++]=eph;
}
/* decode glonass ephemeris --------------------------------------------------*/
static void decode_gnav(nav_t *nav, int ver, int sat, gtime_t toc, double *data)
{
    geph_t geph={0};
    gtime_t tof;
    double tow;
    int week,day;
    
    trace(3,"decode_gnav: ver=%d sat=%2d\n",ver,sat);
    
    if (satsys(sat,NULL)!=SYS_GLO) return;
    
    if (nav->ngmax<=nav->ng) {
        nav->ngmax+=1024;
        if (!(nav->geph=(geph_t *)realloc(nav->geph,sizeof(geph_t)*nav->ngmax))) {
            trace(1,"decode_geph malloc error: n=%d\n",nav->ngmax);
            nav->ng=nav->ngmax=0;
            return;
        }
    }
    geph.sat=sat;
    
    /* toc rounded by 15 min in utc */
    tow=time2gpst(toc,&week);
    toc=gpst2time(week,floor((tow+450.0)/900.0)*900);
    day=(int)floor(tow/86400.0);
    
    /* time of frame in utc */
    tof=gpst2time(week,data[2]+day*86400.0);
    tof=adjday(tof,toc);
    
    geph.toe=utc2gpst(toc);   /* toc (gpst) */
    geph.tof=utc2gpst(tof);   /* tof (gpst) */
    
    geph.taun=-data[0];       /* -taun */
    geph.gamn= data[1];       /* +gamman */
    
    geph.pos[0]=data[3]*1E3; geph.pos[1]=data[7]*1E3; geph.pos[2]=data[11]*1E3;
    geph.vel[0]=data[4]*1E3; geph.vel[1]=data[8]*1E3; geph.vel[2]=data[12]*1E3;
    geph.acc[0]=data[5]*1E3; geph.acc[1]=data[9]*1E3; geph.acc[2]=data[13]*1E3;
    
    geph.svh=(int)data[ 6];
    geph.frq=(int)data[10];
    geph.age=(int)data[14];
    
    /* some receiver output >128 for minus frequency number */
    if (geph.frq>128) geph.frq-=256;
    
    if (geph.frq<MINFREQ_GLO||MAXFREQ_GLO<geph.frq) {
        trace(2,"rinex gnav invalid freq number: freq=%d\n",geph.frq);
        return;
    }
    nav->geph[nav->ng++]=geph;
}
/* decode geo ephemeris ------------------------------------------------------*/
static void decode_hnav(nav_t *nav, int ver, int sat, gtime_t toc, double *data)
{
    seph_t seph={0};
    int week;
    
    trace(3,"decode_hnav: ver=%d sat=%2d\n",ver,sat);
    
    if (satsys(sat,NULL)!=SYS_SBS) return;
    
    if (nav->nsmax<=nav->ns) {
        nav->nsmax+=1024;
        if (!(nav->seph=(seph_t *)realloc(nav->seph,sizeof(seph_t)*nav->nsmax))) {
            trace(1,"decode_seph malloc error: n=%d\n",nav->nsmax);
            nav->ns=nav->nsmax=0;
            return;
        }
    }
    seph.sat=sat;
    seph.t0 =toc;
    
    time2gpst(toc,&week);
    seph.tof=adjweek(gpst2time(week,data[2]),toc);
    
    seph.af0=data[0];
    seph.af1=data[1];
    
    seph.pos[0]=data[3]*1E3; seph.pos[1]=data[7]*1E3; seph.pos[2]=data[11]*1E3;
    seph.vel[0]=data[4]*1E3; seph.vel[1]=data[8]*1E3; seph.vel[2]=data[12]*1E3;
    seph.acc[0]=data[5]*1E3; seph.acc[1]=data[9]*1E3; seph.acc[2]=data[13]*1E3;
    
    seph.svh=(int)data[6];
    seph.sva=uraindex(data[10]);
    
    nav->seph[nav->ns++]=seph;
}
/* read rinex nav/gnav/geo nav -----------------------------------------------*/
static void readrnxnav(FILE *fp, int ver, int sys, nav_t *nav)
{
    gtime_t toc;
    double data[64];
    int i=0,j,prn,sat=0,c=ver<=2?3:4;
    char buff[MAXRNXLEN];
    
    trace(3,"readrnxnav: ver=%d sys=%d\n",ver,sys);
    
    if (!nav) return;
    
    while (fgets(buff,MAXRNXLEN,fp)) {
        if (i==0) {
            if (str2time(buff,c,19,&toc)) {
                trace(2,"rinex nav toc error: %23.23s\n",buff);
                continue;
            }
            if (ver<=2) { /* ver.2 */
                prn=(int)str2num(buff,0,2);
            }
            else { /* ver.3 */
                sys=code2sys(buff[0]);
                prn=(int)str2num(buff,1,2);
            }
            sat=satno(sys,prn+(sys==SYS_SBS?100:0));
        }
        /* decode data fields */
        for (j=i==0?c+19:c;j<=c+19*3;j+=19) {
            data[i++]=str2num(buff,j,19);
        }
        /* decode ephemeris */
        if (i>=31) {
            decode_nav (nav,ver,sat,toc,data); i=0;
        }
        else if (i>=15&&sys==SYS_GLO) {
            decode_gnav(nav,ver,sat,toc,data); i=0;
        }
        else if (i>=15&&sys==SYS_SBS) {
            decode_hnav(nav,ver,sat,toc,data); i=0;
        }
    }
}
/* read rinex clock ----------------------------------------------------------*/
static void readrnxclk(FILE *fp, int index, nav_t *nav)
{
    gtime_t time;
    double data[2];
    int i,j,sat;
    char buff[MAXRNXLEN],satid[8]="";
    
    trace(3,"readrnxclk: index=%d\n", index);
    
    if (!nav) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (str2time(buff,8,26,&time)) {
            trace(2,"rinex clk invalid epoch: %34.34s\n",buff);
            continue;
        }
        strncpy(satid,buff+3,4);
        
        /* only read AS (satellite clock) record */
        if (strncmp(buff,"AS",2)||!(sat=satid2no(satid))) continue;
        
        for (i=0,j=40;i<2;i++,j+=20) data[i]=str2num(buff,j,19);
        
        if (nav->nc>=nav->ncmax) {
            nav->ncmax+=1024;
            if (!(nav->pclk=(pclk_t *)realloc(nav->pclk,sizeof(pclk_t)*(nav->ncmax)))) {
                trace(1,"readrnxclk malloc error: nmax=%d\n",nav->ncmax);
                nav->nc=nav->ncmax=0;
                return;
            }
        }
        if (nav->nc<=0||fabs(timediff(time,nav->pclk[nav->nc-1].time))>1E-9) {
            nav->nc++;
            nav->pclk[nav->nc-1].time =time;
            nav->pclk[nav->nc-1].index=index;
            for (i=0;i<MAXSAT;i++) {
                nav->pclk[nav->nc-1].clk[i][0]=0.0;
                nav->pclk[nav->nc-1].std[i][0]=0.0f;
            }
        }
        nav->pclk[nav->nc-1].clk[sat-1][0]=data[0];
        nav->pclk[nav->nc-1].std[sat-1][0]=(float)data[1];
    }
}
/* read rinex file -----------------------------------------------------------*/
static char readrnxfp(FILE *fp, gtime_t ts, gtime_t te, double tint, int flag,
                      int index, obs_t *obs, nav_t *nav, sta_t *sta)
{
    int ver,sys,tsys;
    char type,tobs[NUMSYS][MAXOBSTYPE][4]={{""}};
    
    trace(3,"readrnxfp: flag=%d index=%d\n",flag,index);
    
    /* read rinex header */
    if ((type=readrnxh(fp,&ver,&sys,&tsys,tobs,nav,sta))==' ') return ' ';
    
    /* flag=0:except for clock,1:clock */
    if ((!flag&&type=='C')||(flag&&type!='C')) return ' ';
    
    /* read rinex body */
    switch (type) {
        case 'O': readrnxobs(fp,ts,te,tint,index,ver,tsys,tobs,obs); break;
        case 'N': readrnxnav(fp,ver,ver<=2?SYS_GPS:sys,nav); break;
        case 'G': readrnxnav(fp,ver,SYS_GLO,nav); break;
        case 'H': readrnxnav(fp,ver,SYS_SBS,nav); break;
        case 'J': readrnxnav(fp,ver,SYS_QZS,nav); break; /* extenstion */
        case 'L': readrnxnav(fp,3,  SYS_GAL,nav); break; /* for javad jps2rin */
        case 'C': readrnxclk(fp,index,nav);       break;
        default : trace(2,"unsupported rinex type ver=%d type=%c\n",ver,type);
    }
    return type;
}
/* uncompress and read rinex file --------------------------------------------*/
static char readrnxfile(const char *file, gtime_t ts, gtime_t te, double tint,
                        int flag, int index, obs_t *obs, nav_t *nav, sta_t *sta)
{
    FILE *fp;
    int cstat;
    char type,tmpfile[1024];
    
    trace(3,"readrnxfile: file=%s flag=%d index=%d\n",file,flag,index);
    
    if (sta) init_sta(sta);
    
    /* uncompress file */
    if ((cstat=uncompress(file,tmpfile))<0) {
        trace(2,"rinex file uncompact error: %s\n",file);
        return ' ';
    }
    if (!(fp=fopen(cstat?tmpfile:file,"r"))) {
        trace(2,"rinex file open error: %s\n",cstat?tmpfile:file);
        return ' ';
    }
    /* read rinex file */
    type=readrnxfp(fp,ts,te,tint,flag,index,obs,nav,sta);
    
    fclose(fp);
    
    /* delete temporary file */
    if (cstat) remove(tmpfile);
    
    return type;
}
/* compare ephemeris ---------------------------------------------------------*/
static int cmpeph(const void *p1, const void *p2)
{
    eph_t *q1=(eph_t *)p1,*q2=(eph_t *)p2;
    return q1->ttr.time!=q2->ttr.time?(int)(q1->ttr.time-q2->ttr.time):
           (q1->toe.time!=q2->toe.time?(int)(q1->toe.time-q2->toe.time):
            q1->sat-q2->sat);
}
/* sort and unique ephemeris -------------------------------------------------*/
static void uniqeph(nav_t *nav)
{
    int i,j;
    
    trace(3,"uniqeph: n=%d\n",nav->n);
    
    if (nav->n<=0) return;
    
    qsort(nav->eph,nav->n,sizeof(eph_t),cmpeph);
    
    for (i=j=0;i<nav->n;i++) {
        if (nav->eph[i].sat!=nav->eph[j].sat||
            nav->eph[i].iode!=nav->eph[j].iode) {
            nav->eph[++j]=nav->eph[i];
        }
    }
    nav->n=j+1;
    
    if (!(nav->eph=(eph_t *)realloc(nav->eph,sizeof(eph_t)*nav->n))) {
        nav->n=nav->nmax=0;
        trace(1,"uniqeph malloc error n=%d\n",nav->n);
        return;
    }
    nav->nmax=nav->n;
    
    trace(4,"uniqeph: n=%d\n",nav->n);
}
/* compare glonass ephemeris -------------------------------------------------*/
static int cmpgeph(const void *p1, const void *p2)
{
    geph_t *q1=(geph_t *)p1,*q2=(geph_t *)p2;
    return q1->tof.time!=q2->tof.time?(int)(q1->tof.time-q2->tof.time):
           (q1->toe.time!=q2->toe.time?(int)(q1->toe.time-q2->toe.time):
            q1->sat-q2->sat);
}
/* sort and unique glonass ephemeris -----------------------------------------*/
static void uniqgeph(nav_t *nav)
{
    int i,j;
    
    trace(3,"uniqgeph: ng=%d\n",nav->ng);
    
    if (nav->ng<=0) return;
    
    qsort(nav->geph,nav->ng,sizeof(geph_t),cmpgeph);
    
    for (i=j=0;i<nav->ng;i++) {
        if (nav->geph[i].sat!=nav->geph[j].sat||
            nav->geph[i].toe.time!=nav->geph[j].toe.time||
            nav->geph[i].svh!=nav->geph[j].svh) {
            nav->geph[++j]=nav->geph[i];
        }
    }
    nav->ng=j+1;
    
    if (!(nav->geph=(geph_t *)realloc(nav->geph,sizeof(geph_t)*nav->ng))) {
        nav->ng=nav->ngmax=0;
        trace(1,"uniqgeph malloc error ng=%d\n",nav->ng);
        return;
    }
    nav->ngmax=nav->ng;
    
    trace(4,"uniqgeph: ng=%d\n",nav->ng);
}
/* compare sbas ephemeris ----------------------------------------------------*/
static int cmpseph(const void *p1, const void *p2)
{
    seph_t *q1=(seph_t *)p1,*q2=(seph_t *)p2;
    return q1->tof.time!=q2->tof.time?(int)(q1->tof.time-q2->tof.time):
           (q1->t0.time!=q2->t0.time?(int)(q1->t0.time-q2->t0.time):
            q1->sat-q2->sat);
}
/* sort and unique sbas ephemeris --------------------------------------------*/
static void uniqseph(nav_t *nav)
{
    int i,j;
    
    trace(3,"uniqseph: ns=%d\n",nav->ns);
    
    if (nav->ns<=0) return;
    
    qsort(nav->seph,nav->ns,sizeof(seph_t),cmpseph);
    
    for (i=j=0;i<nav->ns;i++) {
        if (nav->seph[i].sat!=nav->seph[j].sat||
            nav->seph[i].t0.time!=nav->seph[j].t0.time) {
            nav->seph[++j]=nav->seph[i];
        }
    }
    nav->ns=j+1;
    
    if (!(nav->seph=(seph_t *)realloc(nav->seph,sizeof(seph_t)*nav->ns))) {
        nav->ns=nav->nsmax=0;
        trace(1,"uniqseph malloc error ns=%d\n",nav->ns);
        return;
    }
    nav->nsmax=nav->ns;
    
    trace(4,"uniqseph: ns=%d\n",nav->ns);
}
/* read rinex obs and nav files ------------------------------------------------
* read rinex obs and nav files
* args   : char *file    I      file (wild-card * expanded) ("": stdin)
*          int   rcv     I      receiver number for obs data
*         (gtime_t ts)   I      observation time start (ts.time==0: no limit)
*         (gtime_t te)   I      observation time end   (te.time==0: no limit)
*         (double tint)  I      observation time interval (s) (0:all)
*          obs_t *obs    IO     observation data   (NULL: no input)
*          nav_t *nav    IO     navigation data    (NULL: no input)
*          sta_t *sta    IO     station parameters (NULL: no input)
* return : number of observation data epochs
* notes  : read data are appended to obs and nav struct
*          before calling the function, obs and nav should be initialized.
*-----------------------------------------------------------------------------*/
extern int readrnxt(const char *file, int rcv, gtime_t ts, gtime_t te,
                    double tint, obs_t *obs, nav_t *nav, sta_t *sta)
{
    int i,j,n,nep=0;
    const char *p;
    char type=' ',*files[MAXEXFILE]={0};
    
    trace(3,"readrnxt: file=%s rcv=%d\n",file,rcv);
    
    if (*file) {
        for (i=0;i<MAXEXFILE;i++) {
            if (!(files[i]=(char *)malloc(1024))) {
                for (i--;i>=0;i--) free(files[i]); return -1;
            }
        }
        /* expand wild-card */
        n=expath(file,files,MAXEXFILE);
        
        /* read rinex files */
        for (i=0;i<n;i++) {
            type=readrnxfile(files[i],ts,te,tint,0,rcv,obs,nav,sta);
        }
        /* if station name empty, set 4-char name from file head */
        if (type=='O'&&sta) {
            if (!(p=strrchr(file,FILEPATHSEP))) p=file-1;
            setstr(sta->name,p+1,4);
        }
        for (i=0;i<MAXEXFILE;i++) free(files[i]);
    }
    else {
        readrnxfp(stdin,ts,te,tint,0,1,obs,nav,sta);
    }
    if (obs&&obs->n>0) {
        /* sort obs data */
        nep=sortobs(obs);
    }
    if (nav) {
        /* unique ephemeris */
        uniqeph (nav);
        uniqgeph(nav);
        uniqseph(nav);
        
        /* update carrier wave length */
        for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ;j++) {
            nav->lam[i][j]=satwavelen(i+1,j,nav);
        }
    }
    return nep;
}
extern int readrnx(const char *file, int rcv, obs_t *obs, nav_t *nav, sta_t *sta)
{
    gtime_t t={0};
    
    trace(3,"readrnx : file=%s rcv=%d\n",file,rcv);
    
    return readrnxt(file,rcv,t,t,0.0,obs,nav,sta);
}
/* compare precise clock -----------------------------------------------------*/
static int cmppclk(const void *p1, const void *p2)
{
    pclk_t *q1=(pclk_t *)p1,*q2=(pclk_t *)p2;
    double tt=timediff(q1->time,q2->time);
    return tt<-1E-9?-1:(tt>1E-9?1:q1->index-q2->index);
}
/* combine precise clock -----------------------------------------------------*/
static void combpclk(nav_t *nav)
{
    int i,j,k;
    
    trace(3,"combpclk: nc=%d\n",nav->nc);
    
    if (nav->nc<=0) return;
    
    qsort(nav->pclk,nav->nc,sizeof(pclk_t),cmppclk);
    
    for (i=0,j=1;j<nav->nc;j++) {
        if (fabs(timediff(nav->pclk[i].time,nav->pclk[j].time))<1E-9) {
            for (k=0;k<MAXSAT;k++) {
                if (nav->pclk[j].clk[k][0]==0.0) continue;
                nav->pclk[i].clk[k][0]=nav->pclk[j].clk[k][0];
                nav->pclk[i].std[k][0]=nav->pclk[j].std[k][0];
            }
        }
        else if (++i<j) nav->pclk[i]=nav->pclk[j];
    }
    nav->nc=i+1;
    
    if (!(nav->pclk=(pclk_t *)realloc(nav->pclk,sizeof(pclk_t)*nav->nc))) {
        nav->nc=nav->ncmax=0;
        trace(1,"combpclk malloc error nc=%d\n",nav->nc);
        return;
    }
    nav->ncmax=nav->nc;
    
    trace(4,"combpclk: nc=%d\n",nav->nc);
}
/* read rinex clock files ------------------------------------------------------
* read rinex clock files
* args   : char *file    I      file (wild-card * expanded)
*          nav_t *nav    IO     navigation data    (NULL: no input)
* return : number of precise clock
*-----------------------------------------------------------------------------*/
extern int readrnxc(const char *file, nav_t *nav)
{
    gtime_t t={0};
    int i,n,index=0;
    char *files[MAXEXFILE]={0};
    
    trace(3,"readrnxc: file=%s\n",file);
    
    for (i=0;i<MAXEXFILE;i++) {
        if (!(files[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(files[i]); return 0;
        }
    }
    /* expand wild-card */
    n=expath(file,files,MAXEXFILE);
    
    /* read rinex clock files */
    for (i=0;i<n;i++) {
        readrnxfile(files[i],t,t,0.0,1,index++,NULL,nav,NULL);
    }
    for (i=0;i<MAXEXFILE;i++) free(files[i]);
    
    /* unique and combine ephemeris and precise clock */
    combpclk(nav);
    
    return nav->nc;
}
/*------------------------------------------------------------------------------
* output rinex functions
*-----------------------------------------------------------------------------*/

/* output obs types ver.2 ----------------------------------------------------*/
static void outobstype_ver2(FILE *fp, const rnxopt_t *opt)
{
    const char label[]="# / TYPES OF OBSERV";
    int i,j,n=0;
    char obstype[MAXOBSTYPE][3]={{""}};
    
    trace(3,"outobstype_ver2:\n");
    
    for (i=0;i<NFREQ;i++) for (j=0;j<4;j++) {
        if (!(opt->freqtype&(1<<i))||!(opt->obstype&(1<<j))) continue;
        
        /* obs type code */
        sprintf(obstype[n],"%c%c",obscodes[j],frqcodes[i]);
        
        /* only support P2 for ver.2 */
        if (!strcmp(obstype[n],"C2")) strcpy(obstype[n],"P2");
        
        if (++n>=MAXOBSTYPE) break;
    }
    fprintf(fp,"%6d",n);
    
    for (i=0;i<n;i++) {
        if (i>0&&i%9==0) fprintf(fp,"      ");
        
        fprintf(fp,"%6s",obstype[i]);
        
        if (i%9==8) fprintf(fp,"%-20s\n",label);
    }
    if (n==0||i%9>0) {
        fprintf(fp,"%*s%-20s\n",(9-i%9)*6,"",label);
    }
}
/* output obs types ver.3 ----------------------------------------------------*/
static void outobstype_ver3(FILE *fp, const rnxopt_t *opt)
{
    const char label[]="SYS / # / OBS TYPES";
    int i,j,k,n;
    char obstype[MAXOBSTYPE][4]={{""}};
    
    trace(3,"outobstype_ver3:\n");
    
    for (i=0;navsys[i];i++) {
        if (!(navsys[i]&opt->navsys)) continue;
        
        for (j=n=0;j<NFREQ;j++) for (k=0;k<4;k++) {
            if (!(opt->freqtype&(1<<j))||!(opt->obstype&(1<<k))) continue;
            
            if (!frqvalid[i][j]) continue;
            
            /* obs type code */
            sprintf(obstype[n],"%c%c%c",obscodes[k],frqcodes[j],' ');
            
            if (++n>=MAXOBSTYPE) break;
        }
        fprintf(fp,"%c  %3d",syscodes[i],n);
        
        for (j=0;j<n;j++) {
            if (j>0&&j%13==0) fprintf(fp,"      ");
            
            fprintf(fp," %3s",obstype[j]);
            
            if (j%13==12) fprintf(fp,"  %-20s\n",label);
        }
        if (n==0||j%13>0) {
            fprintf(fp,"%*s  %-20s\n",(13-j%13)*4,"",label);
        }
    }
}
/* output rinex obs header -----------------------------------------------------
* output rinex obd file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxobsh(FILE *fp, const rnxopt_t *opt)
{
    double ep[6],pos[3]={0},del[3]={0},ver;
    int i;
    char date[32],*sys,*tsys="GPS";
    
    trace(3,"outrnxobsh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2) { /* ver.2 */
        ver=RNX2VER;
        sys=opt->navsys==SYS_GPS?"G (GPS)":"M (MIXED)";
    }
    else { /* ver.3 */
        ver=RNX3VER;
        if      (opt->navsys==SYS_GPS) sys="G: GPS";
        else if (opt->navsys==SYS_GLO) sys="R: GLONASS";
        else if (opt->navsys==SYS_GAL) sys="E: Galielo";
        else if (opt->navsys==SYS_QZS) sys="J: QZSS";    /* extension */
        else if (opt->navsys==SYS_SBS) sys="S: SBAS Payload";
        else sys="M: Mixed";
    }
    fprintf(fp,"%9.2f%-11s%-20s%-20s%-20s\n",ver,"","OBSERVATION DATA",sys,
            "RINEX VERSION / TYPE");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    fprintf(fp,"%-60.60s%-20s\n",opt->marker,"MARKER NAME");
    fprintf(fp,"%-20.20s%-40.40s%-20s\n",opt->markerno,"","MARKER NUMBER");
    
    if (opt->rnxver>=3) {
        fprintf(fp,"%-20.20s%-40.40s%-20s\n",opt->markertype,"","MARKER TYPE");
    }
    fprintf(fp,"%-20.20s%-40.40s%-20s\n",opt->name[0],opt->name[1],
            "OBSERVER / AGENCY");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->rec[0],opt->rec[1],
            opt->rec[2],"REC # / TYPE / VERS");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->ant[0],opt->ant[1],
            opt->ant[2],"ANT # / TYPE");
    
    for (i=0;i<3;i++) if (fabs(opt->apppos[i])<1E8) pos[i]=opt->apppos[i];
    for (i=0;i<3;i++) if (fabs(opt->antdel[i])<1E8) del[i]=opt->antdel[i];
    fprintf(fp,"%14.4f%14.4f%14.4f%-18s%-20s\n",pos[0],pos[1],pos[2],"",
            "APPROX POSITION XYZ");
    fprintf(fp,"%14.4f%14.4f%14.4f%-18s%-20s\n",del[0],del[1],del[2],"",
            "ANTENNA: DELTA H/E/N");
    
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"%6d%6d%-48s%-20s\n",1,1,"","WAVELENGTH FACT L1/2");
        outobstype_ver2(fp,opt);
    }
    else { /* ver.3 */
        outobstype_ver3(fp,opt);
    }
    time2epoch(opt->tstart,ep);
    fprintf(fp,"  %04.0f%6.0f%6.0f%6.0f%6.0f%13.7f     %-12s%-20s\n",ep[0],
            ep[1],ep[2],ep[3],ep[4],ep[5],tsys,"TIME OF FIRST OBS");
    
    time2epoch(opt->tend,ep);
    fprintf(fp,"  %04.0f%6.0f%6.0f%6.0f%6.0f%13.7f     %-12s%-20s\n",ep[0],
            ep[1],ep[2],ep[3],ep[4],ep[5],tsys,"TIME OF LAST OBS");
    
    return fprintf(fp,"%-60.60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output obs data field -----------------------------------------------------*/
static void outrnxobsf(FILE *fp, double obs, int lli)
{
    if (obs==0.0||obs<=-1E9||obs>=1E9) fprintf(fp,"              ");
    else fprintf(fp,"%14.3f",obs);
    if (lli<=0) fprintf(fp,"  "); else fprintf(fp,"%1.1d ",lli);
}
/* output rinex obs body -------------------------------------------------------
* output rinex obs body
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          obsd_t *obs      I   observation data
*          int    n         I   number of observation data
*          int    flag      I   epoch flag (0:ok,1:power failure,>1:event flag)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxobsb(FILE *fp, const rnxopt_t *opt, const obsd_t *obs, int n,
                      int flag)
{
    double ep[6];
    int i,j,k,nt,ns,ind[MAXOBS];
    char sats[MAXOBS][4]={""};
    
    trace(3,"outrnxobsb: n=%d\n",n);
    
    time2epoch(obs[0].time,ep);
    
    for (i=ns=0;i<n&&ns<MAXOBS;i++) {
        if (!(satsys(obs[i].sat,NULL)&opt->navsys)) continue;
        if (opt->exsats[obs[i].sat-1]) continue;
        if (!sat2code(obs[i].sat,sats[ns])) continue;
        ind[ns++]=i;
    }
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp," %02d %2.0f %2.0f %2.0f %2.0f%11.7f  %d%3d",
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5],flag,ns);
        for (i=0;i<ns;i++) {
            if (i>0&&i%12==0) fprintf(fp,"\n%32s","");
            fprintf(fp,"%-3s",sats[i]);
        }
    }
    else { /* ver.3 */
        fprintf(fp,"> %04.0f %2.0f %2.0f %2.0f %2.0f%11.7f  %d%3d%21s\n",
                ep[0],ep[1],ep[2],ep[3],ep[4],ep[5],flag,ns,"");
    }
    for (i=0;i<ns;i++) {
        
        if (opt->rnxver>=3) fprintf(fp,"%-3s",sats[i]);
        
        for (j=nt=0;j<NFREQ;j++) for (k=0;k<4;k++) {
            if (!(opt->freqtype&(1<<j))||!(opt->obstype&(1<<k))) continue;
            
            if (opt->rnxver<=2) { /* ver.2 */
                if (nt%5==0) fprintf(fp,"\n");
            }
            else { /* ver.3 */
                if ((*sats[i]==syscodes[0]&&!frqvalid[0][j])||
                    (*sats[i]==syscodes[1]&&!frqvalid[1][j])||
                    (*sats[i]==syscodes[2]&&!frqvalid[2][j])||
                    (*sats[i]==syscodes[3]&&!frqvalid[3][j])||
                    (*sats[i]==syscodes[4]&&!frqvalid[4][j])) continue;
            }
            if      (k==0) outrnxobsf(fp,obs[ind[i]].P[j],-1);
            else if (k==1) outrnxobsf(fp,obs[ind[i]].L[j],obs[ind[i]].LLI[j]);
            else if (k==2) outrnxobsf(fp,obs[ind[i]].D[j],-1);
            else if (k==3) outrnxobsf(fp,obs[ind[i]].SNR[j],-1);
            nt++;
        }
        if (opt->rnxver>=3&&fprintf(fp,"\n")==EOF) return 0;
    }
    if (opt->rnxver>=3) return 1;
    
    return fprintf(fp,"\n")!=EOF;
}
/* output nav member by rinex nav format -------------------------------------*/
static void outnavf(FILE *fp, double value)
{
    double e=fabs(value)<1E-99?0.0:floor(log10(fabs(value))+1.0);
    fprintf(fp," %s.%012.0fE%+03.0f",value<0.0?"-":" ",fabs(value)/pow(10.0,e-12.0),e);
}
/* output rinex nav header -----------------------------------------------------
* output rinex nav file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64],*sys;
    
    trace(3,"outrnxnavh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",RNX2VER,"N: GPS NAV DATA",
                "","RINEX VERSION / TYPE");
    }
    else { /* ver.3 */
        if      (opt->navsys==SYS_GPS) sys="G: GPS";
        else if (opt->navsys==SYS_GLO) sys="R: GLONASS";
        else if (opt->navsys==SYS_GAL) sys="E: Galielo";
        else if (opt->navsys==SYS_QZS) sys="J: QZSS";    /* extension */
        else if (opt->navsys==SYS_SBS) sys="S: SBAS Payload";
        else sys="M: Mixed";
        
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",RNX3VER,"N: GNSS NAV DATA",
                sys,"RINEX VERSION / TYPE");
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",nav->ion_gps[0],
                nav->ion_gps[1],nav->ion_gps[2],nav->ion_gps[3],"","ION ALPHA");
        fprintf(fp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",nav->ion_gps[4],
                nav->ion_gps[5],nav->ion_gps[6],nav->ion_gps[7],"","ION BETA");
        fprintf(fp,"   ");
        outnavf(fp,nav->utc_gps[0]);
        outnavf(fp,nav->utc_gps[1]);
        fprintf(fp,"%9.0f%9.0f %-20s\n",nav->utc_gps[2],nav->utc_gps[3],
                "DELTA-UTC: A0,A1,T,W");
    }
    else { /* ver.3 */
        if (opt->navsys&SYS_GPS) {
            fprintf(fp,"GPSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",nav->ion_gps[0],
                    nav->ion_gps[1],nav->ion_gps[2],nav->ion_gps[3],"",
                    "IONOSPHERIC CORR");
            fprintf(fp,"GPSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",nav->ion_gps[4],
                    nav->ion_gps[5],nav->ion_gps[6],nav->ion_gps[7],"",
                    "IONOSPHERIC CORR");
        }
        if (opt->navsys&SYS_GAL) {
            fprintf(fp,"GAL  %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",nav->ion_gal[0],
                    nav->ion_gal[1],nav->ion_gal[2],0.0,"","IONOSPHERIC CORR");
        }
        if (opt->navsys&SYS_GPS) {
            fprintf(fp,"GPUT %17.10E%16.9E%7.0f%5.0f %-5s %2d %-20s\n",
                    nav->utc_gps[0],nav->utc_gps[1],nav->utc_gps[2],
                    nav->utc_gps[3],"",0,"TIME SYSTEM CORR");
        }
    }
    fprintf(fp,"%6d%54s%-20s\n",nav->leaps,"","LEAP SECONDS");
    
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex nav body -------------------------------------------------------
* output rinex nav file body record
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          eph_t  *eph      I   ephemeris
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxnavb(FILE *fp, const rnxopt_t *opt, const eph_t *eph)
{
    double ep[6],ttr;
    int week,sys,prn;
    char code[32],*sep;
    
    trace(3,"outrnxgnavb: sat=%2d\n",eph->sat);
    
    if (!(sys=satsys(eph->sat,&prn))) return 0;
    
    time2epoch(eph->toc,ep);
    
    if (opt->rnxver<=2) { /* ver.2 */
        if ((sys&opt->navsys)!=SYS_GPS) return 0;
        fprintf(fp,"%2d %02d %2.0f %2.0f %2.0f %2.0f %4.1f",prn,
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="   ";
    }
    else { /* ver.3 */
        if (!sat2code(eph->sat,code)) return 0;
        fprintf(fp,"%-3s %04.0f %2.0f %2.0f %2.0f %2.0f %2.0f",code,ep[0],ep[1],
                ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    outnavf(fp,eph->f0     );
    outnavf(fp,eph->f1     );
    outnavf(fp,eph->f2     );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->iode   );
    outnavf(fp,eph->crs    );
    outnavf(fp,eph->deln   );
    outnavf(fp,eph->M0     );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->cuc    );
    outnavf(fp,eph->e      );
    outnavf(fp,eph->cus    );
    outnavf(fp,sqrt(eph->A));
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->toes   );
    outnavf(fp,eph->cic    );
    outnavf(fp,eph->OMG0   );
    outnavf(fp,eph->cis    );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->i0     );
    outnavf(fp,eph->crc    );
    outnavf(fp,eph->omg    );
    outnavf(fp,eph->OMGd   );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->idot   );
    outnavf(fp,eph->code   );
    outnavf(fp,eph->week   );    /* GPS:gps week,GAL:galileo week */
    outnavf(fp,eph->flag   );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,uravalue(eph->sva));
    outnavf(fp,eph->svh    );
    outnavf(fp,eph->tgd[0] );    /* GPS:TGD, GAL:BGD E5a/E1 */
    if (sys==SYS_GAL) {
        outnavf(fp,eph->tgd[1]); /* GPS:IODC */
    }
    else {
        outnavf(fp,eph->iodc);   /* GAL:BGD E5b/E1 */
    }
    fprintf(fp,"\n%s",sep  );
    
    ttr=sys==SYS_GAL?time2gst(eph->ttr,&week):time2gpst(eph->ttr,&week);
    outnavf(fp,ttr+(week-eph->week)*604800.0);
    outnavf(fp,eph->fit    ); /* GPS */
    
    return fprintf(fp,"\n")!=EOF;
}
/* output rinex gnav header ----------------------------------------------------
* output rinex gnav (glonass navigation) file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxgnavh(FILE *fp, const rnxopt_t *opt)
{
    int i;
    char date[64];
    
    trace(3,"outrnxgnavh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",RNX2VER,"GLONASS NAV DATA",
                "","RINEX VERSION / TYPE");
    }
    else { /* ver.3 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",RNX3VER,"N: GNSS NAV DATA",
                "R: GLONASS","RINEX VERSION / TYPE");
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex gnav body ------------------------------------------------------
* output rinex gnav (glonass navigation) file body record
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          geph_t  *geph    I   glonass ephemeris
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxgnavb(FILE *fp, const rnxopt_t *opt, const geph_t *geph)
{
    gtime_t toe;
    double ep[6],tof;
    int prn;
    char code[32],*sep;
    
    trace(3,"outrnxgnavb: sat=%2d\n",geph->sat);
    
    if ((satsys(geph->sat,&prn)&opt->navsys)!=SYS_GLO) return 0;
    
    tof=fmod(time2gpst(gpst2utc(geph->tof),NULL),86400.0);
    toe=gpst2utc(geph->toe); /* gpst -> utc */
    time2epoch(toe,ep);
    
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"%2d %02d %2.0f %2.0f %2.0f %2.0f %4.1f",prn,(int)ep[0]%100,
                ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="   ";
    }
    else { /* ver.3 */
        if (!sat2code(geph->sat,code)) return 0;
        fprintf(fp,"%-3s %04.0f %2.0f %2.0f %2.0f %2.0f %2.0f",code,ep[0],ep[1],
                ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    outnavf(fp,-geph->taun     );
    outnavf(fp,geph->gamn      );
    outnavf(fp,tof             );
    fprintf(fp,"\n%s",sep      );
    
    outnavf(fp,geph->pos[0]/1E3);
    outnavf(fp,geph->vel[0]/1E3);
    outnavf(fp,geph->acc[0]/1E3);
    outnavf(fp,geph->svh       );
    fprintf(fp,"\n%s",sep      );
    
    outnavf(fp,geph->pos[1]/1E3);
    outnavf(fp,geph->vel[1]/1E3);
    outnavf(fp,geph->acc[1]/1E3);
    outnavf(fp,geph->frq       );
    fprintf(fp,"\n%s",sep      );
    
    outnavf(fp,geph->pos[2]/1E3);
    outnavf(fp,geph->vel[2]/1E3);
    outnavf(fp,geph->acc[2]/1E3);
    outnavf(fp,geph->age       );
    
    return fprintf(fp,"\n")!=EOF;
}
/* output rinex geo nav header -------------------------------------------------
* output rinex geo nav file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxhnavh(FILE *fp, const rnxopt_t *opt)
{
    int i;
    char date[64];
    
    trace(3,"outrnxhnavh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",RNX2VER,
                "H: GEO NAV MSG DATA","","RINEX VERSION / TYPE");
    }
    else { /* ver.3 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",RNX3VER,
                "N: GNSS NAV DATA","S: SBAS Payload","RINEX VERSION / TYPE");
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex geo nav body ---------------------------------------------------
* output rinex geo nav file body record
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          seph_t  *seph    I   sbas ephemeris
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxhnavb(FILE *fp, const rnxopt_t *opt, const seph_t *seph)
{
    double ep[6];
    int prn;
    char code[32],*sep;
    
    trace(3,"outrnxhnavb: sat=%2d\n",seph->sat);
    
    if ((satsys(seph->sat,&prn)&opt->navsys)!=SYS_SBS) return 0;
    
    time2epoch(seph->t0,ep);
    
    if (opt->rnxver<=2) { /* ver.2 */
        fprintf(fp,"%2d %02d %2.0f %2.0f %2.0f %2.0f %4.1f",prn-100,
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="   ";
    }
    else { /* ver.3 */
        if (!sat2code(seph->sat,code)) return 0;
        fprintf(fp,"%-3s %04.0f %2.0f %2.0f %2.0f %2.0f %2.0f",code,ep[0],ep[1],
                ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    outnavf(fp,seph->af0          );
    outnavf(fp,seph->af1          );
    outnavf(fp,time2gpst(seph->tof,NULL));
    fprintf(fp,"\n%s",sep         );
    
    outnavf(fp,seph->pos[0]/1E3   );
    outnavf(fp,seph->vel[0]/1E3   );
    outnavf(fp,seph->acc[0]/1E3   );
    outnavf(fp,seph->svh          );
    fprintf(fp,"\n%s",sep         );
    
    outnavf(fp,seph->pos[1]/1E3   );
    outnavf(fp,seph->vel[1]/1E3   );
    outnavf(fp,seph->acc[1]/1E3   );
    outnavf(fp,uravalue(seph->sva));
    fprintf(fp,"\n%s",sep         );
    
    outnavf(fp,seph->pos[2]/1E3   );
    outnavf(fp,seph->vel[2]/1E3   );
    outnavf(fp,seph->acc[2]/1E3   );
    outnavf(fp,0                  );
    
    return fprintf(fp,"\n")!=EOF;
}
