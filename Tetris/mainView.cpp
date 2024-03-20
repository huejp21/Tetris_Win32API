#include <windows.h>
#include "resource.h"

#define BUFFER_MAIN   0
#define BUFFER_NEXT_1 1
#define BUFFER_NEXT_2 2
#define BUFFER_NEXT_3 3
#define BUFFER_NOW    4
#define BUFFER_HOLD   5
#define BUFFER_ALL    6

#define IN_UP     0
#define IN_DOWN   1
#define IN_LEFT   2
#define IN_RIGHT  3

struct bufferInfo
{
  //block type
  int type = 0;
  
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPTSTR lpszClass = TEXT("Tetris");
HDC hdc, memdc, memdc2;
PAINTSTRUCT ps;

HBITMAP bitRed, bitBlue, bitGreen, bitOrange, bitPurple, bitSkyblue, bitYellow, bitBlank, bitTemp;


//////////블럭 버퍼의 사용/////////////////
//좌표가 해당 배열의 위치가 되며
//그 배열안의 type 변수에 따라 블럭을 그린다.
// 0 : blank
// 1 : SKYBLUE (LINE)
// 2 : YELLOW (SQUARE)
// 3 : ORANGE (L BLOCK)
// 4 : BLUE (J BLOCK)
// 5 : PURPLE (T BLOCK)
// 6 : RED (Z BLOCK)
// 7 : GREEN (S BLOCK)
///////////////////////////////////////////
bufferInfo mainBuffer[10][20];
bufferInfo nextBuffer1[4][4];
bufferInfo nextBuffer2[4][4];
bufferInfo nextBuffer3[4][4];
bufferInfo holdBuffer[4][4];
bufferInfo runningBuffer[4][4];
//////////////위치 버퍼의 사용/////////////
// 현재 mainBuffer 안의 블럭의 좌표를 저장한다.
// 이를 이용해 움직임을 제어함
///////////////////////////////////////////
int locationBuffer[4][2];
int blockRotate = 0;
/////////////blockType순서////////////////
// 0 runningBuffer 의 blockType
// 1 nextBuffer1 의 blockType
// 2 nextBuffer2 의 blockType
// 3 nextBuffer3 의 blockType
// 4 nextBuffer3 의 blockType
//////////////////////////////////////////
int blockType[5] = {0,};
///////////////변수 설명//////////////////
// score : 현재 점수
// combo : 현재 콤보횟수
// speed : 현재 레벨의 속도
// nowX : 현재 이동중인 블럭의 X좌표 보정값
// nowY : 현재 이동중인 블럭의 Y좌표 보정값
//////////////////////////////////////////
int score = 0;
int combo = 1;
int speed = 100;
int speedCount = 0;
int nowX = 3;
int nowY = -3;
int level = 0;
int gameStatus = 0; // 게입중에 블럭의 상태 표시
BOOL gameStart = FALSE;


// [블럭종류][블럭방향][블럭개수][블럭좌표]
int pattern[8][4][4][2] = 
{ 
  // BLANK 임시 
  {
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }
  },
  // LINE Block
  { 
    { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 } },
    { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } },
    { { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 } },
    { { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 } }
  },

  // SQUARE Block
  {
    { { 1, 1 }, { 2, 1 }, { 1, 2 }, { 2, 2 } },
    { { 1, 1 }, { 2, 1 }, { 1, 2 }, { 2, 2 } },
    { { 1, 1 }, { 2, 1 }, { 1, 2 }, { 2, 2 } },
    { { 1, 1 }, { 2, 1 }, { 1, 2 }, { 2, 2 } }
  },

  // L Block
  {
    { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 } },
    { { 0, 1 }, { 1, 1 }, { 2, 0 }, { 2, 1 } },
    { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 2 } },
    { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 0, 2 } }
  },

  // J Block
  {
    { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 0 } },
    { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 2, 2 } },
    { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 } },
    { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } }
  },

  // T Block
  {
    { { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
    { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 1 } },
    { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 1, 2 } },
    { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 1 } }
  },

  // Z Block
  {
    { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 2 } },
    { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 2, 1 } },
    { { 2, 0 }, { 2, 1 }, { 1, 1 }, { 1, 2 } },
    { { 0, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 } }
  },

  // S Block
  {
    { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 2 } },
    { { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 } },
    { { 1, 0 }, { 1, 1 }, { 2, 1 }, { 2, 2 } },
    { { 1, 1 }, { 2, 1 }, { 0, 2 }, { 1, 2 } }
  }
};


