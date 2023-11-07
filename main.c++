#include <iostream>
#include <cstdlib>
#include <cmath>
#include <raylib.h>
#include <portaudio.h>
#include <deque>

#define MAX_HISTORY 140
#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512



using namespace std;

float calculateRMS(const float* buffer, int bufferSize)
{
    float sum = 0.0f;
    for (int i = 0; i < bufferSize; i++) {
        sum += buffer[i] * buffer[i];
    }
    float rms = sqrt(sum / bufferSize);
    return rms;
}

int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    float* input = (float*)inputBuffer;
    float rms = calculateRMS(input, framesPerBuffer);
    float gain = 2.0f; // Adjust this value to increase or decrease the sensitivity
    rms *= gain; // Apply the gain
    float db;
    if (rms == 0)
    {
        db = 0;
    }
    else
    {
        db = 20 * log10(rms + 0.000001f); // Add a small constant to avoid log(0)
    }
    *(float*)userData = db; 
    return paContinue;
}

int main()
{ 
    const int screen_width = 1024;
    const int screen_height = 768;
    InitWindow(screen_width, screen_height, "Decibel Meter");
    SetTargetFPS(60);
    PaStream *stream;
    PaError err;
    float db;

    err = Pa_Initialize();
    if(err != paNoError) return -1;

    err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, audioCallback, &db);
    if(err != paNoError) return -1;

    err = Pa_StartStream(stream);
    if(err != paNoError) return -1;

   
    deque<float> dbHistory;
    float maxDb = -numeric_limits<float>::infinity();

    double start = GetTime(); 
    const int HEIGHT = 800;
    const int GRAPH_WIDTH = 600;
    const int GRAPH_HEIGHT = 600;
    
    while(WindowShouldClose() == false)
    {
        double now = GetTime();
        double elapsed = now - start;
        DrawText("Decibel Meter", 100, 100, 20,GREEN);

        dbHistory.push_back(db);
        printf("Current db: %f\n", db); 

        maxDb = max(maxDb, db);

        if(dbHistory.size() > MAX_HISTORY)
        {
            dbHistory.pop_front(); 
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < dbHistory.size() - 1; i++)
        {
            int x1 = i * GRAPH_WIDTH / MAX_HISTORY;
            int y1 = HEIGHT - dbHistory[i] * GRAPH_HEIGHT / maxDb;
            int x2 = (i + 1) * GRAPH_WIDTH / MAX_HISTORY;
            int y2 = HEIGHT - dbHistory[i + 1] * GRAPH_HEIGHT / maxDb;
            DrawLine(x1, y1, x2, y2, GREEN);
        }

        DrawText(("Current db: " + to_string(db)).c_str(), 100, 125, 20, GREEN);

        DrawText(("Maximum db: " + to_string(maxDb)).c_str(), 100, 150, 20, RED);
        EndDrawing();

    
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}



   



