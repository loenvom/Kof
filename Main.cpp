#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <mmsystem.h>
#include "resource.h"
#pragma comment(lib, "WINMM.LIB")
using namespace std;
const int harm_j = 1, harm_k = 2, harm_i = 10;
char content[100]; // 存放输出的内容
const int width = 800, height = 500; // 整个游戏画面的大小
SIZE client, blood, enegy;
RECT rect;
const int step = 27; // 控制游戏的人物步长
const int jump = 300; // 控制人物的跳跃高度
const int interval = 120; //控制帧率
const int total_health = 50; //控制血量
int mirror = 0;
BITMAP bm;
void debug(HDC &hDC, HWND &hWnd)
{
	PAINTSTRUCT PtStr;
	hDC = BeginPaint(hWnd, &PtStr);
	HFONT hFont = (HFONT)CreateFont(100, 0, 0, 0,
			0, 0, 0, 0,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, "粗体字");
	SelectObject(hDC, hFont);
	TextOut(hDC, 0, 0, content, strlen(content));
	DeleteObject(hFont);
	DeleteObject(hDC);
	EndPaint(hWnd, &PtStr);
}
class Role
{
	
public:
	int x, y, l, r, top, bottom; //人物位置的判定
	int dir; // 记录人物的方向
	HBITMAP * hbm[13][2]; //进行位图的储存
	int num[13];
	int index, key; //判断关键帧动画
	int angry = 0;

	Role()
	{
	}
	void set(int key, int n, char *fpath)
	{
		int i;
		char path[100];
		num[key] = n;
		hbm[key][0] = new HBITMAP[n];
		hbm[key][1] = new HBITMAP[n];
		for (i = 0; i < n; i++)
		{
			sprintf(path, "%s\\%d_0.bmp", fpath, i);
			hbm[key][0][i] = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP,
				0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
			sprintf(path, "%s\\%d_1.bmp", fpath, i);
			hbm[key][1][i] = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP,
				0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
		}
		index = 0;
		dir = 0;
	}
	void set(int _x, int _y)
	{
		x = _x; y = _y;
	}
	/*
		0 防御、1 死亡、2 蹲下、3 蹲踢、4 出拳、5 跳、6 踢
		7 移动、8 踱步、9 站立、10 获胜、11 受击打、12 倒下
	*/
	void set(int _key)
	{
		if (_key == 11 && key != 9) return;
		if (_key != key)
		{
			key = _key; 
			index = 0;
			GetObject(hbm[key][dir][index], sizeof(BITMAP), &bm);
			y = client.cy - 64 - bm.bmHeight / 4;
		}
	}
	void play(HDC &hDC, HDC &hdcmem, BITMAP &bm)
	{
		if (num[key] == 0)
		{
			key = 9; index = 0;
		}
		GetObject(hbm[key][dir][index], sizeof(BITMAP), &bm);
		SelectObject(hdcmem, hbm[key][dir][index]);
		l = x - bm.bmWidth / 2;
		r = x + bm.bmWidth / 2;
		top = y - bm.bmHeight / 4;
		bottom = y + bm.bmHeight / 4;
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight / 2,
			hdcmem, 0, bm.bmHeight / 2, SRCAND);
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight / 2,
			hdcmem, 0, 0, SRCPAINT);
		if (key == 5)
		{
			if (index <= num[key] / 2)
			{
				y -= jump / num[key];
			}
			else
			{
				y += jump / num[key];
			}
			if (index == num[key] - 1)
			{
				GetObject(hbm[key][dir][index], sizeof(BITMAP), &bm);
				y = client.cy - 64 - bm.bmHeight / 4;
			}
		}
		if (key == 2 || key == 1 || key == 0)
		{
			if (index == num[key] - 1) return;
		}else if(key != 9)
		{
			if (index == num[key] - 1)
			{
				set(9); return;
			}
		}
		index = (index + 1) % num[key];
	}

	void operator = (Role B)
	{
		int i, j;
		for (i = 0; i < 13; i++)
		{
			num[i] = B.num[i];
			hbm[i][0] = new HBITMAP[num[i]];
			hbm[i][1] = new HBITMAP[num[i]];
			for (j = 0; j < num[i]; j++)
			{
				hbm[i][0][j] = B.hbm[i][0][j];
				hbm[i][1][j] = B.hbm[i][1][j];
			}
		}
	}
	bool IsReady()
	{
		return (key == 0 || key == 2 || key == 7 || key == 8 || key == 9);
	}
};

