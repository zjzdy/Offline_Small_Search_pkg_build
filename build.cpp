#include "build.h"

magic_t magic;
std::map<std::string, unsigned int> counters;
std::map<std::string, std::string> fileMimeTypes;
std::map<std::string, std::string> extMimeTypes;
char *data = NULL;
unsigned int dataSize = 0;
std::queue<std::string> metadataQueue;
std::string directoryPath = "";
std::string language = "zh_CN";
std::string creator = "oss_pkg_build";
std::string publisher = "";
std::string title = "";
std::string description = "";
std::string welcome = "";
std::string code_name;
unsigned long long count_index = 0;
//std::string favicon = "";
Xapian::WritableDatabase db;
Xapian::TermGenerator termgen;
class MetadataArticle : public Article {
  public:
  MetadataArticle(std::string &id) {
      aid = "/M/" + id;
      mimeType="text/plain";
      ns = 'M';
      url = id;
  }
};

#include <str.h>
static string format_doc_termlist(const Xapian::Document & doc)
{
    string output;
    Xapian::TermIterator it;
    for (it = doc.termlist_begin(); it != doc.termlist_end(); ++it) {
    if (!output.empty()) output += ' ';
    output += *it;
    if (it.positionlist_count() != 0) {
        // If we've got a position list, only display the wdf if it's not
        // the length of the positionlist.
        if (it.get_wdf() != it.positionlist_count()) {
            output += ':';
        output += str(it.get_wdf());
        }
        char ch = '[';
        Xapian::PositionIterator posit;
        for (posit = it.positionlist_begin(); posit != it.positionlist_end(); posit++) {
        output += ch;
        ch = ',';
        output += str(*posit);
        }
        output += ']';
    } else if (it.get_wdf() != 0) {
        // If no position list, display any non-zero wdfs.
        output += ':';
        output += str(it.get_wdf());
    }
    }
    return output;
}

/* Decompress an STL string using zlib and return the original data. */
inline std::string inflateString(const std::string& str) {
  z_stream zs; // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (inflateInit(&zs) != Z_OK)
    throw(std::runtime_error("inflateInit failed while decompressing."));

  zs.next_in = (Bytef*)str.data();
  zs.avail_in = str.size();

  int ret;
  char outbuffer[32768];
  std::string outstring;

  // get the decompressed bytes blockwise using repeated calls to inflate
  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = inflate(&zs, 0);

    if (outstring.size() < zs.total_out) {
      outstring.append(outbuffer,
               zs.total_out - outstring.size());
    }

  } while (ret == Z_OK);

  inflateEnd(&zs);

  if (ret != Z_STREAM_END) { // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib decompression: (" << ret << ") "
        << zs.msg;
        throw(std::runtime_error(oss.str()));
  }

  return outstring;
}

inline bool seemsToBeHtml(const std::string &path) {
  if (path.find_last_of(".") != std::string::npos) {
    std::string mimeType = path.substr(path.find_last_of(".")+1);
    if (extMimeTypes.find(mimeType) != extMimeTypes.end()) {
      return "text/html" == extMimeTypes[mimeType];
    }
  }

  return false;
}

inline std::string getFileContent(const std::string &path) {
  std::ifstream in(path.c_str(), ::std::ios::binary);
  if (in) {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    return(contents);
 }
  std::cerr << "读取文件: 无法打开文件: " << path << std::endl;
  throw(errno);
}

inline bool fileExists(const std::string &path) {
  bool flag = false;
  std::fstream fin;
  fin.open(path.c_str(), std::ios::in);
  if (fin.is_open()) {
    flag = true;
  }
  fin.close();
  return flag;
}

/* base64 */
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
    {
      for(j = i; j < 3; j++)
    char_array_3[j] = '\0';

      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (j = 0; (j < i + 1); j++)
    ret += base64_chars[char_array_4[j]];

      while((i++ < 3))
    ret += '=';

    }

  return ret;

}


inline std::string decodeUrl(const std::string &encodedUrl) {
  std::string decodedUrl = encodedUrl;
  std::string::size_type pos = 0;
  char ch;

  while ((pos = decodedUrl.find('%', pos)) != std::string::npos &&
     pos + 2 < decodedUrl.length()) {
    sscanf(decodedUrl.substr(pos + 1, 2).c_str(), "%x", (unsigned int*)&ch);
    decodedUrl.replace(pos, 3, 1, ch);
    ++pos;
  }

  return decodedUrl;
}

