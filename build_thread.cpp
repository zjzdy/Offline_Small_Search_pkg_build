#include "build_thread.h"

build_thread::build_thread()
{

}

void build_thread::run()
{
    build build_obj;
    QObject::connect(&build_obj,SIGNAL(input_status(QString)),this,SIGNAL(input_status(QString)));
    QObject::connect(&build_obj,SIGNAL(change_statu_word(QString)),this,SIGNAL(change_statu_word(QString)));
    QObject::connect(&build_obj,SIGNAL(change_progress(int)),this,SIGNAL(change_progress(int)));
    QObject::connect(&build_obj,SIGNAL(done()),this,SIGNAL(done()));
    QObject::connect(this,SIGNAL(build_start(QString,QString,QString,QString,QString,QString)),&build_obj,SLOT(on_build_start(QString,QString,QString,QString,QString,QString)));
    exec();
}
