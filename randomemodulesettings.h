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

    int savePeriod = 0;
    int timeCheckLogs = 0;
};

inline RandomModuleSettings::RandomModuleSettings()
{
    moduleEnum = SettingsModulesNames::RandomName;
    moduleName = "RandomModuleSettings";
}


#endif // LogUpdateModuleSettings_H