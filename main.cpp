#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <tuple>
#include <algorithm>

#define PARAM_MENU_EXIT 1
#define CLICK_PAD_CLICKED 2
#define PARAM_MENU_CUSTOM_GAME 3
#define HANDLE_CUSTOM_GAME_CHOICE 4
#define TEST 3000

#define RIBON_HEIGHT 30
#define EXTRA_HEIGHT_FOR_SCREEN 50

#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) \
                                 | ((Color << 16) & 0xff0000)

void RegisterDialogForCustomGame(HINSTANCE hInst);
void DisplayDialogForCustomGame(HWND hParentWnd);

std::ofstream MyFile("test.txt");

struct ClickPad
{
    int x, y, id;
    HWND handler;
};

struct HandlersDialogCustomGame
{
    HWND height, width, visiblePads;
};

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

int dialogWidthCustomGame = 300;
int dialogHeightCustomGame = 135;
struct HandlersDialogCustomGame handlersDialogCustomGame;

HMENU hMenu;
HBITMAP hClickPadImage;
HWND hScore, hSpeed, hClickPad;

int playerScore = 0;
int playerSpeed = 0;
DWORD startTime;

int screenWidth = 1000;
int screenHeight = 700;
int mapXClickPadsCount = 4;
int mapYClickPadsCount = 4;
int clickPadsVisible = 3;
int clickPadWidth = screenWidth / mapXClickPadsCount;
int clickPadHeight = screenHeight / mapYClickPadsCount;

std::vector<std::tuple <int, int>> allClickPadsPos;
std::vector<HBITMAP> clickPadImages;
std::deque<ClickPad> currClickPads;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR args,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    wincl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    RegisterDialogForCustomGame(hThisInstance);

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("SpeedyFingers"), /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           screenWidth,         /* The programs width */
           screenHeight + EXTRA_HEIGHT_FOR_SCREEN + RIBON_HEIGHT,        /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void testFilePrint(std::string s)
{
    std::ofstream MyFile("test.txt");
    MyFile << s;
    MyFile.close();
}



//-------------------------------------------------------------------------------
// hBmp         : Source Bitmap
// cOldColor : Color to replace in hBmp
// cNewColor : Color used for replacement
// hBmpDC    : DC of hBmp ( default NULL ) could be NULL if hBmp is not selected
//
// Retcode   : HBITMAP of the modified bitmap or NULL for errors
//-------------------------------------------------------------------------------
HBITMAP ReplaceColor(HBITMAP hBmp,COLORREF cOldColor,COLORREF cNewColor,HDC hBmpDC)
{
    HBITMAP RetBmp=NULL;
    if (hBmp)
    {
        HDC BufferDC=CreateCompatibleDC(NULL);    // DC for Source Bitmap
        if (BufferDC)
        {
            HBITMAP hTmpBitmap = (HBITMAP) NULL;
            if (hBmpDC)
                if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
            {
                hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
                SelectObject(hBmpDC, hTmpBitmap);
            }

            HGDIOBJ PreviousBufferObject=SelectObject(BufferDC,hBmp);
            // here BufferDC contains the bitmap

            HDC DirectDC=CreateCompatibleDC(NULL); // DC for working
            if (DirectDC)
            {
                // Get bitmap size
                BITMAP bm;
                GetObject(hBmp, sizeof(bm), &bm);

                // create a BITMAPINFO with minimal initilisation
                // for the CreateDIBSection
                BITMAPINFO RGB32BitsBITMAPINFO;
                ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
                RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
                RGB32BitsBITMAPINFO.bmiHeader.biWidth=bm.bmWidth;
                RGB32BitsBITMAPINFO.bmiHeader.biHeight=bm.bmHeight;
                RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
                RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;

                // pointer used for direct Bitmap pixels access
                UINT * ptPixels;

                HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
                                       (BITMAPINFO *)&RGB32BitsBITMAPINFO,
                                       DIB_RGB_COLORS,
                                       (void **)&ptPixels,
                                       NULL, 0);
                if (DirectBitmap)
                {
                    // here DirectBitmap!=NULL so ptPixels!=NULL no need to test
                    HGDIOBJ PreviousObject=SelectObject(DirectDC, DirectBitmap);
                    BitBlt(DirectDC,0,0,
                                   bm.bmWidth,bm.bmHeight,
                                   BufferDC,0,0,SRCCOPY);

                       // here the DirectDC contains the bitmap

                    // Convert COLORREF to RGB (Invert RED and BLUE)
                    cOldColor=COLORREF2RGB(cOldColor);
                    cNewColor=COLORREF2RGB(cNewColor);

                    // After all the inits we can do the job : Replace Color
                    for (int i=((bm.bmWidth*bm.bmHeight)-1);i>=0;i--)
                    {
                        if (ptPixels[i]==cOldColor) ptPixels[i]=cNewColor;
                    }
                    // little clean up
                    // Don't delete the result of SelectObject because it's
                    // our modified bitmap (DirectBitmap)
                       SelectObject(DirectDC,PreviousObject);

                    // finish
                    RetBmp=DirectBitmap;
                }
                // clean up
                DeleteDC(DirectDC);
            }
            if (hTmpBitmap)
            {
                SelectObject(hBmpDC, hBmp);
                DeleteObject(hTmpBitmap);
            }
            SelectObject(BufferDC,PreviousBufferObject);
            // BufferDC is now useless
            DeleteDC(BufferDC);
        }
    }
    return RetBmp;
}

