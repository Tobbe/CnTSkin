#include <windows.h>
#include <sstream>
#include <string>
#include <map>
#include "buttonids.h"
#include <Gdiplus.h>

LONG OldWndProc;
LONG OldWndProc2;
HBRUSH hColorBrush;
int count = 0;
bool countPaintForm = true;
bool countPaintFrame = true;
bool countEraseFrame = true;
bool countEraseForm = true;
HDC hdc = NULL;
HBITMAP oldBmp;
HBITMAP bmp = NULL;
ULONG_PTR gdiplusToken;

class Key {
private:
	std::string text;
	std::string imagePath;
public:
	Key(const std::string &text, const std::string &imageID);
	bool hasImage() const;
	std::string getText() const;
	std::string getImagePath() const;
};

Key::Key(const std::string &text, const std::string &imageID) {
	this->text = text;
	this->imagePath = "C:\\Program\\Click-N-Type\\LCG_Swedish\\K-" + imageID + ".gif";
	DWORD a = GetFileAttributes(imagePath.c_str());
	if (a != (DWORD)-1 && (a & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		//File exists
	} else {
		imagePath = "";
	}
}

bool Key::hasImage() const {
	return imagePath.size() != 0;
}

std::string Key::getText() const {
	return text;
}

std::string Key::getImagePath() const {
	return imagePath;
}

std::map<int, Key> keys;

LRESULT CALLBACK NewWndProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message) {
		case WM_COMMAND:
			switch(wParam) {
				case 2000:
					MessageBox(HWND_DESKTOP, "You pressed our new menu button!", "Yay!", MB_OK);
					break;
			}
			break;
	}

	return CallWindowProc((WNDPROC)OldWndProc, Hwnd, Message, wParam, lParam);
}

LRESULT CALLBACK NewWndProc2(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message) {
		case WM_COMMAND:
			switch(wParam) {
				case 2000:
					MessageBox(HWND_DESKTOP, "You pressed our new menu button!", "Yay!", MB_OK);
					break;
			}
			break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(Hwnd, &ps);
			HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));

			FillRect(ps.hdc, &ps.rcPaint, brush);

			DeleteObject(brush);
			EndPaint(Hwnd, &ps);

			return 0;
		}
		case WM_DRAWITEM: {
			std::map<int, Key>::iterator it = keys.find((UINT)wParam);
			if (it == keys.end()) {
				break;
			}

			LPDRAWITEMSTRUCT lpdis = (DRAWITEMSTRUCT*)lParam;

			RECT btnRect;
			GetWindowRect(lpdis->hwndItem, &btnRect);
			int btnWidth = btnRect.right - btnRect.left;
			int btnHeight = btnRect.bottom - btnRect.top;

			std::string text = it->second.getText();
			SIZE size;
			GetTextExtentPoint32(lpdis->hDC, text.c_str(), text.length(), &size);
			SetTextColor(lpdis->hDC, RGB(0, 0, 0));
			SetBkColor(lpdis->hDC, RGB(220, 220, 255));

			HBRUSH hOldBrush = (HBRUSH)SelectObject(lpdis->hDC, CreateSolidBrush(RGB(220, 220, 255)));
			HPEN hOldPen = (HPEN)SelectObject(lpdis->hDC, CreatePen(PS_SOLID, 3, RGB(255, 255, 255)));

			Rectangle(lpdis->hDC, 0, 0, btnWidth, btnHeight);
			SetPixel(lpdis->hDC, 2, 2, RGB(255, 255, 255));
			SetPixel(lpdis->hDC, btnWidth - 3, 2, RGB(255, 255, 255));
			SetPixel(lpdis->hDC, 2, btnHeight - 3, RGB(255, 255, 255));
			SetPixel(lpdis->hDC, btnWidth - 3, btnHeight - 3, RGB(255, 255, 255));

			hOldPen = (HPEN)SelectObject(lpdis->hDC, CreatePen(PS_SOLID, 1, RGB(0, 0, 255)));
			hOldBrush = (HBRUSH)SelectObject(lpdis->hDC, GetStockObject(HOLLOW_BRUSH));
			RoundRect(lpdis->hDC, 1, 1, btnWidth - 1, btnHeight - 1, 10, 10);

			SelectObject(lpdis->hDC, hOldPen);
			SelectObject(lpdis->hDC, hOldBrush);

			if (!it->second.hasImage()) {
				ExtTextOut(lpdis->hDC,
				((lpdis->rcItem.right - lpdis->rcItem.left) - size.cx) / 2,
				((lpdis->rcItem.bottom - lpdis->rcItem.top) - size.cy) / 2,
				ETO_CLIPPED, &lpdis->rcItem, text.c_str(), text.length(), NULL);
			} else {
				std::string path = it->second.getImagePath();
				std::wstring wpath(path.length(), L' '); // Make room for characters
				std::copy(path.begin(), path.end(), wpath.begin());

				Gdiplus::Image m_pPicture(wpath.c_str());
				Gdiplus::Graphics graphics(lpdis->hDC);

				int picWidth = m_pPicture.GetWidth();
				int picHeight = m_pPicture.GetHeight();
				int x = max(0, (btnWidth - picWidth)/2);
				int y = max(0, (btnHeight - picHeight)/2);
				int width = min(picWidth, btnWidth);
				int height = min(picHeight, btnHeight);
				graphics.DrawImage(&m_pPicture, x, y, width, height);
			}

			return TRUE;
		}
	}

	return CallWindowProc((WNDPROC)OldWndProc2, Hwnd, Message, wParam, lParam);
}