inline std::string removeLastPathElement(const std::string path, const bool removePreSeparator, const bool removePostSeparator) {
  std::string newPath = path;
  size_t offset = newPath.find_last_of(SEPARATOR);

  if (removePreSeparator && offset == newPath.length()-1) {
    newPath = newPath.substr(0, offset);
    offset = newPath.find_last_of(SEPARATOR);
  }
  newPath = removePostSeparator ? newPath.substr(0, offset) : newPath.substr(0, offset+1);

  return newPath;
}

/* Split string in a token array */
std::vector<std::string> split(const std::string & str,
                                      const std::string & delims=" *-")
{
  std::string::size_type lastPos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, lastPos);
  std::vector<std::string> tokens;

  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delims, pos);
      pos     = str.find_first_of(delims, lastPos);
    }

  return tokens;
}

std::vector<std::string> split(const char* lhs, const char* rhs){
  const std::string m1 (lhs), m2 (rhs);
  return split(m1, m2);
}

std::vector<std::string> split(const char* lhs, const std::string& rhs){
  return split(lhs, rhs.c_str());
}

std::vector<std::string> split(const std::string& lhs, const char* rhs){
  return split(lhs.c_str(), rhs);
}

/* Warning: the relative path must be with slashes */
inline std::string computeAbsolutePath(const std::string path, const std::string relativePath) {

  /* Add a trailing / to the path if necessary */
  std::string absolutePath = path[path.length()-1] == '/' ? path : removeLastPathElement(path, false, false);

  /* Go through relative path */
  std::vector<std::string> relativePathElements;
  std::stringstream relativePathStream(relativePath);
  std::string relativePathItem;
  while (std::getline(relativePathStream, relativePathItem, '/')) {
    if (relativePathItem == "..") {
      absolutePath = removeLastPathElement(absolutePath, true, false);
    } else if (!relativePathItem.empty() && relativePathItem != ".") {
      absolutePath += relativePathItem;
      absolutePath += "/";
    }
  }

  /* Remove wront trailing / */
  return absolutePath.substr(0, absolutePath.length()-1);
}

/* Warning: the relative path must be with slashes */
std::string computeRelativePath(const std::string path, const std::string absolutePath) {
  std::vector<std::string> pathParts = split(path, "/");
  std::vector<std::string> absolutePathParts = split(absolutePath, "/");

  unsigned int commonCount = 0;
  while (commonCount < pathParts.size() &&
     commonCount < absolutePathParts.size() &&
     pathParts[commonCount] == absolutePathParts[commonCount]) {
    if (!pathParts[commonCount].empty()) {
      commonCount++;
    }
  }

  std::string relativePath;
  for (unsigned int i = commonCount ; i < pathParts.size()-1 ; i++) {
    relativePath += "../";
  }

  for (unsigned int i = commonCount ; i < absolutePathParts.size() ; i++) {
    relativePath += absolutePathParts[i];
    relativePath += i + 1 < absolutePathParts.size() ? "/" : "";
  }

  return relativePath;
}

static bool isLocalUrl(const std::string url) {
  if (url.find(":") != std::string::npos) {
    return (!(
          url.find("://") != std::string::npos ||
          url.find("//") == 0 ||
          url.find("tel:") == 0 ||
          url.find("geo:") == 0
          ));
  }
  return true;
}

static std::string extractRedirectUrlFromHtml(const GumboVector* head_children) {
  std::string url;

  for (int i = 0; i < head_children->length; ++i) {
    GumboNode* child = (GumboNode*)(head_children->data[i]);
    if (child->type == GUMBO_NODE_ELEMENT &&
    child->v.element.tag == GUMBO_TAG_META) {
      GumboAttribute* attribute;
      if (attribute = gumbo_get_attribute(&child->v.element.attributes, "http-equiv")) {
    if (!strcmp(attribute->value, "refresh")) {
      if (attribute = gumbo_get_attribute(&child->v.element.attributes, "content")) {
        std::string targetUrl = attribute->value;
        std::size_t found = targetUrl.find("URL=") != std::string::npos ? targetUrl.find("URL=") : targetUrl.find("url=");
        if (found!=std::string::npos) {
          url = targetUrl.substr(found+4);
        } else {
          throw std::string("Unable to find the redirect/refresh target url from the HTML DOM");
        }
      }
    }
      }
    }
  }

  return url;
}

