#ifndef DEFINES_H
#define DEFINES_H

enum class SettingsModulesNames{
    NotSetted = 0,
    RandomName
};

//your checkThread, if want
#define checkThread(val) (void*)val
extern void* mainThreadID;

#endif // DEFINES_H