class Materail
{
public:
	int x, y, num, index; //用以记录位置
	int l, r, top, bottom;
	HBITMAP *hbm;
	Materail()
	{
		index = 0;
	}
	Materail(int n, char *fpath)
	{
		int i;
		char path[100];
		num = n;
		hbm = new HBITMAP[num];
		for(i = 0; i < num; i++)
		{
			sprintf(path, "%s\\%d.bmp", fpath, i);
			hbm[i] = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 
		0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
		}
		index= 0;
	}
	void set(int _x, int _y)
	{
		x = _x; y = _y;
	}
	void set(int n, char *fpath)
	{
		int i;
		char path[100];
		num = n;
		hbm = new HBITMAP[num];
		for(i = 0; i < num; i++)
		{
			sprintf(path, "%s\\%d.bmp", fpath, i);
			hbm[i] = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 
			0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
		}
		index= 0;	
	}
	
	void play(HDC &hDC, HDC &hdcmem, BITMAP &bm)
	{
		GetObject(hbm[index], sizeof(BITMAP), &bm);
		l = x - bm.bmWidth / 2;
		r = x + bm.bmWidth / 2;
		top = y - bm.bmHeight / 2;
		bottom = y + bm.bmHeight / 2;
		SelectObject(hdcmem, hbm[index]);
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight, hdcmem, 0, 0, SRCCOPY);
	}
	void paint(HDC &hDC, HDC &hdcmem, BITMAP &bm)
	{
		
		GetObject(hbm[index], sizeof(BITMAP), &bm);
		SelectObject(hdcmem, hbm[index]);
		l = x - bm.bmWidth / 2;
		r = x + bm.bmWidth / 2;
		top = y - bm.bmHeight / 4;
		bottom = y + bm.bmHeight / 4;
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight / 2,
			hdcmem, 0, bm.bmHeight / 2, SRCAND);
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight / 2,
			hdcmem, 0, 0, SRCPAINT);
		index = (index + 1) % num;
	}
	void paint_once(HDC &hDC, HDC &hdcmem, BITMAP &bm)
	{
		if (index==-1) return;
		GetObject(hbm[index], sizeof(BITMAP), &bm);
		SelectObject(hdcmem, hbm[index]);
		l = x - bm.bmWidth / 2;
		r = x + bm.bmWidth / 2;
		top = y - bm.bmHeight / 4;
		bottom = y + bm.bmHeight / 4;
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight / 2,
			hdcmem, 0, bm.bmHeight / 2, SRCAND);
		BitBlt(hDC, l, top, bm.bmWidth, bm.bmHeight / 2,
			hdcmem, 0, 0, SRCPAINT);
		index = (index + 1) % num;
		if (index == 0) index = -1;
	}
	void operator = (Materail b)
	{
		int i;
		num = b.num;
		hbm = new HBITMAP[b.num];
		for (i = 0; i < num; i++)
			hbm[i] = b.hbm[i];
		index = 0;
	}
};

class Materail bg(5, "src\\bg"), demo(10, "src\\demo\\role");
class Materail  shine(16, "src\\details\\shine"),
wing(19, "src\\details\\wing"), wave(15, "src\\details\\wave"), black(16, "src\\details\\black");
class Materail fire(21, "src\\details\\fire");
class Materail bgdemo(5, "src\\demo\\bg"), arrow(2, "src\\arrow");
class Materail vs(1, "src\\details\\vs"), font(5, "src\\details\\font");
class Materail death(14, "src\\details\\death"), ko(48, "src\\details\\ko");
class Materail spec1, spec2;
class Role role1, role2, role[5];

int role_index[2], choice[2]; // 人物选择 
int health[2] = {0}; // 人物血量
int power[2] = {0};  // 怒气值

int gamestatus = 0;

void AddPower(int &val, int a)
{
	val += a;
	if (val > 100) val = 100;
}


/*
	利用 gamestatus 控制整个游戏的画面转变 
	对于整个过程进行封装 便于后期的维护和调用 
*/

