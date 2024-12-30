#include "imodulesettings.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <fstream>

bool IModuleSettings::readFromFile(QFile& stream){
    if(not stream.isOpen()){
        setValuesOnJsonString("prompt_setOnDefault");
        return false;
    }
    try{
        std::string str = stream.readAll().toStdString();
        setValuesOnJsonString(str.c_str()); // ? string view
        return false;
    } catch (const std::exception & e) {
        qWarning() << (QString("UserMsg Исключение в IModuleSettings::readFromFile: %1").arg(e.what()));
        setValuesOnJsonString("prompt_setOnDefault");
        return false;
    }
}

void IModuleSettings::writeToFile(const std::string &path){
    this->path = path;
}

bool IModuleSettings::flush() const{
    std::ofstream stream(path, std::ios::out);
    if(not stream){
        qCritical()/*Warning?*/ << "UserMsg Failed to open file for writing: " << std::string(this->getModuleName()).c_str() << " error: "<< strerror(errno);
        return false;
    }
    
    auto js = getJson();
    QJsonDocument doc(js);
    errno = 0;
#ifndef NDEBUG
    bool result = [&stream]()->bool{stream.seekp(0, std::ios::end);size_t end = stream.tellp();stream.seekp(0, std::ios::beg);/*useless func call in valid context*/return end == 0;}();
    assert(result);
#endif
    if (stream << doc.toJson().toStdString(); errno) {
        qCritical() << "UserMsg Failed to write data to file. Error: " << strerror(errno);
#ifndef NDEBUG
        // cppcheck-suppress shadowVariable
        bool result = [&stream, size = js.size()]()->bool{stream.seekp(0, std::ios::end);size_t end = stream.tellp();stream.seekp(0, std::ios::beg);/*useless func call in valid context*/return end == (size_t)size;}();
        assert(result);
#endif
        return true;
    }else{
        qInfo() << "UserMsg Settings saved to file. " << std::strerror(errno);
        return false;
    }
}