static void getLinks(GumboNode* node, std::map<std::string, bool> &links) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }

  GumboAttribute* attribute = NULL;
  attribute = gumbo_get_attribute(&node->v.element.attributes, "href");
  if (attribute == NULL) {
    attribute = gumbo_get_attribute(&node->v.element.attributes, "src");
  }

  if (attribute != NULL && isLocalUrl(attribute->value)) {
    links[attribute->value] = true;
  }

  GumboVector* children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i) {
    getLinks(static_cast<GumboNode*>(children->data[i]), links);
  }
}

static void replaceStringInPlaceOnce(std::string& subject, const std::string& search,
                 const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
    return; /* Do it once */
  }
}

static void replaceStringInPlace(std::string& subject, const std::string& search,
                 const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }

  return;
}

static std::string getMimeTypeForFile(const std::string& filename) {
  std::string mimeType;

  /* Try to get the mimeType from the file extension */
  if (filename.find_last_of(".") != std::string::npos) {
    mimeType = filename.substr(filename.find_last_of(".")+1);
    if (extMimeTypes.find(mimeType) != extMimeTypes.end()) {
      return extMimeTypes[mimeType];
    }
  }

  /* Try to get the mimeType from the cache */
  if (fileMimeTypes.find(filename) != fileMimeTypes.end()) {
    return fileMimeTypes[filename];
  }

  /* Try to get the mimeType with libmagic */
  try {
    std::string path = directoryPath + "/" + filename;
    mimeType = std::string(magic_file(magic, path.c_str()));
    //cout<<path<<mimeType;
    if (mimeType.find(";") != std::string::npos) {
      mimeType = mimeType.substr(0, mimeType.find(";"));
    }
    fileMimeTypes[filename] = mimeType;
    return mimeType;
  } catch (...) {
    return "";
  }
}

inline std::string removeLocalTag(const std::string &url) {
  std::size_t found = url.find("#");

  if (found != std::string::npos) {
    return url.substr(0, found-1);
  }
  return url;
}

