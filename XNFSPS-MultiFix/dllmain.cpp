// Xanvier's NFS ProStreet MultiFix
// TODO: add executable pattern detection

#include "stdio.h"
#include <windows.h>
#include "..\includes\injector\injector.hpp"
#include "..\includes\IniReader.h"
#include "XNFSPS-MultiFix.h"
#include "ProStreet_v11.h"
#include "ProStreet_v10.h"

unsigned int MonitorToUse = 0;
unsigned int ActualResCount = 0;
unsigned int StructSize = 0;
unsigned int StructPointer = NULL;
unsigned int StuffToCompare = 0;

unsigned int loc_5FE120 = UNKLOC1_11;
unsigned int loc_5C479A = UNKLOC2_11;
unsigned int loc_5C47B0 = UNKLOC3_11;
unsigned int CreatePostRaceFlow = CREATEPOSTRACEFLOW_11;
unsigned int ResolutionCheckExit = RESCHECKEXIT_11;
unsigned int DamageModelFixExit = DAMAGEMODELFIXEXIT_11;

int hWndPointer = HWNDPOINTER_11;
int ExitPoint = UNKEXITPOINT_11;

struct FEColor
{
	unsigned int Blue;
	unsigned int Green;
	unsigned int Red;
	unsigned int Alpha;
}FEColorVersion;

unsigned int FEStringPrintf = FESTRINGPRINTF_11;
void*(__cdecl *FEObject_FindObject)(const char *pkg_name, unsigned int obj_hash) = (void*(__cdecl*)(const char*, unsigned int))FEOBJECT_FINDOBJECT_11;
void(__cdecl *FEObject_SetColor)(FEColor color, bool bRelative) = (void(__cdecl*)(FEColor, bool))FEOBJECT_SETCOLOR_11;

unsigned int MonitorResAddress1 = MONITORRESADDRESS1_11;
unsigned int MonitorResAddress2 = MONITORRESADDRESS2_11;
unsigned int CheckResAddress = CHECKRESADDRESS_11;

unsigned int PostRaceFixAddr1 = POSTRACEFIXADDR1_11;
unsigned int PostRaceFixAddr2 = POSTRACEFIXADDR2_11;
unsigned int PostRaceFixAddr3 = POSTRACEFIXADDR3_11;

unsigned int TrackStreamerAddr1 = TRACKSTREAMERADDR1_11;
unsigned int TrackStreamerJmpPoint = TRACKSTREAMERJMPPOINT_11;
unsigned int TrackStreamerAddr2 = TRACKSTREAMERADDR2_11;

unsigned int WindowedAddr1 = WINDOWEDADDR1_11;
unsigned int WindowedAddr2 = WINDOWEDADDR2_11;
unsigned int WindowedAddr3 = WINDOWEDADDR3_11;

unsigned int EnablePrintsAddr = RELEASEPRINTS_11;

unsigned int DamageModelAddr = DAMAGEMODELADDR1_11;

unsigned int VersionSprintfAddr = VERSIONSPRINTFADDR_11;
unsigned int FEStringPrintfCaveAddr = FESTRINGPRINFCAVEADDR_11;

bool bAccessedPostRace = 0;

bool bPostRaceFix = 1;
bool bFramerateUncap = 1;
bool bResDetect = 1;
bool bAntiTrackStreamerCrash = 1;
bool bEnablePrints = 0;
bool bAntiDamageModelCrash = 1;
int StopWhining = 0;
int WindowedMode = 0;
int MinorVersion = 1;

LONG dwNewLong;

bool __stdcall memory_readable(void *ptr, size_t byteCount)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(ptr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
		return false;

	if (mbi.State != MEM_COMMIT)
		return false;

	if (mbi.Protect == PAGE_NOACCESS || mbi.Protect == PAGE_EXECUTE)
		return false;

	// This checks that the start of memory block is in the same "region" as the
	// end. If it isn't you "simplify" the problem into checking that the rest of 
	// the memory is readable.
	size_t blockOffset = (size_t)((char *)ptr - (char *)mbi.AllocationBase);
	size_t blockBytesPostPtr = mbi.RegionSize - blockOffset;

	if (blockBytesPostPtr < byteCount)
		return memory_readable((char *)ptr + blockBytesPostPtr,
			byteCount - blockBytesPostPtr);

	return true;
}

int CustomVersionSprintf(char* Buffer, const char* Format, ...)
{
	return sprintf(Buffer, "%d.%d - MultiFix v%d", 1, MinorVersion, MULTIFIX_BULDNUM);
}

int __stdcall SetTheColor(const char *pkg_name, unsigned int object_hash)
{
	_asm

	{
		push object_hash
		push pkg_name
		call FEObject_FindObject
		add esp, 0x8
		mov ecx, eax
		push 0
		push offset FEColorVersion
		call FEObject_SetColor
		add esp, 0x14
	}
	return 0;
}

int __cdecl FEStringPrintfCave(const char *pkg_name, unsigned int object_hash, const char *fmt, ...)
{
	_asm
	{
		push eax
		push ecx
		push edx
		push esi
	}
	SetTheColor(pkg_name, object_hash);
	_asm
	{
		pop esi
		pop edx
		pop ecx
		pop eax
		pop esi
		pop ebp
		jmp FEStringPrintf
	}
}