void paintBase();
void blockPrint();
void process(HWND hWnd);
void delay(DWORD msec);
int randomNumber();
void blockNew();
void blockClear(int bufferType);
void setLocation();
void deleteLocation();
void command(WPARAM wParam);
int checkMove(int commandType);
void moving(int commandType);
HBITMAP bitmapType(int type);
BOOL isMine(int x, int y);
void isFull();
void deleteLine(int y);
void levelUp();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
  , LPSTR lpszCmdParam, int nCmdShow)
{
  HWND hWnd;
  MSG Message;
  WNDCLASS WndClass;
  g_hInst = hInstance;

  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;
  WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  WndClass.hInstance = hInstance;
  WndClass.lpfnWndProc = (WNDPROC)WndProc;
  WndClass.lpszClassName = lpszClass;
  WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
  WndClass.style = CS_NOCLOSE;//CS_HREDRAW | CS_VREDRAW;
  RegisterClass(&WndClass);

  hWnd = CreateWindow(lpszClass, lpszClass, WS_CAPTION | WS_SYSMENU, //WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, (HMENU)NULL, hInstance, NULL);

  
  ShowWindow(hWnd, nCmdShow);



  while (GetMessage(&Message, 0, 0, 0)) {
    TranslateMessage(&Message);
    DispatchMessage(&Message);
  }
  return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
  switch (iMessage) {
  case WM_CREATE:

    bitRed = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_RED));
    bitBlue = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_BLUE));
    bitGreen = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_GREEN));
    bitOrange = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_ORANGE));
    bitPurple = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_PURPLE));
    bitSkyblue = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_SKYBLUE));
    bitYellow = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_YELLOW));
    bitBlank = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK_BLANK));
    ::SetTimer(hWnd, 0, 10, NULL);
    break;
  case WM_COMMAND:
    command(wParam);
    break;
  case WM_KEYDOWN:
    switch (wParam)
    {
    case VK_UP:
      moving(IN_UP);
      break;
    case VK_DOWN:
      moving(IN_DOWN);
      break;
    case VK_LEFT:
      moving(IN_LEFT);
      break;
    case VK_RIGHT:
      moving(IN_RIGHT);
      break;
    default:
      break;
    }
    break;
  case WM_GETMINMAXINFO:
    ((MINMAXINFO*)lParam)->ptMaxTrackSize.x = 415;
    ((MINMAXINFO*)lParam)->ptMaxTrackSize.y = 490;
    ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 415;
    ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 490;
    break;
  case WM_PAINT:
  {
    hdc = BeginPaint(hWnd, &ps);
    RECT rt;
    GetClientRect(hWnd, &rt);

    HBITMAP hOldBit, hBit;

    memdc = CreateCompatibleDC(hdc);
    memdc2 = CreateCompatibleDC(memdc);

    hBit = CreateCompatibleBitmap(hdc, rt.right - rt.left, rt.bottom - rt.top);
    hOldBit = (HBITMAP) SelectObject(memdc, hBit);

    HBRUSH hBrush;
    hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(memdc, &rt, hBrush);

    paintBase();
    blockPrint();

    int nX, nY, cx, cy;
    nX = rt.left;
    nY = rt.top;
    cx = rt.right - rt.left;
    cy = rt.bottom - rt.top;
    BitBlt(hdc, nX, nY, cx, cy, memdc, nX, nY, SRCCOPY);

    (HBITMAP)SelectObject(memdc, hOldBit);
    DeleteObject(hBit);
    DeleteObject(hBrush);
    DeleteDC(memdc2);
    DeleteDC(memdc);
    EndPaint(hWnd, &ps);
  }
    break;
  case WM_TIMER:
    if (gameStart)
    {
      process(hWnd); 
    }
    ::InvalidateRect(hWnd, NULL, FALSE);
    
    break;
  case WM_DESTROY:
    ::KillTimer(hWnd, 0);
    DeleteObject(bitRed);
    DeleteObject(bitBlue);
    DeleteObject(bitGreen);
    DeleteObject(bitOrange);
    DeleteObject(bitPurple);
    DeleteObject(bitSkyblue);
    DeleteObject(bitYellow);
    DeleteObject(bitBlank);
    DeleteObject(bitTemp);
    DeleteDC(memdc);
    PostQuitMessage(0);
    break;
  }
  return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void command(WPARAM wParam)
{
  switch (LOWORD(wParam))
  {
  case ID_START:

    blockClear(BUFFER_ALL);
    blockNew();
    blockNew();
    blockNew();
    blockNew();
    gameStart = TRUE;
    break;
  case ID_END:
    gameStart = FALSE;
    break;
  case ID_LOG:
    break;
  case ID_EXIT:
    PostQuitMessage(0);
    break;
  default:
    break;
  }
}