inline std::string computeNewUrl(const std::string &aid, const std::string &url) {
  std::string filename = computeAbsolutePath(aid, url);
  //std::string targetMimeType = getMimeTypeForFile(removeLocalTag(decodeUrl(filename)));
  //std::string originMimeType = getMimeTypeForFile(aid);
  std::string newUrl = "/A/" + filename;
  std::string baseUrl = "/A/" + aid;
  return computeRelativePath(baseUrl, newUrl);
}
build::build(QObject *parent) : QObject(parent)
{
    zim_minChunkSize = 2048;
    zimCreator = new zim::writer::ZimCreator();
#ifdef HAVE__PUTENV_S
    _putenv_s("ZIM_LZMA_LEVEL", "9e");
    _putenv_s("XAPIAN_CJK_NGRAM", "1");
#elif defined HAVE_SETENV
    setenv("ZIM_LZMA_LEVEL", "9e", 1);
    setenv("XAPIAN_CJK_NGRAM", "1", 1);
#else
    putenv(const_cast<char*>("ZIM_LZMA_LEVEL=9e"));
    putenv(const_cast<char*>("XAPIAN_CJK_NGRAM=1"));
#endif
    /* Init file extensions hash */
    extMimeTypes["HTML"] = "text/html";
    extMimeTypes["html"] = "text/html";
    extMimeTypes["HTM"] = "text/html";
    extMimeTypes["htm"] = "text/html";
    extMimeTypes["PHP"] = "text/html";
    extMimeTypes["php"] = "text/html";
    extMimeTypes["ASP"] = "text/html";
    extMimeTypes["asp"] = "text/html";
    extMimeTypes["ASPX"] = "text/html";
    extMimeTypes["aspx"] = "text/html";
    extMimeTypes["JSP"] = "text/html";
    extMimeTypes["jsp"] = "text/html";
    extMimeTypes["PNG"] = "image/png";
    extMimeTypes["png"] = "image/png";
    extMimeTypes["TIFF"] = "image/tiff";
    extMimeTypes["tiff"] = "image/tiff";
    extMimeTypes["TIF"] = "image/tiff";
    extMimeTypes["tif"] = "image/tiff";
    extMimeTypes["JPEG"] = "image/jpeg";
    extMimeTypes["jpeg"] = "image/jpeg";
    extMimeTypes["JPG"] = "image/jpeg";
    extMimeTypes["jpg"] = "image/jpeg";
    extMimeTypes["GIF"] = "image/gif";
    extMimeTypes["gif"] = "image/gif";
    extMimeTypes["SVG"] = "image/svg+xml";
    extMimeTypes["svg"] = "image/svg+xml";
    extMimeTypes["TXT"] = "text/plain";
    extMimeTypes["txt"] = "text/plain";
    extMimeTypes["XML"] = "text/xml";
    extMimeTypes["xml"] = "text/xml";
    extMimeTypes["EPUB"] = "application/epub+zip";
    extMimeTypes["epub"] = "application/epub+zip";
    extMimeTypes["PDF"] = "application/pdf";
    extMimeTypes["pdf"] = "application/pdf";
    extMimeTypes["OGG"] = "application/ogg";
    extMimeTypes["ogg"] = "application/ogg";
    extMimeTypes["JS"] = "application/javascript";
    extMimeTypes["js"] = "application/javascript";
    extMimeTypes["JSON"] = "application/json";
    extMimeTypes["json"] = "application/json";
    extMimeTypes["CSS"] = "text/css";
    extMimeTypes["css"] = "text/css";
    extMimeTypes["otf"] = "application/vnd.ms-opentype";
    extMimeTypes["OTF"] = "application/vnd.ms-opentype";
    extMimeTypes["eot"] = "application/vnd.ms-fontobject";
    extMimeTypes["EOT"] = "application/vnd.ms-fontobject";
    extMimeTypes["ttf"] = "application/font-ttf";
    extMimeTypes["TTF"] = "application/font-ttf";
    extMimeTypes["woff"] = "application/font-woff";
    extMimeTypes["WOFF"] = "application/font-woff";
    extMimeTypes["vtt"] = "text/vtt";
    extMimeTypes["VTT"] = "text/vtt";
    magic = magic_open(MAGIC_MIME);
    magic_load(magic, NULL);
}


void build::on_build_start(QString input, QString output, QString mode, QString publisher, QString title, QString welcome)
{
    articleS = new ArticleSource(input, output, mode);
    //qDebug()<<input;
    metadataQueue = std::queue<std::string>();
    /* Prepare metadata */
    //metadataQueue.push("Language");
    metadataQueue.push("Publisher");
    metadataQueue.push("Creator");
    metadataQueue.push("Title");
    metadataQueue.push("Description");
    metadataQueue.push("Date");
    //metadataQueue.push("Favicon");
    metadataQueue.push("Counter");
    counters.clear();
    data = NULL;
    dataSize = 0;
    count_index = 0;
    description = mode.toStdString();//description == type == mode
    ::title = title.toStdString();
    ::welcome = welcome.toStdString();
    ::publisher = publisher.toStdString();
    QRegExp code_name_reg("^.*[/\\\\]");
    code_name_reg.setMinimal(false);
    QString _code_name = output;
    code_name = _code_name.remove(code_name_reg).toStdString();
    db = Xapian::Chert::open(output.toStdString(), Xapian::DB_CREATE);
    QFile type_file(QString(output.remove(QRegExp("[/\\\\]$"))+"/type"));
    type_file.open(QFile::WriteOnly);
    QDataStream type_stream(&type_file);
    type_stream << mode;
    type_file.close();
    QFile name_file(QString(output.remove(QRegExp("[/\\\\]$"))+"/name"));
    name_file.open(QFile::WriteOnly);
    QDataStream name_stream(&name_file);
    name_stream << title;
    name_file.close();
    QObject::connect(articleS,SIGNAL(input_status(QString)),this,SIGNAL(input_status(QString)));
    QObject::connect(articleS,SIGNAL(done()),this,SIGNAL(done()));

    try
    {
        zimCreator = new zim::writer::ZimCreator();
        zimCreator->setMinChunkSize(zim_minChunkSize);
        dir.mkdir(output);
        zimCreator->create(QString(output+"/"+"data").toStdString(), *articleS);
        delete zimCreator;
        Q_EMIT input_status(tr("Xapian 索引总数:")+QString::number(count_index));
        QFile count_file(output+"/count");
        count_file.open(QFile::WriteOnly);
        QDataStream count_stream(&count_file);
        count_stream << count_index;
        count_file.close();
        db.commit();
        db.close();
        Q_EMIT input_status(tr("离线包打包成功."));

    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        Q_EMIT input_status(QString(e.what()));
        Q_EMIT input_status(tr("离线包打包失败."));
    }
    Q_EMIT done();
    if (articleS != NULL)
    {
        if (articleS->dir_iterator != NULL)
        {
            delete articleS->dir_iterator;
        }
        delete articleS;
    }
}

