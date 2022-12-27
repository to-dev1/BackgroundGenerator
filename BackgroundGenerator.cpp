#include <windows.h>
#include <vector>
#include <sstream>
#include <math.h>
#include <chrono>

//STRUCTS

//Color class, will be converted to int with the windows RGB
struct Color
{
	int r, g, b;

	//Default
	Color()
	{
		r = 0;
		g = 0;
		b = 0;
	}

	Color(int red, int green, int blue)
	{
		r = red;
		g = green;
		b = blue;
	}
};

//GLOBAL

//Image system
int bufferWidth;
int bufferHeigth;
int bufferWidthOffset;
int bufferHeigthOffset;
BITMAPINFO bitmapInfo;

//Time and frames
std::chrono::duration<float> TotalTime;
bool running = true;

//Random
int generatorSeed = 567563;
int seed = 5623672567;

//Rendering
std::vector<int> ScreenPixels;
std::vector<Color> ScreenColors;

//MISC FUNCTIONS

inline int Random(int Seed)
{
	generatorSeed = (214013 * generatorSeed + Seed);
	return (generatorSeed >> 16) & 0x7FFF;
}

//GENERATOR FUNCTIONS

void PlaceCell(int x, int y, int cell, std::vector<int>* cells)
{
	if (x > 0 && x < bufferWidth && y > 0 && y < bufferHeigth && cell > 0)
	{
		if ((*cells)[x + y * bufferWidth] == 0) (*cells)[x + y * bufferWidth] = cell;
	}
}

void SetNearCells(int x, int y, int cell, std::vector<int>* cells)
{
	PlaceCell(x + 1, y, cell, cells);
	PlaceCell(x - 1, y, cell, cells);
	PlaceCell(x, y + 1, cell, cells);
	PlaceCell(x, y - 1, cell, cells);
}

void SpreadCell(int x, int y, int cell, std::vector<int>* cells)
{
	if (Random(seed) % 4 == 1)
	{
		SetNearCells(x, y, cell - 1, cells);
	}
}

void SpawnCells(int amount, int cell, std::vector<int>* cells)
{
	for (int i = 0; i < amount; i++)
	{
		int x = Random(seed) % (bufferWidth - 1);
		int y = Random(seed) % (bufferHeigth - 1);

		PlaceCell(x, y, cell, cells);
	}
}

//GENERATOR EFFECT FUNCTIONS

void ApplyCells(int inital, int repeat, int r, int g, int b, Color base)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	bool rUsed = (r != 0);
	bool gUsed = (g != 0);
	bool bUsed = (b != 0);

	std::vector<int> cells(ScreenPixels.size());
	std::vector<int> newCells(ScreenPixels.size());

	SpawnCells(inital, 255, &cells);
	newCells = cells;

	for (int i = 0; i < repeat; i++)
	{
		for (int y = 0; y < bufferHeigth; y++)
		{
			for (int x = 0; x < bufferWidth; x++)
			{
				int cell = cells[x + y * bufferWidth];

				if (cell != 0)
				{
					SpreadCell(x, y, cell, &newCells);
				}
			}
		}

		cells = newCells;
	}

	for (int i = 0; i < cells.size(); i++)
	{
		int cell = cells[i];
		if (cell != 0) ScreenColors[i] = Color(base.r + (cell / (r + (1 - rUsed))) * rUsed * 3, base.g + (cell / (g + (1 - gUsed))) * gUsed * 3, base.b + (cell / (b + (1 - bUsed))) * bUsed * 3);
	}

	//Measure time taken
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

	std::ostringstream spawnString;
	spawnString << "* ApplyCells: " << totalTime.count() / 1000.0f << " ms" << "\n";
	OutputDebugString(spawnString.str().c_str());
}

void ApplyFilter(Color target, float multiplier, bool flip, std::vector<Color>* applyTo)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < bufferHeigth; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			float m = ((float)y / (float)bufferHeigth) * 0.4f + ((float)x / (float)bufferWidth) * 0.6f;
			m = (flip * 2 - 1) * (flip - m);
			m = m * m * m;

			Color color = (*applyTo)[x + y * bufferWidth];
			if (color.r != 0 || color.g != 0 || color.b != 0)
			{
				color.r += (int)((float)(target.r - color.r) * multiplier * m);
				color.g += (int)((float)(target.g - color.g) * multiplier * m);
				color.b += (int)((float)(target.b - color.b) * multiplier * m);
				(*applyTo)[x + y * bufferWidth] = color;
			}
		}
	}

	//Measure time taken
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

	std::ostringstream spawnString;
	spawnString << "* ApplyFilter: " << totalTime.count() / 1000.0f << " ms" << "\n";
	OutputDebugString(spawnString.str().c_str());
}