void paintBase()
{
  TCHAR strScore[255] = TEXT("");
  int szScore = 0;
  TCHAR strLevel[255] = TEXT("");
  int szLevel = 0;
  Rectangle(memdc, 99, 19, 302, 422); //main
  Rectangle(memdc, 309, 19, 392, 102); //next1
  Rectangle(memdc, 309, 102, 392, 184); //next2
  Rectangle(memdc, 309, 184, 392, 266); //next3
  Rectangle(memdc, 7, 19, 90, 102); //hold

  Rectangle(memdc, 7, 150, 90, 200); //combo
  Rectangle(memdc, 7, 207, 90, 257); //score
  Rectangle(memdc, 7, 264, 90, 314); //level


 

  SetTextAlign(memdc, TA_CENTER);
  TextOut(memdc, 350, 0, TEXT("NEXT"), 4);
  TextOut(memdc, 48, 0, TEXT("HOLD"), 4);

  SetTextAlign(memdc, TA_LEFT);
  TextOut(memdc, 10, 155, TEXT("COMBO"), 5);
  TextOut(memdc, 10, 175, TEXT(""), 8);

  wsprintf(strScore, TEXT("%d"), score);
  szScore = lstrlen(strScore);
  TextOut(memdc, 10, 212, TEXT("SCORE"), 5);
  TextOut(memdc, 10, 232, strScore, szScore);

  wsprintf(strLevel, TEXT("%d"), level);
  szLevel = lstrlen(strLevel);
  TextOut(memdc, 10, 269, TEXT("LEVEL"), 5);
  TextOut(memdc, 10, 289, strLevel, szLevel);

  if (!gameStart)
  {
    TextOut(memdc, 160, 0, TEXT("Game Over"), 9);
  }
}

void blockPrint()
{
  HBITMAP hOldBit;
  //print main buffer;
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      bitTemp = bitmapType(mainBuffer[i][j].type);
      hOldBit = (HBITMAP)SelectObject(memdc2, bitTemp);
      BitBlt(memdc, 100 + (20 * i), 20 + (20 * j), 20, 20, memdc2, 0, 0, SRCCOPY);
      SelectObject(memdc2, hOldBit);
    }
  }

  //print nextBuffer1
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      bitTemp = bitmapType(nextBuffer1[i][j].type);
      hOldBit = (HBITMAP)SelectObject(memdc2, bitTemp);
      BitBlt(memdc, 310 + (20 * i), 20 + (20 * j), 20, 20, memdc2, 0, 0, SRCCOPY);
      SelectObject(memdc2, hOldBit);
    }
  }

  //print nextBuffer2 
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      bitTemp = bitmapType(nextBuffer2[i][j].type);
      hOldBit = (HBITMAP)SelectObject(memdc2, bitTemp);
      BitBlt(memdc, 310 + (20 * i), 102 + (20 * j), 20, 20, memdc2, 0, 0, SRCCOPY);
      SelectObject(memdc2, hOldBit);
    }
  }

  //print nextBuffer3
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      bitTemp = bitmapType(nextBuffer3[i][j].type);
      hOldBit = (HBITMAP)SelectObject(memdc2, bitTemp);
      BitBlt(memdc, 310 + (20 * i), 184 + (20 * j), 20, 20, memdc2, 0, 0, SRCCOPY);
      SelectObject(memdc2, hOldBit);
    }
  }
  
  //print holdBuffer
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      bitTemp = bitmapType(holdBuffer[i][j].type);
      hOldBit = (HBITMAP)SelectObject(memdc2, bitTemp);
      BitBlt(memdc, 8 + (20 * i), 20 + (20 * j), 20, 20, memdc2, 0, 0, SRCCOPY);
      SelectObject(memdc2, hOldBit);
    }
  }
}

