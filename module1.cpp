#include "module1.h"
#include "settings.h"

bool module1::event(QEvent *event)
{
    if(event->type() != EventSettingsChanged::settingsChanged){
        return QObject::event(event); // cuz single event handler
    }
    bool okSettings = true;
    bool wasChanges = true;

    auto pair = Settings::getGlobInstance()->getIModuleSettings();
    IModuleSettings* set = dynamic_cast<IModuleSettings*>(pair.setModule);
    {///
        std::lock_guard lock(pair.mutex);
        if(this->value not_eq set->value){
            wasChanges = true;
        }


        if(set->value not_eq condition){
            okSettings = false;
        }

        if(not okSettings){
            emit settingsChangeResult(static_cast<EventSettingsChanged*>(event)->getId(),false,"field 1","field 2");
            return true;}


        if(not wasChanges){
            emit settingsChangeResult(static_cast<EventSettingsChanged*>(event)->getId(),true);
            return true;}

        this->value = set->value;
    }/// mutex unlock
    emit settingsChangeResult(static_cast<EventSettingsChanged*>(event)->getId(),true);
    return true;
}
