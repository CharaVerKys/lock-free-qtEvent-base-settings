#include "module1.h"
#include <QEvent>
#include <settings.h>
#include <QThread>
#include <iostream>

module1::module1(QObject *parent)
    : QObject{parent}
{

}

bool module1::event(QEvent *event)
{
    if(event->type() == EventSettingsChanged::MyEventType){
        Settings* settings = Settings::getGlobInstance(); // thread safe
        ModuleLockFreePair pair = settings->getModule2Set(); // thread safe
        std::mutex* mutex = pair.mutex; // get mutex
        std::lock_guard lock(*mutex); // lock when need
        Module2Set* module2 = static_cast<Module2Set*>(pair.setModule); // under mutex lock
        myValue = module2->value2; // under mutex lock
        EventSettingsChanged* custedEvent = static_cast<EventSettingsChanged*>(event);
        if(myValue < 1000){
            emit settingsChangeResult(custedEvent->getId(), true);
        }else{
            emit settingsChangeResult(custedEvent->getId(), false,"module1","value2");
        }
        std::cout << "seted value is " <<myValue<< std::endl;
        return true;
    }
    return QObject::event(event);
}