HBITMAP bitmapType(int type)
{
  switch (type)
  {
  case 0:
    return bitBlank;
  case 1:
    return bitSkyblue;
  case 2:
    return bitYellow;
  case 3:
    return bitOrange;
  case 4:
    return bitBlue;
  case 5:
    return bitPurple;
  case 6:
    return bitRed;
  case 7:
    return bitGreen;
  }
}

void delay(DWORD msec)
{
  LARGE_INTEGER perfCnt, start, now;

  QueryPerformanceFrequency(&perfCnt);
  QueryPerformanceCounter(&start);

  do {
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
  } while ((now.QuadPart - start.QuadPart) / float(perfCnt.QuadPart) * 1000 * 1000 < msec);
}

int randomNumber()
{
  int randomNumber = 0;
  LARGE_INTEGER now;
  QueryPerformanceCounter((LARGE_INTEGER*)&now);
  unsigned int nNow = (unsigned int)(now.QuadPart & 0xFFFF);
  srand(nNow);
  randomNumber = rand();
  return randomNumber;
}

void process(HWND hWnd)
{
  speedCount++;
  if (speed <= speedCount)
  {
    moving(IN_DOWN);
    speedCount = 0;
  }
  

  setLocation();
  for (int i = 0; i < 4; i++)
  {
    if ((locationBuffer[i][0] >= 0 && locationBuffer[i][0] < 10) && (locationBuffer[i][1] >= 0 && locationBuffer[i][1] < 20))
    {
      mainBuffer[locationBuffer[i][0]][locationBuffer[i][1]].type = blockType[0];
    }
  }
}

void blockNew()
{
  int random = 0;
  while (random == 0)
  {
    random = randomNumber() % 8;
  }
  // [블럭종류][블럭방향][블럭개수][블럭좌표]
  // int pattern[7][4][4][2]
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      runningBuffer[i][j].type = nextBuffer1[i][j].type;
      nextBuffer1[i][j].type = nextBuffer2[i][j].type;
      nextBuffer2[i][j].type = nextBuffer3[i][j].type;
    }
  }
  blockType[0] = blockType[1];
  blockType[1] = blockType[2];
  blockType[2] = blockType[3];


  blockClear(BUFFER_NEXT_3);
    for (int i = 0; i < 4; i++)
    {
      nextBuffer3[pattern[random][blockRotate][i][0]][pattern[random][blockRotate][i][1]].type = random;
    }
    blockType[3] = random;
}

void blockClear(int bufferType)
{
  switch (bufferType)
  {
  case BUFFER_MAIN:
    for (int i = 0; i < 10; i++)
    {
      for (int j = 0; j < 20; j++)
      {
        mainBuffer[i][j].type = 0;
      }
    }
    break;
  case BUFFER_NEXT_1:
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        nextBuffer1[i][j].type = 0;
      }
    }
    break;
  case BUFFER_NEXT_2:
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        nextBuffer2[i][j].type = 0;
      }
    }
    break;
  case BUFFER_NEXT_3:
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        nextBuffer3[i][j].type = 0;
      }
    }
    break;
  case BUFFER_NOW:
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        runningBuffer[i][j].type = 0;
      }
    }
    break;
  case BUFFER_HOLD:
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        runningBuffer[i][j].type = 0;
      }
    }
    break;
  case BUFFER_ALL:
    for (int i = 0; i < 10; i++)
    {
      for (int j = 0; j < 20; j++)
      {
        mainBuffer[i][j].type = 0;
      }
    }
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        nextBuffer1[i][j].type = 0;
        nextBuffer2[i][j].type = 0;
        nextBuffer3[i][j].type = 0;
        holdBuffer[i][j].type = 0;
        runningBuffer[i][j].type = 0;
      }
    }
    speed = 100;
    speedCount = 0;
    score = 0;
    combo = 1;
    nowX = 3;
    nowY = -3;
    gameStatus = 0; // 게입중에 블럭의 상태 표시
    blockRotate = 0;

    for (int i = 0; i < 4; i++)
    {
      for (int j= 0; j < 2; j++)
      {
        locationBuffer[i][j] = 0;
      }
    }
    for (int i = 0; i < 5; i++)
    {
      blockType[i] = 0;
    }

    


    break;
  default:
    break;
  }
}

