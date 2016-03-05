#ifndef BUILD_H
#define BUILD_H

#include <QObject>
#include <QFile>
#include <QFileInfoList>
#include <QDir>
#include <QByteArray>
#include <QDirIterator>
#include <QDataStream>
#include <iostream>
#include <QDebug>

#include <iomanip>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <zlib.h>
#include <xapian.h>

#include <zim/writer/zimcreator.h>
#include <zim/blob.h>
#include <magic.h>
#include "gumbo/gumbo.h"
#include "parse/myhtmlparse.h"

#define MAX_QUEUE_SIZE 100

#ifdef _WIN32
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

using namespace std;


class Article : public QObject,public zim::writer::Article
{
    Q_OBJECT

public:
    Article(){}
    explicit Article(const string &path);

    virtual std::string getAid() const;
    virtual char getNamespace() const;
    virtual std::string getUrl() const;
    virtual bool isInvalid() const;
    virtual std::string getTitle() const;
    virtual bool isRedirect() const;
    virtual std::string getMimeType() const;
    virtual std::string getRedirectAid() const;
    virtual bool shouldCompress() const;
    char ns;
    std::string aid;
    std::string url;
    std::string title;
    std::string mimeType;
    std::string redirectAid;
    bool invalid;
    std::string data;
Q_SIGNALS:
    void done();
    void change_progress(int progress);
    void change_statu_word(QString msg);
    void input_status(QString msg);
};

class ArticleSource : public QObject,public zim::writer::ArticleSource
{
    Q_OBJECT

public:
    explicit ArticleSource(QString input2, QString output2, QString mode);
    virtual const zim::writer::Article* getNextArticle();
    virtual zim::Blob getData(const std::string& aid);
    virtual std::string getMainPage();
    void add_to_index(const std::string &text,const std::string &aid);
    MyHtmlParser htmlparse;
    QStringList filters;
    QDirIterator *dir_iterator;
    QString idir_str;
    QString input,output;
    int a;
    QString current_dir;
    vector<string> words;
    QFileInfo file_info;
    QString absolute_file_path;
    QString filePath;
    QFile srcfile;
    QString c;
    QByteArray s;
    Article *article;
    std::string str1;
    unsigned long long int count;
    unsigned long long int data_count;
    int progress;
    bool get_data_stat;
    bool get_next_stat;
Q_SIGNALS:
    void done();
    void change_progress(int progress);
    void change_statu_word(QString msg);
    void input_status(QString msg);
};

class build : public QObject
{
    Q_OBJECT
public:
    explicit build(QObject *parent = 0);

Q_SIGNALS:
    void done();
    void change_progress(int progress);
    void change_statu_word(QString msg);
    void input_status(QString msg);

public Q_SLOTS:
    void on_build_start(QString input, QString output, QString mode, QString publisher, QString title, QString welcome);

private:
    QDir dir;
    zim::writer::ZimCreator *zimCreator;
    ArticleSource *articleS;
    int zim_minChunkSize;
};

#endif // BUILD_H
