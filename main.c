#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

// Prints the message for the error code
void PrintLastError(DWORD err) {
	LPWSTR message;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 1024, NULL);
	int utf8_str_length = WideCharToMultiByte(CP_UTF8, 0, message, -1, NULL, 0, NULL, NULL);
	char utf8_message[utf8_str_length];
	WideCharToMultiByte(CP_UTF8, 0, message, -1, utf8_message, utf8_str_length, NULL, NULL);
	LocalFree(message);
	printf("error %u: %s\n", err, utf8_message);
}

// For the EnumWindows function. Continues search until the end, prints handles of the windows with matching names
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM target_window_title) {
	static wchar_t this_window_title[512];
	// succcessfully got window title and it matched
	if (GetWindowTextW(hWnd, this_window_title, ARRAYSIZE(this_window_title)) && !wcscmp(this_window_title, (wchar_t*)target_window_title)) {
		wprintf(L"%08X %ls\n", hWnd, this_window_title);
	}
	return TRUE;
}

int main() {
	HWND window;
	RECT window_rect;
	wchar_t window_title[512];
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	printf("Enter the title of the desired window. Leave blank to specify its handle: ");
	_getws_s(window_title, ARRAYSIZE(window_title));
	if (wcslen(window_title)) {
		puts("Matching windows:\n"
			 "Handle   Title");
		//	  01234567 title
		EnumWindows(EnumWindowsProc, (LPARAM)window_title);
	}
	printf("Handle to the target window (hex): ");
	scanf("%X", &window);
	BOOL success = GetWindowRect(window, &window_rect);
	if (success) {
		printf(
			"Window rectangle: LT (%d, %d) - RB (%d, %d), size: %dx%d\n",
			window_rect.left,
			window_rect.top,
			window_rect.right,
			window_rect.bottom,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top
		);
	}
	else {
		printf("Cannot get the window's rectangle: ");
		PrintLastError(GetLastError());
	}
	bool move_window = MessageBoxA(NULL, "Move the window?", "Confirmation", MB_YESNO) == IDYES;
	bool resize_window = MessageBoxA(NULL, "Resize the window?", "Confirmation", MB_YESNO) == IDYES;
	if (!move_window && !resize_window) {
		puts("Exiting without doing anything.");
		return 0;
	}
	if (move_window || !success) {
		printf("New window position (left-X top-Y): ");
		scanf("%d%d", &window_rect.left, &window_rect.top);
	}
	int width, height;
	if (resize_window) {
		printf("New window size (width height): ");
		scanf("%d%d", &width, &height);
	}
	else {
		width = window_rect.right - window_rect.left;
		height = window_rect.bottom - window_rect.top;
	}
	puts("Trying the SetWindowPos function.");
	success = SetWindowPos(window, NULL, window_rect.left, window_rect.top, width, height, SWP_NOZORDER | (move_window ? 0 : SWP_NOMOVE) | (resize_window ? 0 : SWP_NOSIZE));
	if (success) {
		puts("Window position set.");
		return 0;
	}
	PrintLastError(GetLastError());

	puts("Trying the MoveWindow function.");
	if (MoveWindow(window, window_rect.left, window_rect.top, width, height, TRUE)) {
		puts("Window position set.");
		return 0;
	}
	PrintLastError(GetLastError());

	puts("Trying to maximize the window with ShowWindow function.");
	ShowWindow(window, SW_SHOWMAXIMIZED);
	DWORD err = GetLastError();
	if (!err) {
		puts("Window maximized.");
		return 0;
	}
	PrintLastError(err);
	return 1;
}