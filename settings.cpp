#include "settings.h"
#include <QCoreApplication>
#include <iostream>

const QEvent::Type EventSettingsChanged::MyEventType = static_cast<QEvent::Type>(QEvent::registerEventType());

Settings::Settings(QObject *parent)
    : QObject{parent}
    , mutex(new std::mutex())
{}

Settings::~Settings()
{
    assert(mutex);
    delete mutex;
    mutex = nullptr;
    saveAllSettings();
}

//other thread
Settings *const Settings::getGlobInstance()
{
    static Settings settings; // удивительно как просто, вот тот случай когда нужно использовать статик в функции
    return &settings;
}

//other thread
bool Settings::loadSettings()
{
    std::lock_guard lock(*mutex);

    // dummy code
    allModules.emplace(module1,&set1);
    allModules.emplace(module2,&set2);
    // dummy code

    /*
    for(each file in dir){
         read module name -> make module
    }
    */
    return true;
}


//other thread
ModuleLockFreePair Settings::createReturnSetPair(IModuleSettings *modSet)
{
    ModuleLockFreePair pair;
    pair.mutex = mutex;
    pair.setModule = modSet;
    return pair;
}

//other thread
ModuleLockFreePair Settings::getModule1Set()
{
    std::lock_guard lock(*mutex);
    return createReturnSetPair(allModules[module1]);
}

//other thread
ModuleLockFreePair Settings::getModule2Set()
{
    std::lock_guard lock(*mutex);
    return createReturnSetPair(allModules[module2]);
}

//settings thread
void Settings::changeSettings()
{
    this->success = true;
    assert(setObjectsRemain == 0);
    setObjectsRemain = eventSetChangedReceivers.size();
    for(auto object : eventSetChangedReceivers){
        QCoreApplication::postEvent(object, new EventSettingsChanged());
    }
}

//settings thread
void Settings::resultFromObject(bool success, const char *moduleName, const char *paramName)
{
    if(!success){ // may override if multiple error, not critical issue
        this->moduleName = moduleName;
        this->paramName = paramName;
        this->success = success;
    }
    --setObjectsRemain;
    if(setObjectsRemain == 0){
        //saveSettings();
        if(this->success){
            moduleName = paramName = nullptr;
        }
        emit settingsChangeResult(this->success, this->moduleName, this->paramName);
    }
}
//settings thread
void Settings::saveAllSettings()
{
    // end of life only
    for(auto & module : allModules){
        saveSettings(module.second);
    }
}
//settings thread
void Settings::saveSettings(IModuleSettings* settings)
{
    // load to file
    // settings->saveParams
    std::cout << "settings saved" << std::endl;
}