ArticleSource::ArticleSource(QString input2, QString output2, QString mode)
{
    article = NULL;
    data = NULL;
    dataSize = 0;
    filters<<QString("*");
    //input = input2;
    output = output2;
    idir_str = input2;
    a = idir_str.size();
    current_dir = idir_str;
    count = 0;
    dir_iterator = new QDirIterator(input2,filters,QDir::Files|QDir::NoDotDot,QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
    directoryPath = input2.remove(QRegExp("[/\\\\]$")).toStdString();
}

std::string ArticleSource::getMainPage()
{
  return welcome;
}

const zim::writer::Article* ArticleSource::getNextArticle()
{
    std::string path;

    if (article != NULL) {
      delete(article);
    }

    if (!metadataQueue.empty()) {
      path = metadataQueue.front();
      metadataQueue.pop();
      article = new MetadataArticle(path);
    } else
    {
      article = NULL;
      while(dir_iterator->hasNext())
      {
          dir_iterator->next();
          file_info = dir_iterator->fileInfo();
          if(file_info.isFile())
          {
              absolute_file_path = file_info.absoluteFilePath();
              filePath = file_info.absolutePath();
              if (current_dir != filePath)
              {
                  current_dir = filePath;
                  Q_EMIT input_status(tr("正在统计文件: 进入目录:")+current_dir);
              }
              article = new Article(absolute_file_path.toStdString());
              break;
              //Q_EMIT input_status(absolute_file_path);
          }
      }
    }


    /* Count mimetypes */
    if (article != NULL && !article->isRedirect()) {
      std::string mimeType = article->getMimeType();
      if (counters.find(mimeType) == counters.end()) {
        counters[mimeType] = 1;
      } else {
        counters[mimeType]++;
      }
    }
    //Q_EMIT input_status("File: "+QString::fromStdString(article->getAid()));
    ++count;
    if(count%1000 == 1 && count > 1000)
    {
        Q_EMIT input_status(tr("已扫描:")+QString::number(count));
    }
    return article;
}

zim::Blob ArticleSource::getData(const std::string& aid)
{
    QString aid_url = input + QString::fromStdString(directoryPath + "/" + aid);

    if (data != NULL) {
      delete(data);
      data = NULL;
    }
    //qDebug()<<aid_url;
    string b;
    QString b2;

    if (aid.substr(0, 3) == "/M/") {
      std::string value;

      if ( aid == "/M/Language") {
        value = language;
      } else if (aid == "/M/Creator") {
        value = creator;
      } else if (aid == "/M/Publisher") {
        value = publisher;
      } else if (aid == "/M/Title") {
        value = title;
      } else if (aid == "/M/Description") {
        value = description;
      } else if ( aid == "/M/Date") {
        time_t t = time(0);
        struct tm * now = localtime( & t );
        std::stringstream stream;
        stream << (now->tm_year + 1900) << '-'
           << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '-'
           << std::setw(2) << std::setfill('0') << now->tm_mday;
        value = stream.str();
      } else if ( aid == "/M/Counter") {
        std::stringstream stream;
        for (std::map<std::string, unsigned int>::iterator it = counters.begin(); it != counters.end(); ++it) {
      stream << it->first << "=" << it->second << ";";
        }
        value = stream.str();
      }

      dataSize = value.length();
      data = new char[dataSize];
      memcpy(data, value.c_str(), dataSize);
    }
    else
    {
        srcfile.setFileName(aid_url);
        file_info.setFile(aid_url);
        if(srcfile.open(QFile::ReadOnly))
        {
            s = srcfile.readAll();
            srcfile.close();
            dataSize = file_info.size();
        }
        else
        {
            Q_EMIT input_status(tr("读取文件: 无法打开文件:")+aid_url);
            s = NULL;
            dataSize = 0;
        }
        dataSize = file_info.size();
        filePath = file_info.absolutePath();
        if (current_dir != filePath)
        {
            current_dir = filePath;
            Q_EMIT input_status(tr("正在读取文件: 进入目录:")+current_dir);
        }

        b = s.toStdString();
        //cout<<aid<<" | "<<getMimeTypeForFile(aid)<<" | "<<extMimeTypes["html"];
        if (getMimeTypeForFile(aid).find("text/html") == 0) {
              std::string html = b;
              htmlparse.reset();
              htmlparse.parse_html(b,"utf-8",true);
              add_to_index(htmlparse.dump,aid);

              /* Rewrite links (src|href|...) attributes */
              GumboOutput* output = gumbo_parse(html.c_str());
              GumboNode* root = output->root;

              std::map<std::string, bool> links;
              getLinks(root, links);
              std::map<std::string, bool>::iterator it;
              std::string aidDirectory = removeLastPathElement(aid, false, false);

              /* If a link appearch to be duplicated in the HTML, it will
             occurs only one time in the links variable */
              for(it = links.begin(); it != links.end(); it++) {
            if (!it->first.empty() && it->first[0] != '#') {
              replaceStringInPlace(html, "\"" + it->first + "\"", "\"" + computeNewUrl(aid, it->first) + "\"");
            }
              }
              gumbo_destroy_output(&kGumboDefaultOptions, output);

              dataSize = html.length();
              data = new char[dataSize];
              memcpy(data, html.c_str(), dataSize);
            } else if (getMimeTypeForFile(aid).find("text/css") == 0) {
              std::string css = b;

              /* Rewrite url() values in the CSS */
              size_t startPos = 0;
              size_t endPos = 0;
              std::string url;

              while ((startPos = css.find("url(", endPos)) && startPos != std::string::npos) {

            /* URL delimiters */
            endPos = css.find(")", startPos);
            startPos = startPos + (css[startPos+4] == '\'' || css[startPos+4] == '"' ? 5 : 4);
            endPos = endPos - (css[endPos-1] == '\'' || css[endPos-1] == '"' ? 1 : 0);
            url = css.substr(startPos, endPos - startPos);
            std::string startDelimiter = css.substr(startPos-1, 1);
            std::string endDelimiter = css.substr(endPos, 1);

            if (url.substr(0, 5) != "data:") {
              /* Deal with URL with arguments (using '? ') */
              std::string path = url;
              size_t markPos = url.find("?");
              if (markPos != std::string::npos) {
                path = url.substr(0, markPos);
              }

              /* Embeded fonts need to be inline because Kiwix is
                 otherwise not able to load same because of the
                 same-origin security */
              std::string mimeType = getMimeTypeForFile(path);
              if (mimeType == "application/font-ttf" ||
                  mimeType == "application/font-woff" ||
                  mimeType == "application/vnd.ms-opentype" ||
                  mimeType == "application/vnd.ms-fontobject") {

                try {
                  std::string fontContent = getFileContent(directoryPath + "/" + computeAbsolutePath(aid, path));
                  replaceStringInPlaceOnce(css,
                               startDelimiter + url + endDelimiter,
                               startDelimiter + "data:" + mimeType + ";base64," +
                               base64_encode(reinterpret_cast<const unsigned char*>(fontContent.c_str()), fontContent.length()) +
                               endDelimiter
                               );
                } catch (...) {
                }
              } else {

                /* Deal with URL with arguments (using '? ') */
                if (markPos != std::string::npos) {
                  endDelimiter = url.substr(markPos, 1);
                }

                replaceStringInPlaceOnce(css,
                             startDelimiter + url + endDelimiter,
                             startDelimiter + computeNewUrl(aid, path) + endDelimiter);
              }
            }
              }

              dataSize = css.length();
              data = new char[dataSize];
              memcpy(data, css.c_str(), dataSize);
            }
        else {
            if (getMimeTypeForFile(aid).find("text/plain") == 0)
                add_to_index(b,aid);
            data = new char[dataSize];
            str1 = b;
            memcpy(data, str1.c_str(), dataSize);
        }
    }
    return zim::Blob(data, dataSize);
}

void ArticleSource::add_to_index(const std::string &text,const std::string &aid)
{
    Xapian::Document doc;
    termgen.set_document(doc);
    termgen.index_text_without_positions(text,1,"C");
    if(doc.termlist_count()!=0)
    {
        ++count_index;
        doc.add_value(1,code_name);
        doc.set_data(aid);
        db.add_document(doc);
        if(count_index%1000 == 1 && count_index > 1000)
        {
            //Q_EMIT input_status(tr("Xapian test:")+QString::fromStdString(format_doc_termlist(doc)+text));
            Q_EMIT input_status(tr("Xapian 已索引:")+QString::number(count_index));
            db.commit();
        }
    }
}

Article::Article(const string& path)
{

  /* aid */
  aid = path.substr(directoryPath.size()+1);

  /* url */
  url = aid;

  /* mime-type */
  mimeType = getMimeTypeForFile(aid);
  //Q_EMIT input_status(QString::fromStdString(mimeType));
  /* HTML specific code */
  if (mimeType.find("text/html") != std::string::npos) {
    std::size_t found;
    std::string html = getFileContent(path);
    GumboOutput* output = gumbo_parse(html.c_str());
    GumboNode* root = output->root;

    /* Search the content of the <title> tag in the HTML */
    if (root->type == GUMBO_NODE_ELEMENT && root->v.element.children.length >= 2) {
      const GumboVector* root_children = &root->v.element.children;
      GumboNode* head = NULL;
      for (int i = 0; i < root_children->length; ++i) {
    GumboNode* child = (GumboNode*)(root_children->data[i]);
    if (child->type == GUMBO_NODE_ELEMENT &&
        child->v.element.tag == GUMBO_TAG_HEAD) {
      head = child;
      break;
    }
      }

      if (head != NULL) {
    GumboVector* head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
      GumboNode* child = (GumboNode*)(head_children->data[i]);
      if (child->type == GUMBO_NODE_ELEMENT &&
          child->v.element.tag == GUMBO_TAG_TITLE) {
        if (child->v.element.children.length == 1) {
          GumboNode* title_text = (GumboNode*)(child->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT) {
        title = title_text->v.text.text;
          }
        }
      }
    }

    /* Detect if this is a redirection */
    std::string targetUrl;
    try {
      targetUrl = extractRedirectUrlFromHtml(head_children);
    } catch (std::string &error) {
      std::cerr << error << std::endl;
    }

    if (!targetUrl.empty()) {
      redirectAid = computeAbsolutePath(aid, decodeUrl(targetUrl));
      if (!fileExists(directoryPath + "/" + redirectAid)) {
        redirectAid.clear();
        invalid = true;
      }
    }
      }

      /* If no title, then compute one from the filename */
      if (title.empty()) {
    found = path.rfind("/");
    if (found!=std::string::npos) {
      title = path.substr(found+1);
      found = title.rfind(".");
      if (found!=std::string::npos) {
        title = title.substr(0, found);
      }
    } else {
      title = path;
    }
    std::replace(title.begin(), title.end(), '_',  ' ');
      }
    }

    gumbo_destroy_output(&kGumboDefaultOptions, output);
  }
}

std::string Article::getAid() const
{
  return aid;
}

char Article::getNamespace() const
{
  return 'A';
}

std::string Article::getUrl() const
{
  return url;
}

std::string Article::getTitle() const
{
  return title;
}

bool Article::isRedirect() const
{
  return !redirectAid.empty();
}

bool Article::isInvalid() const
{
  return invalid;
}

std::string Article::getMimeType() const
{
  return mimeType;
}

std::string Article::getRedirectAid() const
{
  return redirectAid;
}

bool Article::shouldCompress() const {
  return true;
}
