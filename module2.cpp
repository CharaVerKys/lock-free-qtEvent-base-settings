#include "module2.h"
#include <settings.h>
#include <iostream>
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


//now this code work
    //dead lock, cus getModule2Set have same mutex
    Settings::getGlobInstance()->getModule2Set().mutex->lock();
    module->value2 = 10;
    Settings::getGlobInstance()->getModule2Set().mutex->unlock();
//now this code work

std::cout << module->value2 << std::endl;
    //only way is
    std::lock_guard(*Settings::getGlobInstance()->getModule2Set().mutex);
    module->value2 = 100;


    transId = Settings::getGlobInstance()->getNewId();
    assert(QMetaObject::invokeMethod(Settings::getGlobInstance(),"changeSettings",Qt::QueuedConnection,Q_ARG(/*id_t*/uint, transId))); //id_t unreg
}

void senderModule::slot_changeValueInModule2Set(id_t id, bool success, const char* moduleName, const char* paramName)
{
    assert(lockOper);
    lockOper = false;
    assert(success);
    assert(transId == id);
    std::cout << "end" << std::endl;
    disconnect(Settings::getGlobInstance(),&Settings::settingsChangeResult,this,&senderModule::slot_changeValueInModule2Set);
}
