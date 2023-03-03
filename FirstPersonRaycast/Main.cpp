#include <iostream>
#include <vector>		// Vectors
#include <algorithm>	// Sort
#include <chrono>		// Time
#include <string>		// String manipulation


// Windows API
#include <Windows.h>

// Window
const int SCREEN_WIDTH{ 120 };
const int SCREEN_HEIGHT{ 40 };

// Map
const int MAP_WIDTH{ 16 };
const int MAP_HEIGHT{ 16 };


// Player
float playerX{ 8.0f };
float playerY{ 8.0f };
float playerA{ 0.0f };

// Field of View
float FOV{ static_cast<float>(3.14159f / 4.0f )};
float depth{ 16.0f };

int main()
{
	// Create screen buffer
	wchar_t* screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten{ 0 };


	std::wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#..#...........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.........######";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";
	
	// Variable to make loop run
	bool is_Running{false};

	auto tp1{ std::chrono::system_clock::now() };
	auto tp2{ std::chrono::system_clock::now() };

	// Game loop
	while (!is_Running)
	{
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float dt = elapsedTime.count();

		// Controls
		// Handle CCW Rotation
		if (GetAsyncKeyState(static_cast<unsigned short>('A')) & 0x8000)
		{
			playerA -= (1.0f) * dt;
		}
		
		if (GetAsyncKeyState(static_cast<unsigned short>('D')) & 0x8000)
		{
			playerA += (1.0f) * dt;
		}

		if (GetAsyncKeyState(static_cast<unsigned short>('W')) & 0x8000)
		{
			playerX += sinf(playerA) * 5.0f * dt;
			playerY += cosf(playerA) * 5.0f * dt;

			// Detect collision with Wall and  'W'
			if (map[static_cast<int>(playerY) * MAP_WIDTH + static_cast<int>(playerX)] == '#')
			{
				playerX -= sinf(playerA) * 5.0f * dt;
				playerY -= cosf(playerA) * 5.0f * dt;
			}
		}


		if (GetAsyncKeyState(static_cast<unsigned short>('S')) & 0x8000)
		{
			playerX -= sinf(playerA) * 5.0f * dt;
			playerY -= cosf(playerA) * 5.0f * dt;

			// Detect collision with Wall upon 'S'
			if (map[static_cast<int>(playerY) * MAP_WIDTH + static_cast<int>(playerX)] == '#')
			{
				playerX += sinf(playerA) * 5.0f * dt;
				playerY += cosf(playerA) * 5.0f * dt;
			}
		}

		// Strafing RIGHT
		if (GetAsyncKeyState(static_cast<unsigned short>('H')) & 0x8000)
		{
			float playerStriftX = cosf(playerA);
			float playerStriftY = sinf(playerA);
			playerX += tanf(playerStriftX) * 5.0f * dt;
			playerY += tanf(playerStriftY) * 5.0f * dt;

			// Detect collision with Wall upon 'G'
			if (map[static_cast<int>(playerY) * MAP_WIDTH + static_cast<int>(playerX)] == '#')
			{
				playerX -= tanf(playerStriftX) * 5.0f * dt;
				playerY -= tanf(playerStriftY) * 5.0f * dt;
			}

		}

		// Strafing LEFT
		if (GetAsyncKeyState(static_cast<unsigned short>('G')) & 0x8000)
		{
			float playerStriftX = cosf(playerA);
			float playerStriftY = sinf(playerA);
			playerX -= tanf(playerStriftX) * 5.0f * dt;
			playerY -= tanf(playerStriftY) * 5.0f * dt;

			// Detect collision with Wall upon 'G'
			if (map[static_cast<int>(playerY) * MAP_WIDTH + static_cast<int>(playerX)] == '#')
			{
				playerX += tanf(playerStriftX) * 5.0f * dt;
				playerY += tanf(playerStriftY) * 5.0f * dt;
			}

		}

		for (int x{ 0 }; x < SCREEN_WIDTH; x++)
		{
			// For each column, calculate the projected rya angle into world space
			float rayAngle{ static_cast<float>((playerA - FOV / 2.0f)) + (static_cast<float>(x) / static_cast<float>(SCREEN_WIDTH) * FOV) };

			float distanceToWall{ 0.0f };

			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			bool boundaryBool{ false };
			bool hitWall{ false };
			while (!hitWall && distanceToWall < depth)
			{
				// Increment distance to wall
				distanceToWall += 0.1f;

				// Calculate distance to wall
				int testX{ static_cast<int>(playerX + (eyeX * distanceToWall)) };
				int testY{ static_cast<int>(playerY + (eyeY * distanceToWall)) };

				// Test if ray is out of bounds
				if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT)
				{
					hitWall = true;
					distanceToWall = depth;
				}
				else
				{
					// Ray is inbounds so test to see if ray cell is a wall block
					if (map[testY * MAP_WIDTH + testX] == '#')
					{
						hitWall = true;
						
						// Is to the rendering ray, closer we are to a tile
						// boundary, which we'll shade to add detail to the walls
						std::vector<std::pair<float, float>> p; // Distance, dot
						
						// Test each corner of hit tile, storing distance from
						// The player, and the calculated dot product of the two rays
						for (int tx{ 0 }; tx < 2; tx++)
						{
							for (int ty{ 0 }; ty < 2; ty++)
							{
								// Angle of corner to eye
								float vy{ static_cast<float>(testY) + ty - playerY };
								float vx{ static_cast<float>(testX) + tx - playerX };
								float d{ sqrt(vx * vx + vy * vy) };
								float dot{ (eyeX * vx / d) + (eyeY * vy / d) };
								p.push_back(std::make_pair(d, dot));
							}
						}

						// Sort pairs from closest to farthest
						std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });

						float bound{ static_cast<float>(0.004f) };
						if (acos(p.at(0).second) < bound) boundaryBool = true;
						if (acos(p.at(1).second) < bound) boundaryBool = true;
						//if (acos(p.at(2).second) < bound) boundaryBool = true;
					}
				}
			}

			// Calculate distance to ceiling and floor
			const float CEILING{ static_cast<float>((SCREEN_HEIGHT / 2.0f)) - static_cast<float>(SCREEN_HEIGHT / (distanceToWall)) };
			const int FLOOR{ SCREEN_HEIGHT - static_cast<int>(CEILING) };

			short Shade{ ' ' };

			// If very close
			if (distanceToWall <= depth / 4.0f)
			{
				// Use this ASCI unicode
				Shade = 0x2588;
			}
			else if (distanceToWall < depth / 3.0f)
			{
				Shade = 0x2593;
			}
			else if (distanceToWall < depth / 2.0f)
			{
				Shade = 0x2592;
			}
			else if (distanceToWall < depth)
			{
				Shade = 0x2591;
			}
			// If too far away
			else
			{
				Shade = ' ';
			}

			if (boundaryBool)
			{
				// Black it out
				Shade = ' ';
			}

			for (int y{ 0 }; y < SCREEN_HEIGHT; y++)
			{
				if (y < CEILING)
				{
					screen[y * SCREEN_WIDTH + x] = ' ';
				}
				else if(y > CEILING && y <= FLOOR)
				{
					screen[y * SCREEN_WIDTH + x] = Shade;
				}
				else
				{
					// Shade floor based on distance
					float b{ 1.0f - ((static_cast<float>(y) - SCREEN_HEIGHT / 2.0f) / (static_cast<float>(SCREEN_HEIGHT) / 2.0f)) };
					if (b < 0.25)
					{
						Shade = '-';
					}
					else if (b < 0.5)
					{
						Shade = '-';
					}
					else if (b < 0.75)
					{
						Shade = 'x';
					}
					else if (b < 0.9)
					{
						Shade = 'x';
					}
					screen[y * SCREEN_WIDTH + x] = Shade;
				}
			}

		}

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", playerX, playerY, playerA, 1.0f / dt);

		// Display Map
		for (int nx{ 0 }; nx < MAP_WIDTH; nx++)
		{
			for (int ny{ 0 }; ny < MAP_WIDTH; ny++)
			{
				screen[(ny + 1) * SCREEN_WIDTH + nx] = map[ny * MAP_WIDTH + nx];
			}
		}

		// Marker to show the player
		screen[static_cast<int>(playerY + 1) * SCREEN_WIDTH + static_cast<int>(playerX)] = 'P';

		// Display Frame
		screen[SCREEN_WIDTH * SCREEN_HEIGHT - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &dwBytesWritten);
	}



	return 0;
}