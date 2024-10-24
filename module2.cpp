#include "module2.h"
#include "settings.h"

void senderModule::changeValueInModule2Set()
{
    if(lockOper){return;}
    lockOper = true;
    connect(Settings::getGlobInstance(),&Settings::settingsChangeResult,this,&senderModule::slot_changeValueInModule2Set, Qt::QueuedConnection);

    transId = Settings::getGlobInstance()->getNewId();
    QMetaObject::invokeMethod(
  /*fail to cast by default*/(QObject*)Settings::getGlobInstance(),
                            "changeSettings", Qt::QueuedConnection,
                            Q_ARG(/*id_t*/uint, transId)); //id_t unreg metaType
}

void senderModule::slot_changeValueInModule2Set(id_t id, bool success, const char* moduleName, const char* paramName)
{
    if(transId not_eq id){
        return;
    }
    assert(lockOper);
    lockOper = false;
    assert(success);
    disconnect(Settings::getGlobInstance(),&Settings::settingsChangeResult,this,&senderModule::slot_changeValueInModule2Set);
}
