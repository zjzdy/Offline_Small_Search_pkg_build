#include "opkg_build.h"
#include "ui_opkg_build.h"

opkg_build::opkg_build(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::opkg_build)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    QObject::connect(&build_thread_obj,SIGNAL(input_status(QString)),this,SLOT(on_input_statu(QString)));
    QObject::connect(&build_thread_obj,SIGNAL(change_statu_word(QString)),this,SLOT(on_change_statu_word(QString)));
    QObject::connect(&build_thread_obj,SIGNAL(change_progress(int)),this,SLOT(on_change_progress(int)));
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
    QString a = QFileDialog::getExistingDirectory(this,tr("选择待打包的目录"));
    if(!(a.isEmpty()||a.isNull()))
    {
    ui->Input_line->setText(a);
    }
}

void opkg_build::on_Output_dir_button_clicked()
{
    QString a = QFileDialog::getExistingDirectory(this,tr("选择保存离线包的目录"));
    if(!(a.isEmpty()||a.isNull()))
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
        ui->welcome->setText("");
    }
    if (ui->publisher->text().isEmpty()|ui->publisher->text().isNull())
    {
        ui->publisher->setText("");
    }
    ui->progressBar->setValue(0);
    Q_EMIT make_start(QString(ui->Input_line->text().remove(QRegExp("[/\\\\]$"))+"/"),QString(ui->Output_line->text().remove(QRegExp("[/\\\\]$"))+"/"+ui->code_name->text()),ui->type->currentText().remove(QRegExp("\\(.*\\)")),ui->publisher->text(),ui->name->text(),ui->welcome->text());
    disable_ui();

}

void opkg_build::on_input_statu(QString msg)
{
    ui->status_text->append(msg);
}

void opkg_build::on_change_statu_word(QString msg)
{
    ui->label_4->setText(msg);
    if(msg == tr("正在读取并索引文件 ")) ui->progressBar->show();
    else ui->progressBar->hide();
}

void opkg_build::on_change_progress(int progress)
{
    ui->progressBar->setValue(progress);
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
    if(ui->checkBox->isChecked())
    {
        QFile home(ui->Output_line->text().remove(QRegExp("[/\\\\]$"))+"/"+ui->code_name->text()+"/home");
        home.open(QFile::WriteOnly);
        QDataStream home_s(&home);
        home_s << ui->welcome->text() << ui->checkBox_2->isChecked();
        home.close();
    }
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

void opkg_build::on_checkBox_clicked(bool checked)
{
    ui->checkBox_2->setEnabled(checked);
    ui->welcome->setEnabled(checked);
}

void opkg_build::on_read_from_zeal_clicked()
{
    QString a = QFileDialog::getOpenFileName(this,tr("选择待打包的目录"),QString(),"meta.json");
    if(a.isEmpty()||a.isNull())
    {
        return;
    }
    QString f = a;
    f = f.remove(QRegExp("[^/\\\\]*$"))+"Contents/Resources/Documents";
    ui->Input_line->setText(f);
    QFile b(a);
    b.open(QFile::ReadOnly);
    QString c(b.readAll());
    QRegExp d("indexFilePath\": \"([^\"]*)\"");
    if(d.indexIn(c) != -1)
        ui->welcome->setText(d.cap(1));
    else
    {
        QFile e(f+"/../../Info.plist");
        if(e.open(QFile::ReadOnly))
        {
            QString g(e.readAll());
            QRegExp h("dashIndexFilePath<[^<]*<string>([^<]*)");
            if(h.indexIn(g) != -1)
                ui->welcome->setText(h.cap(1));
            else
            {
                QFile e(f+"/index.html");
                if(e.open(QFile::ReadOnly))
                {
                    QString g(e.readAll());
                    QRegExp h("meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=([^\"]*)\"");
                    if(h.indexIn(g) != -1)
                        ui->welcome->setText(h.cap(1));
                    else ui->welcome->setText("index.html");
                }
                else
                {
                    QFile e(f+"/html/index.html");
                    if(e.open(QFile::ReadOnly))
                    {
                        QString g(e.readAll());
                        QRegExp h("meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=([^\"]*)\"");
                        if(h.indexIn(g) != -1)
                            ui->welcome->setText(h.cap(1));
                        else ui->welcome->setText("/html/index.html");
                    }
                    else
                    {
                        QFile e(f+"/doc/index.html");
                        if(e.open(QFile::ReadOnly))
                        {
                            QString g(e.readAll());
                            QRegExp h("meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=([^\"]*)\"");
                            if(h.indexIn(g) != -1)
                                ui->welcome->setText(h.cap(1));
                            else ui->welcome->setText("/doc/index.html");
                        }
                        else
                        {
                            ui->welcome->setText("");
                            ui->checkBox->setChecked(false);
                        }
                    }
                }
            }
        }
        else
        {
            QFile e(f+"/index.html");
            if(e.open(QFile::ReadOnly))
            {
                QString g(e.readAll());
                QRegExp h("meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=([^\"]*)\"");
                if(h.indexIn(g) != -1)
                    ui->welcome->setText(h.cap(1));
                else ui->welcome->setText("index.html");
            }
            else
            {
                QFile e(f+"/html/index.html");
                if(e.open(QFile::ReadOnly))
                {
                    QString g(e.readAll());
                    QRegExp h("meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=([^\"]*)\"");
                    if(h.indexIn(g) != -1)
                        ui->welcome->setText(h.cap(1));
                    else ui->welcome->setText("/html/index.html");
                }
                else
                {
                    QFile e(f+"/doc/index.html");
                    if(e.open(QFile::ReadOnly))
                    {
                        QString g(e.readAll());
                        QRegExp h("meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=([^\"]*)\"");
                        if(h.indexIn(g) != -1)
                            ui->welcome->setText(h.cap(1));
                        else ui->welcome->setText("/doc/index.html");
                    }
                    else
                    {
                        ui->welcome->setText("");
                        ui->checkBox->setChecked(false);
                    }
                }
            }
        }
    }
    QString name,title,version,code_name,pkg_name;
    d.setPattern("name\": \"([^\"]*)\"");
    if(d.indexIn(c) != -1)
        name = d.cap(1);
    d.setPattern("title\": \"([^\"]*)\"");
    if(d.indexIn(c) != -1)
        title = d.cap(1);
    code_name = "DevDoc_"+name+"_";
    d.setPattern("version\": \"([^\"]*)\"");
    if(d.indexIn(c) != -1)
    {
        version = d.cap(1);
        if(title.right(1) == version.left(1))
            title.remove(QRegExp("\\s*"+title.right(1)+"$"));
        pkg_name = title+" "+version+" Document";
        code_name = code_name+version+"_";
    }
    else pkg_name = title+" Document";
    d.setPattern("revision\": \"([^\"]*)\"");
    if(d.indexIn(c) != -1)
        code_name = code_name+d.cap(1)+"_";
    code_name = code_name+"0";
    ui->name->setText(pkg_name);
    ui->code_name->setText(code_name);
    ui->type->setCurrentText(tr("DevDoc(开发文档,开发者文档)"));
    ui->checkBox_2->setChecked(ui->checkBox->isChecked());
}