/////////////////////////////////////////////////////////////////////
//////////////////////////GameRibbon/////////////////////////////////
/////////////////////////////////////////////////////////////////////
HWND AppendRibbonStatsElement(HWND hRibon, std::string text, int number, int x, int lenght)
{
    std::string elString = text;
    elString += std::to_string(number);

    std::wstring elWString = std::wstring(elString.begin(), elString.end());
    HWND h = CreateWindowW(L"static", elWString.c_str(), WS_VISIBLE | WS_CHILD, x, 0, lenght, RIBON_HEIGHT, hRibon, NULL, NULL, NULL);
    return h;
}

void SetRibbonStatsElementText(HWND handler, std::string text, int number)
{
    std::string elString = text;
    elString += std::to_string(number);

    std::wstring elWString = std::wstring(elString.begin(), elString.end());
    SetWindowTextW(handler, elWString.c_str());
}

std::string DWORDToString(DWORD value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string doubleToString(double value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

double GetPassedTimeInSeconds()
{
    DWORD currentTime = GetTickCount();
    DWORD passedSecondsDWORD = (currentTime - startTime)/1000;
    return (double)passedSecondsDWORD;
}

double GetClicksPerMinute()
{
    double secsPassed = GetPassedTimeInSeconds();
    if(secsPassed < 1)
    {
        return 99999.0;
    }

    double minutesPassed = secsPassed / 60;
    double result = playerScore / minutesPassed;
    return result;
}

void AppendGameStatusRibbon(HWND hParentWnd){
    int currX = 0;
    int length = 0;
    HWND hRibon = CreateWindowW(L"static", L"", WS_VISIBLE | WS_CHILD, 0, screenHeight, screenWidth, RIBON_HEIGHT, hParentWnd, NULL, NULL, NULL);

    length = 200;
    hScore = AppendRibbonStatsElement(hRibon, "Score: ", 0, currX, length);

    currX += (length+(100));
    hSpeed = AppendRibbonStatsElement(hRibon, "Speed: ", 0, currX, length);
}

void ReloadGameRibbon()
{
    SetRibbonStatsElementText(hScore, "Score: ", playerScore);
    SetRibbonStatsElementText(hSpeed, "Speed: ", (int)GetClicksPerMinute());
}
/////////////////////////////////////////////////////////////////////
//////////////////////////GameRibbon/////////////////////////////////
/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
/////////////////////////////////////////////////////////////////////
///////////////////////////GameMap///////////////////////////////////
/////////////////////////////////////////////////////////////////////

void LoadImages()
{
    hClickPadImage = (HBITMAP)LoadImageW(NULL, L".\\clickPad.bmp", IMAGE_BITMAP, clickPadWidth, clickPadHeight, LR_LOADFROMFILE);
}

COLORREF GetDarkenedRed(int shade)
{
    COLORREF res = 0x0000ff;
    if (shade > 255)
    {
        shade = 255;
    }
    res -= shade;
    return res;
}

COLORREF GetWhitenedRed(int shade)
{
    COLORREF res = 0x0000ff;
    if (shade > 255)
    {
        shade = 255;
    }
    res += 65792 * shade;
    return res;
}

void CreateClickPadImages(int count)
{
    int darkenByShade = (255 - 10) / count;
    for(int i=0; i<count; i++)
    {
        COLORREF newClickPadImageColor = GetDarkenedRed(darkenByShade * i);
        HBITMAP newClickPadImage = ReplaceColor(hClickPadImage, 0x0000ff, newClickPadImageColor, NULL);
        clickPadImages.push_back(newClickPadImage);
    }
}

void SetClickPadsPossiblePositions()
{
    for(int i=0; i<mapXClickPadsCount; i++)
    {
        for(int j=0; j<mapYClickPadsCount; j++)
        {
            std::tuple <int, int> pos = std::make_tuple(clickPadWidth*i, clickPadHeight*j);
            allClickPadsPos.push_back(pos);
        }
    }
}

bool IsClickPadExistCurrently(int id)
{
    for(int i=0; i<clickPadsVisible; i++)
    {
        if(currClickPads[i].id == id)
        {
            return true;
        }
    }

    return false;
}

int GetRandomCurrentlyNotVisibleClickPadId()
{
    int res;
    int x = 0;

    int maxClickPadsCount = mapXClickPadsCount * mapYClickPadsCount;
    while(true)
    {
        res = rand() % maxClickPadsCount + 0;   // range 0-(maxClickPadsCount-1)

        if (!IsClickPadExistCurrently(res))
        {
            break;
        }

        x++;
    }
    return res;
}

void PushBackNewClickPad(HWND hParentWnd)
{
    int id = GetRandomCurrentlyNotVisibleClickPadId();
    std::tuple<int, int> pos = allClickPadsPos[id];

    int x = std::get<0>(pos);
    int y = std::get<1>(pos);
    HWND handler = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, x, y, clickPadWidth, clickPadHeight, hParentWnd, NULL, NULL, NULL);

    struct ClickPad newClickPad = {x, y, id, handler};
    currClickPads.push_back(newClickPad);
}

void SwapFirstClickPadForButton(HWND hParentWnd)
{
    struct ClickPad fstClickPad = currClickPads.front();
    currClickPads.pop_front();

    DestroyWindow(fstClickPad.handler);

    HWND handler = CreateWindowW(L"button", NULL, WS_VISIBLE | WS_CHILD | BS_BITMAP, fstClickPad.x, fstClickPad.y, clickPadWidth, clickPadHeight, hParentWnd, (HMENU)CLICK_PAD_CLICKED, NULL, NULL);
    SendMessageW(handler, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)clickPadImages[0]);

    struct ClickPad newClickPad = {fstClickPad.x, fstClickPad.y, fstClickPad.id, handler};
    currClickPads.push_front(newClickPad);
}