void ApplyFilterFull(Color target, float multiplier, std::vector<Color>* applyTo)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < bufferHeigth; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			Color color = (*applyTo)[x + y * bufferWidth];
			if (color.r != 0 || color.g != 0 || color.b != 0)
			{
				color.r += (int)((float)(target.r - color.r) * multiplier);
				color.g += (int)((float)(target.g - color.g) * multiplier);
				color.b += (int)((float)(target.b - color.b) * multiplier);
				(*applyTo)[x + y * bufferWidth] = color;
			}
		}
	}

	//Measure time taken
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

	std::ostringstream spawnString;
	spawnString << "* ApplyFilter: " << totalTime.count() / 1000.0f << " ms" << "\n";
	OutputDebugString(spawnString.str().c_str());
}


void ApplyFilterSharp(Color target, float multiplier, std::vector<Color>* applyTo)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < bufferHeigth; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			Color color = (*applyTo)[x + y * bufferWidth];
			if (color.r != 0 || color.g != 0 || color.b != 0)
			{
				float r = (int)((float)(target.r - color.r) * multiplier);
				float g = (int)((float)(target.g - color.g) * multiplier);
				float b = (int)((float)(target.b - color.b) * multiplier);
				float used = min(min(r, g), b);
				color.r += used;
				color.g += used;
				color.b += used;
				(*applyTo)[x + y * bufferWidth] = color;
			}
		}
	}

	//Measure time taken
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

	std::ostringstream spawnString;
	spawnString << "* ApplyFilterSharp: " << totalTime.count() / 1000.0f << " ms" << "\n";
	OutputDebugString(spawnString.str().c_str());
}

void ApplyColor(Color c, std::vector<Color>* applyTo)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < bufferHeigth; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			Color color = (*applyTo)[x + y * bufferWidth];
			color.r += c.r;
			color.g += c.g;
			color.b += c.b;
			(*applyTo)[x + y * bufferWidth] = color;
		}
	}

	//Measure time taken
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

	std::ostringstream spawnString;
	spawnString << "* ApplyColor: " << totalTime.count() / 1000.0f << " ms" << "\n";
	OutputDebugString(spawnString.str().c_str());
}

//WINDOWS FUNCTIONS

