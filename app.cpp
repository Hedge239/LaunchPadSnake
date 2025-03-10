#include <iostream>
#include <vector>
#include <algorithm>

#include <windows.h>
#include <mmsystem.h>

// GLOBAL VARS //
HMIDIIN _hMidiInput;
HMIDIOUT _hMidiOutput;

std::vector<std::pair<int, int>> playerBody;
bool isPlayerDead = false;
int playerMovementDirection = 4; // Start going right
int foodX, foodY;
int playerScore;

#define DEVICEOUTPUT_ID 1         // Sets the deviceID to 1 instead of 0
#define DEVICEINPUT_ID 0

#define INPUT_KEY_UP 104    // 1
#define INPUT_KEY_DOWN 105  // 2
#define INPUT_KEY_LEFT 110  // 7
#define INPUT_KEY_RIGHT 111 // 8

#define EXTRALOG_INPUT 0        // Logs all input from the MIDI device to the terminal

// FORWARD CALLS //
void CALLBACK MidiCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
void changeButtonColor(int posX, int posY, int color);


void updateBoard()
{
    // Clear Board
    for(int i1 = 0; i1 < 8; ++i1)
    {
        for(int i2 = 0l; i2 < 8; ++i2)
        {
            changeButtonColor(i1, i2, 0);
        }
    }

    // Draw Player
    for(size_t i = 0; i < playerBody.size(); ++i)
    {
        if(i == 0)
        {
            // head
            changeButtonColor(playerBody[i].first, playerBody[i].second, 0x3F);
        }else 
        {
            // body
            changeButtonColor(playerBody[i].first, playerBody[i].second, 0x3C);
        }
    }

    // Draw Food
    changeButtonColor(foodX, foodY, 0x0F);
}

void spawnFood()
{
    playerScore = playerScore + 1;

    std::cout << "Updated playerScore [" << playerScore << "]" << std::endl;

    // Check to make sure food does not spawn on player
    do
    {
        foodX = rand() % 8;
        foodY = rand() % 8;
    }
    while(std::find(playerBody.begin(), playerBody.end(), std::make_pair(foodX, foodY)) != playerBody.end());

    std::cout << "Spawned food: X[" << foodX << "], Y[" << foodY << "]" << std::endl;
}

void movePlayer()
{
    std::pair<int, int> playerFront = playerBody.front();

    switch (playerMovementDirection) 
    {
        case 1:
            playerFront.second = (playerFront.second - 1 + 8) % 8;
            break;
        case 2:
            playerFront.second = (playerFront.second + 1) % 8; 
            break;
        case 3:
            playerFront.first = (playerFront.first - 1 + 8) % 8; 
            break;
        case 4: 
            playerFront.first = (playerFront.first + 1) % 8; 
            break;
    }

    // Check if player collieded with self
    for (const auto& segment : playerBody) 
    {
        if (segment == playerFront) 
        {
            isPlayerDead = true;
            return;
        }
    }

    // Mvoe playerFront to begining of vector
    playerBody.insert(playerBody.begin(), playerFront);

    // Check if food
    if(playerFront.first == foodX && playerFront.second == foodY)
    {
        spawnFood();
    }else 
    {
        playerBody.pop_back();
    }
}

void gameLoop()
{
    while(true)
    {
        // When player dies, change all squres to red and print to console
        if(isPlayerDead) 
        {
            for(int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    changeButtonColor(i, j, 0x0F);
                }
            }

            break;
        };

        movePlayer();
        updateBoard();

        Sleep(200);
    }
}

int main()
{
    // Allocate A console
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);

    std::cout << "Initalizing MIDI Interface" << std::endl;

    // Get Device
    std::cout << "Getting Devices" << std::endl;
    if(midiInGetNumDevs() == 0) 
    {
        std::cout << "[ERROR] : No MIDI input detected" << std::endl;
        std::cout << "Error Detected. Press Enter to exit." << std::endl;
        std::cin.get();

        return 1;
    }

    // Connect to a MIDI Device
    std::cout << "Connecting to device : Input ID [" << DEVICEINPUT_ID << "] : Output ID [" << DEVICEOUTPUT_ID << "]" << std::endl;
    midiInOpen(&_hMidiInput, DEVICEINPUT_ID, (DWORD_PTR)MidiCallback, 0, CALLBACK_FUNCTION);
    midiOutOpen(&_hMidiOutput, DEVICEOUTPUT_ID, 0, 0, CALLBACK_NULL);

    std::cout << "Now listening for MIDI input" << std::endl;
    midiInStart(_hMidiInput);

    // Create the inital state of the game
    std::cout << "Createing inital state" << std::endl;
    playerBody.push_back({4,4});
    spawnFood();

    std::cout << "Setting score to 0" << std::endl;
    playerScore = 0;

    std::cout << "Starting game loop" << std::endl;
    gameLoop();

    std::cout << "Game Over : Final Score [" << playerScore << "] : Press enter to exit" << std::endl;
    std::cin.get();

    std::cout << "Cleaning up" << std::endl;
    midiInStop(_hMidiInput);

    midiOutClose(_hMidiOutput);
    midiInClose(_hMidiInput);
}

// Midi Control
void changeButtonColor(int posX, int posY, int color)
{
    int targetNote = (posY * 16) + posX;
    DWORD message = 0x90 | (targetNote << 8) | (color << 16); 

    midiOutShortMsg(_hMidiOutput, message);
}

void CALLBACK MidiCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (wMsg == MIM_DATA) 
    {
        BYTE note = (dwParam1 >> 8) & 0xFF;
        BYTE velocity = (dwParam1 >>  16) & 0xFF;

        if(velocity > 0)
        {
            if(note == INPUT_KEY_UP) {playerMovementDirection = 1;}
            if(note == INPUT_KEY_DOWN) {playerMovementDirection = 2;}
            if(note == INPUT_KEY_LEFT) {playerMovementDirection = 3;}
            if(note == INPUT_KEY_RIGHT) {playerMovementDirection = 4;}
        }

        if(EXTRALOG_INPUT == 1) {std::cout << "MIDI Message Reviced : Button " <<  (int)note << "  : Status " << (int)velocity << std::endl;}
    }
}
