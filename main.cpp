#include <QCoreApplication>
#include "module1.h"
#include <iostream>
#include "module2.h"
#include "settings.h"
#include <QThread>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    {
        Settings::getGlobInstance()->loadSettings();
        auto pair = Settings::getGlobInstance()->getModule1Set();

        std::lock_guard lock(*pair.mutex);
        static_cast<Module1Set*>(pair.setModule)->value1 = false;
        Module1Set* module = static_cast<Module1Set*>(pair.setModule);
        module->value1 = true;
        // settings changed
    }

    {
        bool value;
        std::mutex* mutex = Settings::getGlobInstance()->getModule1Set().mutex;
        std::lock_guard lock(*mutex);
        //value = static_cast<Module1Set*>(Settings::getGlobInstance()->getModule1Set().setModule)->value1;// dead lock here, cannot use after ->lock()
        //assert(value); //default value false
    }

    {
        Settings* settings = Settings::getGlobInstance(); // thread safe
        ModuleLockFreePair pair = settings->getModule2Set(); // thread safe
        std::mutex* mutex = pair.mutex; // get mutex
        Module2Set* module2 = static_cast<Module2Set*>(pair.setModule); // get module
        std::lock_guard lock(*mutex); // lock when need
        int value = module2->value2; // under mutex lock
        assert(value == 42); // default value
    }

    class module1 clas; // overlap names.......

    {
        Settings::getGlobInstance()->registerObjectAsSettingsChangedEventHandler<class module1>(&clas);
        assert(clas.myValue != 42); // value wasnt setted
    }

    senderModule sender;
    sender.changeValueInModule2Set(); // start events when exec();

    std::cout << "start"<< std::endl;
    return a.exec();
}
