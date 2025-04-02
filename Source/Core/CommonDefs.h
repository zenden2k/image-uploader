#ifndef IU_CORE_COMMONDEFS_H
#define IU_CORE_COMMONDEFS_H

#pragma once
//#include "Core/Utils/CoreUtils.h"

#define APPNAME_ "Image Uploader"
#define APPNAME _T(APPNAME_)


// QString <-> std::string (utf-8) conversion macroses
#define Q2U(str) str.toUtf8().data()
#define U2Q(str) QString::fromUtf8(str.c_str())
//#define _(str) str

// CString <-> std::string(utf-8) conversion macroses
#define W2U(str) IuCoreUtils::WstringToUtf8((static_cast<LPCTSTR>(str)))
#define U2W(str) CString(IuCoreUtils::Utf8ToWstring(str).c_str())
#define U2WC(str) (IuCoreUtils::Utf8ToWstring(str).c_str())

#pragma warning(disable:4996)
#pragma warning( disable: 4100 ) // unreferenced formal parameter

constexpr auto IU_VIDEOFILES_EXTENSIONS = "asf;avi;mpeg;mpg;mp2;divx;vob;flv;wmv;mkv;mp4;ts;mov;mpeg2ts;3gp;mpeg1;mpeg2;mpeg4;mv4;rmvb;qt;hdmov;m4v;ogv;m2v;webm;wmp;wm;mpe;m1v;mpv2;mp2v;tp;tpr;trp;ifo;ogm;m4p;m4b;3gpp;3g2;3gp2;rm;ram;rpm;nsv;dpg;m2ts;m2t;mts;dvr-ms;k3g;skm;evo;nsr;amv;wtv;f4v;mxf";
constexpr auto IU_AUDIOFILES_EXTENSIONS = "aa;aac;ac3;adx;ahx;aiff;ape;asf;asx;au;snd;aud;dmf;dts;dxd;flac;la;m4a;mmf;mod;mp1;mp2;mp3;mp4;mpc;ofr;oga;ogg;opus;pac;pxd;ra;rka;sb0;shn;tta;voc;vqf;wav;wma;wv;xm;cd;mqa;mid;mpa;m1a;m2a;mka;eac3;dtshd;tak;cda;dsf;aif;amr";
constexpr auto IU_EXTENSIONS_LIST_SEPARATOR = ";";

#endif
