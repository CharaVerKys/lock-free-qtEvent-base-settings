#include <QCoreApplication>
#include "defines.h"
#include "settings.h"

void* mainThreadId = nullptr;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Settings::getGlobInstance()->loadSettings();
    Settings::getGlobInstance()->registerObjectAsSettingsChangedEventHandler<YourQObject>(YourQObject* object);
    //it work.
    // maybe i will complete demo but..
    // see module1 and module2 to more modules info

    return a.exec();
}