void __declspec(naked) DamageModelMemoryCheck()
{
	_asm
	{
		push edi
		mov edi, [esp + 8]
		mov StuffToCompare, edi
	}

	if (memory_readable((void*)StuffToCompare, 8))
		_asm jmp DamageModelFixExit
	_asm 
	{
		pop edi
		retn
	}
}

void __declspec(naked) ExitPostRaceFixPropagator()
{
	if (bAccessedPostRace)
		_asm jmp loc_5C479A
	else
		_asm jmp loc_5C47B0
}

void __declspec(naked) ExitPostRaceFix()
{
	_asm
	{
		mov bAccessedPostRace, 1
		call CreatePostRaceFlow
		jmp loc_5FE120
	}
}

void __declspec(naked) ExitPostRaceFixPart2()
{
	_asm
	{
		mov bAccessedPostRace, 0
		pop esi
		retn 4
	}
}

void __declspec(naked) CheckResolutionTableEnd()
{
	_asm
	{
		add eax, 8
		cmp eax, StructSize
		jmp ResolutionCheckExit
	}
}

struct ScreenRes
{
	unsigned int ResX;
	unsigned int ResY;
}MonitorRes[255];

void InitConfig()
{
	CIniReader inireader("");

	bPostRaceFix = inireader.ReadInteger("MultiFix", "PostRaceFix", 1);
	bFramerateUncap = inireader.ReadInteger("MultiFix", "FramerateUncap", 1);
	bEnablePrints = inireader.ReadInteger("MultiFix", "EnablePrints", 0);
	bResDetect = inireader.ReadInteger("MultiFix", "ResDetect", 1);
	bAntiTrackStreamerCrash = inireader.ReadInteger("MultiFix", "AntiTrackStreamerCrash", 1);
	bAntiDamageModelCrash = inireader.ReadInteger("MultiFix", "AntiDamageModelCrash", 1);
	StopWhining = inireader.ReadInteger("MultiFix", "StopWhining", 0);
	WindowedMode = inireader.ReadInteger("MultiFix", "WindowedMode", 0);
	if (bResDetect)
		MonitorToUse = inireader.ReadInteger("ResDetect", "MonitorIndex", 0);
}

void DetectResolutions(unsigned int MonitorIndex)
{
	unsigned int PreviousResX = 0;
	unsigned int PreviousResY = 0;
	DEVMODE dm = { 0 };
	DISPLAY_DEVICE dispdev = { 0 };

	dispdev.cb = 424;
	EnumDisplayDevices(NULL, MonitorIndex, &dispdev, 1);
	dm.dmSize = sizeof(dm);
	for (int iModeNum = 0; EnumDisplaySettings(dispdev.DeviceName, iModeNum, &dm) != 0; iModeNum++) {
		if ((dm.dmPelsWidth != PreviousResX) || (dm.dmPelsHeight != PreviousResY))
		{
			MonitorRes[ActualResCount].ResX = dm.dmPelsWidth;
			MonitorRes[ActualResCount].ResY = dm.dmPelsHeight;
			ActualResCount++;
		}
		PreviousResX = dm.dmPelsWidth;
		PreviousResY = dm.dmPelsHeight;
	}

}

