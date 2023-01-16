/////////////////////////////////////
// binaryGame - a game by Geckoo1337
/////////////////////////////////////
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <future>

typedef unsigned char byte;
// required library
#pragma comment(lib, "winmm.lib")
/////////////////////////////////////
// MakeNoise.h
/////////////////////////////////////
class MakeNoise
{
public:
	void playTrack(const int& time, const int& SamplesPerSec, int loop = 1);

private:
	char* buffer;
	HWAVEOUT hWaveOut = NULL;
	WAVEFORMATEX wfx;
};
// here we can write directly our algorithm so as to generate a track
inline char algorithm(int t)
{
	return (t * 3 & t >> 10) | (t * 5 & t >> 7);
}

void MakeNoise::playTrack(const int& time, const int& samplesPerSec, int loop)
{	// allocate memory
	size_t size = static_cast<size_t>((time * samplesPerSec) / 1000);
	buffer = new char[size];
	// minimal instance
	if (loop < 1)
		loop = 1;

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 0x01;
	wfx.nSamplesPerSec = samplesPerSec;
	wfx.nAvgBytesPerSec = samplesPerSec;
	wfx.nBlockAlign = 0x01;
	wfx.wBitsPerSample = 0x08;
	wfx.cbSize = NULL;
	// https://docs.microsoft.com/en-us/windows/win32/api/mmeapi/nf-mmeapi-waveoutopen
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL);
	// fills our buffer with your algorithm for track
	for (DWORD t = 0; t < size; t++)
		buffer[t] = static_cast<char>(algorithm(t));
	// header
	WAVEHDR header = { buffer, static_cast<DWORD>(size), NULL, NULL, WHDR_BEGINLOOP | WHDR_ENDLOOP, static_cast<DWORD>(loop), 0, static_cast<int>(NULL) };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); // clean up
	// The waveOutClose function closes the given waveform-audio output device
	waveOutClose(hWaveOut);
	waveOutSetVolume(hWaveOut, 0x0FFF0FFF); // set volume
	waveOutSetPlaybackRate(hWaveOut, 0x0000FFFF); // pitch
	// sleep for a while - track is running
	Sleep((time * loop));
	// clean up
	delete[] buffer;
}
/////////////////////////////////////
// customConsole.h
/////////////////////////////////////
class customConsole {

protected:
	HANDLE hConsole;
	HWND hWnd;
	// constructor
	customConsole();
	const std::string card1 = ">>> CARD 1";
	const std::string card2 = ">>> CARD 2";

private:
	static std::pair<int, int> GetDesktopResolution();
	void setFont();
	const int consoleSize = 600;
	const int fontSize = 22;
};
/////////////////////////////////////
// customConsole.cpp
/////////////////////////////////////
customConsole::customConsole()
{
	hWnd = GetConsoleWindow(); // no resize
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	// get desktop resolution
	const std::pair<int, int> rs = GetDesktopResolution();
	MoveWindow(
		hWnd,
		rs.first / 2 - (consoleSize / 2),
		rs.second / 2 - (consoleSize / 2) - (consoleSize / 5),
		consoleSize - (consoleSize / 3),
		consoleSize,
		false);
	// title feature
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle(L"OLC Code Jam 2022");
	// no scrollbar
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);

	COORD _size =
	{
		info.srWindow.Right - info.srWindow.Left + 1,
		info.srWindow.Bottom - info.srWindow.Top + 1
	};

	SetConsoleScreenBufferSize(hConsole, _size);
	setFont();
}

std::pair<int, int>customConsole::GetDesktopResolution()
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	return std::make_pair(desktop.right, desktop.bottom);
}

void customConsole::setFont()
{
	CONSOLE_FONT_INFOEX info = { 0 };
	info.cbSize = sizeof(info);
	info.dwFontSize.X = 0;
	info.dwFontSize.Y = fontSize;
	info.FontWeight = FW_NORMAL;
	wcscpy_s(info.FaceName, L"Consolas"); // instead of terminal
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &info);
}
/////////////////////////////////////
// binaryGame.h
/////////////////////////////////////
class binaryGame : customConsole {

public:
	binaryGame(int w, int h);

private: // randomize a float
	inline float fRnd() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }
	byte* field, * target;
	int width{}, height{}, steps{};

	void intro();
	void gameLoop();
	void display();
	void boards(std::string t, byte* f);
	bool isSolved();
	void createTarget();
	void fColumn(int c, bool s); // flip column
	void fRow(int r, bool s); // flip row	
	void shuffleBits();
	void createBoard();
	void winner();
};
/////////////////////////////////////
// binaryGame.cpp
/////////////////////////////////////
binaryGame::binaryGame(int w, int h)
{
	intro();

	width = w;
	height = h;

	createBoard();
	gameLoop();
}

