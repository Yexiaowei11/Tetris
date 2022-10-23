#include <iostream>
#include <thread>
#include <vector>

#include <stdio.h>
#include <Windows.h>
using namespace std;

// 定义屏幕
int nScreenWidth = 80; //屏幕宽
int nScreenHeight = 40; //屏幕高
//画布
int nFieldWidth = 13;//画布宽
int nFieldHeight = 18;//画布高
//方块
wstring tetromino[7];
//画面填充字符
unsigned char* pField = nullptr;

//坐标旋转函数
int Rotate(int px, int py, int r) {

	int pi = 0;

	switch (r % 4) {
	case 0: // 0 degrees			
		pi = py * 4 + px;
		break;
	case 1: // 90 degrees			
		pi = 12 + py - (px * 4);
		break;
	case 2: // 180 degrees			
		pi = 15 - (py * 4) - px;
		break;
	case 3: // 270 degrees			
		pi = 3 - py + (px * 4);
		break;
	}

	return pi;
}

//碰撞检测函数
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	// The field is full so zero spaces
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++) {
			int pi = Rotate(px, py, nRotation);

			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			// Check that test is in bounds. Note out of bounds does
			// not necessarily mean a fail, as the long vertical piece
			// can have cells that lie outside the boundary, so we'll
			// just ignore them
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
						return false; // fail on first hit
				}
			}
		}

	return true;
}


int main()
{	//  方块4x4，一共7种形状
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	//画布字符初始化
	//9表示边框，0是空白
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	//屏幕初始化
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;





	bool bKey[4];  //存储按键 ←→↓Z
	bool bGameOver = false; //为true时游戏结束
	int nCurrentPiece = 1;  //当前方块类型（有7种）
	int nCurrentRotation = 0;//当前旋转角度
	int nCurrentX = nFieldWidth / 2;//当前方块的X坐标
	int nCurrentY = 0;      //当前方块的Y坐标

	int nSpeed = 20;    //速度，时间是更新间隔数，nSpeed越小，方块下沉速度越快。
	int nSpeedCount = 0;//配合nSpeed来控制定时器
	int nPieceCount = 0;//方块计数
	bool bForceDown = false;//强制方块下降

	bool bRotateHold = false;//旋转键锁定，防止鬼畜旋转
	int nScore = 0;		    //游戏得分
	vector<int> vLines;     //记录整行

	//主循环：
	//1.定时器
	//2.接收输入
	//3.游戏逻辑处理
	//4.画面输出
	while (!bGameOver)
	{

		//定时器===========
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		//输入===================
		for (int k = 0; k < 4; k++)                               //R L D Z  按下返回true
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		// 游戏 逻辑     =================

		//按键←→↓
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		//Z
		if (bKey[3]) {
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else
			bRotateHold = true;


		//下沉检测
		if (bForceDown) {
			// Update difficulty every 10 pieces 每10块就加快速度（更新难度）
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 10 == 0) //每累积10块，就加快速度（这里的nSpeed是间隔，间隔减小，下落加快）
				if (nSpeed >= 10) nSpeed--;

			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; // It can, so do it!
			else {
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				// Check for lines 检查整行
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine) {
							// Remove Line if it's complete  整行时消除该行
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// If the player reachs the field limits, game over!
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}



		//显示输出  （渲染） ==============

		//输出字符
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
		//输出当前方块
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;  //65=‘A’

		// 显示分数
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCIRE: %8d", nScore);

		//消除一行，将用迭代方式逐行下沉
		if (!vLines.empty()) {
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto& v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++) {
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}																									  //向屏幕输出
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	// 游戏结束，显示得分
	CloseHandle(hConsole);
	cout << "游戏结束 你的分数是:" << nScore << endl;
	cout << " Game over.Your score is :" << nScore << endl;
	system("pause");
	return 0;
}