DWORD WINAPI Creation(LPVOID)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL);

	keys.insert(std::pair<int, Key>(BTNID_ESC, Key("Esc", "24")));
	keys.insert(std::pair<int, Key>(BTNID_F1, Key("F1", "32")));
	keys.insert(std::pair<int, Key>(BTNID_F2, Key("F2", "33")));
	keys.insert(std::pair<int, Key>(BTNID_F3, Key("F3", "34")));
	keys.insert(std::pair<int, Key>(BTNID_F4, Key("F4", "35")));
	keys.insert(std::pair<int, Key>(BTNID_F5, Key("F5", "36")));
	keys.insert(std::pair<int, Key>(BTNID_F6, Key("F6", "37")));
	keys.insert(std::pair<int, Key>(BTNID_F7, Key("F7", "38")));
	keys.insert(std::pair<int, Key>(BTNID_F8, Key("F8", "39")));
	keys.insert(std::pair<int, Key>(BTNID_F9, Key("F9", "40")));
	keys.insert(std::pair<int, Key>(BTNID_F10, Key("F10", "41")));
	keys.insert(std::pair<int, Key>(BTNID_F11, Key("F11", "42")));
	keys.insert(std::pair<int, Key>(BTNID_F12, Key("F12", "43")));
	keys.insert(std::pair<int, Key>(BTNID_1, Key("1", "1")));
	keys.insert(std::pair<int, Key>(BTNID_2, Key("2", "2")));
	keys.insert(std::pair<int, Key>(BTNID_3, Key("3", "3")));
	keys.insert(std::pair<int, Key>(BTNID_4, Key("4", "4")));
	keys.insert(std::pair<int, Key>(BTNID_5, Key("5", "5")));
	keys.insert(std::pair<int, Key>(BTNID_6, Key("6", "6")));
	keys.insert(std::pair<int, Key>(BTNID_7, Key("7", "7")));
	keys.insert(std::pair<int, Key>(BTNID_8, Key("8", "8")));
	keys.insert(std::pair<int, Key>(BTNID_9, Key("9", "9")));
	keys.insert(std::pair<int, Key>(BTNID_0, Key("0", "10")));
	keys.insert(std::pair<int, Key>(BTNID_A, Key("A", "A")));
	keys.insert(std::pair<int, Key>(BTNID_B, Key("B", "B")));
	keys.insert(std::pair<int, Key>(BTNID_C, Key("C", "C")));
	keys.insert(std::pair<int, Key>(BTNID_D, Key("D", "D")));
	keys.insert(std::pair<int, Key>(BTNID_E, Key("E", "E")));
	keys.insert(std::pair<int, Key>(BTNID_F, Key("F", "F")));
	keys.insert(std::pair<int, Key>(BTNID_G, Key("G", "G")));
	keys.insert(std::pair<int, Key>(BTNID_H, Key("H", "H")));
	keys.insert(std::pair<int, Key>(BTNID_I, Key("I", "I")));
	keys.insert(std::pair<int, Key>(BTNID_J, Key("J", "J")));
	keys.insert(std::pair<int, Key>(BTNID_K, Key("K", "K")));
	keys.insert(std::pair<int, Key>(BTNID_L, Key("L", "L")));
	keys.insert(std::pair<int, Key>(BTNID_M, Key("M", "M")));
	keys.insert(std::pair<int, Key>(BTNID_N, Key("N", "N")));
	keys.insert(std::pair<int, Key>(BTNID_O, Key("O", "O")));
	keys.insert(std::pair<int, Key>(BTNID_P, Key("P", "P")));
	keys.insert(std::pair<int, Key>(BTNID_Q, Key("Q", "Q")));
	keys.insert(std::pair<int, Key>(BTNID_R, Key("R", "R")));
	keys.insert(std::pair<int, Key>(BTNID_S, Key("S", "S")));
	keys.insert(std::pair<int, Key>(BTNID_T, Key("T", "T")));
	keys.insert(std::pair<int, Key>(BTNID_U, Key("U", "U")));
	keys.insert(std::pair<int, Key>(BTNID_V, Key("V", "V")));
	keys.insert(std::pair<int, Key>(BTNID_W, Key("W", "W")));
	keys.insert(std::pair<int, Key>(BTNID_X, Key("X", "X")));
	keys.insert(std::pair<int, Key>(BTNID_Y, Key("Y", "Y")));
	keys.insert(std::pair<int, Key>(BTNID_Z, Key("Z", "Z")));
	keys.insert(std::pair<int, Key>(BTNID_ALTGR, Key("AltGr", "57")));
	keys.insert(std::pair<int, Key>(BTNID_SPACE, Key(" ", "19")));
	keys.insert(std::pair<int, Key>(BTNID_AA, Key("[", "14")));
	keys.insert(std::pair<int, Key>(BTNID_AE, Key("'", "18")));
	keys.insert(std::pair<int, Key>(BTNID_OE, Key("));", "17")));
	keys.insert(std::pair<int, Key>(BTNID_PARAG, Key("`", "0")));
	keys.insert(std::pair<int, Key>(BTNID_PLUS, Key("-", "11")));
	keys.insert(std::pair<int, Key>(BTNID_TICK, Key("=", "12")));
	keys.insert(std::pair<int, Key>(BTNID_BKSPACE, Key("<==", "13")));
	keys.insert(std::pair<int, Key>(BTNID_DEL, Key("Del", "25")));
	keys.insert(std::pair<int, Key>(BTNID_UML, Key("]", "15")));
	keys.insert(std::pair<int, Key>(BTNID_APO, Key("\\", "16")));
	keys.insert(std::pair<int, Key>(BTNID_HOME, Key("Home", "49")));
	keys.insert(std::pair<int, Key>(BTNID_END, Key("End", "50")));
	keys.insert(std::pair<int, Key>(BTNID_MENU, Key("Menu", "61")));
	keys.insert(std::pair<int, Key>(BTNID_LEFT, Key("<-", "44")));
	keys.insert(std::pair<int, Key>(BTNID_RIGHT, Key("->", "45")));
	keys.insert(std::pair<int, Key>(BTNID_UP, Key("^", "46")));
	keys.insert(std::pair<int, Key>(BTNID_DOWN, Key("v", "47")));
	keys.insert(std::pair<int, Key>(BTNID_ENTER, Key("<-|", "23")));
	keys.insert(std::pair<int, Key>(BTNID_CTRL, Key("Ctrl", "28")));
	keys.insert(std::pair<int, Key>(BTNID_SHIFT, Key("Shift", "27")));
	keys.insert(std::pair<int, Key>(BTNID_TAB, Key("Tab", "26")));
	keys.insert(std::pair<int, Key>(BTNID_WIN, Key("Win", "59")));
	keys.insert(std::pair<int, Key>(BTNID_ALT, Key("Alt", "29")));
	keys.insert(std::pair<int, Key>(BTNID_TAG, Key("Meta", "81")));
	keys.insert(std::pair<int, Key>(BTNID_COMMA, Key(",", "20")));
	keys.insert(std::pair<int, Key>(BTNID_PERIOD, Key(".", "21")));
	keys.insert(std::pair<int, Key>(BTNID_DASH, Key("/", "22")));

	int tries = 0;
	HWND hWnd = NULL;
	do {
		tries++;
		hWnd = FindWindow("ThunderRT6FormDC", NULL);
		Sleep(1);
	} while (hWnd == NULL && tries < 1000);

	if (hWnd == NULL) {
		MessageBox(NULL, "Could NOT find the Click-N-Type window", "FindWindow", MB_OK | MB_ICONERROR);
	}

	tries = 0;
	HWND hWndFrame = NULL;
	do {
		tries++;
		hWndFrame = FindWindowEx(hWnd, NULL, "ThunderRT6Frame", NULL);
		Sleep(1);
	} while (hWndFrame == NULL && tries < 1000);

	if (hWndFrame == NULL) {
		MessageBox(NULL, "Could NOT find the Click-N-Type frame", "FindWindow", MB_OK | MB_ICONERROR);
	}

	tries = 0;
	HMENU hCurrent = NULL;
	do {
		tries++;
		hCurrent = GetMenu(hWnd);
		Sleep(1);
	} while (hCurrent == NULL && tries < 1000);

	if (hCurrent == NULL) {
		MessageBox(NULL, "Could NOT find the Click-N-Type menu", "GetMenu", MB_OK | MB_ICONERROR);
	}

	DeleteMenu(hCurrent, 0, MF_BYPOSITION);
	DeleteMenu(hCurrent, 0, MF_BYPOSITION);
	DeleteMenu(hCurrent, 0, MF_BYPOSITION);
	DeleteMenu(hCurrent, 0, MF_BYPOSITION);
	DeleteMenu(hCurrent, 0, MF_BYPOSITION);
	DeleteMenu(hCurrent, 0, MF_BYPOSITION);
	DrawMenuBar(hWnd);

	OldWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (long)NewWndProc); //subclass the original window procedure
	OldWndProc2 = SetWindowLong(hWndFrame, GWL_WNDPROC, (long)NewWndProc2);

	InvalidateRect (hWnd, NULL, TRUE);
	UpdateWindow (hWnd);
	InvalidateRect (hWndFrame, NULL, TRUE);
	UpdateWindow (hWndFrame);

	return TRUE;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		CreateThread(0, NULL, (LPTHREAD_START_ROUTINE)&Creation, NULL, NULL, NULL);
	} else if (fdwReason == DLL_PROCESS_DETACH) {
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}

	return TRUE;
}