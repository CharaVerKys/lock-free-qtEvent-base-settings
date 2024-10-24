#ifndef SETTINGS_H
#define SETTINGS_H

#include <mutex>
#include <QEvent>
#include <QObject>
#include "defines.h"
#include "imodulesettings.h"

// type check
template<typename T>
class has_settingsChangeResult
{
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().settingsChangeResult(
                                          std::declval<id_t>(),
                                          std::declval<bool>(),
                                            std::declval<const char*>(),
                                            std::declval<const char*>()
                                          ), std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};
// type check

struct changeResult;

class EventSettingsChanged : public QEvent{
public:
    static const QEvent::Type settingsChanged;
    EventSettingsChanged(id_t id) : QEvent(settingsChanged), id(id){}
    id_t getId(){return id;}
private:
    id_t id;
};

struct ModuleLockFreePair{
    IModuleSettings* setModule; // point of your module
    std::mutex* mutex; // global settings access mutex
};

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings& operator=(Settings&&) = delete;
    
    ~Settings();

    static Settings* getGlobInstance();
    bool loadSettings();

    ////////////////////////////
    template<typename QObject_typename>
    std::enable_if_t<has_settingsChangeResult<QObject_typename>::value, void>
    registerObjectAsSettingsChangedEventHandler(QObject_typename *object)
    {
        std::lock_guard lock(*mutex);
        assert(object);
        eventSetChangedReceivers.push_back(object);
        connect(object, &QObject_typename::settingsChangeResult, this, &Settings::resultFromObject, Qt::QueuedConnection);
    }

    template<typename QObject_typename>
    std::enable_if_t<!has_settingsChangeResult<QObject_typename>::value, void>
    registerObjectAsSettingsChangedEventHandler(QObject_typename *object)
    {
        (void*)object;
        static_assert(has_settingsChangeResult<QObject_typename>::value, "Object does not have the required signal: settingsChangeResult");
    }
////////////////////////////

    ModuleLockFreePair getIModuleSettings();

    static id_t getNewId(){
        static std::atomic<uint> idGen = 0;
        return idGen++;
    }

signals:
    void settingsChangeResult(id_t id, bool success, const char* moduleName = nullptr, const char* paramName = nullptr);

public slots:
    void changeSettings(uint id);

private slots:
    void resultFromObject(id_t id, bool success, const char* moduleName = nullptr, const char* paramName = nullptr);

private:
    Settings();
    ModuleLockFreePair createReturnSetPair(IModuleSettings* modSet);
    void saveAllSettings();
    void saveSettings(IModuleSettings *settings);
    QString getSettingsVariant(bool forSave);
    std::optional<QJsonObject> getJsonFromSingleFile();

private:
    std::map<id_t, uint16_t> transactionObjectsRemain;
    std::map<id_t, changeResult> transactionResult;
    std::map<SettingsModulesNames, IModuleSettings*> allModules;
    std::mutex* mutex = nullptr;
    std::vector<QObject*> eventSetChangedReceivers;
    std::atomic<bool> loaded = false;

    //modules
    IModuleSettings module;

    const uint8_t assertnumOfModules = 1;
    std::vector<IModuleSettings*> helperInitSet{&module};

}; // end of class Settings

struct changeResult{
    bool success = true; const char *moduleName = nullptr; const char *paramName = nullptr;
};

#endif // SETTINGS_H
