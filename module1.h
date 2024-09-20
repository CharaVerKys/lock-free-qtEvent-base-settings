#ifndef MODULE1_H
#define MODULE1_H

#include <QObject>

class module1 : public QObject
{
    Q_OBJECT
public:
    explicit module1(QObject *parent = nullptr);

signals:
    void settingsChangeResult(id_t id, bool success, const char* moduleName = nullptr, const char* paramName = nullptr);
protected:
    bool event(QEvent* event);
public:
    int myValue = 1234;

};

#endif // MODULE1_H