void SetGameStatus(int val)
{
	int i;
	gamestatus = val;
	switch(val)
	{
	case 0:

		 break;
	case 1:
		bgdemo.index = 0;
		break;
	case 2:
		bg.index = bgdemo.index;
		choice[0] = choice[1] = -1;
		role_index[0] = role_index[1] = 2;
		for (i = 0; i < 5; i++)
		{
			role[i].set(10);
			role[i].set(width / 5 * i + width / 10, height / 2);
		}
		break;
	case 3:
		
		bg.index = bgdemo.index;
		role1 = role[role_index[0]];
		role2 = role[role_index[1]];
		
		role1.angry = role2.angry = 0;
		health[0] = health[1] = total_health;
		power[0] = power[1] = 0;
		role1.dir = 0;
		role2.dir = 1;
		role1.x = client.cx / 10;
		role2.x = client.cx * 4 / 5 + client.cx / 10;

		role1.set(10);
		role2.set(10);

		spec1.index = spec2.index = -1;
		spec1.num = spec2.num = 0;

		break;
	case 4:
	
		if(health[0] < 0)
		{
			role1.set(1);
			role2.set(10);
		}else
		{
			role1.set(10);
			role2.set(1);
		}
		ko.index = 0;
		break;
	default:
		break;
	}
}

long WINAPI WndProc(HWND hWnd, UINT iMessage, UINT wParam, LONG lParam);
BOOL InitWindowsClass(HINSTANCE hInstance);
BOOL InitWindows(HINSTANCE hInstance, int nCmdShow);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG Message;
	if (!InitWindowsClass(hInstance)) return FALSE;
	if (!InitWindows(hInstance, nCmdShow)) return FALSE;
	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

