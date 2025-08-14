#ifndef IModuleSettings_H
#define IModuleSettings_H

#include <defines.h>
#include <qfile.h>

#define VALIDATE_MODULE_PTR_TYPE \
    assert(this); \
    assert(0== std::strcmp(this->moduleName,MODULENAME_DEFINED));

class IModuleSettings
{
public:
    IModuleSettings();
    virtual ~IModuleSettings(){}
    const char* getModuleName()const {assert(moduleName); return moduleName;}
    eSettingsModulesNames getModuleEnum()const {assert(moduleEnum != eSettingsModulesNames::eNotSetted); return moduleEnum;}

    virtual bool isValidPtr() = 0;
    virtual bool readFromFile(QFile& stream);
    void writeToFile(std::string path); // actually do nothing, only set target path
    virtual QJsonObject getJson() const = 0;
    virtual bool setValuesOnJsonString(const char* jsonStr) = 0;
    virtual bool flush() const;

protected:
    const char* moduleName = nullptr;
    eSettingsModulesNames moduleEnum = eSettingsModulesNames::eNotSetted;
    std::string path;
};

#endif // IModuleSettings_H