void setLocation()
{
  // [블럭종류][블럭방향][블럭개수][블럭좌표]
  // pattern[8][4][4][2]

  //블럭의 좌표를 실제 mainBuffer 와 매치 하기위해 값을 더하거나 빼준다.
  for (int i = 0; i < 4; i++)
  {
    locationBuffer[i][0] = pattern[blockType[0]][blockRotate][i][0] + nowX;
    locationBuffer[i][1] = pattern[blockType[0]][blockRotate][i][1] + nowY;
  }
}

void deleteLocation()
{
  for (int i = 0; i < 4; i++)
  {
    if (0 <= locationBuffer[i][0] && 9 >= locationBuffer[i][0] && 0 <= locationBuffer[i][1] && 19 >= locationBuffer[i][1])
    {
      mainBuffer[locationBuffer[i][0]][locationBuffer[i][1]].type = 0;
    }
  }
}

BOOL isMine(int x, int y)
{
  BOOL check = FALSE;
  for (int i = 0; i < 4; i++)
  {
    if ((locationBuffer[i][0] == x) && (locationBuffer[i][1] == y))
    {
      check = TRUE;
    }
  }
  return check;
}

int checkMove(int commandType)
{
  int possible = 0;
  int temp[4][2];
  int isTemp[4][2] = {-100,};
  int checkMine[4] = {0,};
  int checkCount = 0;
  int checkIndex = 0;
  int tempRotate;
  BOOL check = FALSE;
  switch (commandType)
  {
  case IN_UP:
    if (blockRotate == 3)
    {
      tempRotate = 0;
    }
    else
    {
      tempRotate = blockRotate + 1;
    }
    // [블럭종류][블럭방향][블럭개수][블럭좌표]
    // pattern[8][4][4][2]
    for (int i = 0; i < 4; i++)
    {
      temp[i][0] = pattern[blockType[0]][tempRotate][i][0] + nowX;
      temp[i][1] = pattern[blockType[0]][tempRotate][i][1] + nowY;


      if (temp[i][0] < 0 || temp[i][0] >= 10 || temp[i][1] >= 20)
      {
        possible = 1;
        return possible;
      }
    }//다음꺼 좌표입력



    for (int i = 0; i < 4; i++)
    {
      check = 0;
      for (int j = 0; j < 4; j++)
      {
        if (((temp[i][0] == locationBuffer[j][0]) && (temp[i][1] == locationBuffer[j][1])))
        {
          check = TRUE;
          break;
        }
      }
      if (!check)
      {
        isTemp[checkCount][0] = temp[i][0];
        isTemp[checkCount][1] = temp[i][1];
        checkCount++;
      }
      check = FALSE;
    }//변경된 값이 자기 값과 같은 인지 검사하고 자기 좌표가 아니면 isTemp에 저장함.

    if (checkCount != 0)
    {
      for (int i = 0; i < checkCount; i++)
      {
        if (isTemp[i][0] >= 0 && isTemp[i][1] >=0)
        {
          if (!(mainBuffer[isTemp[i][0]][isTemp[i][1]].type == 0))
          {
            possible = 1;
          }
        }
      }
    }

    if (possible == 0)
    {
      blockRotate = tempRotate;
    }
    break;
  case IN_DOWN:    
    for (int i = 0; i < 4; i++)
    {
      temp[i][0] = locationBuffer[i][0];
      temp[i][1] = locationBuffer[i][1] + 1;
      if (temp[i][1] >= 20)
      {
        possible = 2;
        return possible;
      }
    }//다음꺼 좌표입력 하고 밑벽에 닿으면 끝 2

    for (int i = 0; i < 4; i++)
    {
      if ((temp[i][0] >= 0) && (temp[i][1] >= 0))
      {
        if (!isMine(temp[i][0], temp[i][1]))
        {
          if (mainBuffer[temp[i][0]][temp[i][1]].type != 0)
          {
            check = TRUE;
          }
        }
      }
    }
    if (check)
    {
      possible = 2;
    }

    break;
  case IN_LEFT:
    for (int i = 0; i < 4; i++)
    {
      temp[i][0] = locationBuffer[i][0] - 1;
      temp[i][1] = locationBuffer[i][1];
      if (temp[i][0] < 0 || temp[i][0] >= 10)
      {
        possible = 1;
        return possible;
      }
    }//다음꺼 좌표입력 하고 밑벽에 닿으면 끝 2

    for (int i = 0; i < 4; i++)
    {
      if ((temp[i][0] >= 0) && (temp[i][1] >= 0))
      {
        if (!isMine(temp[i][0], temp[i][1]))
        {
          if (mainBuffer[temp[i][0]][temp[i][1]].type != 0)
          {
            check = TRUE;
          }
        }
      }
    }
    if (check)
    {
      possible = 1;
    }

    break;
  case IN_RIGHT:
    for (int i = 0; i < 4; i++)
    {
      temp[i][0] = locationBuffer[i][0] + 1;
      temp[i][1] = locationBuffer[i][1];
      if (temp[i][0] < 0 || temp[i][0] >= 10)
      {
        possible = 1;
        return possible;
      }
    }//다음꺼 좌표입력 하고 밑벽에 닿으면 끝 2


    for (int i = 0; i < 4; i++)
    {
      if ((temp[i][0] >= 0) && (temp[i][1] >= 0))
      {
        if (!isMine(temp[i][0], temp[i][1]))
        {
          if (mainBuffer[temp[i][0]][temp[i][1]].type != 0)
          {
            check = TRUE;
          }
        }
      }
    }
    if (check)
    {
      possible = 1;
    }

    break;
  default:
    break;
  }
  // 0 : OK
  // 1 : there is wall
  // 2 : there is block(finish)
  gameStatus =  possible;
  return possible;
}

