#include "module2.h"
#include <settings.h>

senderModule::senderModule(QObject *parent)
    : QObject{parent}
{

}

void senderModule::changeValueInModule2Set()
{
    if(lockOper){return;}
    lockOper = true;
    connect(Settings::getGlobInstance(),&Settings::settingsChangeResult,this,&senderModule::slot_changeValueInModule2Set, Qt::QueuedConnection);
    Module2Set* module = static_cast<Module2Set*>(Settings::getGlobInstance()->getModule2Set().setModule);

    // dead lock, cus getModule2Set have same mutex
    //Settings::getGlobInstance()->getModule2Set().mutex->lock();
    //module->value2 = 100;
    //Settings::getGlobInstance()->getModule2Set().mutex->unlock();

    //only way is
    std::lock_guard(*Settings::getGlobInstance()->getModule2Set().mutex);
    module->value2 = 100;


    QMetaObject::invokeMethod(Settings::getGlobInstance(),"changeSettings",Qt::QueuedConnection);
}

void senderModule::slot_changeValueInModule2Set(bool success, const char* moduleName, const char* paramName)
{
    assert(lockOper);
    lockOper = false;
    assert(success);
    disconnect(Settings::getGlobInstance(),&Settings::settingsChangeResult,this,&senderModule::slot_changeValueInModule2Set);
}
