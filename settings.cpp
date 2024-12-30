#include "settings.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QStandardPaths>
#include <qdir.h>

#include <randomemodulesettings.h>

const QEvent::Type EventSettingsChanged::settingsChanged = static_cast<QEvent::Type>(QEvent::registerEventType());

constexpr uint64_t operator""_id(unsigned long long value) {
    return value - 1;
}

void Settings::initHolderSetModules()noexcept{
    holderSetModules[1_id] = std::make_unique<RandomModuleSettings>(RandomModuleSettings());
}

Settings::Settings()
    : QObject{nullptr}
    , mutex(new std::mutex())
{
    checkThread(&mainThreadID);
    initHolderSetModules();
}

Settings::~Settings()
{
    checkThread(&mainThreadID);
    assert(mutex);
    delete mutex;
    mutex = nullptr;
    for(auto&[key,module] : allModules){module->flush();}
}

Settings* Settings::getGlobInstance()
{
    static Settings settings;
    return &settings;
}

bool Settings::loadSettings()
{
    checkThread(&mainThreadID);
    for(auto& uptr :holderSetModules){
        IModuleSettings* module = uptr.get();
        std::string path = getSettingsVariant(false,std::string(module->getModuleName()));
        QFile file(path.c_str());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            switch (file.error()) {
                case QFileDevice::OpenError:
                    qCritical() << (QString("UserMsg Файла %1 не существует").arg(file.fileName()));
                    break;
                case QFileDevice::PermissionsError:
                    qCritical() << (QString("UserMsg Не могу открыть файл %1 для чтения настроек: нет доступа")
                                    .arg(file.fileName()));
                    break;
                default:
                    qCritical() << (QString("UserMsg Не могу открыть файл %1 для чтения настроек: неизвестная ошибка")
                                    .arg(file.fileName()));
            }
        }
        module->readFromFile(file);
        file.close();
        assert(file.atEnd());

        allModules.emplace(module->getModuleEnum(), module);
    }
    setPathsForSettings();

    [[maybe_unused]] bool loadedOnes = !loaded.exchange(true);
    assert(loadedOnes);

    return true;
}

ModuleLockFreePair Settings::getRandomModuleSettings()
{
    assert(loaded.load());
    return createReturnSetPair(allModules[SettingsModulesNames::RandomName]);
}

ModuleLockFreePair Settings::createReturnSetPair(IModuleSettings *modSet) noexcept
{
    ModuleLockFreePair pair;
    pair.mutex = mutex;
    pair.setModule = modSet;
    return pair;
}

void Settings::changeSettings(uint id)
{
    checkThread(&mainThreadID);
    assert(transactionResult.find(id) == transactionResult.end());
    transactionResult.emplace(id, changeResult() /* construct_from_default(transactionResult)*/);
    assert(transactionResult.at(id).success == true);

    assert(transactionObjectsRemain.find(id) == transactionObjectsRemain.end());
    transactionObjectsRemain.emplace(id, __null);
    transactionObjectsRemain.at(id) = eventSetChangedReceivers.size();
    for(auto object : eventSetChangedReceivers){
        QCoreApplication::postEvent(object, new EventSettingsChanged(id));
    }
    if(eventSetChangedReceivers.empty()){
        emit settingsChangeResult(id, true);
    }
}

void Settings::resultFromObject(id_t id, bool success, const char *moduleName, const char *paramName)
{
    checkThread(&mainThreadID);
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
            // cppcheck-suppress [uselessAssignmentPtrArg,unreadVariable]
            moduleName = paramName = nullptr;
        }
        emit settingsChangeResult(id, transactionResult.at(id).success, transactionResult.at(id).moduleName, transactionResult.at(id).paramName);
        transactionResult.erase(transactionObjectsRemain.at(id));
        transactionObjectsRemain.erase(transactionObjectsRemain.at(id));
    }
}

void Settings::setPathsForSettings() noexcept
{
    checkThread(&mainThreadID);
    for(auto& module : allModules){
        assert(module.first not_eq SettingsModulesNames::NotSetted);
        std::string path = getSettingsVariant(true,std::string(module.second->getModuleName()));
        module.second->writeToFile(path);
    }
}

std::string Settings::getSettingsVariant(bool forSave, const std::string& moduleName) noexcept
{
    checkThread(&mainThreadID);
    std::string path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString();
    std::string currentPath = path +"/"+ "settings" +"/"+ (moduleName + ".json");

    if(not forSave){
        std::string defpath = std::string(":/settings") +"/"+ (moduleName + ".json");
        return QFileInfo::exists(currentPath.c_str()) ? currentPath : defpath;
    }
    QDir cd((path + "/settings").c_str());
    if(!cd.exists())
    {
        if(cd.mkpath(".")){
            qInfo () << (QString("Создана директория для файла настроек %1").arg(currentPath.c_str()));
        }else{
            qCritical() << (QString("UserMsg Не удалось создать директорию для файла настроек %1").arg(currentPath.c_str()));
        }
    }
    return currentPath;
}
