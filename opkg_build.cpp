#include "opkg_build.h"
#include "ui_opkg_build.h"

opkg_build::opkg_build(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::opkg_build)
{
    ui->setupUi(this);
    QObject::connect(&build_thread_obj,SIGNAL(input_status(QString)),this,SLOT(on_input_statu(QString)));
    QObject::connect(&build_thread_obj,SIGNAL(done()),this,SLOT(on_done()));
    QObject::connect(this,SIGNAL(make_start(QString,QString,QString,QString,QString,QString)),&build_thread_obj,SIGNAL(build_start(QString,QString,QString,QString,QString,QString)));
    build_thread_obj.start();
}

opkg_build::~opkg_build()
{
    build_thread_obj.quit();
    delete ui;
}

void opkg_build::on_Input_dir_button_clicked()
{
    QString a = QFileDialog::getExistingDirectory(this,tr("选择待打包的目录"),".");
    if(!(a.isEmpty()|a.isNull()))
    {
    ui->Input_line->setText(a);
    }
}

void opkg_build::on_Output_dir_button_clicked()
{
    QString a = QFileDialog::getExistingDirectory(this,tr("选择保存离线包的目录"),".");
    if(!(a.isEmpty()|a.isNull()))
    {
    ui->Output_line->setText(a);
    }
}

void opkg_build::on_about_button_clicked()
{
    QMessageBox::information(this,tr("About&Help"),tr("This program uses Qt base GPL version 3.0.\n"
                                                      "Use xapian build index.\nhttp://xapian.org/\n"
                                                      "Use zim save data.\nhttp://openzim.org/\n"
                                                      "This program's author is by ZJZDY.\n"
                                                      "http://git.oschina.net/zjzdy/Offline_Small_Search_pkg_build\n"
                                                      "懒得打中文了!\n"),QMessageBox::Ok);
}

void opkg_build::on_Start_button_clicked()
{
    if (ui->Input_line->text().isEmpty()|ui->Input_line->text().isNull())
    {
        ui->status_text->append(tr("请输入 选择待打包的目录."));
        return;
    }
    if (ui->Output_line->text().isEmpty()|ui->Output_line->text().isNull())
    {
        ui->status_text->append(tr("请输入 保存离线包的目录."));
        return;
    }
    QDir Idir(ui->Input_line->text());
    if(!Idir.exists())
    {
        ui->status_text->append(tr("待打包的目录不存在."));
        return;
    }
    QDir Odir(ui->Output_line->text());
    if(!Odir.exists())
    {
        Odir.mkpath(ui->Output_line->text());
    }
    if (ui->name->text().isEmpty()|ui->name->text().isNull())
    {
        ui->status_text->append(tr("请输入 离线包名称."));
        return;
    }
    if (ui->code_name->text().isEmpty()|ui->code_name->text().isNull())
    {
        ui->status_text->append(tr("请输入 离线包标识码."));
        return;
    }
    if (ui->welcome->text().isEmpty()|ui->welcome->text().isNull())
    {
        ui->status_text->append(tr("请输入 离线包主页."));
        return;
    }
    if (ui->publisher->text().isEmpty()|ui->publisher->text().isNull())
    {
        ui->status_text->append(tr("请输入 离线包发布者."));
        return;
    }
    Q_EMIT make_start(QString(ui->Input_line->text().remove(QRegExp("[/\\\\]$"))+"/"),QString(ui->Output_line->text().remove(QRegExp("[/\\\\]$"))+"/"+ui->code_name->text()),ui->type->currentText().remove(QRegExp("\\(.*\\)")),ui->publisher->text(),ui->name->text(),ui->welcome->text());
    disable_ui();
}

void opkg_build::on_input_statu(QString msg)
{
    ui->status_text->append(msg);
}

void opkg_build::on_done()
{
    ui->Start_button->setEnabled(true);
    ui->Input_dir_button->setEnabled(true);
    ui->Output_dir_button->setEnabled(true);
    ui->type->setEnabled(true);
    ui->code_name->setReadOnly(false);
    ui->name->setReadOnly(false);
    ui->publisher->setReadOnly(false);
    ui->welcome->setReadOnly(false);
    ui->Input_line->setReadOnly(false);
    ui->Output_line->setReadOnly(false);
}

void opkg_build::disable_ui()
{
    ui->Start_button->setEnabled(false);
    ui->Input_dir_button->setEnabled(false);
    ui->Output_dir_button->setEnabled(false);
    ui->type->setEnabled(false);
    ui->code_name->setReadOnly(true);
    ui->name->setReadOnly(true);
    ui->publisher->setReadOnly(true);
    ui->welcome->setReadOnly(true);
    ui->Input_line->setReadOnly(true);
    ui->Output_line->setReadOnly(true);
}
