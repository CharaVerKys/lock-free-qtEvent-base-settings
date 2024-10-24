#ifndef IMODULESETTINGS_H
#define IMODULESETTINGS_H

#include "defines.h"
#include <assert.h>
#include <fstream>
class QJsonObject;

class IModuleSettings
{
public:
    IModuleSettings(){}
    virtual ~IModuleSettings(){}
    const char* getModuleName()const {assert(moduleName); return moduleName;}
    SettingsModulesNames getModuleEnum()const {assert(moduleEnum != SettingsModulesNames::NotSetted); return moduleEnum;}

    virtual bool readFromFile(std::ifstream& stream) = 0;
    virtual bool writeToFile(std::ofstream& stream) const = 0;
    virtual QJsonObject getJson() const = 0;
    virtual bool setValuesOnJsonString(const char* jsonStr) = 0;

protected:
    const char* moduleName = nullptr;
    SettingsModulesNames moduleEnum = SettingsModulesNames::NotSetted;
};

#endif // IMODULESETTINGS_H