void ReloadClickPadImagesAndHandlers(HWND hParentWnd)
{
    SwapFirstClickPadForButton(hParentWnd);

    for(int i=1; i<clickPadsVisible; i++)
    {
        struct ClickPad clickPad = currClickPads.at(i);
        SendMessageW(clickPad.handler, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)clickPadImages[i]);
    }
}

void InitializeClickPads(HWND hParentWnd)
{
    for(int i=0; i<clickPadsVisible; i++)
    {
        PushBackNewClickPad(hParentWnd);
    }
    ReloadClickPadImagesAndHandlers(hParentWnd);
}

void AddGameMap(HWND hParentWnd){
    AppendGameStatusRibbon(hParentWnd);

    CreateClickPadImages(clickPadsVisible);
    SetClickPadsPossiblePositions();

    InitializeClickPads(hParentWnd);
}

void RemoveClickedClickPadAndPushNew(HWND hParentWnd)
{
    struct ClickPad clickPad = currClickPads.front();
    DestroyWindow(clickPad.handler);

    currClickPads.pop_front();
    PushBackNewClickPad(hParentWnd);
    ReloadClickPadImagesAndHandlers(hParentWnd);
}
/////////////////////////////////////////////////////////////////////
///////////////////////////GameMap///////////////////////////////////
/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
/////////////////////////////////////////////////////////////////////
/////////////////////CUSTOM_GAME_DIALOG//////////////////////////////
/////////////////////////////////////////////////////////////////////
LRESULT CALLBACK DialogProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_COMMAND:
            switch(wp)
            {
                case HANDLE_CUSTOM_GAME_CHOICE:
                    DestroyWindow(hWnd);

                    break;
            }
            break;
        default:
            return DefWindowProc(hWnd, msg, wp, lp);
    }
}

void RegisterDialogForCustomGame(HINSTANCE hInst)
{
    WNDCLASSW wincl = {0};

    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.hInstance = hInst;
    wincl.lpszClassName = L"CustomGameDialogClass";
    wincl.lpfnWndProc = DialogProcedure;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassW (&wincl))
        return;
}

