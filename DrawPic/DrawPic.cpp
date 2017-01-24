// DrawPic.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "DrawPic.h"
#include "list"
#include "windowsx.h"
#include"commdlg.h"
#include"process.h"

using namespace std;
#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;                                // ��ǰʵ��
WCHAR szTitle[MAX_LOADSTRING];                  // �������ı�
WCHAR szWindowClass[MAX_LOADSTRING];            // ����������

// �˴���ģ���а����ĺ�����ǰ������: 
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
//��ǰ����ͼ������

struct stPicInfo
{
	tagPOINT ptFirst;
	tagPOINT ptEnd;
	Draw_Type type;
};
list <stPicInfo> g_listPicInfo;
//����ͼ�α���
bool g_bIsMouseDown = false;

stPicInfo g_curDrawPicInfo; //��ǰ�����Ƶ�ͼ��

//��ǰ���ڻ���ͼ�εĵ�
tagPOINT g_ptFirst;
tagPOINT g_ptEnd;

HWND g_hWnd;

//��ǰ����Ƿ���
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

    // TODO: �ڴ˷��ô��롣

    // ��ʼ��ȫ���ַ���
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DRAWPIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ִ��Ӧ�ó����ʼ��: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAWPIC));

    MSG msg;

    // ����Ϣѭ��: 
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
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

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
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND  - ����Ӧ�ó���˵�
//  WM_PAINT    - ����������
//  WM_DESTROY  - �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// �����˵�ѡ��: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_DrawLine:
		//	MessageBox(hWnd, L"��ֱ��", L"��ʾ", MB_OK);

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
		// TODO: �ڴ˴����ʹ�� hdc ���κλ�ͼ����...
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

	//������Ϊ��ɫ
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
		//��ǰ����ͼ��  ������
		tagPOINT pt;
		GetCursorPos(&pt);
		ScreenToClient(g_hWnd, &pt); //ת�� �ɴ��ڿͻ�������

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
		HGDIOBJ hBrush = GetStockBrush(NULL_BRUSH); //�ջ�ˢ
	SelectObject(hDc, hBrush);
	Rectangle(hDc, info.ptFirst.x, info.ptFirst.y, info.ptEnd.x, info.ptEnd.y);
	DeleteObject(hBrush);

	}		break;
	case Type_Ellipse:
	{
		HGDIOBJ hBrush = GetStockBrush(NULL_BRUSH); //�ջ�ˢ
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
	//Ҫ�ҵ�������ڵ������
 
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
		char buf[100]; //�ļ�ͷ��Ϣ
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

		char bufTemp[100]; //�ļ�ͷ��Ϣ
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

// �����ڡ������Ϣ�������
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