void binaryGame::intro()
{
	const int ooo = 32;
	const int l = 44;
	const std::string name = "Punched-Cards Game";

	for (int i = 0; i < ooo; ++i)
	{	// create a simple intro system with game title
		if (i == (ooo / 2))
		{
			std::cout << static_cast<char>(255);
			SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_INTENSITY);

			for (int j = 0; j < ((l / 2) - (name.length() / 2) - 1); ++j)
				std::cout << static_cast<char>(255);
			// title
			std::cout << name;

			for (int j = 0; j < ((l / 2) - (name.length() / 2)); ++j)
				std::cout << static_cast<char>(255);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
		}
		else
			for (int j = 0; j < l; ++j)
				if (j % 2)
					std::cout << static_cast<char>(254);
				else
					std::cout << static_cast<char>(255);

		std::cout << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
	// short melody here
	Beep(1000, 100);
	Beep(1200, 100);
	Beep(1400, 100);
	Beep(1000, 300);

	std::cout << " Press a key to start a game ... ";
	int x = _getch();
}

void binaryGame::gameLoop()
{
	int moves{};

	while (!isSolved() && steps > 0)
	{
		display();
		// simple tutorial at start
		if (width == 3)
		{
			std::cout << std::endl;
			std::cout << " HOW TO PLAY ?" << std::endl;
			std::cout << " Hello! Above you have a first punch card" << std::endl;
			std::cout << " (Card 1) which must be reproduced in the" << std::endl;
			std::cout << " second card (Card 2). Select a row or a " << std::endl;
			std::cout << " column so as to invert each byte to true" << std::endl;
			std::cout << " or false until cards fit one with other." << std::endl;
			std::cout << " A red cell means that this byte does not" << std::endl;
			std::cout << " exist in the standard card ... Steps are" << std::endl;
			std::cout << " limited according to the scramble round." << std::endl;
			std::cout << " Seven levels are available. Have fun :) " << std::endl;
			std::cout << std::endl;
			std::cout << std::endl;
		}

		std::string r;
		std::cout << " Enter rows letters and/or column numbers" << std::endl;
		std::cout << " >>> ";
		std::cin >> r;
		// boring?
		if (r == "exit")
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			exit(0);
		}
		// limit length flips
		if (r.length() > steps)
			r.resize(steps);

		for (std::string::iterator i = r.begin(); i != r.end(); ++i)
		{
			byte ii = (*i);

			if (ii - 1 >= '0' && ii - 1 <= '9')
			{
				fColumn(ii - '1', false);
				++moves;
			}
			else if (ii >= 'a' && ii <= 'z')
			{
				fRow(ii - 'a', false);
				++moves;
			}
		}
	}

	display();

	if (isSolved())
	{
		Beep(660, 100);
		Beep(880, 100);
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | BACKGROUND_GREEN);
		std::cout << " Well done! Puzzle solved using " << moves << " steps " << std::endl;
		std::cout << std::endl;
		// next level
		++height;
		++width;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

		if (height > 9 || width > 9)
			winner();
	}
	else {
		for (int b = 0; b < 3; ++b)
			Beep(220, 80); // bip bip bip

		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | BACKGROUND_RED);
		std::cout << " No More Step Available :/ " << std::endl;
		std::cout << std::endl;
		// retrograde
		--height;
		--width;

		if (height < 3)height = 3;
		if (width < 3)width = 3;
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

	std::cout << " Press any key to continue game ... ";
	int x = _getch();
	// restart
	createBoard();
	gameLoop();
}

void binaryGame::display()
{
	Beep(440, 200);
	system("cls");
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_INTENSITY);

	if (steps > 0)
		std::cout << " LVL " << (width - 2) << " > Allowed Steps : " << steps << " " << std::endl << std::endl;
	else
		std::cout << " LVL " << (width - 2) << " > Allowed Steps : 0 " << std::endl << std::endl;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

	boards(card1, target);
	boards(card2, field);
}