void DisplayDialogForCustomGame(HWND hParentWnd)
{
    HWND hDialogWnd = CreateWindowW (
           L"CustomGameDialogClass",
           L"Custom game",
           WS_VISIBLE | WS_OVERLAPPEDWINDOW,
           200,
           200,
           dialogWidthCustomGame,
           dialogHeightCustomGame,
           hParentWnd,
           NULL,
           NULL,
           NULL
           );

    int labelWidth = 90;
    int labelHeight = 20;
    int buttonWidth = 50;
    int currY = 5;

    CreateWindowW(L"static",L"Map width", WS_VISIBLE | WS_CHILD | WS_TILED, 5, currY, labelWidth, labelHeight, hDialogWnd, NULL, NULL, NULL);
    HWND hMapWidth = CreateWindowW(L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_TILED, 5+labelWidth, currY, dialogWidthCustomGame - labelWidth - 27, labelHeight, hDialogWnd, NULL, NULL, NULL);
    currY += labelHeight;
    CreateWindowW(L"static",L"Map height", WS_VISIBLE | WS_CHILD | WS_TILED, 5, currY, labelWidth, labelHeight, hDialogWnd, NULL, NULL, NULL);
    HWND hMapHeight = CreateWindowW(L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_TILED, 5+labelWidth, currY, dialogWidthCustomGame - labelWidth - 27, labelHeight, hDialogWnd, NULL, NULL, NULL);
    currY += labelHeight;
    CreateWindowW(L"static",L"Visible pads", WS_VISIBLE | WS_CHILD | WS_TILED, 5, currY, labelWidth, labelHeight, hDialogWnd, NULL, NULL, NULL);
    HWND hVisiblePads = CreateWindowW(L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_TILED, 5+labelWidth, currY, dialogWidthCustomGame - labelWidth - 27, labelHeight, hDialogWnd, NULL, NULL, NULL);
    currY += labelHeight;
    CreateWindowW(L"button", L"OK", WS_VISIBLE | WS_CHILD, (dialogWidthCustomGame-buttonWidth-8)/2, currY+5, buttonWidth, labelHeight, hDialogWnd, (HMENU)HANDLE_CUSTOM_GAME_CHOICE, NULL, NULL);

    handlersDialogCustomGame = {hMapHeight, hMapWidth, hVisiblePads};
}
/////////////////////////////////////////////////////////////////////
/////////////////////CUSTOM_GAME_DIALOG//////////////////////////////
/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
//-----------------------------------------------------------------//
/////////////////////////////////////////////////////////////////////
///////////////////////////MENU//////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void AppendGameNewSubMenu(HMENU hParentMenu)
{
    HMENU hGameNewMenu = CreateMenu();
    AppendMenu(hParentMenu, MF_POPUP, (UINT_PTR)hGameNewMenu, "New");

    AppendMenu(hGameNewMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hGameNewMenu, MF_STRING, PARAM_MENU_CUSTOM_GAME, "Custom");
}

void AppendGameSubMenu(HMENU hParentMenu)
{
    HMENU hGameMenu = CreateMenu();
    AppendMenu(hParentMenu, MF_POPUP, (UINT_PTR)hGameMenu, "Game");

    AppendGameNewSubMenu(hGameMenu);
    AppendMenu(hGameMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hGameMenu, MF_STRING, NULL, "SaveSettings");
}

void AddMenus(HWND hWnd)
{
    hMenu = CreateMenu();

    AppendGameSubMenu(hMenu);

    AppendMenu(hMenu, MF_STRING, PARAM_MENU_EXIT, "Exit");

    SetMenu(hWnd, hMenu);
}
/////////////////////////////////////////////////////////////////////
///////////////////////////MENU//////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void HandleClickPadClick(HWND hParentWnd)
{
    playerScore += 1;
    ReloadGameRibbon();

    RemoveClickedClickPadAndPushNew(hParentWnd);
}

void HandleWmCommand(WPARAM wParam, HWND hParentWnd){
    switch(wParam)
    {
        case PARAM_MENU_EXIT:
            PostQuitMessage (0);
            break;
        case PARAM_MENU_CUSTOM_GAME:
            DisplayDialogForCustomGame(hParentWnd);
            break;
        case CLICK_PAD_CLICKED:
            HandleClickPadClick(hParentWnd);
            break;
        case TEST:
            break;
    }
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_COMMAND:
            HandleWmCommand(wParam, hwnd);
            break;
        case WM_CREATE:
            startTime = GetTickCount();
            LoadImages();
            AddMenus(hwnd);
            AddGameMap(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            MyFile.close();
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
