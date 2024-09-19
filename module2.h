#ifndef MODULE2_H
#define MODULE2_H

#include <QObject>

class senderModule : public QObject
{
    Q_OBJECT
public:
    explicit senderModule(QObject *parent = nullptr);

    void changeValueInModule2Set();

public slots:
    void slot_changeValueInModule2Set(bool success, const char* moduleName = nullptr, const char* paramName = nullptr);

private:
    bool lockOper = false;
};

#endif // MODULE2_H
