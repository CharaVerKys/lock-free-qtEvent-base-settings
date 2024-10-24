#include "settings.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

const QEvent::Type EventSettingsChanged::settingsChanged = static_cast<QEvent::Type>(QEvent::registerEventType());

Settings::Settings()
    : QObject{nullptr}
    , mutex(new std::mutex())
{
    checkThread(&mainThreadID);
}

Settings::~Settings()
{
    checkThread(&mainThreadID);
    assert(mutex);
    delete mutex;
    mutex = nullptr;
    saveAllSettings();
}

Settings* Settings::getGlobInstance()
{
    static Settings settings;
    return &settings;
}

bool Settings::loadSettings()
{
    checkThread(&mainThreadID);
    assert(assertnumOfModules == helperInitSet.size());

    auto opt = getJsonFromSingleFile();
    if(not opt.has_value()){
        throw std::runtime_error("not opt.has_value()");
    }
    const QJsonObject json = opt.value();
    //assert(util::settings::countFirstLevelJsonObjects(json) == assertnumOfModules);

    for(auto& module :helperInitSet){
        const char* objectName = module->getModuleName();
        assert(objectName);

        bool moduleSetRes;
        if (json.contains(objectName)) {
            QJsonValue jsonValue = json[objectName];
            QJsonDocument jsonDoc(jsonValue.toObject());
            QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);
            auto byteArray = jsonString.toUtf8();
            const char* jsonStr = byteArray.constData();
            moduleSetRes = module->setValuesOnJsonString(jsonStr); // принимает const char*
        }else{
            moduleSetRes = module->setValuesOnJsonString("promb_setOnDefault"); //immediately change on variant if logic extending
            //variant warn or try set on default, but default in code may be not valid
            //throw;
        }

        if(moduleSetRes){
            qInfo() << QString("UserMsg Success load %1").arg(objectName);
        }else{
            qWarning() << QString("UserMsg Fail load %1").arg(objectName);
        }

        allModules.emplace(module->getModuleEnum(), module);
    }

    bool loadedOnes = !loaded.exchange(true);
    assert(loadedOnes);

    return true;
}

ModuleLockFreePair Settings::getIModuleSettings()
{
    assert(loaded.load());
    return createReturnSetPair(allModules[SettingsModulesNames::NotSetted]);
}


ModuleLockFreePair Settings::createReturnSetPair(IModuleSettings *modSet)
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
            moduleName = paramName = nullptr;
        }
        emit settingsChangeResult(id, transactionResult.at(id).success, transactionResult.at(id).moduleName, transactionResult.at(id).paramName);
        transactionResult.erase(transactionObjectsRemain.at(id));
        transactionObjectsRemain.erase(transactionObjectsRemain.at(id));
    }
}

void Settings::saveAllSettings()
{
    assert(not mutex); // only on exit
    checkThread(&mainThreadID);
    QJsonObject allSettingsJson;
    for(auto & module : allModules){
        assert(module.first != SettingsModulesNames::NotSetted);
        QJsonObject moduleJson = module.second->getJson();
        allSettingsJson[module.second->getModuleName()] = moduleJson;
        assert(moduleJson != QJsonObject());
        //saveSettings(module.second); if can
    }
    QFile fileToSave("your/path/to.json");
    if (fileToSave.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(allSettingsJson); // Convert the QJsonObject to a QJsonDocument
        fileToSave.write(doc.toJson()); // Write the JSON data to the file
        fileToSave.close(); // Close the file
        qInfo() << "UserMsg Settings saved to file.";
    } else {
        qCritical()/*Warning?*/ << "UserMsg Failed to open file for writing.";
    }
}

void Settings::saveSettings(IModuleSettings* settings)
{
    (void)settings;
    assert("not allowed in current variant" && false);
    // settings->writeToFile();
}

std::optional<QJsonObject> Settings::getJsonFromSingleFile()
{
    checkThread(&mainThreadID);
    QString filePath = "your/path/to.json";
    QByteArray jsonData;

    try {

       QFile file(filePath);

       if (!file.exists()) {
           qCritical() << (QString("UserMsg File %1 notExist").arg(filePath));
           return std::nullopt;
       }

       if (!file.open(QIODevice::ReadOnly)) {
           qCritical() << (QString("UserMsg Fail to open file %1").arg(filePath));
           return std::nullopt;
       }
       jsonData = file.readAll();
       file.close();
    }
    catch(std::exception &e)
    {
       qCritical() << QString("UserMsg File %1 Exception: %2").arg(filePath).arg(e.what());
       return std::nullopt;
    }

    QJsonObject settingsObject;
    try{
       QJsonDocument doc = QJsonDocument::fromJson(jsonData);

       if(!doc.isEmpty() && !doc.isNull())
       {
           settingsObject = doc.object();
       }
       else
       {
           qCritical() << (QString("UserMsg Fail convert %1 to json").arg(filePath));
           return std::nullopt;
       }
    } catch (const std::exception & e) {
         qWarning() << (QString("UserMsg Exception: %1").arg(e.what()));
         return std::nullopt;
    }
    return settingsObject;
}