void binaryGame::boards(std::string t, byte* f)
{
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
	std::cout << static_cast<char>(255) << t << std::endl;

	std::cout << static_cast<char>(255) << static_cast<char>(201);

	for (int i = 0; i < (width * 2) + 3; ++i)
		std::cout << static_cast<char>(205);

	std::cout << static_cast<char>(187);
	std::cout << std::endl;
	std::cout << static_cast<char>(255) << static_cast<char>(186) << static_cast<char>(255);

	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
	std::cout << static_cast<char>(255);

	for (int x = 0; x < width; ++x)
		std::cout << static_cast<char>(255) << static_cast<char>(x + '1');

	std::cout << static_cast<char>(255);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
	std::cout << static_cast<char>(186);
	std::cout << std::endl;

	for (int y = 0; y < height; ++y)
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
		std::cout << static_cast<char>(255) << static_cast<char>(186);

		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
		std::cout << static_cast<char>(255) << static_cast<char>(y + 'A') << static_cast<char>(255);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

		for (int x = 0; x < width; ++x)
		{
			if (target[x + y * width] != f[x + y * width])
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			else
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			// this way we have a block or a space
			std::cout << static_cast<char>(f[x + y * width] + 254) << static_cast<char>(255);
		}

		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
		std::cout << static_cast<char>(186);
		std::cout << std::endl;
	}

	std::cout << static_cast<char>(255) << static_cast<char>(200);

	for (int i = 0; i < (width * 2) + 3; ++i)
		std::cout << static_cast<char>(205);

	std::cout << static_cast<char>(188);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	std::cout << std::endl << std::endl;
}

bool binaryGame::isSolved()
{
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
			if (target[x + y * width] != field[x + y * width])
				return false; // this cell is not good

	return true;
}

void binaryGame::winner()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	system("cls");

	for (int i = 1; i < 16; ++i)
	{
		SetConsoleTextAttribute(hConsole, i);
		std::cout << " Congratulations! You've solved all levels." << std::endl;
	}

	for (int i = 14; i >= 0; --i)
	{
		SetConsoleTextAttribute(hConsole, i);
		std::cout << " Congratulations! You've solved all levels." << std::endl;
	}
	// short melody here
	Beep(1000, 100);
	Beep(1200, 100);
	Beep(1400, 100);
	Beep(1000, 300);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	std::cout << std::endl;
	std::cout << " Thanks for playing :) ";

	int x = _getch();
	PostMessage(hWnd, WM_CLOSE, 0, 0);
	exit(0);
}

void binaryGame::createTarget()
{
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
			if (fRnd() < 0.5f)
				target[x + y * width] = 1;
			else
				target[x + y * width] = 0;

	memcpy(field, target, width * height);
}

void binaryGame::fColumn(int c, bool s)
{
	for (int x = 0; x < height; ++x)
		field[c + x * width] = !field[c + x * width];

	if (!s)
		--steps;
}

void binaryGame::fRow(int r, bool s)
{
	for (int x = 0; x < width; ++x)
		field[x + r * width] = !field[x + r * width];

	if (!s)
		--steps;
}

void binaryGame::shuffleBits()
{
	int flips{};
	steps = 0;
	std::pair<std::vector<int>, std::vector<int>> check;
	// it does not allows easy pattern
	do {
		flips = (rand() % width + rand() % height);
	} while (flips < ((width + height) / 2) - 1);

	for (int x = 0; x < flips; ++x)
	{
		if (fRnd() < 0.5f)
		{
			int c = rand() % width;

			std::vector<int>::iterator it;
			it = find(check.first.begin(), check.first.end(), c);

			if (it == check.first.end())
			{
				fColumn(c, true);
				check.first.push_back(c);
				++steps;
			}
		}
		else {
			int c = rand() % height;

			std::vector<int>::iterator it;
			it = find(check.second.begin(), check.second.end(), c);

			if (it == check.second.end())
			{
				fRow(c, true);
				check.second.push_back(c);
				++steps;
			}
		}
	}
}

void binaryGame::createBoard()
{
	int t = width * height;
	field = new byte[t];
	target = new byte[t];

	memset(field, 0, t);
	memset(target, 0, t);
	createTarget();

	while (true)
	{
		shuffleBits();

		if (!isSolved())
			break;
	}
}
/////////////////////////////////////
// multithread for background sound
/////////////////////////////////////
bool newThread()
{	// thread and argument
	// std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::thread t(&MakeNoise::playTrack,
		MakeNoise(),// class
		32767,		// sequence duration (byte multiple)
		8000,		// samples per second
		INT_MAX);	// loop number

	t.detach(); // don't use join()
	return true;
}
/////////////////////////////////////
// main part
/////////////////////////////////////
int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	// multithreading
	std::future<bool> bgThread = std::async(std::launch::async, newThread);
	std::future_status status{}; // alive or not ?

	binaryGame* g = new binaryGame(3, 3);
	delete g;

	return 0;
}