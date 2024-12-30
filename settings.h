#ifndef SETTINGS_H
#define SETTINGS_H

#include <mutex>
#include <QEvent>
#include <QObject>
#include "defines.h"
#include "imodulesettings.h"


/*
## guideline:
1. init settings, noting special
2. settings-setter (aka setter) want to change settings
3. he remember old setting params and push them to module and emit signal change settings
4. setter blocking next settings-send event (other-way should support multiple settings events) and wait for answer
5. Settings:: posting events to settings-receivers (aka receivers)
6. receivers validate input settings and if pass returns true, or if validate fails return false + module name and reason
7. setter receive success and (if he want) 'flush()' module to write settings to file
7.5 if received fail then revers settings and emit changeSettings again
*/




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
    // cppcheck-suppress noExplicitConstructor
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
        (void)object;
        static_assert(has_settingsChangeResult<QObject_typename>::value, "Object does not have the required signal: settingsChangeResult");
    }
////////////////////////////

    ModuleLockFreePair getRandomModuleSettings();

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
    ModuleLockFreePair createReturnSetPair(IModuleSettings* modSet) noexcept;
    void setPathsForSettings() noexcept;
    std::string getSettingsVariant(bool forSave, const std::string& moduleName) noexcept;
    void initHolderSetModules() noexcept;

private:
    std::map<id_t, uint16_t> transactionObjectsRemain;
    std::map<id_t, changeResult> transactionResult;
    std::map<SettingsModulesNames, IModuleSettings*> allModules;
    std::mutex* mutex = nullptr;
    std::vector<QObject*> eventSetChangedReceivers;
    std::atomic<bool> loaded = false;

    #define numOfModules 9
    std::array<std::unique_ptr<IModuleSettings>, numOfModules> holderSetModules;
}; // end of class Settings

struct changeResult{
    bool success = true; const char *moduleName = nullptr; const char *paramName = nullptr;
};

#endif // SETTINGS_H
