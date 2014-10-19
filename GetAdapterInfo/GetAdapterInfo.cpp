// GetAdapterInfo.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "windows.h"
#include "Iphlpapi.h"
#include "stdlib.h"
#include "stdio.h"
#include "Tlhelp32.h"
#define _daixin_support
#include "GetAdapterInfo.h"
#include "Winsock2.h" 
#include <string>
#include <fstream>
using namespace std;
#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// GetAdaptherInfo.cpp : Defines the entry point for the console application.
//



bool IP_Exits(char *pIpAddr)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	MIB_IPNETROW mi;
	mi.dwAddr=inet_addr(pIpAddr);

	
	char szIpBuf[18]="";
	strcpy_s(szIpBuf,sizeof(szIpBuf),pIpAddr);	
	strrev(szIpBuf);
	char *pIpAddPrex=strchr(szIpBuf,'.');
	strrev(pIpAddPrex);
	
	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen); 
	}

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) 
		{
			if(strstr(pAdapter->IpAddressList.IpAddress.String,pIpAddPrex))
			{
				mi.dwIndex=pAdapter->Index;
				DeleteIpNetEntry(&mi);
				return true;
			}
			pAdapter = pAdapter->Next;
		}
	}
	else 
	{
		return false;
	}
	
	return false;
}

void DeleteSpecifyExe(TCHAR* exeName)
{
	if(exeName == NULL)
		return;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(INVALID_HANDLE_VALUE == hProcess)
		return;
	BOOL fPOK = Process32First(hProcess,&pe);
	for(; fPOK;fPOK = Process32Next(hProcess,&pe))
	{
		char szVal1[256]="";
		char szVal2[256]="";
		sprintf_s(szVal1,sizeof(szVal1),"%s",exeName);
		sprintf_s(szVal2,sizeof(szVal2),"%s",pe.szExeFile);
		/*char *pszVal1 = _strlwr(exeName);
		char *pszVal2 = _strlwr(pe.szExeFile);*/
		if(strstr(strlwr(szVal1),strlwr(szVal2)) != NULL)
		{
			HANDLE hPr = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID );
			if( hPr == NULL )
				return;
			else
			{
				TerminateProcess(hPr,0);
			}
		}
	}
	if(INVALID_HANDLE_VALUE != hProcess)
		CloseHandle(hProcess);
	Sleep(200);
}

bool IP_Ping(char *pIpAddr, unsigned int uiPingTimes, unsigned int uiPacketSize)
{
	string strBufPipe;
	HANDLE hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa,sizeof(sa));
	sa.bInheritHandle=true;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(sa);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,sizeof(sa)))
	{
		return false;
	}

	char szCmd[512]="";
	sprintf_s(szCmd,sizeof(szCmd),"ping.exe %s -n %d -l %d",pIpAddr,uiPingTimes,uiPacketSize);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	GetStartupInfo(&si);
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdError=hWritePipe;
	si.hStdOutput=hWritePipe;
	si.hStdInput=hReadPipe;

	if(!CreateProcess(NULL,szCmd,NULL,NULL,true,0,NULL,NULL,&si,&pi))
	{
		int i=GetLastError();
		printf("%d\n",i);
	}
	WaitForSingleObject(pi.hProcess,1000);

	//CreateThread(NULL,0,__ReadPipe,NULL,0,NULL);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hWritePipe);

	char szRead[1024]="";
	DWORD dwToRead=0;
	while(1)
	{
		memset(szRead,'\0',sizeof(szRead));
		if(!ReadFile(hReadPipe,szRead,sizeof(szRead),&dwToRead,NULL) || dwToRead == 0)
		{
			break;
		}
		else
		{			
			printf(szRead);
			strBufPipe+=szRead;
			if(strstr( strBufPipe.c_str(), "Reply from"))
			{
				printf(strBufPipe.c_str());
				DeleteSpecifyExe("ping.exe");
				TerminateProcess(pi.hProcess,0);
				CloseHandle(hReadPipe);	
				return true;
			}
		}
		Sleep(10);
	}
	CloseHandle(hReadPipe);	

	return false;
}

bool IP_PingContinous(char *pIpAddr, unsigned int uiPingTimes, unsigned int uiPacketSize)
{
	string strBufPipe;
	HANDLE hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa,sizeof(sa));
	sa.bInheritHandle=true;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(sa);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,sizeof(sa)))
	{
		return false;
	}

	char szCmd[512]="";
	sprintf_s(szCmd,sizeof(szCmd),"ping.exe %s -n %d -l %d",pIpAddr,uiPingTimes,uiPacketSize);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	GetStartupInfo(&si);
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdError=hWritePipe;
	si.hStdOutput=hWritePipe;
	si.hStdInput=hReadPipe;

	if(!CreateProcess(NULL,szCmd,NULL,NULL,true,0,NULL,NULL,&si,&pi))
	{
		int i=GetLastError();
		printf("%d\n",i);
	}
	WaitForSingleObject(pi.hProcess,1000);

	//CreateThread(NULL,0,__ReadPipe,NULL,0,NULL);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hWritePipe);

	char szRead[1024]="";
	DWORD dwToRead=0;
	while(1)
	{
		memset(szRead,'\0',sizeof(szRead));
		if(!ReadFile(hReadPipe,szRead,sizeof(szRead),&dwToRead,NULL) || dwToRead == 0)
		{
			break;
		}
		else
		{			
			printf(szRead);
			strBufPipe+=szRead;
			if(strstr( strBufPipe.c_str(), "Request timed out.") ||
			   strstr( strBufPipe.c_str(), "Destination host unreachable") ||
			   strstr( strBufPipe.c_str(), "error"))
			{
				ofstream os;
				os.open("pingresult.log",ios_base::out);
				os<<strBufPipe<<endl;
				os.close();

				DeleteSpecifyExe("ping.exe");
				TerminateProcess(pi.hProcess,0);
				CloseHandle(hReadPipe);	
				return false;
			}
		}
		Sleep(10);
	}
	CloseHandle(hReadPipe);	
	
	return true;
}