void moving(int commandType)
{
  int movingStatus = checkMove(commandType);
  
  if (movingStatus == 2)
  {
    if (gameStart)
    {
      score += 1;
      isFull();
      levelUp();
      nowX = 3;
      nowY = -3;
      blockNew();
    }
    if (level >= 100)
    {
      gameStart = FALSE;
      return;
    }
    for (int i = 3; i < 7; i++)
    {
      if (!(mainBuffer[i][0].type == 0))
      {
        gameStart = FALSE;
        return;
      }
    }
  }

  if (movingStatus == 0)
  {
    deleteLocation();
    switch (commandType)
    {
    case IN_UP:
      break;
    case IN_DOWN:
      nowY++;
      break;
    case IN_LEFT:
      nowX--;
      break;
    case IN_RIGHT:
      nowX++;
      break;
    default:
      break;
    }
    setLocation();
  }
}

void isFull()
{
  int count = 0;
  BOOL check = FALSE;
  do
  {
    check = FALSE;
    for (int i = 19; i >= 0; i--)
    {
      for (int j = 0; j < 10; j++)
      {
        if (mainBuffer[j][i].type != 0)
        {
          count++;
        }
      }
      if (count == 10)
      {
        deleteLine(i);
        score += 10;
        check = TRUE;
      }
      count = 0;
    }
  } while (check);
}

void deleteLine(int y)
{
  for (int i = 0; i < 10; i++)
  {
    mainBuffer[i][y].type = 0;
  }

  for (int i = y; i > 0; i--)
  {
    for (int j = 0; j < 10; j++)
    {
      mainBuffer[j][i].type = mainBuffer[j][i - 1].type;
    }
  }
}

void levelUp()
{
  level = (score / 100);
  speed = 100 - level;
}