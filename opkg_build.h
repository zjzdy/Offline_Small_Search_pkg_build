#ifndef OPKG_BUILD_H
#define OPKG_BUILD_H

#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QDataStream>
#include <QByteArray>
#include <QMessageBox>
#include <iostream>
#include "build_thread.h"
#include <QDebug>

namespace Ui {
class opkg_build;
}

class opkg_build : public QWidget
{
    Q_OBJECT

public:
    explicit opkg_build(QWidget *parent = 0);
    ~opkg_build();

Q_SIGNALS:
    void make_start(QString input, QString output, QString mode, QString publisher, QString title, QString welcome);

public Q_SLOTS:
    void on_input_statu(QString msg);
    void on_done();
    void disable_ui();

private Q_SLOTS:
    void on_Input_dir_button_clicked();

    void on_Output_dir_button_clicked();

    void on_about_button_clicked();

    void on_Start_button_clicked();

private:
    Ui::opkg_build *ui;
    build_thread build_thread_obj;
};

#endif // OPKG_BUILD_H