LRESULT CALLBACK windowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (uMsg)
	{
	case WM_CLOSE:
	{
		running = false;
		break;
	}
	case WM_DESTROY:
	{
		running = false;
		break;
	}

	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		bufferWidth = rect.right - rect.left;
		bufferHeigth = rect.bottom - rect.top;
		bufferWidthOffset = bufferWidth / 2;
		bufferHeigthOffset = bufferHeigth / 2;

		int bufferSize = bufferWidth * bufferHeigth * sizeof(unsigned int);

		bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
		bitmapInfo.bmiHeader.biWidth = bufferWidth;
		bitmapInfo.bmiHeader.biHeight = bufferHeigth;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;

		break;
	}

	default:
	{
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}

	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//Create
	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpszClassName = "GWclass";
	windowClass.lpfnWndProc = windowCallback;

	//Register
	RegisterClass(&windowClass);

	//Window
	//1920, 1080
	//835, 1080
	//WS_OVERLAPPEDWINDOW
	//HWND window = CreateWindow(windowClass.lpszClassName, "Generator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 835, 1080, 0, 0, hInstance, 0);
	HWND window = CreateWindow(windowClass.lpszClassName, "Generator", WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080, 0, 0, hInstance, 0);
	HDC hdc = GetDC(window);

	ScreenPixels.resize(bufferWidth * bufferHeigth);
	ScreenColors.resize(bufferWidth * bufferHeigth);

	int background = 0;

	//Apply effects
	switch (background)
	{
	case 0:
		//Marble
		ApplyCells(200, 1000, -9, -9, -9, Color(255, 255, 255));
		ApplyFilter(Color(0, 0, 0), 0.7f, false, &ScreenColors);
		ApplyFilter(Color(255, 125, 0), 0.3f, true, &ScreenColors);
		break;
	case 1:
		//Lightning
		ApplyCells(100, 1000, 0, 0, -3, Color(0, 0, 255));
		ApplyFilter(Color(255, 125, 0), 0.1f, false, &ScreenColors);
		ApplyFilter(Color(0, 50, 125), 0.3f, true, &ScreenColors);
		break;
	case 2:
		//Shatter
		ApplyCells(1200, 300, 2, 2, 2, Color(0, 0, 0));
		ApplyFilter(Color(0, 0, 0), 0.5f, false, &ScreenColors);
		ApplyFilter(Color(0, 200, 255), 0.02f, false, &ScreenColors);
		ApplyFilter(Color(125, 0, 0), 0.15f, true, &ScreenColors);
		break;
	case 3:
		//Storm
		ApplyCells(1000, 400, 0, 0, -3, Color(0, 0, 255));
		ApplyFilter(Color(255, 255, 0), 0.05f, false, &ScreenColors);
		ApplyFilter(Color(255, 255, 0), 0.05f, true, &ScreenColors);
		break;
	case 4:
		//Lava
		ApplyCells(300, 800, -5, -3, 0, Color(255, 255, 0));
		ApplyFilter(Color(255, 125, 0), 1.0f, false, &ScreenColors);
		ApplyFilter(Color(255, 0, 0), 1.0f, true, &ScreenColors);
		ApplyFilter(Color(255, 255, 255), 0.3f, false, &ScreenColors);
		ApplyFilter(Color(0, 0, 0), 0.6f, true, &ScreenColors);
		break;
	case 5:
		//Lava2
		ApplyCells(300, 800, -5, -3, 0, Color(255, 255, 0));
		ApplyFilter(Color(255, 125, 0), 1.0f, false, &ScreenColors);
		ApplyFilter(Color(255, 0, 0), 1.0f, true, &ScreenColors);
		ApplyFilter(Color(255, 255, 255), 0.7f, false, &ScreenColors);
		ApplyFilter(Color(0, 0, 0), 0.6f, true, &ScreenColors);
		//ApplyFilterFull(Color(0, 0, 0), 0.85f, &ScreenColors);
		ApplyFilterFull(Color(0, 0, 0), 0.80f, &ScreenColors);
		break;
	case 6:
		//Scales
		ApplyCells(200, 400, 0, 12, 0, Color(20, 0, 0));
		ApplyFilter(Color(0, 0, 0), 0.2f, false, &ScreenColors);
		ApplyFilter(Color(100, 255, 0), 0.1f, true, &ScreenColors);
		ApplyFilterFull(Color(0, 0, 0), 0.65f, &ScreenColors);
		break;
	case 7:
		//Lightning2
		ApplyCells(200, 1000, 0, 0, -3, Color(0, 0, 255));
		ApplyFilter(Color(255, 125, 0), 0.1f, false, &ScreenColors);
		ApplyFilter(Color(0, 50, 125), 0.3f, true, &ScreenColors);
		ApplyFilterFull(Color(0, 0, 0), 0.80f, &ScreenColors);
		break;

	default:
		ApplyCells(200, 1000, 3, 3, 3, Color(255, 255, 255));
		break;
	}

	//Apply to screen pixels
	for (int i = 0; i < ScreenPixels.size(); i++)
	{
		Color color = ScreenColors[i];
		ScreenPixels[i] = RGB(color.b, color.g, color.r);
	}

	while (running == true)
	{
		//Input
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		TotalTime = TotalTime.zero();

		auto startTime = std::chrono::high_resolution_clock::now();

		//Render with windows
		StretchDIBits(hdc, 0, 0, bufferWidth, bufferHeigth, 0, 0, bufferWidth, bufferHeigth, &ScreenPixels[0], &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

		//Time
		auto endTime = std::chrono::high_resolution_clock::now();
		TotalTime += endTime - startTime;

		std::chrono::microseconds TotalTime2 = std::chrono::duration_cast<std::chrono::microseconds>(TotalTime);

		std::ostringstream TimeString;
		TimeString << "FPS: " << (int)(1000.0f / (TotalTime2.count() / 1000.0f)) << "|" << TotalTime2.count() / 1000.0f << " ms" << "\n";
		OutputDebugString(TimeString.str().c_str());
	}
}