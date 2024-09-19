#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <mutex>
#include <QEvent>
#include <QThread>

// type check
template<typename T>
class has_settingsChangeResult
{
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().settingsChangeResult(
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


class IModuleSettings{

};

class Module1Set : public IModuleSettings{
public:
    bool value1 = false;
};

class Module2Set : public IModuleSettings{
public:
    int value2 = 42;
};


enum ModulesNames{
    module1,
    module2
};


class EventSettingsChanged : public QEvent{
public:
    static const QEvent::Type MyEventType;
    EventSettingsChanged() : QEvent(MyEventType){}
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
    
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    static Settings *const getGlobInstance();
    bool loadSettings();

    ////////////////////////////
    //other thread
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
    // должен зарегистрироваться, реализовать хендлер event для ивента EventSettingsChanged, отправить в конце слота сигнал void settingsChangeResult() и всё
////////////////////////////


    ModuleLockFreePair getModule1Set();
    ModuleLockFreePair getModule2Set();

signals:
    void settingsChangeResult(bool success, const char* moduleName = nullptr, const char* paramName = nullptr);
    // if succes not additional info, otherwise when error send data for message box

public slots:
    void changeSettings();
    // должен отправить сигнал, гарантировать что сигнал не отправится до завершения всех операций и ждать сигнал settingsChangeResult о завершении

private slots:
    void resultFromObject(bool success, const char* moduleName = nullptr, const char* paramName = nullptr);

private:
    ModuleLockFreePair createReturnSetPair(IModuleSettings* modSet);
    void saveAllSettings();
    void saveSettings(IModuleSettings *settings);

private:
    uint16_t setObjectsRemain = 0;
    bool success; const char *moduleName; const char *paramName;

    std::map<ModulesNames, IModuleSettings*> allModules;
    std::mutex* mutex = nullptr;
    std::vector<QObject*> eventSetChangedReceivers;

    Module1Set set1;
    Module2Set set2;
};




#endif // SETTINGS_H
