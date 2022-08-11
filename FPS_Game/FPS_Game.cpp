
#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>

int nScreenWidth = 120;
int nScreenHeight = 40;

//store where player is
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
//store the angle where player is looking at
float fPlayerA = 0.0f;

//map data

int nMapHeight = 16;
int nMapWidth = 16;
//narrow field of view
float fFOV = 3.14159 / 4.0;

float fDepth = 16.0f;

int main()
{   
    //grab console buffer and write directly to it
    //create screen buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwByteWritten = 0;
        
    std::wstring map;

    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";


    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now();

    //Game Loop
    while (true) {

        //normalize speed of our movement by checking measuring the time elapsed on each interation of a loop
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();



        //Controls
        //Handle CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
            fPlayerA -= (1.0f) * fElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
            fPlayerA += (1.0f) * fElapsedTime;
        }

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            //collison detaction, if the player hits the wall he is just reversed back to his previous location
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            if (map[static_cast<int>(fPlayerY) * nMapWidth + static_cast<int>(fPlayerX)] == '#') {
                   fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                   fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
                };

        }



        //computation for each column of the screen
        for (int x = 0; x < nScreenWidth; x++) {

            //for each column calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + (static_cast<float>(x) / static_cast<float>(nScreenWidth * fFOV));

            //variable to track what is the distance from the player to the wall for the given angle 
            float fDistanceToWall = 0.0f;
       
            bool bHitWall = false;
            bool bBoundry = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            //loop to for testing if the created "ball" hit a wall, if yes "generate" a wall block
            while (!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;

                int nTestX = static_cast<int>(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = static_cast<int>(fPlayerY + fEyeY * fDistanceToWall);


                //test if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                
                    bHitWall = true; //Just set distance to maximum depth
                    fDistanceToWall = fDepth;
                }
                else {
                    //Ray is inbounds so test to see if the ray cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#') {

                        bHitWall = true;
                        std::vector<std::pair<float, float>> p;

                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = static_cast<float>(nTestY) + ty - fPlayerY;
                                float vx = static_cast<float>(nTestX) + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.vector::push_back(std::make_pair(d, dot));
                            }
                            //Sort Pairs from closest to fahrest
                            sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });

                            //ray pretty much has to hit a corner, very narrow
                            float fBound = 0.01;
                            if (acos(p.at(0).second) < fBound) { bBoundry = true; };
                            if (acos(p.at(1).second) < fBound)
                            {
                                bBoundry = true;
                            };
                           
                           
                        }
                    }

                }

            }

            //Calculate distance to ceiling  
            int nCeiling = static_cast<float>(nScreenHeight / 2.0) - nScreenHeight / (static_cast<float> (fDistanceToWall));

            //calculate the distance to floor
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';
            short bShade = ' ';

            

            if (fDistanceToWall <= fDepth / 4.0f) {
                nShade = 0x2588; //Very close to a player
            }
            else if(fDistanceToWall < fDepth / 3.0f) {
                nShade = 0x2593;
            }
            else if (fDistanceToWall < fDepth / 2.0f) {
                nShade = 0x2592;
            }
            else if (fDistanceToWall < fDepth) {
                nShade = 0x2591;
            }else{
                nShade = ' '; //Too far away from the player
            }
            if (bBoundry) {
                nShade = ' ';
            }


            for (int y = 0; y < nScreenHeight; y++) {
                if (y < nCeiling) {
                    screen[y * nScreenWidth + x] = ' ';
                }
                else if(y > nCeiling && y <= nFloor) {
                    screen[y * nScreenWidth + x] = nShade;
                }
                else {
                    //shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25) {
                        bShade = '#';
                    }
                    else if (b < 0.5) {
                        bShade = 'X';
                    }
                    else if (b < 0.75) {
                        bShade = '.';
                    }
                    else if (b < 0.9) {
                        bShade = '-';
                    }
                    else {
                        bShade = ' ';
                    }
                    screen[y * nScreenWidth + x] = bShade;
                }
            }

        }

        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwByteWritten);
    }



    std::cout << "Hello World!\n";
    return 0;
}


