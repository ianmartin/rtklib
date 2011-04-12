/*------------------------------------------------------------------------------
* convbin.c : convert receiver binary log file to rinex obs/nav, sbas messages
*
*          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
*
* options : -DWIN32 use windows file path separator
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 22:13:04 $
* history : 2008/06/22 1.0 new
*           2009/06/17 1.1 support glonass
*           2009/12/19 1.2 fix bug on disable of glonass
*                          fix bug on inproper header for rtcm2 and rtcm3
*           2010/07/18 1.3 add option -v, -t, -h, -x
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "rtklib.h"

static const char rcsid[]="$Id: convbin.c,v 1.1 2008/07/17 22:13:04 ttaka Exp $";

#define PRGNAME "CONVBIN"

/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" Synopsys",
"",
" convbin [-ts y/m/d h:m:s] [-te y/m/d h:m:s] [-ti tint] [-r format] [-f freq]",
"         [-v ver] [-od] [-os] [-x sat] ... [-d dir] [-o ofile] [-n nfile]",
"         [-g gfile] [-h hfile] [-s sfile] file",
"",
" Description",
"",
" Convert RTCM, receiver raw data log and RINEX file to RINEX and SBAS message",
" file. SBAS message file complies with RTKLIB SBAS messsage format. It",
" supports the following messages or files.",
"",
" RTCM 2                : Type 1, 3, 9, 14, 16, 17, 18, 19, 22",
" RTCM 3                : Type 1002, 1004, 1005, 1006, 1010, 1012, 1019, 1020",
" NovAtel OEMV/4,OEMStar: RANGECMPB, RANGEB, RAWEPHEMB, IONUTCB, RAWWASSFRAMEB",
" NovAtel OEM3          : RGEB, REGD, REPB, FRMB, IONB, UTCB",
" u-blox LEA-4T/LEA-5T  : RXM-RAW, RXM-SFRB",
" NovAtel Superstar II  : ID#20, ID#21, ID#22, ID#23, ID#67",
" Hemisphere            : BIN76, BIN80, BIN94, BIN95, BIN96",
" SkyTraq S1315F        : msg0xDD, msg0xE0, msg0xDC",
" RINEX                 : OBS, NAV, GNAV, HNAV",
"",
" Options [default]",
"",
"     file      input receiver binary log file",
"     -ts y/m/d h:m:s  start time [all]",
"     -te y/m/d h:m:s  end time [all]",
"     -tr y/m/d h:m:s  approximated time for rtcm messages",
"     -ti tint  observation data interval (s) [all]",
"     -r format log format type",
"               rtcm2= RTCM 2",
"               rtcm3= RTCM 3",
"               nov  = NovAtel OEMV/4,OEMStar",
"               oem3 = NovAtel OEM3",
"               ubx  = ublox LEA-4T/LEA-5T",
"               ss2  = NovAtel Superstar II",
"               hemis= Hemisphere Eclipse/Crescent",
"               stq  = SkyTraq S1315F",
"               rinex= RINEX",
"     -f freq   number of frequencies [2]",
"     -v ver    rinex version (2:ver.2,3:ver.3) [2]",
"     -od       include doppler frequency [off]",
"     -os       include snr [off]",
"     -x sat    exclude satellite (multiple options are ok)",
"     -d dir    output directory [same as input file]",
"     -o ofile  output RINEX OBS file",
"     -n nfile  output RINEX NAV file",
"     -g gfile  output RINEX GNAV file",
"     -h hfile  output RINEX HNAV file",
"     -s sfile  output SBAS message file",
"",
" If any output file specified, default output files (<file>.obs,",
" <file>.nav, <file>.gnav and <file>.sbs) are used.",
"",
" If receiver type is not specified, type is recognized by the input",
" file extention as follows.",
"     *.rtcm2    RTCM 2",
"     *.rtcm3    RTCM 3",
"     *.gps      NovAtel OEMV/4,OEMStar",
"     *.ubx      u-blox LEA-4T/LEA-5T",
"     *.log      NovAtel Superstar II",
"     *.bin      Hemisphere Eclipse/Crescent",
"     *.stq      SkyTraq S1315F",
"     *.obs,*.*o RINEX OBS",
"     *.nav,*.gnav,*hnav,*.*n,*.*g,*.*h",
"                RINEX NAV/GNAV/HNAV",
};
/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<sizeof(help)/sizeof(*help);i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* show message --------------------------------------------------------------*/
extern int showmsg(char *format, ...)
{
    va_list arg;
    va_start(arg,format); vfprintf(stderr,format,arg); va_end(arg);
    fprintf(stderr,"\r");
    return 0;
}
/* convert main --------------------------------------------------------------*/
static int convbin(int format, rnxopt_t *opt, const char *ifile, char **file,
                   char *dir)
{
    int i,def=!file[0]&&!file[1]&&!file[2]&&!file[3]&&!file[4];
    char work[1024],ofile_[5][1024],*ofile[5],*p;
    
    for (i=0;i<5;i++) ofile[i]=ofile_[i];
    
    if (file[0]) strcpy(ofile[0],file[0]);
    else if (def) {
        strcpy(ofile[0],ifile);
        if ((p=strrchr(ofile[0],'.'))) strcpy(p,".obs"); else strcat(ofile[0],".obs");
    }
    if (file[1]) strcpy(ofile[1],file[1]);
    else if (def) {
        strcpy(ofile[1],ifile);
        if ((p=strrchr(ofile[1],'.'))) strcpy(p,".nav"); else strcat(ofile[1],".nav");
    }
    if (file[2]) strcpy(ofile[2],file[2]);
    else if (def) {
        strcpy(ofile[2],ifile);
        if ((p=strrchr(ofile[2],'.'))) strcpy(p,".gnav"); else strcat(ofile[2],".gnav");
    }
    if (file[3]) strcpy(ofile[3],file[3]);
    else if (def) {
        strcpy(ofile[3],ifile);
        if ((p=strrchr(ofile[3],'.'))) strcpy(p,".hnav"); else strcat(ofile[3],".hnav");
    }
    if (file[4]) strcpy(ofile[4],file[4]);
    else if (def) {
        strcpy(ofile[4],ifile);
        if ((p=strrchr(ofile[4],'.'))) strcpy(p,".sbs"); else strcat(ofile[4],".sbs");
    }
    for (i=0;i<5;i++) {
        if (!dir||!*ofile[i]) continue;
        if ((p=strrchr(ofile[i],FILEPATHSEP))) strcpy(work,p+1);
        else strcpy(work,ofile[i]);
        sprintf(ofile[i],"%s%c%s",dir,FILEPATHSEP,work);
    }
    fprintf(stderr,"input file  : %s (%s)\n",ifile,formatstrs[format]);
    if (*ofile[0]) fprintf(stderr,"->rinex obs : %s\n",ofile[0]);
    if (*ofile[1]) fprintf(stderr,"->rinex nav : %s\n",ofile[1]);
    if (*ofile[2]) fprintf(stderr,"->rinex gnav: %s\n",ofile[2]);
    if (*ofile[3]) fprintf(stderr,"->rinex hnav: %s\n",ofile[3]);
    if (*ofile[4]) fprintf(stderr,"->sbas log  : %s\n",ofile[4]);
    
    if (!convrnx(format,opt,ifile,ofile)) {
        fprintf(stderr,"\n");
        return -1;
    }
    fprintf(stderr,"\n");
    return 0;
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    rnxopt_t opt={{0}};
    double eps[]={1980,1,1,0,0,0},epe[]={2037,12,31,0,0,0};
    double epr[]={2010,1,1,0,0,0};
    int i,sat,format=-1,nf=2;
    char *fmt=NULL,*dir=NULL,*ifile="",*file[5]={0},*p;
    
    opt.rnxver=2;
    opt.obstype=OBSTYPE_PR|OBSTYPE_CP;
    opt.navsys =SYS_GPS|SYS_SBS;
#ifdef ENAGLO
    opt.navsys|=SYS_GLO;
#endif
#ifdef ENAGAL
    opt.navsys|=SYS_GAL;
#endif
#ifdef ENAQZS
    opt.navsys|=SYS_QZS;
#endif
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",eps,eps+1,eps+2);
            sscanf(argv[++i],"%lf:%lf:%lf",eps+3,eps+4,eps+5);
            opt.ts=epoch2time(eps);
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",epe,epe+1,epe+2);
            sscanf(argv[++i],"%lf:%lf:%lf",epe+3,epe+4,epe+5);
            opt.te=epoch2time(epe);
        }
        else if (!strcmp(argv[i],"-tr")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",epr,epr+1,epr+2);
            sscanf(argv[++i],"%lf:%lf:%lf",epr+3,epr+4,epr+5);
            opt.trtcm=epoch2time(epr);
        }
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) opt.tint=atof(argv[++i]);
        else if (!strcmp(argv[i],"-f" )&&i+1<argc) nf=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-v" )&&i+1<argc) opt.rnxver=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-od")) opt.obstype|=OBSTYPE_DOP;
        else if (!strcmp(argv[i],"-os")) opt.obstype|=OBSTYPE_SNR;
        else if (!strcmp(argv[i],"-x" )&&i+1<argc) {
            if ((sat=satid2no(argv[++i]))) opt.exsats[sat-1]=1;
        }
        else if (!strcmp(argv[i],"-r" )&&i+1<argc) fmt=argv[++i];
        else if (!strcmp(argv[i],"-d" )&&i+1<argc) dir=argv[++i];
        else if (!strcmp(argv[i],"-o" )&&i+1<argc) file[0]=argv[++i];
        else if (!strcmp(argv[i],"-n" )&&i+1<argc) file[1]=argv[++i];
        else if (!strcmp(argv[i],"-g" )&&i+1<argc) file[2]=argv[++i];
        else if (!strcmp(argv[i],"-h" )&&i+1<argc) file[3]=argv[++i];
        else if (!strcmp(argv[i],"-s" )&&i+1<argc) file[4]=argv[++i];
        else if (!strncmp(argv[i],"-",1)) printhelp();
        else ifile=argv[i];
    }
    if (!*ifile) {
        fprintf(stderr,"no input file\n");
        return -1;
    }
    if (fmt) {
        if      (!strcmp(fmt,"rtcm2")) format=STRFMT_RTCM2;
        else if (!strcmp(fmt,"rtcm3")) format=STRFMT_RTCM3;
        else if (!strcmp(fmt,"nov"  )) format=STRFMT_OEM4;
        else if (!strcmp(fmt,"oem3" )) format=STRFMT_OEM3;
        else if (!strcmp(fmt,"ubx"  )) format=STRFMT_UBX;
        else if (!strcmp(fmt,"ss2"  )) format=STRFMT_SS2;
        else if (!strcmp(fmt,"hemis")) format=STRFMT_CRES;
        else if (!strcmp(fmt,"stq"  )) format=STRFMT_STQ;
        else if (!strcmp(fmt,"rinex")) format=STRFMT_RINEX;
    }
    else if ((p=strrchr(ifile,'.'))) {
        if      (!strcmp(p,".rtcm2"))  format=STRFMT_RTCM2;
        else if (!strcmp(p,".rtcm3"))  format=STRFMT_RTCM3;
        else if (!strcmp(p,".gps"  ))  format=STRFMT_OEM4;
        else if (!strcmp(p,".ubx"  ))  format=STRFMT_UBX;
        else if (!strcmp(p,".log"  ))  format=STRFMT_SS2;
        else if (!strcmp(p,".bin"  ))  format=STRFMT_CRES;
        else if (!strcmp(p,".stq"  ))  format=STRFMT_STQ;
        else if (!strcmp(p,".obs"  ))  format=STRFMT_RINEX;
        else if (!strcmp(p,".nav"  ))  format=STRFMT_RINEX;
        else if (!strcmp(p,".gnav" ))  format=STRFMT_RINEX;
        else if (!strcmp(p,".hnav" ))  format=STRFMT_RINEX;
        else if (!strcmp(p+3,"o"   ))  format=STRFMT_RINEX;
        else if (!strcmp(p+3,"n"   ))  format=STRFMT_RINEX;
        else if (!strcmp(p+3,"g"   ))  format=STRFMT_RINEX;
        else if (!strcmp(p+3,"h"   ))  format=STRFMT_RINEX;
    }
    if (nf>=1) opt.freqtype|=FREQTYPE_L1;
    if (nf>=2) opt.freqtype|=FREQTYPE_L2;
    if (nf>=3) opt.freqtype|=FREQTYPE_L5;
    if (nf>=4) opt.freqtype|=FREQTYPE_L7;
    if (nf>=5) opt.freqtype|=FREQTYPE_L6;
    if (nf>=6) opt.freqtype|=FREQTYPE_L8;
    if (format<0) {
        fprintf(stderr,"input format can not be recognized\n");
        return -1;
    }
    sprintf(opt.prog,"%s %s",PRGNAME,VER_RTKLIB);
    sprintf(opt.comment[0],"log: %-55.55s",ifile);
    sprintf(opt.comment[1],"format: %s",formatstrs[format]);
    
    return convbin(format,&opt,ifile,file,dir);
}
