#ifndef LogUpdateModuleSettings_H
#define LogUpdateModuleSettings_H

#include "imodulesettings.h"

// include qjson in cpp file
#include <qjsonobject.h>

class RandomModuleSettings : public IModuleSettings
{
public:
    RandomModuleSettings();

    QJsonObject getJson() const override{return {};}
    bool setValuesOnJsonString(const char* jsonStr) override{(void)jsonStr;return true;}
    bool isValidPtr()override final;

    int savePeriod = 0;
    int timeCheckLogs = 0;
};

inline RandomModuleSettings::RandomModuleSettings()
{
    moduleEnum = SettingsModulesNames::RandomName;
    moduleName = MODULENAME_DEFINED; // yap thats better
}

// that ideally should be in cpp file
#define MODULENAME_DEFINED "RandomModuleSettings"
include bool RandomModuleSettings::isValidPtr(){
    VALIDATE_MODULE_PTR_TYPE
    return 0== std::strcmp(this->moduleName,MODULENAME_DEFINED);
}
#indef MODULENAME_DEFINED
//or you can def/undef in header


#endif // LogUpdateModuleSettings_H
