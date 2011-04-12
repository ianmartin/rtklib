/*------------------------------------------------------------------------------
* ionex.c : input ionex ionospheric tec model
*
*          Copyright (C) 2009 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2009/09/22 1.0 new
*
*                     This code is not supported yet.
*
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define NMAX        10      /* order of polynomial interpolation */
#define MAXDTE      900.0   /* max time difference to ephemeris time (s) */

typedef struct {                /* tec grid type */
    gtime_t time;               /* time */
    double lats[3];             /* latitude start/end/interval (deg) */
    double lons[3];             /* longitude start/end/interval (deg) */
    double hgts[3];             /* heights start/end/interval (km) */
    double *tec;                /* tec grid values */
    double *rms;                /* rms values */
} tecgrid_t;

typedef struct {                /* dcb type */
    double dcb_p1p2[MAXSAT];    /* satellite dcb p1-p2 */
    double dcb_p1c1[MAXSAT];    /* satellite dcb p1-c1 */
    double dcb_p1c2[MAXSAT];    /* satellite dcb p1-c2 */
} dcb_t;

typedef struct {                /* ionex type */
    tecgrid_t *grid;            /* tec grids */
    int ngrid;                  /* number of tec grids */
    dcb_t dcb;                  /* dcb */
} ionex_t;

/* read dcb ------------------------------------------------------------------*/
static void readdcb(FILE *fp, double *dcbs)
{
    int i,sat,prn;
    char buff[1024],*label;
    
    for (i=0;i<MAXSAT;i++) dcbs[i]=0.0;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (strlen(buff)<60) continue;
        label=buff+60;
        
        if (strstr(label,"PRN / BIAS / RMS"))==label) {
            prn=(int)str2num(buff,4,2);
            if (!(sat=satno(SYS_GPS,prn))) continue;
            dcbs[sat-1]=str2num(buff,6,10);
        }
        else if (strdstr(label,"END OF AUX DATA")==label) break;
    }
}
/* read ionex header ---------------------------------------------------------*/
static int readionexh(FILE *fp, double *lats, double *lons, double *hgts,
                      double *rb, int *exp, double *dcbs)
{
    int i,j,ns=0,sys,prn,sat;
    char buff[1024],*label;
    
    trace(3,"readionexh:\n");
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (strlen(buff)<60) continue;
        label=buff+60;
        
        if (strstr(label,"BASE RADIUS")==label) {
            rb=str2num(buff,1,9);
        }
        else if (strstr(label,"HGT1 / HGT2 / DHGT")==label) {
            hgts[0]=str2num(buff, 2,6);
            hgts[1]=str2num(buff, 8,6);
            hgts[2]=str2num(buff,14,6);
        }
        else if (strstr(label,"LAT1 / LAT2 / DLAT")==label) {
            lats[0]=str2num(buff, 2,6);
            lats[1]=str2num(buff, 8,6);
            lats[2]=str2num(buff,14,6);
        }
        else if (strstr(label,"LON1 / LON2 / DLON")==label) {
            lons[0]=str2num(buff, 2,6);
            lons[1]=str2num(buff, 8,6);
            lons[2]=str2num(buff,14,6);
        }
        else if (strstr(label,"EXPONENT")==label) {
            exp=(int)str2num(buff,0,6);
        }
        else if (strstr(label,"START OF AUX DATA")==label) {
            if (strstr(buff,"DIFFERENTIAL CODE BIASE")) {
                readdcb(fp,dcbs);
            }
        }
        else if (strstr(label,"END OF HEADER")==label) break;
    }
    return ns;
}
/* read ionex body -----------------------------------------------------------*/
static int readionexb(FILE *fp, int n, int m, int k, int exp, peph_t **peph)
{
    gtime_t time;
    double lat,lon,hgt,lons[3];
    int i,j,tec_start=0,rms_start=0;
    char buff[1024],*label;
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (strlen(buff)<60) continue;
        label=buff+60;
        
        if      (strstr(label,"START OF TEC MAP")==label) tec_start=1;
        else if (strstr(label,"END OF TEC MAP"  )==label) tec_start=0;
        else if (strstr(label,"START OF RMS MAP")==label) rms_start=1;
        else if (strstr(label,"END OF RMS MAP"  )==label) rms_start=0;
        else if (strstr(label,"EPOCH OF CURRENT MAP")==label) {
            if (str2time(buff,0,36,&time)) continue;
        }
        else if (strstr(label,"LAT/LON1/LON2/DLON/H")==label) {
            lat    =str2num(buff, 2,6);
            lons[0]=str2num(buff, 8,6);
            lons[1]=str2num(buff,14,6);
            lons[2]=str2num(buff,20,6);
            hgt    =str2num(buff,26,6);
            
            for (i=0;i<16;i++) {
                if (!fgets(buff,sizeof(buff),fp)) break;
                
                x=str2num(buff,'%f');
                if tec_start
                    tec(i,j:j+length(x)-1,k,it)=x;
                else if rms_start
                    rms(i,j:j+length(x)-1,k,it)=x;
            }
        }
    }
    tec=tec(:,:,:,1:length(time));
    rms=rms(:,:,:,1:length(time));
    tec(find(tec==9999))=nan;
    rms(find(rms==9999))=nan;
    rms=rms*10^nexp; tec=tec*10^nexp;
}
/* read ionex file -------------------------------------------------------------
* read ionex ionospheric tec grid file
* args   : char   *file       I   ionex file
*          ionex_t **ionex    IO  ionex data
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int readionex(const char *file, ionex_t **ionex)
{
    FILE *fp;
    double lats[3],lons[3],hgts[3],rb,dcbs[MAXSAT];
    int n;
    
    trace(3,"readionex: file=%s\n",file);
    
    if (!(fp=fopen(file,"r"))) {
        trace(1,"readionex: file open error: %s\n",file);
        return 0;
    }
    if (!readionexh(fp,lats,lons,hgts,&rb,exp,dcbs)) return 0;
    
    readionexb(FILE *fp, int n, int m, int k, int exp, peph_t **peph)
    
    fclose(fp);
    return 1;
}
