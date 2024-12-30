#ifndef IModuleSettings_H
#define IModuleSettings_H

#include <defines.h>
#include <qfile.h>

class IModuleSettings
{
public:
    IModuleSettings(){}
    virtual ~IModuleSettings(){}
    const char* getModuleName()const {assert(moduleName); return moduleName;}
    SettingsModulesNames getModuleEnum()const {assert(moduleEnum != SettingsModulesNames::NotSetted); return moduleEnum;}

    virtual bool readFromFile(QFile& stream);
    void writeToFile(const std::string& path); // actually do nothing, only set target path
    virtual QJsonObject getJson() const = 0;
    virtual bool setValuesOnJsonString(const char* jsonStr) = 0;
    virtual bool flush() const;

protected:
    const char* moduleName = nullptr;
    SettingsModulesNames moduleEnum = SettingsModulesNames::NotSetted;
    std::string path;
};

#endif // IModuleSettings_H