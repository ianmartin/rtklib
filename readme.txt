--------------------------------------------------------------------------------

           RTKLIB: An Open Source Program Package for GNSS Positioning

--------------------------------------------------------------------------------

OVERVIEW

RTKLIB is an open source program package for standard and precise positioning
with GNSS. RTKLIB consists of a portable program library and several application
programs (APs) utilizing the library. The features of RTKLIB are:

(1) Supports standard and precise positioning algorithms with:

    GPS, GLONASS, Galileo, QZSS and SBAS (Galileo is not yet formally supported)

(2) Supports various positioning modes with GNSS for both real-time and
    post-processing:

    Single-point, DGPS/DGNSS, Kinematic, Static, Moving-baseline, Fixed,
    PPP-Kinematic, PPP-Static and PPP-Fixed

(3) Supports many standard formats and protocols for GNSS:

    RINEX 2.10,2.11,2.12 OBS/NAV/GNAV/HNAV, RINEX 3.00 OBS/NAV/CLK, RTCM v.2.3, 
    RTCM v.3.1, NTRIP 1.0, NMEA 0183,SP3-c, IONEX 1.0, ANTEX 1.3, NGS PCV and
    EMS 2.0 (see Release Notes for supported RTCM messages)

(4) Supports several GNSS receivers' proprietary messages:

    NovAtel: OEM4/V, OEM3, OEMStar, Superstar II, Hemisphere: Eclipse, 
    Crescent, u-blox: LEA-4T, 5T, 6T and SkyTraq: S1315F, JAVAD and Furuno
    GW-10-II/III (see Release Notes for supported messages)

(6) Supports external communication via: 

    Serial, TCP/IP, NTRIP, local log file (record and playback) and FTP/HTTP
    (automatic download)

(7) Provides many library functions and APIs:

    Satellite and navigation system functions, matrix and vector functions,
    time and string functions, coordinates transformation, input and output
    functions, debug trace functions, platform dependent functions, positioning
    models, atmosphere models, antenna models, earth tides models, geoid models,
    datum transformation, RINEX functions, ephemeris and clock functions,
    precise ephemeris and clock functions, receiver raw data functions, RTCM
    functions, solution functions, Google Earth KML converter, SBAS functions,
    options functions, stream data input and output functions, integer ambiguity
    resolution, standard positioning, precise positioning, post-processing
    positioning, stream server functions, RTK server functions

(8) Provides GUI and console (command line) APs:

    - RTKNAVI, RTKRCV   : real-time positioning
    - RTKPOST, RNX2RTKP : post-processing analysis
    - RTKPLOT           : visualization of solutions and observation data
    - RTKCONV, CONVBIN  : RINEX translator for RTCM and receiver raw data log
    - STRSVR,  STR2STR  : communication utility
    - NTRIPSRCBROWS     : NTRIP source table browser
    - Other positioning utilities

All of the executable binary APs for Windows are included in the package as well
as whole source programs of the library and the APs.

--------------------------------------------------------------------------------

LICENSE

    RTKLIB is distributed under GPLv3. (http://gplv3.fsf.org/)

--------------------------------------------------------------------------------

ENVIRONMENT/PORTABILITY

    All of the library functions and APIs are written in ANSI C (C89). The 
    library internally uses standard socket and pthread for Linux/UNIX or
    winsock and WIN32 thread for Windows. By setting compiler option -DLAPACK
    or -DMKL, the library uses LAPACK/BLAS or Intel MKL for fast matrix
    computation. The console APs are also written in standard C. The library
    and console APs can be built on many environments like gcc on Linux. The GUI
    APs are written in C++ and utilize Borland VCL for the GUI toolkit. All of
    the executable binary APs in the package were built by free edition Borland
    Turbo C++ 2006 (http://www.turboexplorer.com) * on Windows. The binary APs
    were tested on Windows XP SP3 32bit and Windows 7 64bit. The console APs
    are also built and tested on Ubuntu 9.04 Linux.

    * Free edition Turbo C++ 2006 is no longer availabile. We plan to change
    the GUI toolkit for RTKLIB GUI APs to open source one in future versions.

--------------------------------------------------------------------------------

PACKAGE

  rtklib_{ver}.zip
  ./src           source programs
  ./bin           application binary programs
  ./app           application source programs and make environments
    ./rtkrcv      realtime rtk positioning (console)
    ./rnx2rtkp    rinex to rtk positioning (console)
    ./str2str     stream server (console)
    ./pos2kml     convert position to google earth kml (console)
    ./convbin     convert receiver log file to rinex (console)
    ./rtkpost     post-positioning  (gui)
    ./rtkpost_mkl post-positioning with intel mkl (gui)
    ./rtkplot     plot solutions or observation data (gui)
    ./rtkconv     receiver log converter (gui)
    ./rtknavi     realtime rtk positioning (gui)
    ./rtknavi_mkl realtime rtk positioning (gui)
    ./strsvr      stream server (gui)
    ./srctblbrows ntrip source table browser
    ./appcmn      common application routines
    ./icon        common application icons
  ./util          utilities
  ./test          test programs and data
  ./doc           documents
  ./lib           library genration environment

--------------------------------------------------------------------------------

HISTORY

  ver.1.0    2007/01/25 new release
  ver.1.1    2007/03/20 add rnx2rtkp_gui, fix bugs, improve performance
  ver.2.1.0  2008/07/15 refactored, add applications
  ver.2.1.1  2008/10/19 fix bugs
  ver.2.2.0  2009/01/20 add stream.c,rtksvr.c,preceph.c in src
                        add rtknavi,rtkpost_mkl,srctblbrows,strsvr,str2str in app
  ver.2.2.1  2009/05/17 see relnotes_2.2.1.txt
  ver.2.2.2  2009/09/07 see relnotes_2.2.2.txt
  ver.2.3.0  2009/12/17 see relnotes_2.3.0.txt
  ver.2.4.0  2010/08/08 see relnotes_2.4.0.pdf
  ver.2.4.1  2011/06/01 see relnotes_2.4.1.htm

--------------------------------------------------------------------------------

           Copyright (C) 2007-2011 by T.TAKASU, All rights reserved.


