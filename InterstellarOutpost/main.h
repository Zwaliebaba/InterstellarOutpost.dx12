#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

extern double g_gameTime; // Updated from GetHighResTime every frame
extern double g_advanceTime; // How long the last frame took
extern int g_lastProcessedSequenceId;
extern int g_sliceNum; // Most recently advanced slice

void AppMain(const char* _cmdLine = nullptr);
void UpdateAdvanceTime();

bool IsFirsttimeSequencing();
void RemoveAllWindows();

#endif
