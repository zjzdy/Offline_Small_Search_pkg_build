#ifndef BUILD_THREAD_H
#define BUILD_THREAD_H

#include <QObject>
#include <QThread>
#include "build.h"

class build_thread : public QThread
{
    Q_OBJECT

public:
    build_thread();
    void run();

Q_SIGNALS:
    void done();
    void change_statu_word(QString msg);
    void change_progress(int progress);
    void input_status(QString msg);
    void build_start(QString input, QString output, QString mode, QString publisher, QString title, QString welcome);
};

#endif // BUILD_THREAD_H