bool IsTouch()
{
	
	if(role1.r >= role2.l && role1.dir == 0 && role1.x < role2.x) return true;
	if(role2.r >= role1.l && role2.dir == 0 && role2.x < role1.x) return true;
	if(role1.l <= role2.r && role1.dir == 1 && role1.x > role2.x) return true;
	if(role2.l <= role1.r && role2.dir == 1 && role2.x > role1.x) return true;
	
	
	return false;
}
long WINAPI WndProc(HWND hWnd, UINT iMessage, UINT wParam, LONG lParam)
{
	HDC hDC;
	HDC hdcmem;
	HDC canvasDC;
	HBRUSH hBrush;
	HPEN hPen;
	PAINTSTRUCT PtStr;
	HFONT hFont;
	TEXTMETRIC tm;
	BITMAP bm;
	SIZE size;
	HBITMAP canvas;
	int i;
	switch (iMessage)
	{
	case WM_KEYDOWN:
		switch(gamestatus)
		{
			/*
				当前的游戏状态 
				0 开场动画 | 1 选择游戏背景 | 2 选择游戏人物 | 3 格斗画面 | 4 游戏结束画面
			*/
			case 0:
				switch(wParam)
				{
				case VK_RETURN:
					
					SetGameStatus(1);
					break;
				}
				break;
			case 1:
				switch(wParam)
				{
				case VK_ESCAPE:
					SetGameStatus(0);
					break;
				case 'A':
				case 'a':
					bgdemo.index = (bgdemo.index - 1 + bgdemo.num) % bgdemo.num;
					break;
				case 'D':
				case 'd':
					bgdemo.index = (bgdemo.index + 1) % bgdemo.num;
					break;
				case VK_RETURN:
					role_index[0] = role_index[1] = 2;
					choice[0] = choice[1] = -1;
					SetGameStatus(2);
					
					break;
				default:
					break;
				}
				break;
			case 2:
				switch(wParam)
				{
				case VK_ESCAPE:
					SetGameStatus(1);
					break;
				case VK_LEFT:
					if(choice[1]!=-1) break;
					role_index[1] = (role_index[1] + 4) % 5;
 					break;
				case VK_RIGHT:
					if(choice[1]!=-1) break;
					role_index[1] = (role_index[1] + 1) % 5;
					break;
				default:
					break;
				}
				
				break;
			case 3:
				if (spec1.index != -1 && spec2.index != -1) break;
				switch(wParam)
				{
				case VK_ESCAPE:
					SetGameStatus(2);
					
					break;
				case VK_LEFT:
					if (role2.key == 5)
					{
						if (role2.l <= step) break;
						role2.x -= step;
						break;
					}
					if (GetKeyState('3') < 0)
					{
						role2.dir = 0;
						role2.set(8);
					}
					else
					{
						role2.dir = 1;
						role2.set(7);
					}
					
					if (role2.l <= step) break;
					role2.x -= step;
 					break;
				case VK_RIGHT:
					if (role2.key == 5)
					{
						if (role2.r >= client.cx - step) break;
						role2.x += step;
						break;
					}
					if (GetKeyState('3') < 0)
					{
						role2.dir = 1;
						role2.set(8);
					}
					else
					{
						role2.dir = 0;
						role2.set(7);
					}
					
					if(role2.r >= client.cx - step) break;
					role2.x += step;
					break;
				case VK_UP:
					
					role2.set(5);
					break;
				case VK_DOWN:
					role2.set(2);
					break;
				default:
					break;
				}
				break;
			case 4:
				switch(wParam)
				{
				case VK_RETURN:
					SetGameStatus(0);
					
					break;
				default:
					break;
				}
				break;
			default:
				break;
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_CHAR:
		switch(gamestatus)
		{
		case 2:
			switch(wParam)
			{
				case 'A':
				case 'a':
					if(choice[0]!=-1) break;
					role_index[0] = (role_index[0] + 4) % 5;
					break;
				case 'D':
				case 'd':
					if(choice[0]!=-1) break;
					role_index[0] = (role_index[0] + 1) % 5;
					break;
				case 'J':
				case 'j':
					choice[0] = role_index[0];
					break;
				case '1':
					choice[1] = role_index[1];
					break;
				default:
					break;
			}
			if(choice[0]!=-1 && choice[1]!=-1)
			{
				SetGameStatus(3);
			}
			break;
		case 3:
			if (spec1.index!=-1 && spec2.index!=-1) break;
			switch(wParam)
			{
			case 'A':
			case 'a':
				
				if (role1.key == 5)
				{
					if (role1.l <= step) break;
					role1.x -= step;
					break;
				}
				if (GetKeyState('L') < 0 || GetKeyState('l') < 0)
				{
					role1.set(8);
					role1.dir = 0;
				}
				else
				{
					role1.set(7);
					role1.dir = 1;
				}

				if(role1.l <= step) break;
				role1.x -= step;
				break;
			case 'D':
			case 'd':
				if (role1.key == 5)
				{
					if (role1.r >= client.cx - step) break;
					role1.x += step;
					break;
				}
				
				if (GetKeyState('L') < 0 || GetKeyState('l') < 0)
				{
					role1.set(8);
					role1.dir = 1;
				}
				else
				{
					role1.set(7);
					role1.dir = 0;
				}
				
				if(role1.r >= client.cx - step) break;
				role1.x += step;
				break;
			case 'W':
			case 'w':
				
				role1.set(5);
				break;
			case 'S':
			case 's':
				
				role1.set(2);
				break;
			case 'J':
			case 'j':

				if (!role1.IsReady()) break;
				
				if(IsTouch()) 
				{
					if (role2.key != 0)
					{
						if (role1.angry)
						{
							health[1] -= harm_j * 2; AddPower(power[1], harm_j * 20);
						}
						else
						{
							health[1] -= harm_j; AddPower(power[1], harm_j * 10);
						}
							
						role2.set(11);
					}
					else
					{
						if (role1.angry)
						{
							health[1] -= harm_j; AddPower(power[1], harm_j * 10);
						}
						else
						{
							health[1] -= harm_j / 2; AddPower(power[1], harm_j * 5);
						}
	
					}
				}
				role1.set(4);
				
				break;
			case 'K':
			case 'k':

				if (!role1.IsReady()) break;
				
				if(IsTouch())
				{
					
					if (GetKeyState('S') < 0 || GetKeyState('s') < 0)
					{
						if (role1.angry)
						{
							health[1] -= 2 * harm_k;
							AddPower(power[1], harm_k * 20);
						}
						else
						{
							health[1] -= harm_k;
							AddPower(power[1], harm_k * 10);
						}
							
						role2.set(12);
					}
					else
					{
						if (role2.key != 0)
						{
							if (role1.angry)
							{
								health[1] -= 2 * harm_k;
								AddPower(power[1], harm_k * 20);
							}
							else
							{
								health[1] -= harm_k;
								AddPower(power[1], harm_k * 10);
							}
								
							role2.set(11);
						}
						else
						{
							if (role1.angry)
							{
								health[1] -= harm_k;
								AddPower(power[1], harm_k * 10);
							}
							else
							{
								health[1] -= harm_k / 2;
								AddPower(power[1], harm_k * 5);
							}
								
						}
					}
				}
				if (GetKeyState('S') < 0 || GetKeyState('s') < 0)
				{
					role1.set(3); 
				}
				else
				{
					role1.set(6); 
				}
					
				break;
			case 'L':
			case 'l':
				role1.set(0);
				break;
			case 'U':
			case 'u':
				if (power[0] == 100)
				{
					power[0] = 0;
					spec1 = wing;
					role1.angry = 1;
				}
				break;
			case 'I':
			case 'i':
				if (power[0] == 100)
				{
					power[0] = 0;
					spec1 = wave;
					spec2 = black;
					health[1] -= harm_i;
					role1.x = client.cx / 10;
					role2.x = client.cx * 4 / 5 + client.cx / 10;
					role1.dir = 0;
					role2.dir = 1; 
					role1.set(10);
					role2.set(11);
				}
				break;
			case '1':
				if (!role2.IsReady()) break;
				if(IsTouch()) 
				{
					if (GetKeyState('L') >= 0 && GetKeyState('l') >= 0)
					{
						if (role2.angry)
						{
							health[0] -= 2 * harm_j;
							AddPower(power[0], harm_j * 20);
						}
						else
						{
							health[0] -= harm_j;
							AddPower(power[0], harm_j * 10);
						}
								
						role1.set(11);
					}
					else
					{
						if (role2.angry)
						{
							health[0] -= harm_j;
							AddPower(power[0], harm_j * 10);
						}
						else
						{
							health[0] -= harm_j / 2;
							AddPower(power[0], harm_j * 5);
						}
							
					}
					
				}
				role2.set(4);
				break;
			case '2':
				if (!role2.IsReady()) break;

				if(IsTouch()) 
				{
					
					if (GetKeyState(VK_DOWN) < 0)
					{
						role1.set(12);
						if (role2.angry)
						{
							health[0] -= 2 * harm_j;
							AddPower(power[0], harm_k * 20);
						}
						else
						{
							health[0] -= harm_j;
							AddPower(power[0], harm_k * 10);
						}
							
					}
					else
					{
						if (GetKeyState('L') >= 0 && GetKeyState('l') >= 0)
						{
							if (role2.angry)
							{
								health[0] -= 2 * harm_k;
								AddPower(power[0], harm_k * 20);
							}
							else
							{
								health[0] -= harm_k;
								AddPower(power[0], harm_k * 10);
							}
							role1.set(11);
						}
						else
						{
							if (role2.angry)
							{
								health[0] -= harm_k;
								AddPower(power[0], harm_k * 10);
							}
							else
							{
								health[0] -= harm_k / 2;
								AddPower(power[0], harm_k * 5);
							}
								
						}
					}
						
				}
				if (GetKeyState(VK_DOWN) < 0)
					role2.set(3);
				else
					role2.set(6);
				break;
			case '3':
				if (GetKeyState(VK_RIGHT) < 0)
				{
					role2.set(8);
					role2.dir = 1;
				}
				else if (GetKeyState(VK_LEFT) < 0)
				{
					role2.set(8);
					role2.dir = 0;
				}else
					role2.set(0);	
				break;
			case '4':
				if (power[1] == 100)
				{
					power[1] = 0;
					spec2 = wing;
					role2.angry = 1;
				}
				break;
			case '5':
				if (power[1] == 100)
				{
					power[1] = 0;
					spec2 = wave;
					spec1 = black;
					health[0] -= harm_i;
					role1.x = client.cx / 10;
					role2.x = client.cx * 4 / 5 + client.cx / 10;
					role1.dir = 0;
					role2.dir = 1;
					role1.set(11);
					role2.set(10);
				}
				break;
			default:
				break;
			}
			if(health[0] < 0 || health[1] < 0)
			{
				SetGameStatus(4);
			}
			break;
		default:
			break;
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &PtStr);
		canvasDC = CreateCompatibleDC(hDC);
		canvas = CreateCompatibleBitmap(hDC, client.cx, client.cy);
		SelectObject(canvasDC, canvas);
		hdcmem = CreateCompatibleDC(canvasDC);
		
		switch(gamestatus)
		{
		case 0:
			bg.play(canvasDC, hdcmem, bm);
			
			
			font.index = 4;
			font.set(client.cx / 2, client.cy / 2);
			font.paint(canvasDC, hdcmem, bm);

			for (i = 0; i < 5; i += 4)
			{
				fire.x = client.cx * i / 5 + client.cx / 10;
				fire.y = client.cy * 3 / 4;
				fire.paint(canvasDC, hdcmem, bm);
			}

			for (i = 0; i < 5; i += 4)
			{
				shine.x = client.cx * i / 5 + client.cx / 10;
				shine.y = client.cy  / 5 ;
				shine.paint(canvasDC, hdcmem, bm);
			}
		
			break;
		case 1:

			bgdemo.play(canvasDC, hdcmem, bm);
			break;
		case 2:
			
			bg.play(canvasDC, hdcmem, bm);
			
			for(i = 0; i < 5; i++)
			{
				role[i].set(client.cx / 10 + client.cx * i / 5, client.cy / 2);
				role[i].play(canvasDC, hdcmem, bm);
			}
			
			arrow.index = 0;

			arrow.x = role_index[0] * (width / 5) + width / 10;
			arrow.y = 130;
			arrow.paint(canvasDC, hdcmem, bm);

			arrow.x = role_index[1] * (width / 5) + width / 10;
			arrow.y = 350;
			arrow.paint(canvasDC, hdcmem, bm);

			break;
		case 3:

			bg.play(canvasDC, hdcmem, bm);
			vs.paint(canvasDC, hdcmem, bm);
		
			blood.cx = 280; blood.cy = 30;
			enegy.cx = 140; enegy.cy = 30;

			hPen = (HPEN) CreatePen(0, 10, RGB(255, 0, 0));
			hBrush = (HBRUSH) CreateSolidBrush(RGB(255, 0, 0));
			SelectObject(canvasDC, hPen);
			SelectObject(canvasDC, hBrush);
			RoundRect(canvasDC, 50, 50 - blood.cy / 2, 50 + blood.cx * health[0] / total_health,
				50 + blood.cy / 2, blood.cy / 2, blood.cy / 2);
			DeleteObject(hPen);
			DeleteObject(hBrush);

			hPen = (HPEN) CreatePen(0, 10, RGB(0, 0, 255));
			hBrush = (HBRUSH) CreateSolidBrush(RGB(0, 0, 255));
			SelectObject(canvasDC, hPen);
			SelectObject(canvasDC, hBrush);
			RoundRect(canvasDC, client.cx - 50 - health[1] * blood.cx / total_health, 50 - blood.cy / 2, 
				client.cx - 50, 50 + blood.cy / 2, blood.cy / 2, blood.cy / 2);
			DeleteObject(hPen);
			DeleteObject(hBrush);

			demo.index = role_index[0] * 2;
			demo.set(25, 50);
			demo.paint(canvasDC, hdcmem, bm);

			demo.index = role_index[1] * 2 + 1;
			demo.set(client.cx - 25, 50);
			demo.paint(canvasDC, hdcmem, bm);

			font.index = 2;
			font.set(49, client.cy - 32);
			font.paint(canvasDC, hdcmem, bm);

			font.index = 3;
			font.set(client.cx-49, client.cy - 32);
			font.paint(canvasDC, hdcmem, bm);

			hPen = (HPEN) CreatePen(0, 10, RGB(255, 255, 0));
			hBrush = (HBRUSH) CreateSolidBrush(RGB(255, 255, 0));
			SelectObject(canvasDC, hPen);
			SelectObject(canvasDC, hBrush);

			RoundRect(canvasDC, 98, client.cy - 32 - blood.cy / 2, 
				98 + enegy.cx * power[0] / 100, client.cy - 32 + enegy.cy / 2, enegy.cy / 2, enegy.cy / 2);

			RoundRect(canvasDC, client.cx - 98 - enegy.cx * power[1] / 100, client.cy - 32 - enegy.cy / 2,
				client.cx - 98, client.cy - 32 + enegy.cy / 2, enegy.cy / 2, enegy.cy / 2);

			if (power[0] == 100)
			{
				font.index = 0;
				font.set(98 + 49 + enegy.cx, client.cy - 32);
				font.paint(canvasDC, hdcmem, bm);
			}

			if (power[1] == 100)
			{
				font.index = 1;
				font.set(client.cx - 98 - 49 - enegy.cx, client.cy - 32);
				font.paint(canvasDC, hdcmem, bm);
			}

			

			spec1.set(role1.x, role1.y);
			spec2.set(role2.x, role2.y);

			mirror = (mirror + 1) % 2;
			

			if (spec1.num == 15)
			{
				spec1.paint_once(canvasDC, hdcmem, bm);
				if (mirror)
				{
					role2.play(canvasDC, hdcmem, bm);
					role1.play(canvasDC, hdcmem, bm);
				}
				else
				{
					role1.play(canvasDC, hdcmem, bm);
					role2.play(canvasDC, hdcmem, bm);
				}
				
				spec2.paint_once(canvasDC, hdcmem, bm);
			}
			else if (spec2.num == 15)
			{
				spec2.paint_once(canvasDC, hdcmem, bm);
				if (mirror)
				{
					role2.play(canvasDC, hdcmem, bm);
					role1.play(canvasDC, hdcmem, bm);
				}
				else
				{
					role1.play(canvasDC, hdcmem, bm);
					role2.play(canvasDC, hdcmem, bm);
				}
				spec1.paint_once(canvasDC, hdcmem, bm);
			}
			else
			{
				if (mirror)
				{
					role2.play(canvasDC, hdcmem, bm);
					role1.play(canvasDC, hdcmem, bm);
				}
				else
				{
					role1.play(canvasDC, hdcmem, bm);
					role2.play(canvasDC, hdcmem, bm);
				}
				spec1.paint_once(canvasDC, hdcmem, bm);
				spec2.paint_once(canvasDC, hdcmem, bm);
			}

			if (spec2.num == 16)
			{
				if (spec2.index == 1) role2.set(11);
				if (spec2.index == 5) role2.set(11);
				if (spec2.index == 13) role2.set(12);
			}

			if (spec1.num == 16)
			{
				if (spec1.index == 1) role1.set(11);
				if (spec1.index == 5) role1.set(11);
				if (spec1.index == 13) role1.set(12);
			}

			DeleteObject(hPen);
			DeleteObject(hBrush);
			

			break;
		case 4:
			
			bg.play(canvasDC, hdcmem, bm);

			
			ko.x = client.cx / 2;
			ko.y = client.cy / 2;

			ko.paint(canvasDC, hdcmem, bm);
			

			if(health[1] < 0)
			{
				death.x = role2.x;
				death.y = role2.y;

				
				role2.play(canvasDC, hdcmem, bm);
				death.paint(canvasDC, hdcmem, bm);
				role1.play(canvasDC, hdcmem, bm);
				
			}else
			{
				death.x = role1.x;
				death.y = role1.y;

			
				role1.play(canvasDC, hdcmem, bm);
				death.paint(canvasDC, hdcmem, bm);
				role2.play(canvasDC, hdcmem, bm);
			}
			
			break;
		default:
			break;
		}
		BitBlt(hDC, 0, 0, client.cx, client.cy, canvasDC, 0, 0, SRCCOPY);
		DeleteObject(canvasDC);
		DeleteObject(hdcmem);
		DeleteObject(hDC);
		EndPaint(hWnd, &PtStr);
		Sleep(interval);
		InvalidateRect(hWnd, NULL, false);
		return 0;
	case WM_DESTROY:
		
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:

		//各个素材位置的初始化 部分没有设置自动对齐 手动调整的位置

		
		MoveWindow(hWnd, 0, 0, width, height, false);

		GetClientRect(hWnd, &rect);

		client.cx = rect.right - rect.left;
		client.cy = rect.bottom - rect.top;
		
		bgdemo.set(client.cx / 2, client.cy / 2);

		bg.set(client.cx / 2, client.cy / 2);

		vs.set(client.cx / 2, 50); 
		
		shine.y = client.cy / 2;

		shine.x = client.cx / 10 + client.cx * 2 / 5;

		return 0;
	case WM_ERASEBKGND:
		return true;
		break;
	default:
		return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
}

void init_role()
{
	/*Egod*/
	role[0].set(0, 5, "src\\role\\Egod\\defend");
	role[0].set(1, 7, "src\\role\\Egod\\die");
	role[0].set(2, 2, "src\\role\\Egod\\down");
	role[0].set(3, 7, "src\\role\\Egod\\downkick");
	role[0].set(4, 10, "src\\role\\Egod\\hit");
	role[0].set(5, 5, "src\\role\\Egod\\jump");
	role[0].set(6, 10, "src\\role\\Egod\\kick");
	role[0].set(7, 8, "src\\role\\Egod\\move");
	role[0].set(8, 28, "src\\role\\Egod\\pave");
	role[0].set(9, 9, "src\\role\\Egod\\stand");
	role[0].set(10, 24, "src\\role\\Egod\\win");
	role[0].set(11, 4, "src\\role\\Egod\\beat");
	role[0].set(12, 8, "src\\role\\Egod\\hurt");

	

	/*Ku*/
	role[1].set(0, 2, "src\\role\\Ku\\defend");
	role[1].set(1, 8, "src\\role\\Ku\\die");
	role[1].set(2, 10, "src\\role\\Ku\\down");
	role[1].set(3, 8, "src\\role\\Ku\\downkick");
	role[1].set(4, 10, "src\\role\\Ku\\hit");
	role[1].set(5, 17, "src\\role\\Ku\\jump");
	role[1].set(6, 7, "src\\role\\Ku\\kick");
	role[1].set(7, 10, "src\\role\\Ku\\move");
	role[1].set(8, 6, "src\\role\\Ku\\pave");
	role[1].set(9, 26, "src\\role\\Ku\\stand");
	role[1].set(10, 16, "src\\role\\Ku\\win");
	role[1].set(11, 4, "src\\role\\Ku\\beat");
	role[1].set(12, 4, "src\\role\\Ku\\hurt");
	/*Wu*/
	role[2].set(0, 2, "src\\role\\Wu\\defend");
	role[2].set(1, 5, "src\\role\\Wu\\die");
	role[2].set(2, 2, "src\\role\\Wu\\down");
	role[2].set(3, 28, "src\\role\\Wu\\downkick");
	role[2].set(4, 10, "src\\role\\Wu\\hit");
	role[2].set(5, 7, "src\\role\\Wu\\jump");
	role[2].set(6, 10, "src\\role\\Wu\\kick");
	role[2].set(7, 6, "src\\role\\Wu\\move");
	role[2].set(8, 7, "src\\role\\Wu\\pave");
	role[2].set(9, 26, "src\\role\\Wu\\stand");
	role[2].set(10, 60, "src\\role\\Wu\\win");
	role[2].set(11, 5, "src\\role\\Wu\\beat");
	role[2].set(12, 5, "src\\role\\Wu\\hurt");
	/*Yin*/
	role[3].set(0, 4, "src\\role\\Yin\\defend");
	role[3].set(1, 7, "src\\role\\Yin\\die");
	role[3].set(2, 0, "src\\role\\Yin\\down");
	role[3].set(3, 0, "src\\role\\Yin\\downkick");
	role[3].set(4, 10, "src\\role\\Yin\\hit");
	role[3].set(5, 9, "src\\role\\Yin\\jump");
	role[3].set(6, 10, "src\\role\\Yin\\kick");
	role[3].set(7, 9, "src\\role\\Yin\\move");
	role[3].set(8, 8, "src\\role\\Yin\\pave");
	role[3].set(9, 30, "src\\role\\Yin\\stand");
	role[3].set(10, 35, "src\\role\\Yin\\win");
	role[3].set(11, 4, "src\\role\\Yin\\beat");
	role[3].set(12, 4, "src\\role\\Yin\\hurt");
	/*You*/
	role[4].set(0, 4, "src\\role\\You\\defend");
	role[4].set(1, 4, "src\\role\\You\\die");
	role[4].set(2, 2, "src\\role\\You\\down");
	role[4].set(3, 7, "src\\role\\You\\downkick");
	role[4].set(4, 10, "src\\role\\You\\hit");
	role[4].set(5, 9, "src\\role\\You\\jump");
	role[4].set(6, 10, "src\\role\\You\\kick");
	role[4].set(7, 10, "src\\role\\You\\move");
	role[4].set(8, 12, "src\\role\\You\\pave");
	role[4].set(9, 5, "src\\role\\You\\stand");
	role[4].set(10, 14, "src\\role\\You\\win");
	role[4].set(11, 4, "src\\role\\You\\beat");
	role[4].set(12, 5, "src\\role\\You\\hurt");
}
BOOL InitWindows(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hWnd = CreateWindow("WinFill",
		"KOF",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		NULL,
		NULL,
		hInstance,
		NULL);

	
	if (!hWnd) return FALSE;
	gamestatus = 0;
	/*
		当前的游戏状态 
		0 开场动画 | 1 选择游戏背景 | 2 选择游戏人物 | 3 格斗画面 | 4 游戏结束画面
	*/
	sndPlaySound("src\\sound\\0.wav", SND_ASYNC);
	init_role();
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
BOOL InitWindowsClass(HINSTANCE hInstance)
{
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = NULL;
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = "WinFill";
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClass(&WndClass);
}
