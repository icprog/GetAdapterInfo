/* Version Histroy */
/* 
version: 1.2
date: 2011-04-29
1. Update function IP_Exits() to fulfil ARP -d with windows API

version: 1.1
date: 2011-04-13
1. Add function IP_KillProcName
*/
#ifdef _daixin_support
#define WINAPIN_DLL extern "C" __declspec(dllexport)
#else
#define WINAPIN_DLL extern "C" __declspec(dllimport)
#endif

#ifndef _dx_get_adapter_info_
#define _dx_get_adapter_info_

/*
judge a network card's IP address exit in pIpAddr range
the function used check the network card is insert ok and link ok or not?
pIpAddr is IP address range
if ping passed once a time, return pass
*/
WINAPIN_DLL bool IP_Exits(char *pIpAddr/*in*/);

/*
ping a IP address(if a time failed, result is failed)
uiPingTimes is total ping time
uiPacketSize is pacakge size
if ping passed once a time, return pass
*/
WINAPIN_DLL bool IP_Ping(char *pIpAddr/*in*/,unsigned int uiPingTimes/*in*/, unsigned int uiPacketSize/*in, max=65535*/);

/*
ping a IP address use continue mode(if a time failed, result is failed)
uiPingTimes is total ping pass time
uiPacketSize is pacakge size
if ping failed once a time, return pass
*/
WINAPIN_DLL bool IP_PingContinous(char *pIpAddr/*in*/,unsigned int uiPingTimes/*in*/, unsigned int uiPacketSize/*in, max=65535*/);

/*
Rnn telnetenable.exe
if run OK, return true
*/
WINAPIN_DLL bool IP_TelnetEnable(char *pIpAddr/*in*/,char* pMac/*in*/);

/*
get MAC address with IP address.
MAC address lenght is 12
if get MAC ok, return true
*/
WINAPIN_DLL bool IP_ArpMac(char *pIpAddr/*in*/,char* pMac/*out*/);

/*
kill a process
pProcName is process name
*/
WINAPIN_DLL bool IP_KillProcName(char *pProcName/*in*/);
#endif