// send failed: 10057
bool IP_TelnetEnable(char *pIpAddr,char* pMac)
{

	ofstream os;
	os.open("result.log");
	string strBufPipe="";
	HANDLE hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa,sizeof(sa));
	sa.bInheritHandle=true;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(sa);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,sizeof(sa)))
	{
		return false;
	}

	char szCmd[512]="";
	/*
	Some new project change telnetenable.exe and username and password
	The new username and password need pair new telnetenable.exe file
	The username change from Gearguy to admin
	The password change from Gearguy to admin
	*/
	//sprintf_s(szCmd,sizeof(szCmd),"telnetenable.exe %s %s Gearguy Geardog",pIpAddr,pMac);
	sprintf_s(szCmd,sizeof(szCmd),"telnetenable.exe %s %s admin admin",pIpAddr,pMac);
	os<<szCmd<<endl;
	printf("%s\n",szCmd);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	GetStartupInfo(&si);
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdError=hWritePipe;
	si.hStdOutput=hWritePipe;
	si.hStdInput=hReadPipe;
	DeleteSpecifyExe("telnetenable.exe");
	if(!CreateProcess(NULL,szCmd,NULL,NULL,true,0,NULL,NULL,&si,&pi))
	{
		int i=GetLastError();
		printf("%d\n",i);
		os<<"GetLastError() "<<i<<endl;
		return false;
	}

	if(WAIT_OBJECT_0 == WaitForSingleObject(pi.hProcess,3000))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hWritePipe);
		char szRead[1024]="";
		DWORD dwToRead=0;
		while(1)
		{
			memset(szRead,'\0',sizeof(szRead));
			if(!ReadFile(hReadPipe,szRead,sizeof(szRead),&dwToRead,NULL) || dwToRead == 0)
			{
				break;
			}
			else
			{			
				printf(szRead);
				os<<szRead;
				strBufPipe+=szRead;				
			}
			Sleep(10);
		}
		CloseHandle(hReadPipe);	
		if(strstr( strBufPipe.c_str(), "send failed: 10057"))
		{
			os<<strBufPipe<<endl;
			//DeleteSpecifyExe("telnetenable.exe");
			CloseHandle(hReadPipe);
			os<<"FAIL"<<endl;
			os.close();
			return false;

		}
		else
		{
			os<<"PASS"<<endl;
			os.close();
			//DeleteSpecifyExe("telnetenable.exe");
			return true;
		}
	}
	else
	{	
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);	
		os<<"FAIL"<<endl;
		DeleteSpecifyExe("telnetenable.exe");
		os.close();
	}

	return false;	
}
bool IP_ArpMac(char *pIpAddr,char* pMac)
{
	HRESULT hr;
	IPAddr  ipAddr;
	ULONG   pulMac[2];
	ULONG   ulLen;

	ipAddr = inet_addr (pIpAddr);
	memset (pulMac, 0xff, sizeof (pulMac));
	ulLen = 6;

	hr = SendARP (ipAddr, 0, pulMac, &ulLen);
	//printf ("Return %08x, length %8d\n", hr, ulLen);
	if(hr != NO_ERROR)
	{
		return false;
	}

	size_t i, j;
	char * szMac = new char[ulLen*3];
	PBYTE pbHexMac = (PBYTE) pulMac;

	//
	// Convert the binary MAC address into human-readable
	//
	for (i = 0, j = 0; i < ulLen - 1; ++i) {
		j += sprintf (szMac + j, "%02X", pbHexMac[i]);
	}

	sprintf (szMac + j, "%02X", pbHexMac[i]);
	printf ("MAC address %s\n", szMac);
	strcpy(pMac,szMac);
	delete [] szMac;
	return true;
}

bool IP_KillProcName(char *pProcName/*in*/)
{
	char szExeName[256]="";
	sprintf_s(szExeName,sizeof(szExeName),"%s",pProcName);
	_strupr_s(szExeName,sizeof(szExeName));

	PROCESSENTRY32 pe32;
	HANDLE hdl=CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hdl == INVALID_HANDLE_VALUE )
	{
		printf( "CreateToolhelp32Snapshot (of processes)" );
		return false;
	}
	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hdl, &pe32 ) )
	{
		printf( "Process32First" );  // Show cause of failure
		CloseHandle( hdl );     // Must clean up the snapshot object!
		return false;
	}
	while(Process32Next(hdl,&pe32))
	{
		if(strstr(_strupr(pe32.szExeFile),szExeName))
		{
			HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS,TRUE,pe32.th32ProcessID);
			if(hProc != NULL)
			{
				TerminateProcess(hProc,0);
				return true;
			}
		}
	}

	return true;
}