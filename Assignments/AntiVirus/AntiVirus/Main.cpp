// Developed in Visual Studio 2019.
// Szymon Janusz G20792986 & Ben Dingwall G20792216

#include <iostream> // Output strings to console
#include <windows.h> // API to interact with windows registry
#include <string> // Adds string class (array of chars)
#include <tchar.h> // Generic text mappings to compile code to multiple formats (single byte, multi byte, unicode)
#include <tlhelp32.h> // Get process snapshots and handles to processes, to be able to close them

#define MAX_VALUE_NAME 16383 // Max length of values in key
#define SLEEP_TIME 100 // How long to sleep for in milliseconds when killing the virus

using namespace std;

// Get the Process ID (PID) of a process from process snapshot
void GetPID(PROCESSENTRY32& pe32, const HANDLE& hProcessSnapshot)
{
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// If snapshot failed
	if (!Process32First(hProcessSnapshot, &pe32))
	{
		// Show error and clean the snapshot
		cout << "Process32First failed." << endl;
		CloseHandle(hProcessSnapshot);
		std::system("pause");
		std::exit(0);
	}
}

// Iterate over current running processes.
// If a process is the virus, return and change processFound.
void IterateOverProcesses(PROCESSENTRY32& pe32, const TCHAR kVirusName[], bool& processFound, HANDLE& hProcessSnapshot)
{
	// Go through all the currently running processes
	do
	{
		// Check if the virus file with .exe extension exists or the virus file without extension exists.
		const TCHAR kVirusEXE[] = "CrazyVirus.exe";
		if (_tcscmp(pe32.szExeFile, kVirusName) == 0 || _tcscmp(pe32.szExeFile, kVirusEXE) == 0)
		{
			cout << "Virus process found." << endl;
			processFound = true;
			return;
		}
	} while (Process32Next(hProcessSnapshot, &pe32));
}

// Create a process snapshot
void CreateSnapshot(HANDLE& handle)
{
	// Create process snapshot
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// If process snapshot failed
	if (handle == INVALID_HANDLE_VALUE)
	{
		// Show error and quit.
		cout << "Process snapshot failed." << endl;
		std::system("pause");
		std::exit(0);
	}
}

// Check if the current value name or value data contains the virus name.
bool IsVirus(const TCHAR kValueName[], const TCHAR kValueData[], const TCHAR kVirusName[])
{
	bool isVirus = false;
	// Check if value name or data contains virus name.
	// _tcscmp is used to compare TCHAR values
	(_tcscmp(kValueName, kVirusName) == 0 || _tcscmp(kValueData, kVirusName) == 0) ? isVirus = true : isVirus = false;
	return isVirus;
}

// Kill the virus by getting a handle to process and terminating it.
void KillVirus(HANDLE& hProcessSnapshot, const PROCESSENTRY32& pe32)
{
	// Get a handle on the process with the intention to terminate it
	hProcessSnapshot = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
	const UINT successfulExit = 1;
	if (TerminateProcess(hProcessSnapshot, successfulExit))
	{
		cout << "Virus process terminated succesfully." << endl;
	}
	else
	{
		cout << "Virus process failed to terminate." << endl;
	}
	
	CloseHandle(hProcessSnapshot); // Close the handle to let the system delete the file.
	Sleep(SLEEP_TIME); // Sleep to let the process shut down, else the file won't be deleted.
}

// Iterate over the values in current key and list the value data and value name. Also Scans the values to see if they contain the virus
// If the value contains the virus, remedies the virus and removes it.
void ListAndScan(const DWORD& valueNameLengthMax, const DWORD& valueDataLengthMax, const HKEY& RunKey,
					const DWORD& numOfValues, const TCHAR kVirusName[])
{
	// Create variables

	TCHAR valueName[MAX_VALUE_NAME]; // Value name
	TCHAR valueData[MAX_PATH]; // Value data
	DWORD valueNameLength; // Length of name of current value
	DWORD valueDataLength; // Length of data of current value

	// Iterate over values and list the value data.
	for (int i = 0; i < static_cast<int>(numOfValues); i++)
	{
		valueNameLength = valueNameLengthMax;
		valueDataLength = valueDataLengthMax;

		// Get the value data from value.
		RegEnumValue(RunKey, i, valueName, &valueNameLength, NULL, NULL, LPBYTE(valueData), &valueDataLength);

		// Use wcout for TCHAR arrays.
		wcout << valueName << " : " << valueData << endl;

		// Check if value name or data contains virus name.
		if (IsVirus(valueName, valueData, kVirusName))
		{
			cout << "Virus found!" << endl;
			wcout << valueName << "    " << valueData << endl;

			// Delete the value from registry
			RegDeleteValue(RunKey, valueName);

			// Handle to process.
			HANDLE hProcessSnapshot;
			CreateSnapshot(hProcessSnapshot);

			// An entry from a list of processes from the snapshot
			PROCESSENTRY32 pe32;
			// Get the Process ID from the snapshot
			GetPID(pe32, hProcessSnapshot);

			// Is the virus process found running?
			bool processFound = false;
			// Iterate over the currently running processes, and see if any are equal to the virus.
			IterateOverProcesses(pe32, kVirusName, processFound, hProcessSnapshot);

			// Stop the process if it's found
			if (processFound)
			{
				KillVirus(hProcessSnapshot, pe32);
			}
			// Delete the file from the system.
			if (remove(valueData) == 0)
			{
				cout << "Virus deleted successfully." << endl;
			}
			else
			{
				cout << "Virus deletion failed." << endl;
			}
		}
	}
}

int main()
{
	// The workspace where the antivirus will look
	const HKEY kWorkspaceKey = HKEY_CURRENT_USER;
	
	HKEY RunKey; // Where to look for the virus

	// The directory to scan for the virus
	const char kDirectoryToScan[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

	// Check if user cannot access the directory by trying to open the run key in the directory
	if (RegOpenKey(kWorkspaceKey, kDirectoryToScan, &RunKey) != ERROR_SUCCESS)
	{
		// Display error message
		string failMsg = "Antivirus cannot access ";
		cout << failMsg << kDirectoryToScan << endl;
	}
	else
	{
		DWORD numOfValues; // How many values are there in the current key.

		DWORD valueNameLengthMax; // Max length of name of current value
		DWORD valueDataLengthMax; // Max length of data of current value

		const TCHAR kVirusName[] = "CrazyVirus"; // The virus name we are looking for.

		// Get number of values in current key
		RegQueryInfoKey(RunKey, NULL, NULL, NULL, NULL, NULL, NULL,
			&numOfValues, &valueNameLengthMax, &valueDataLengthMax, NULL, NULL);

		cout << "Number of values in directory: " << numOfValues << endl;

		// If RegQueryInfoKey succeeded
		if (numOfValues)
		{
			ListAndScan(valueNameLengthMax, valueDataLengthMax, RunKey, numOfValues, kVirusName);
		}
		// Close the key to save the registry, otherwise changes would be reversed.
		RegCloseKey(RunKey);
	}

	std::system("pause");
	std::exit(0);
}