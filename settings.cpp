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
// test meta code
/*
template<typename Map, typename Arg>
auto construct_from_value(const Map& map, const Arg& value) {
    using ValueType = typename std::decay_t<decltype(std::declval<Map>().begin()->second)>;
    return ValueType(value); // Вызываем конструктор второго типа с переданным значением
}

template<typename Map>
auto construct_from_default(const Map& map) {
    using ValueType = typename std::decay_t<decltype(std::declval<Map>().begin()->second)>;
    return ValueType(); // Конструктор по умолчанию
}*/



//settings thread
void Settings::changeSettings(uint id)
{
    assert(transactionResult.find(id) == transactionResult.end());
    transactionResult.emplace(id, changeResult() /* construct_from_default(transactionResult)*/);
    assert(transactionResult.at(id).success == true);

    assert(transactionObjectsRemain.find(id) == transactionObjectsRemain.end());
    transactionObjectsRemain.emplace(id, __null);
    transactionObjectsRemain.at(id) = eventSetChangedReceivers.size();
    for(auto object : eventSetChangedReceivers){
        QCoreApplication::postEvent(object, new EventSettingsChanged(id));
    }
}

//settings thread
void Settings::resultFromObject(id_t id, bool success, const char *moduleName, const char *paramName)
{
    if(!success){ // may override if multiple error, not critical issue
        transactionResult.at(id).moduleName = moduleName;
        transactionResult.at(id).paramName = paramName;
        transactionResult.at(id).success = success;
    }
    uint16_t& remain = transactionObjectsRemain.at(id);
    --remain;
    if(transactionObjectsRemain.at(id) == 0){
        //saveSettings();
        if(transactionResult.at(id).success){
            moduleName = paramName = nullptr;
        }
        emit settingsChangeResult(id, transactionResult.at(id).success, transactionResult.at(id).moduleName, transactionResult.at(id).paramName);
        transactionResult.erase(transactionObjectsRemain.at(id));
        transactionObjectsRemain.erase(transactionObjectsRemain.at(id));
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