void WINAPI SetWindowLongHook(HWND hWndl, int nIndex, LONG dwNewLong)
{
	if (WindowedMode > 1)
		dwNewLong |= WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

	SetWindowLong(hWndl, nIndex, dwNewLong);
	SetWindowPos(hWndl, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void __declspec(naked) SetLongCave()
{
	_asm call ShowWindow
	SetWindowLongHook(*(HWND*)hWndPointer, 0x0FFFFFFF0, 0x10000000);
	_asm jmp ExitPoint
}

int Init()
{
	// Resolution detection
	if (bResDetect)
	{
		DetectResolutions(MonitorToUse);
		injector::WriteMemory<unsigned int>(MonitorResAddress1, (unsigned int)&MonitorRes, true);
		injector::WriteMemory<unsigned int>(MonitorResAddress2, ((unsigned int)&MonitorRes) + 4, true);
		StructSize = (ActualResCount * sizeof(ScreenRes)) + 4;
		injector::MakeJMP(CheckResAddress, CheckResolutionTableEnd, true);
		//injector::WriteMemory<unsigned char>(0x70B3F5, StructSize + 4, true);
	}

	// PostRaceStateManagerFix
	if (bPostRaceFix)
	{
		injector::MakeJMP(PostRaceFixAddr1, ExitPostRaceFix, true);
		injector::MakeJMP(PostRaceFixAddr2, ExitPostRaceFixPart2, true);
		injector::MakeJMP(PostRaceFixAddr3, ExitPostRaceFixPropagator, true);
	}
	// Framerate unlock
	if (bFramerateUncap)
		injector::MakeJMP(FRAMERATECAPADDRESS, FRAMERATECAPJMPADDR, true);

	if (bAntiTrackStreamerCrash)
	{
		injector::MakeJMP(TrackStreamerAddr1, TrackStreamerJmpPoint, true);
		injector::MakeNOP(TrackStreamerAddr2, 2, true);
	}
	// Add screen centering code here
	if (WindowedMode)
	{
		injector::MakeNOP(WindowedAddr1, 3, true);
		injector::WriteMemory<int>(WindowedAddr2, 1, true);
		injector::MakeJMP(WindowedAddr3, SetLongCave, true);
	}

	if (bEnablePrints)
		injector::WriteMemory<int>(EnablePrintsAddr, 1, true);

	if (bAntiDamageModelCrash)
		injector::MakeJMP(DamageModelAddr, DamageModelMemoryCheck, true);

	injector::MakeCALL(VersionSprintfAddr, CustomVersionSprintf, true);
	injector::MakeCALL(FEStringPrintfCaveAddr, FEStringPrintfCave, true);
	FEColorVersion.Red = MULTIFIX_VER_R;
	FEColorVersion.Green = MULTIFIX_VER_G;
	FEColorVersion.Blue = MULTIFIX_VER_B;
	FEColorVersion.Alpha = MULTIFIX_VER_A;
	return 0;
}

int WriteFirstVerAddresses()
{
	loc_5FE120 = UNKLOC1_10;
	loc_5C479A = UNKLOC2_10;
	loc_5C47B0 = UNKLOC3_10;
	CreatePostRaceFlow = CREATEPOSTRACEFLOW_10;
	ResolutionCheckExit = RESCHECKEXIT_10;
	DamageModelFixExit = DAMAGEMODELFIXEXIT_10;

	hWndPointer = HWNDPOINTER_10;
	ExitPoint = UNKEXITPOINT_10;

	FEStringPrintf = FESTRINGPRINTF_10;
	FEObject_FindObject = (void*(__cdecl*)(const char*, unsigned int))FEOBJECT_FINDOBJECT_10;
	FEObject_SetColor = (void(__cdecl*)(FEColor, bool))FEOBJECT_SETCOLOR_10;

	MonitorResAddress1 = MONITORRESADDRESS1_10;
	MonitorResAddress2 = MONITORRESADDRESS2_10;
	CheckResAddress = CHECKRESADDRESS_10;

	PostRaceFixAddr1 = POSTRACEFIXADDR1_10;
	PostRaceFixAddr2 = POSTRACEFIXADDR2_10;
	PostRaceFixAddr3 = POSTRACEFIXADDR3_10;

	TrackStreamerAddr1 = TRACKSTREAMERADDR1_10;
	TrackStreamerJmpPoint = TRACKSTREAMERJMPPOINT_10;
	TrackStreamerAddr2 = TRACKSTREAMERADDR2_10;

	WindowedAddr1 = WINDOWEDADDR1_10;
	WindowedAddr2 = WINDOWEDADDR2_10;
	WindowedAddr3 = WINDOWEDADDR3_10;

	EnablePrintsAddr = RELEASEPRINTS_10;

	DamageModelAddr = DAMAGEMODELADDR1_10;

	VersionSprintfAddr = VERSIONSPRINTFADDR_10;
	FEStringPrintfCaveAddr = FESTRINGPRINFCAVEADDR_10;
	bFramerateUncap = 0;
	MinorVersion = 0;
	return 0;
}

bool TryToDetectFirstVer()
{
	unsigned int VersionValues;

	VersionValues = *(unsigned int*)VERSIONCHECKADDR_10;

	if (VersionValues == 0x016A006A)
		return 1;
	return 0;
}

bool TryToDetectSecondVer()
{
	unsigned int VersionValues;

	VersionValues = *(unsigned int*)VERSIONCHECKADDR_11;

	if (VersionValues == 0x016A016A)
		return 1;
	return 0;
}

int DetectVersion()
{
	if (TryToDetectFirstVer())
		return 1;
	if (TryToDetectSecondVer())
		return 2;
	return 0;
}

void StopWhiningQuestion(int StopWhiningValue)
{
	if (MessageBox(0, STOPWHININGTEXT, MULTIFIXCAPTION, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		CIniReader inireader("");
		inireader.WriteInteger("MultiFix", "StopWhining", StopWhiningValue);
	}
}

BOOL UnknownVersionQuestion()
{
	switch (StopWhining)
	{
	case 1:
		return FALSE;
		break;
	case 2:
		WriteFirstVerAddresses();
	case 3:
		Init();
		return TRUE;
	default:
		if (MessageBox(0, UNKNOWNVERSIONTEXT, MULTIFIXCAPTION, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
		{
			StopWhiningQuestion(1);
			return FALSE;
		}
		else if (MessageBox(0, OLDVERSIONTEXT, MULTIFIXCAPTION, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			StopWhiningQuestion(2);
			WriteFirstVerAddresses();
		}
		else
			StopWhiningQuestion(3);
		Init();
		return TRUE;
	}
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
		freopen("CON", "r", stdin);

		InitConfig();

		switch (DetectVersion())
		{
		case 1:
			WriteFirstVerAddresses();
		case 2:
			Init();
			break;
		default:
			return UnknownVersionQuestion();
		}
	}
	return TRUE;
}
