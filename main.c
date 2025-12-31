#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

void PrintLastError(DWORD err) {
	LPWSTR message;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&message, 1024, NULL);
	int utf8_str_length = WideCharToMultiByte(CP_UTF8, 0, message, -1, NULL, 0, NULL, NULL);
	char utf8_message[utf8_str_length];
	WideCharToMultiByte(CP_UTF8, 0, message, -1, utf8_message, utf8_str_length, NULL, NULL);
	LocalFree(message);
	printf("error %u: %s\n", err, utf8_message);
}

int main() {
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	HWND window;
	RECT window_rect;
	printf("Handle to the target window (hex): ");
	scanf("%X", &window);
	BOOL success = GetWindowRect(window, &window_rect);
	if (success) {
		printf(
			"Window rectangle: (%d, %d) - (%d, %d) (%dx%d)\n",
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
	printf("Do you want to move the window? ");
	char response[4];
	gets_s(response, 4);
	bool move_window = !strcmp(response, "y") || !strcmp(response, "yes");
	
	if (move_window || !success) {
		printf("New window position (left-X top-Y): ");
		scanf("%d%d", &window_rect.left, &window_rect.top);
	}	
	int width, height;
	printf("New window size (width height): ");
	scanf("%d%d", &width, &height);
	
	puts("Trying SetWindowPos.");
	success = SetWindowPos(window, NULL, window_rect.left, window_rect.top, width, height, SWP_NOZORDER | (move_window ? 0 : SWP_NOMOVE));
	if (success) {
		puts("Window position set.");
		return 0;
	}
	PrintLastError(GetLastError());
	puts("Trying MoveWindow.");
	if (MoveWindow(window, window_rect.left, window_rect.top, width, height, TRUE)) {
		puts("Window position set.");
		return 0;
	}
	PrintLastError(GetLastError());
	puts("Trying to maximize the window with ShowWindow.");
	ShowWindow(window, SW_SHOWMAXIMIZED);
	DWORD err = GetLastError();
	if (!err) {
		puts("Window maximized.");
		return 0;
	}
	PrintLastError(err);
	return 1;
}