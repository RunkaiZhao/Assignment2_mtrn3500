#include "SMObject.h"
#include <SMStructs.h>
#include <conio.h>
#include <tlhelp32.h>
#include <iostream>

using namespace std;
using namespace System;
using namespace System::Threading;

TCHAR* Units[10] =
{
	TEXT("LaserModule.exe"),
	TEXT("GPSModule.exe"),
	//TEXT("XBoxModule.exe"),
	//TEXT("VehicleControlModule.exe"),
	//TEXT("OpenGL.exe"),
	//TEXT("DisplayModule.exe"),
};

bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp(entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}

int main() {
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	SMObject GPSObj(_TEXT("GPSObj"), sizeof(GPS));
	SMObject LaserObj(_TEXT("LaserObj"), sizeof(Laser));

	GPS* GPSPtr = nullptr;
	PM* PMSMPtr = nullptr;

	//share memory creation
	PMObj.SMCreate();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory creation failed.");
		return -1;
	}
	GPSObj.SMCreate();
	if (GPSObj.SMAccessError) {
		Console::WriteLine("Shared memory creation failed.");
		return -1;
	}
	LaserObj.SMCreate();
	if (LaserObj.SMAccessError) {
		Console::WriteLine("Shared memory creation failed.");
		return -1;
	}

	//share memory access
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed.");
		return -2;
	}
	GPSObj.SMAccess();
	if (GPSObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed.");
		Console::ReadKey();
		return -2;
	}
	LaserObj.SMAccess();
	if (LaserObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed.");
		Console::ReadKey();
		return -2;
	}

	// Module execution based variable declarations
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	// Starting the processes
	for (int i = 0; i < 2; i++)
	{
		// Check if each process is running
		if (!IsProcessRunning(Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));
			// Start the child processes.

			if (!CreateProcess(NULL,   // No module name (use command line)
				Units[i],        // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory
				&s[i],            // Pointer to STARTUPINFO structure
				&p[i])           // Pointer to PROCESS_INFORMATION structure
				)
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
				return -1;
			}
		}
		std::cout << "Started: " << Units[i] << std::endl;
		Sleep(1000);
	}

	GPSPtr = (GPS*)PMObj.pData;
	PMSMPtr = (PM*)PMObj.pData;
	PMSMPtr->Shutdown.Flags.PM = 0;
	PMSMPtr->Heartbeats.Status = 0x00; // individual module should set flag up if they are awake.
	int waitTime_GPS = 0;
	int waitTime_laser = 0;

	while (!PMSMPtr->Shutdown.Flags.PM) {
		
		PMSMPtr->PMHeartbeats.Flags.GPS = 1;
		PMSMPtr->PMHeartbeats.Flags.Laser = 1;
		//checking GPS critical processor
		Console::WriteLine("GPS heartbeat is {0}.", PMSMPtr->Heartbeats.Flags.GPS);
		Console::WriteLine("PM heartbeat is {0}.", PMSMPtr->Heartbeats.Flags.PM);
		Console::WriteLine("Laser heartbeat is {0}.", PMSMPtr->Heartbeats.Flags.Laser);
		//checking GPS
		if (PMSMPtr->Heartbeats.Flags.GPS == 1)
			PMSMPtr->Heartbeats.Flags.GPS = 0; // set it down by PM
		else
		{	
			if (++waitTime_GPS > 30)
				PMSMPtr->Shutdown.Status = 0xFF;
		}

		//checking Laser
		if (PMSMPtr->Heartbeats.Flags.Laser == 1)
			PMSMPtr->Heartbeats.Flags.Laser = 0;
		else {
			//if (++waitTime_laser > 30)
				//PMSMPtr->Shutdown.Status = 0xFF;
		}
			
		if (_kbhit()) break;
		System::Threading::Thread::Sleep(100);
	}
	

	System::Threading::Thread::Sleep(3000);
	return 0;
}