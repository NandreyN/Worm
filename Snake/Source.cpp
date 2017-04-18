#undef UNICODE
#define MOVETIMER 1
#include <windows.h>
#include  <math.h>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <ctime>
#include <numeric>
#include <map>
#include <queue>
#include <set>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
static deque<POINT> _worm; // Head of the worm is stored as 1st
					   // By default x = -1 , y = -1 

static int _dimension;
static int _wormLength;
static POINT _oldTail;
static bool _proceeds;

bool isInRange(int dim, int x, int y);
void DrawBG(HDC& hdc, int dimension, int x, int y);
void DrawWorm(HDC& hdc, int dimension, int x, int y);
BOOL InitApplication(HINSTANCE hinstance);
BOOL InitInstance(HINSTANCE hinstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prevHinstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	srand(time(NULL));
	if (!InitApplication(hinstance))
	{
		MessageBox(NULL, "Unable to Init App", "Error", MB_OK);
		return FALSE;
	}

	if (!InitInstance(hinstance, nCmdShow))
	{
		MessageBox(NULL, "Unable to Init Instance", "Error", MB_OK);
		return FALSE;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

bool isInRange(int dim, int x, int y)
{
	return (x >= 0 && x <= dim - 1 && y >= 0 && y <= dim -1 ) ? true : false;
}

void DrawBG(HDC& hdc, int dimension, int x, int y)
{
	double X = (double)x / dimension, Y = (double)y / dimension;
	// Draw horizontal lines
	for (int i = 0; i < dimension; i++)
	{
		MoveToEx(hdc, 0, i*Y, NULL);
		LineTo(hdc, x, i * Y);
	}
	// vertical 
	for (int i = 0; i < dimension; i++)
	{
		MoveToEx(hdc, X*i, 0, NULL);
		LineTo(hdc, X*i, y);
	}
}

inline bool operator==(const RECT& lhs, const RECT& rhs)
{
	return (lhs.right == rhs.right && lhs.left == rhs.left && lhs.top == rhs.top && lhs.bottom == rhs.bottom)?true:false;
}
void DrawWorm(HDC& hdc, int dimension, int x, int y)
{
	static vector<RECT> old;
	static bool firstLaunch = true;

	int i = 1;
	double X = (double)x / dimension, Y = (double)y / dimension;

	for_each(old.begin(), old.end(), [&hdc](RECT rect)
	{
		FillRect(hdc, &rect, CreateSolidBrush(RGB(252, 168, 168)));
	});

	for_each(_worm.begin(), _worm.end(), [X,Y,&hdc, &i](POINT pt)
	{
		int left = pt.x * X, top = pt.y * Y, right = (pt.x + 1) * X, bottom = (pt.y + 1)*Y;
		RECT r; SetRect(&r, left, top, right, bottom);
		FillRect(hdc, &r, CreateSolidBrush(RGB(200, 20, 20))); // Fix memory allocation at each iteration
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, to_string(i).data(), to_string(i).size(), &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		++i;
	});
	int left = _oldTail.x * X, top = _oldTail.y * Y, right = (_oldTail.x + 1) * X, bottom = (_oldTail.y + 1)*Y;
	RECT r; SetRect(&r, left, top, right, bottom);
	if (firstLaunch)
	{
		firstLaunch = false;
		return;
	}
	old.push_back(r);
	unique(old.begin(), old.end());
	FillRect(hdc, &r, CreateSolidBrush(RGB(252, 168, 168)));
}

BOOL InitApplication(HINSTANCE hinstance)
{
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hinstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_CROSS);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "Snake";
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, "Cannot register class", "Error", MB_OK);
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int x, y;
	
	static HDC hdc;
	PAINTSTRUCT ps;
	switch (message)
	{
	case WM_CREATE:
		_proceeds = true;
		srand(time(NULL));
		_dimension = 11;
		_wormLength = 10;
		POINT head; head.x = _dimension / 2; head.y = _dimension / 2;;
		_worm.push_back(head);
		SetTimer(hwnd, MOVETIMER, 200, NULL);
		break;

	case WM_SIZE:
		x = LOWORD(lparam);
		y = HIWORD(lparam);
		break;
	case WM_TIMER:
	{
		// Here program should find possible directions for moving
		vector<vector<bool>> boolField(_dimension);
		vector<char> directions;

		for (int i = 0; i < _dimension; i++)
			boolField[i] = vector<bool>(_dimension);

		for_each(_worm.begin(), _worm.end(), [&boolField](POINT pt)
		{
			boolField[pt.x][pt.y] = true;
		});

		int hx = _worm[0].x;
		int hy = _worm[0].y;

		if (isInRange(_dimension, hx + 1, hy) && !boolField[hx + 1][hy]) directions.push_back('r');
		if (isInRange(_dimension, hx - 1, hy) && !boolField[hx - 1][hy]) directions.push_back('l');
		if (isInRange(_dimension, hx, hy - 1) && !boolField[hx][hy - 1]) directions.push_back('u');
		if (isInRange(_dimension, hx, hy + 1) && !boolField[hx][hy + 1]) directions.push_back('d');
		
		if (directions.size() == 0)
		{
			KillTimer(hwnd, MOVETIMER);
			_proceeds = false;
			MessageBox(NULL, "End", "end", MB_OK);
			return TRUE;
		}
		
		int t = directions.size();
		t = rand() % t;
		POINT  oldHead = _worm[0], newHead = oldHead;
			_oldTail = _worm[_worm.size() - 1];

		_worm.pop_back();
		// add new head
		char sign = directions[t];
		switch(sign)
		{
		case 'r':
			newHead.x++;
			break;
		case 'l':
			newHead.x--;
			break;
		case 'u':
			newHead.y--;
			break;
		case 'd':
			newHead.y++;
			break;
		}
		_worm.push_front(newHead);
		if (_worm.size() < _wormLength)
			_worm.push_back(_oldTail);
		InvalidateRect(hwnd, NULL, true);
	}
	break;

	case WM_PAINT:
		if (!_proceeds) return FALSE;
		hdc = BeginPaint(hwnd, &ps);
		DrawWorm(hdc, _dimension, x, y);
		DrawBG(hdc, _dimension, x, y);
		EndPaint(hwnd, &ps);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		KillTimer(hwnd, MOVETIMER);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return FALSE;
}


BOOL InitInstance(HINSTANCE hinstance, int nCmdShow)
{
	HWND hwnd;
	hwnd = CreateWindow(
		"Snake",
		"Snake",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		700,
		700,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!hwnd)
		return FALSE;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	return TRUE;
}
