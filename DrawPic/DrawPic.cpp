// DrawPic.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "DrawPic.h"
#include "list"
#include "windowsx.h"
#include"commdlg.h"
#include"process.h"

using namespace std;
#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

/*


*/
#define Head_Len 20
enum Draw_Type {
	Type_Line,
	Type_Rect,
	Type_Ellipse,
	Type_Num

};
Draw_Type g_curDrawType = Type_Line;
//当前画的图形类型

struct stPicInfo
{
	tagPOINT ptFirst;
	tagPOINT ptEnd;
	Draw_Type type;
};
list <stPicInfo> g_listPicInfo;
//所有图形保存
bool g_bIsMouseDown = false;

stPicInfo g_curDrawPicInfo; //当前鼠标绘制的图形

//当前正在画的图形的点
tagPOINT g_ptFirst;
tagPOINT g_ptEnd;

HWND g_hWnd;

//当前鼠标是否按下
void OnLButtonUp(LPARAM  lParam);
void OnLButtonDown(LPARAM  lParam);

void DrawThread(void *param);
void DrawAll();
void DrawPic(stPicInfo info, HDC hdc);

void SavePicFile();
void OpenPicFile();


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DRAWPIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAWPIC));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DRAWPIC));
wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DRAWPIC);
wcex.lpszClassName = szWindowClass;
wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	g_hWnd = hWnd;
	MoveWindow(hWnd, 400, 200, 600, 400, false);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	_beginthread(&DrawThread, 0, NULL);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_DrawLine:
		//	MessageBox(hWnd, L"画直线", L"提示", MB_OK);

			g_curDrawType = Type_Line;

			break;
		case ID_Save :
			SavePicFile();
			break;
		case ID_Open:
			OpenPicFile();
			break;

		case ID_DrawRect :
			g_curDrawType = Type_Rect;
			break;
		case ID_DrawEllipse:
			g_curDrawType = Type_Ellipse;
			break;

	//	case ID_DrawEllipse:
		//	break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		OnLButtonDown(lParam);
		break;
	case WM_LBUTTONUP:
		OnLButtonUp(lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void DrawThread(void *param)
{
	while (1)
	{
		DrawAll();
		Sleep(50);
	}


}


void DrawAll()
{
	HDC hDc = GetDC(g_hWnd);
	if (hDc == NULL)
	{
		return;
	}

	HDC dcMem = CreateCompatibleDC(hDc);
	HBITMAP bitmap = (HBITMAP)CreateCompatibleBitmap(hDc, 600, 400);
	SelectObject(dcMem, bitmap);

	//背景设为白色
	HBRUSH hBrushBG = CreateSolidBrush(RGB(255, 255, 255));

	RECT rect;
	rect.left = 0;
	rect.right = 600;
	rect.top = 0;
	rect.bottom = 400;

	FillRect(dcMem, &rect, hBrushBG);

	for (list<stPicInfo>::iterator it = g_listPicInfo.begin(); it != g_listPicInfo.end(); it++)
	{

		DrawPic(*it, dcMem);

	}

	if (g_bIsMouseDown)
	{
		//当前画的图形  画出来
		tagPOINT pt;
		GetCursorPos(&pt);
		ScreenToClient(g_hWnd, &pt); //转化 成窗口客户区坐标

		memcpy(&g_curDrawPicInfo.ptEnd, &pt, sizeof(tagPOINT));
		DrawPic(g_curDrawPicInfo, dcMem);
	}




	BitBlt(hDc, 0, 0, 600, 400, dcMem, 0, 0, SRCCOPY);
	ReleaseDC(g_hWnd,hDc);
	DeleteObject(dcMem);
	DeleteObject(bitmap);
}

void DrawPic(stPicInfo info, HDC hDc)
{
	switch (info.type)
	{
	case Type_Line:
		tagPOINT pt;
		MoveToEx(hDc, info.ptFirst.x, info.ptFirst.y, &pt);
		LineTo(hDc, info.ptEnd.x, info.ptEnd.y);
		break;
	case Type_Rect :
	{
		HGDIOBJ hBrush = GetStockBrush(NULL_BRUSH); //空画刷
	SelectObject(hDc, hBrush);
	Rectangle(hDc, info.ptFirst.x, info.ptFirst.y, info.ptEnd.x, info.ptEnd.y);
	DeleteObject(hBrush);

	}		break;
	case Type_Ellipse:
	{
		HGDIOBJ hBrush = GetStockBrush(NULL_BRUSH); //空画刷
		SelectObject(hDc, hBrush);
		Ellipse(hDc, info.ptFirst.x, info.ptFirst.y, info.ptEnd.x, info.ptEnd.y);
		DeleteObject(hBrush);

	}		break;
		break;
	default:
		break;
	}

}

void OnLButtonUp(LPARAM  lParam)
{

	if (g_bIsMouseDown == false)
	{
		return;
	}
	g_bIsMouseDown = false;
	g_ptEnd.x = GET_X_LPARAM(lParam);
	g_ptEnd.y = GET_Y_LPARAM(lParam);

	switch (g_curDrawType)
	{
	case Type_Line:
	{
		stPicInfo stInfo;
		stInfo.type = g_curDrawType;
		memcpy(&stInfo.ptFirst, &g_ptFirst, sizeof(tagPOINT));
		memcpy(&stInfo.ptEnd, &g_ptEnd, sizeof(tagPOINT));
		g_listPicInfo.push_back(stInfo);

	}
		break;
	case Type_Rect:
	{
		stPicInfo stInfo;
		stInfo.type = g_curDrawType;
		memcpy(&stInfo.ptFirst, &g_ptFirst, sizeof(tagPOINT));
		memcpy(&stInfo.ptEnd, &g_ptEnd, sizeof(tagPOINT));
		g_listPicInfo.push_back(stInfo);

	}
	break;
	case Type_Ellipse:
	{
		stPicInfo stInfo;
		stInfo.type = g_curDrawType;
		memcpy(&stInfo.ptFirst, &g_ptFirst, sizeof(tagPOINT));
		memcpy(&stInfo.ptEnd, &g_ptEnd, sizeof(tagPOINT));
		g_listPicInfo.push_back(stInfo);

	}
	break;



	}



}

void OnLButtonDown(LPARAM  lParam)
{
	if (g_bIsMouseDown)
	{
		return;
	}

	g_curDrawPicInfo.type = g_curDrawType;
	// Type_Rect;

	g_bIsMouseDown = true;
	//要找到鼠标所在的坐标点
 
	g_ptFirst.x=GET_X_LPARAM(lParam);
	g_ptFirst.y = GET_Y_LPARAM(lParam);

	memcpy(&g_curDrawPicInfo.ptFirst, &g_ptFirst, sizeof(tagPOINT));
	//g_curDrawType=

}


void SavePicFile()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	TCHAR  fileName[MAX_PATH] = { 0 };
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"*.MM";
	ofn.lpstrDefExt= L"MM";
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;


	
	char strFileName[MAX_PATH] = { 0 };


 


	if (GetSaveFileName(&ofn))
	{
		//char *p = (char*)fileName;

		//for (int i = 0; i < lstrlen(ofn.lpstrFile); i++)
		//{
		//	strFileName[i] = *(p + i * 2);
		//}
	  	size_t len = wcslen(ofn.lpstrFile) + 1;
		 	size_t connerted = 0;
		 	wcstombs_s(&connerted, strFileName, len, ofn.lpstrFile, _TRUNCATE);
			 
		FILE *pFile = fopen(strFileName, "w");

		if (pFile == NULL)
		{
			return;
		}
		char buf[100]; //文件头信息
		sprintf(buf, "%d", g_listPicInfo.size());
		fwrite(buf, Head_Len, 1, pFile);

		for (list<stPicInfo>::iterator it = g_listPicInfo.begin(); it != g_listPicInfo.end(); it++)
		{
			fwrite(&(*it), sizeof(stPicInfo), 1, pFile);
		}

		fclose(pFile);

		//int a = 0;

	}

}

void OpenPicFile()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	TCHAR  fileName[MAX_PATH] = { 0 };
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
 
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;
	char strFileName[MAX_PATH] = { 0 };

	if (GetSaveFileName(&ofn))
	{

		size_t len = wcslen(ofn.lpstrFile) + 1;
		size_t connerted = 0;
		wcstombs_s(&connerted, strFileName, len, ofn.lpstrFile, _TRUNCATE);

		FILE *pFile = fopen(strFileName, "r");
		if (pFile == NULL)
		{
			return;
		}

		char bufTemp[100]; //文件头信息
		//sprintf(buf, "%d", g_listPicInfo.size());
		//fwrite(buf, Head_Len, 1, pFile);
		fread(bufTemp, Head_Len, 1, pFile);
		int sizePic = atoi(bufTemp);


		for (int i=0; i<sizePic; i++)
		{
			stPicInfo stInfo;

			fread (&stInfo, sizeof(stInfo), 1, pFile);
		//	fwrite(&(*it), sizeof(stPicInfo), 1, pFile);
			g_listPicInfo.push_back(stInfo);
		}

		fclose(pFile);
	}

}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
