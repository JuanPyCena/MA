///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Network transparent configuration management classes.
*/


// System includes
#include <iostream>
#include <cstdlib>
#include <assert.h>//TODO
using namespace std;

// QT includes
#include <QDir>
#include <QApplication>

// AVLib includes
#include "avconfig.h" // TODO CM QTP-10
#include "avlog.h"
#include "avmacros.h"
#include "avpackageinfo.h"

//TODO_QT3#include "avapplication.h"
//TODO_QT3#include "avclient.h"
//TODO_QT3#include "avmsgs.h"
//TODO_QT3#include "avcmds.h"
//TODO_QT3#include "avset.h"

///////////////////////////////////////////////////////////////////////////////

const char * const AVConfigDefaultUser = "default";

const int    AVConfigEntry::INT_DEFAULT_MAXMIN        = 2147483647;
const double AVConfigEntry::DOUBLE_DEFAULT_MAXMIN     = 10.0E38;
const int    AVConfigEntry::STRING_DEFAULT_MAX_LENGTH = 1024;
const int    AVConfigEntry::POINT_DEFAULT_MAXMIN      = 65535;

///////////////////////////////////////////////////////////////////////////////


AVConfigEntry::AVConfigEntry()
{
    defaultIsValid=false;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, int *v, const int vd, const char *d,
                             const char *o, int vmin, int vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, uint *v, const uint vd,
                             const char *d, const char *o, uint vmin,
                             uint vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, double *v, const double vd,
                             const char *d, const char *o, double vmin,
                             double vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, bool *v, const bool vd,
                             const char *d, const char *o)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QString *v, const QString &vd,
                             const char *d, const char *o, AVCVariant::Type t,
                             int vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v,t);
    desc=d;
    option=o;
    *v=vd;
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QRect *v, const QRect &vd,
                             const char *d, const char *o, const QRect &vmin,
                             const QRect &vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QColor *v, const QColor &vd,
                             const char *d, const char *o)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QSize *v, const QSize &vd,
              const char *d, const char *o, const QSize &vmin,
              const QSize &vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QPoint *v, const QPoint &vd,
                             const char *d, const char *o, const QPoint &vmin,
                             const QPoint &vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QStringList *v,
                             const QStringList &vd, const char *d,
                             const char *o, int vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, AVIntList *v, const AVIntList &vd,
                             const char *d, const char *o, int vmin, int vmax)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    min.set(vmin);
    max.set(vmax);
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry::AVConfigEntry(const char *k, QFont *v, const QFont &vd,
                             const char *d, const char *o)
{
    key=k;
    AVASSERT(key.contains(' ') == 0);
    var.setAddr(v);
    desc=d;
    option=o;
    *v=vd;
    def=var;
    defaultIsValid=true;
    unused=false;
    help="";
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry &AVConfigEntry::operator=(AVConfigEntry &e)
{
    if (&e == this) return *this;
    copy(e);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigEntry::copy(AVConfigEntry &e)
{
    key       = e.key;
    desc      = e.desc;
    option    = e.option;
    reference = e.reference;
    setVar(e.getVar());
    setMin(e.getMin());
    setMax(e.getMax());
    setDefault(e.getDefault());
    defaultIsValid=e.defaultIsValid;
    unused=e.unused;
    setHelp(e.getHelp());
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigEntry::write(QDataStream &s)
{
    quint8 i=static_cast<quint8>(defaultIsValid);
    s << desc << option << key << var << min << max << def <<
    i << help;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigEntry::writeShort(QDataStream &s)
{
    s << desc << var;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigEntry::read(QDataStream &s)
{
    quint8 i;
    s >> desc >> option >> key >> var >> min >> max >> def >>
    i >> help;
    defaultIsValid=static_cast<bool>(i);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigEntry::write(QTextStream &s)
{
    s << option << "," << desc << "\n";
    s << key << " = ";
    return var.write(s,def,min,max,defaultIsValid,help,reference);
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigEntry::writeShort(QTextStream &s)
{
    s << key << " = ";
    return var.write(s);
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigEntry::read(QTextStream &s)
{
    QString line;
    while (var.readLine(s, line)) {

        // if this is the start of a new parameter set -> return

        if (line.startsWith("[")) return false;

        // parse lines: option,desc

        int pc = line.indexOf(",");
        if (pc < 0) continue;                // invalid line
        option = line.left(pc);
        desc   = line.mid(pc + 1);

        // parse line: key = value

        s >> key;
        QString eq;
        s >> eq;
        if (eq != QString("=")) continue;    // need KEY = VALUE here
        if (!var.read(s,def,min,max,defaultIsValid,help, reference))
        {
            AVLogger->Write(LOG_WARNING, "AVConfigEntry:read failed to parse entry '" + key + "'");
            return false;
        }
        else
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVConfigBase::m_params;
bool AVConfigBase::m_useConfigFileMappings = false;
QMap<QString, QString> AVConfigBase::m_configFileMappings;
bool AVConfigBase::m_verboseLogging = false;

bool AVConfigBase::option_help = false;
bool AVConfigBase::option_version = false;
bool AVConfigBase::option_save = false;
QString AVConfigBase::option_cfg = "";
QString AVConfigBase::option_defps = "";
int AVConfigBase::option_debug = 0;

bool AVConfigBase::process_uses_avconfig2 = false;

///////////////////////////////////////////////////////////////////////////////

AVConfigBase::AVConfigBase(const QString &an, const QString &cfgfile,
                           bool allowOptionCfg, int argc, char **argv)
{
    // TODO CM check what else to omit in the following code...
    if (!AVConfigBase::process_uses_avconfig2)
    {
        // initialize params if called the first time
        params(argc, argv);
        parseSpecialParams();
    }

    // save names and create the internal variable entry list
    app_name = an;
    if (m_useConfigFileMappings) app_name = configFileMapping(an);

    // check the environment variable for the IO name
    ioname = AVEnvironment::getApplicationConfig();
    if (ioname.isEmpty())
    {
        ioname = QDir::homePath();
        if (ioname.isEmpty()) ioname = ".";
    }
    cfg_fname = "";
    if (allowOptionCfg && !option_cfg.isEmpty())
    {
        cfg_fname_override = ioname + "/" + option_cfg; // TODO: why ioname+...?
        option_cfg = ""; // "consume" the -cfg option
    }
    else
    {
        cfg_fname_override = cfgfile;
    }

    if (option_defps.isEmpty()) def_paraset_name = "default";
    else def_paraset_name = option_defps;

    paraset_list  = 0;
    user_list     = 0;
    elist         = new AVConfigEntryList();
    AVASSERT(elist != 0);
    allow_default = false;
    m_unusedParamsPolicy = LogAndAddUnusedParams;

}

///////////////////////////////////////////////////////////////////////////////

AVConfigBase::AVConfigBase(const AVConfigBase &b)
{
    paraset_list  = 0;
    user_list     = 0;
    elist         = 0;
    allow_default = false;
    m_unusedParamsPolicy = LogAndAddUnusedParams;
    copy(b);
};

///////////////////////////////////////////////////////////////////////////////

AVConfigBase::~AVConfigBase()
{
    while (!elist->isEmpty())
        delete elist->takeFirst();
    delete elist;
    delete paraset_list;
    delete user_list;
}

///////////////////////////////////////////////////////////////////////////////

AVConfigBase& AVConfigBase::operator=(const AVConfigBase &b)
{
    if (&b == this) return *this;
    copy(b);
    return *this;
};

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::copy(const AVConfigBase &b, bool onlyData)
{
    allow_default = b.allow_default;
    m_unusedParamsPolicy = b.m_unusedParamsPolicy;
    if (!onlyData) {
        app_name           = b.app_name;
        user_name          = b.user_name;
        paraset_name       = b.paraset_name;
        def_paraset_name   = b.def_paraset_name;
        ioname             = b.ioname;
        cfg_fname          = b.cfg_fname;
        cfg_fname_override = b.cfg_fname_override;
        cfg_name           = b.cfg_name;
    }
    if (b.paraset_list != 0) {
        paraset_list = new QStringList(*b.paraset_list);
        AVASSERT(paraset_list != 0);
    }
    if (b.user_list != 0) {
        user_list    = new QStringList(*b.user_list);
        AVASSERT(user_list != 0);
    }
    if (b.elist != 0) {
        if (elist == 0) {
            elist = new AVConfigEntryList;
            AVASSERT(elist != 0);
            QListIterator<AVConfigEntry*> it(*b.elist);
            while (it.hasNext()) {
                AVConfigEntry *e  = it.next();
                AVASSERT(e != 0);
                AVConfigEntry *ec = new AVConfigEntry(*e);
                AVASSERT(ec != 0);
                elist->append(ec);
            }
        } else {
            QMutableListIterator<AVConfigEntry*> it(*b.elist);
            while (it.hasNext()) {
                AVConfigEntry *e  = it.next();
                int index = indexOfConfigEntry((*e).key);
                if (index >= 0) {
                    AVConfigEntry *c = elist->at(index);
                    if (c != 0) c->setVar(e->getVar());
                }
                else
                    elist->append(new (LOG_HERE) AVConfigEntry(*e));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

QStringList& AVConfigBase::params(int argc, char **argv)
{
    if (m_params.isEmpty()) {
        if (!argc) {
            if (!qApp) {
                if (AVLogger)
                    AVLogger->Write(LOG_ERROR,
                        "** ERROR: AVConfigBase requires QApplication");
                else
                    cerr << "** ERROR: AVConfigBase requires QApplication\n";
                exit(EXIT_FAILURE);
            }

            QStringList arguments = qApp->arguments();

            Q_FOREACH(const QString& arg, arguments)
            {
               m_params << arg;
            }
        }
        else
        {
            for (int i = 0; i < argc; i++) m_params << QString(argv[i]);
        }
    }
    return m_params;
}
///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::Add(AVConfigEntry *e)
{
    if (elist != 0 && e != 0) elist->append(e);
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::AddHelpText(const QString &key, const QString &text)
{
    if (elist != 0)
    {
        AVConfigEntry e;
        e.key=key;
        int index = indexOfConfigEntry(key);
        if (index >= 0)
        {
            AVConfigEntry *c = elist->at(index);
            if (c != 0)
            {
                c->setHelp(text);
                return true;
            }
        }
        else
            return false;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

AVConfigEntry* AVConfigBase::getParam(const QString &key) const
{
    if (elist == 0)
        return 0;

    if (key.isEmpty())
        return 0;

    for (int i = 0; i < elist->size(); ++i)
    {
       if (elist->at(i)->key == key)
         return elist->at(i);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::readConfig(const QString &ps, const QString &un, bool allowOptionSave,
                              bool resetEntries)
{
    if (process_uses_avconfig2)
    {
        if (!app_name.isEmpty())
        {
            AVLogError << "AVConfigBase::readConfig called (\"" << app_name << "\") although this is an AVConfig2 process!";
        }
    }

    AVLogger->Write(LOG_DEBUG, "AVConfigBase:readConfig: "
                    "reading paraset (" + ps + ")");

    if (resetEntries) resetEntriesToDefault();

    // prepare I/O parameters
    prepareIO(ps, un);
    bool rc = readConfigFile();

    // output message
    if (rc)
    {
        writeConfigIOMessage("reading");
        if (allowOptionSave && option_save) saveAndExit();
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::writeConfig(const QString &ps, const QString &un,
                               writeConfigMode mode)
{
    // prepare I/O parameters
    prepareIO(ps, un, false);

    bool rc = writeConfigFile(mode);

    // output message
    if (rc) writeConfigIOMessage("writing");
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::checkUserName(QString &un)
{
    int p;
    if (un.isEmpty()) return;
    while ((p = un.indexOf("_")) >= 0) {
        un = un.left(p) + un.mid(p + 1);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::removeUnusedParams()
{
    for (int i = 0; i < elist->size(); ++i)
    {
        if (elist->at(i)->isUnused())
        {
            delete elist->takeAt(i);
            --i;
        }

    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::readConfigFileMappings(const QString& suffix)
{
    QMutableStringListIterator it(m_params);
    QString param;
    while (it.hasNext())
    {
        param = it.next();
        if (param == "-config") {
            it.remove();
            AVASSERT(it.hasNext());
            QString fileName = it.next();
            it.remove();
            parseConfigFileMappings(fileName, suffix);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfigBase::configFileMapping(const QString& fileName, const QString& suffix)
{
    if (!m_useConfigFileMappings) {
        QString result = fileName;
        if (!result.endsWith(suffix)) result += suffix;
        return result;
    }
    QString key = fileName;
    if (key.endsWith(suffix)) key = key.left(key.length() - suffix.length());
    QString result = fileName;
    if (m_configFileMappings.contains(key)) result = m_configFileMappings[key];
    if (!result.endsWith(suffix)) result += suffix;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::prepareIO(const QString &ps, const QString &un, bool readOnly)
{
    // if a parameter set name is specified, use it. otherwise use the
    // already defined parameter set name
    if (!ps.isNull())
    {
        AVLogger->Write(LOG_DEBUG, "AVConfigBase:prepareIO: "
                        "setting requested paraset (" + paraset_name + ")");
        paraset_name = ps;
    }

    if (paraset_name.isEmpty() || paraset_name == "default")
    {
        AVLogger->Write(LOG_DEBUG, "AVConfigBase:prepareIO: "
                        "setting paraset (" + def_paraset_name + ")");
        paraset_name = def_paraset_name;
    }

    // if a username is specified, use it.
    if (!un.isEmpty())
    {
        user_name = un;
        checkUserName(user_name);
    }

    if (cfg_fname_override.isEmpty())
    {
        QString an(app_name);

        // allow specifying absolute paths
        if (!QDir::isRelativePath(an) && QFile::exists(an))
            cfg_fname = an;
        else {
            if (an.right(4) != ".cfg") an += ".cfg";
            if (user_name.isEmpty() || user_name == "default")
                cfg_fname = ioname + "/" + an;
            else {
                cfg_fname = ioname + "/" + user_name + "_" + an;

                // if the filename does not exist for this user, "allow_default"
                // is set, we try the default user called "default" so ->

                if (allow_default && readOnly) {
                    QFileInfo f(cfg_fname);
                    if (!f.exists() && !f.isReadable()) {
                        cfg_fname = ioname + "/" + AVConfigDefaultUser + "_" + an;
                    }
                }
            }
        }
    } else cfg_fname = cfg_fname_override;

    // make sure the filename has got a .cfg extension
    if (cfg_fname.right(4).toLower() != ".cfg") cfg_fname = cfg_fname + ".cfg";

    // finally, if the cfg_fname is a relative filename, used ioname as the
    // path to the filename
    QFileInfo fi(cfg_fname);
    if (fi.isRelative()) {
        QFileInfo dn(ioname);
        if (dn.isDir()) {
            cfg_fname = ioname + "/" + cfg_fname;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::readConfigFile()
{
    QFile f(cfg_fname);
    if (m_verboseLogging)
        AVLogger->Write(LOG_INFO, "readConfigFile " + cfg_fname);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (m_verboseLogging)
            AVLogger->Write(LOG_WARNING, "AVConfigBase:readConfigFile: "
                            "could not read file (" + cfg_fname + ")");
        else
            AVLogger->Write(LOG_DEBUG, "AVConfigBase:readConfigFile: "
                            "could not read file (" + cfg_fname + ")");
        return false;
    }
    QTextStream s(&f);
    QString line=s.readLine();

    if(line.startsWith("#")) cfg_name=line.mid(1);
    else                     cfg_name=QString();

    s.seek(0);

    if (!paraset_name.isEmpty())
    {
        QString psn("");
        // find paraset
        if (!findParaSet(paraset_name, s,psn) && !option_save)
        {
            QFileInfo file_info(cfg_fname);
            if (!AVConfigBase::process_uses_avconfig2) {
                AVLogger->Write(LOG_ERROR, "AVConfigBase:readConfigFile: "
                            "could not find requested paraset (" + paraset_name + ") in " +
                            file_info.fileName());
            }

            // try again with default set if not found
            s.seek(0);
            // find default paraset
            if (!findParaSet("default", s,psn))
            {
                if (!AVConfigBase::process_uses_avconfig2) {
                    // default not found either, give up
                    AVLogger->Write(LOG_ERROR, "AVConfigBase:readConfigFile: "
                                    "could not find [default] paraset");
                }
                f.close();
                return false;
            }
            else
            {
                paraset_name = "default";
            }
        }
    }
    readEntriesFromStream(s);
    f.close();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::writeConfigFile(writeConfigMode mode)
{
    // see that there is a config directory anyway

    QFileInfo fileinfo(cfg_fname + ".tmp");
    QDir      dir(fileinfo.dir());
    if (!dir.exists()) {
        if (!dir.mkdir(fileinfo.path())) {
            AVLogger->Write(LOG_ERROR, "could not create config directory "+ fileinfo.path());
            return false;
        }
    }

    // if in overwrite mode -> just create the new file

    if (mode == Overwrite) {
        QFile fo(cfg_fname);
        if (m_verboseLogging)
            AVLogger->Write(LOG_INFO, "writeConfigFile (overwrite) " +  cfg_fname);
        if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            AVLogger->Write(LOG_ERROR, "AVConfigBase:writeConfigFile(overwrite): "
                            "could not open file (" + cfg_fname +")");
            return false;
        }
        QTextStream so(&fo);
        if(!cfg_name.isEmpty())
            so << "#" << cfg_name << "\n";
        so << "[" << paraset_name << "]\n";
        writeEntriesToStream(so);
        fo.close();
        return true;
    }

    // if in append mode -> append to the file

    if (mode == Append) {
        QFile fo(cfg_fname);
        if (m_verboseLogging)
            AVLogger->Write(LOG_INFO, "writeConfigFile (append) "+  cfg_fname);
        if (!fo.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            AVLogger->Write(LOG_ERROR, "AVConfigBase:writeConfigFile(append): "
                            "could not open file (" + cfg_fname + ")");
            return false;
        }
        QTextStream so(&fo);
        so << "[" << paraset_name << "]\n";
        writeEntriesToStream(so);
        fo.close();
        return true;
    }

    // here for normal mode: open input and output file

    if (m_verboseLogging)
        AVLogger->Write(LOG_INFO, "writeConfigFile " + cfg_fname);
    QFile fo(cfg_fname + ".tmp");
    if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        AVLogger->Write(LOG_ERROR, "AVConfigBase:writeConfigFile(normal): "
                        "could not open file (" + cfg_fname + ")");
        return false;
    }
    QTextStream so(&fo);

    QFile fi(cfg_fname);
    bool fiopen = fi.open(QIODevice::ReadOnly | QIODevice::Text);

    // only if there's an existing config file anyway ->

    if (fiopen) {
        QTextStream si(&fi);

        // copy lines to the output file, skip the parameter set which
        // we currently write

        bool inps = false;
        while (true) {
            QString line = si.readLine();
            if (line.isNull()) break;
            if (inps) {
                if (compareParaSet(line, QString())) {
                    inps = false;
                    so << line << "\n";
                }
            } else {
                if (compareParaSet(line, paraset_name)) inps = true;
                else                                    so << line << "\n";
            }
        }
        fi.close();
    }

    // append the new paraset

    so << "[" << paraset_name << "]\n";

    // write out the single entries

    writeEntriesToStream(so);
    fo.close();

    // finally rename the old file and

    QFile ft(cfg_fname + ".bck");
    if (ft.exists()) ft.remove();
    if (fiopen) rename(qPrintable(cfg_fname), qPrintable(cfg_fname + ".bck"));
    rename(qPrintable(fo.fileName()), qPrintable(cfg_fname));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::readEntriesFromStream(QTextStream &s)
{
    // This occurs when the old config implementation is used to load the new config. If a key
    // is encountered twice, we want it twice in our list so an appropriate error can be generated.
    // In this case, don't replace/set the value already present in elist.
    bool ignore_existing = AVConfigBase::process_uses_avconfig2 &&
                           m_unusedParamsPolicy == AddUnusedParams;
    if (ignore_existing) AVASSERT(elist->isEmpty());

    while (!s.atEnd())
    {
        AVConfigEntry e;
        if (!e.read(s)) break;
        int index = indexOfConfigEntry(e.key);
        if (index >= 0 && !ignore_existing)
        {
            AVConfigEntry *c = elist->at(index);
            if (c != 0)
            {
                if(c->getVar().type() == e.getVar().type())
                {
                    c->setVar(e.getVar());
                }
                else
                {
                    AVLogger->Write(LOG_WARNING,
                               "LOG_WARNING, AVConfigBase:readEntriesFromStream(text): "
                               "Entry " + c->key + " has type " + c->getVar().typeName() +
                               " in config and mismatching" + e.getVar().typeName() + " in file");
                }
            }
        }
        else
        {
            switch(m_unusedParamsPolicy)
            {
                case AddUnusedParams: {
                    elist->append(new (LOG_HERE) AVConfigEntry(e));
                break;
                }
                case LogAndAddUnusedParams: {
                    AVConfigEntry *entry = new (LOG_HERE) AVConfigEntry(e);elist->append(entry);
                    entry->markUnused();
                }
                case LogAndDiscardUnusedParams: {
                    AVLogger->Write(LOG_WARNING,
                        "AVConfigBase:readEntriesFromStream(text): "
                        "Potentially unused param ( " + e.key + ")");
                    break;
                }
                case DiscardUnusedParams: break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::writeEntriesToStream(QTextStream &s)
{
    for (int i = 0; i < elist->count(); i++) {
        AVConfigEntry *e = elist->at(i);
        if (e != 0 && e->key.length() > 0) {
            if (!e->write(s)) break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::writeEntriesToStreamShort(QTextStream &s)
{
    for (int i = 0; i < elist->count(); i++) {
        AVConfigEntry *e = elist->at(i);
        if (e != 0 && e->key.length() > 0) {
            if (!e->writeShort(s)) break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::readEntriesFromStream(QDataStream &s)
{
    int n;
    s >> cfg_name;      // read the default name
    s >> n;             // read the number of entries
    for (int i = 0; i < n; i++)
    {
        if (s.atEnd()) break;
        AVConfigEntry e;
        if (!e.read(s)) break;
        int index = indexOfConfigEntry(e.key);
        if ( index >= 0)
        {
            AVConfigEntry *c = elist->at(index);
            if (c != 0) c->setVar(e.getVar());
        }
        else
        {
            switch(m_unusedParamsPolicy)
            {
                case AddUnusedParams: {
                    elist->append(new (LOG_HERE) AVConfigEntry(e));
                break;
                }
                case LogAndAddUnusedParams: {
                    AVConfigEntry *entry = new (LOG_HERE) AVConfigEntry(e);elist->append(entry);
                    entry->markUnused();
                }
                case LogAndDiscardUnusedParams: {
                    AVLogger->Write(LOG_WARNING, "AVConfigBase:readEntriesFromStream(data): "
                        "Potentially unused param (" + e.key + ")");
                    break;
                }
                case DiscardUnusedParams: break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::writeEntriesToStream(QDataStream &s)
{
    s << cfg_name;          // write out the default_name
    s << elist->count();    // write out the number of entries
    for (int i = 0; i < elist->count(); i++) {
        AVConfigEntry *e = elist->at(i);
        if (e != 0 && e->key.length() > 0) {
            if (!e->write(s)) break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::standardInit(bool check_for_invalid_params,
                                bool check4help, bool dosave)
{
    bool rc = readConfig(0, 0, false);
    parseParams(check_for_invalid_params, check4help, dosave);
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::standardInitFromParaset(const QString& ps,
                                           bool check_for_invalid_params,
                                           bool check4help, bool dosave)
{
    bool rc = readConfig(ps, QString(), false);
    parseParams(check_for_invalid_params, check4help, dosave);
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::checkForHelp(bool do_exit)
{
    if (option_help) {
        usage(m_params[0]);
        if (do_exit) exitApp(EXIT_SUCCESS);
        else return;
    }
    if (option_version) {
        version(m_params[0]);
        if (do_exit) exitApp(EXIT_SUCCESS);
        else return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::help(const QString& argv0, const QString &msg)
{
    usage(argv0);
    if (!msg.isNull()) cout << endl << qPrintable(msg) << endl;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::parseParams(bool check_for_invalid_params, bool check4help, bool dosave)
{
    AVLogger->Write(LOG_DEBUG, "AVConfigBase:parseParams(%d,%d,%d): ",
        check_for_invalid_params, check4help, dosave);

    bool rc = true;
    int i = 1;
    while(i < m_params.count())
    {
        AVLogger->Write(LOG_DEBUG1, "AVConfigBase:parseParams: Reading cmd argument '"
                                + m_params[i] + "'...");

        if (m_params[i].startsWith("-"))
        {
            QString opt = m_params[i].mid(1);
            AVLogger->Write(LOG_DEBUG1, "AVConfigBase:parseParams: "
                "Cmd line argument param '" + m_params[i] + "' detected as cmd line param "
                "option due to trailing '-'. Trying to find corresponding config entry...");

            QString key;
            int ie = findParam(opt, key);
            if (ie >= 0)
            {
                AVConfigEntry *e = elist->at(ie);
                if (!e)
                {
                    AVLogger->Write(LOG_ERROR, "AVConfigBase:parseParams: "
                    "Found corresponding config entry but could NOT retrieve it from the "
                    "config - ambiguous! (param " + QString::number(ie)
                                    + ", option " + opt + ")");
                    ++i;
                    continue;
                }
                else
                {
                    AVLogger->Write(LOG_DEBUG1, "AVConfigBase:parseParams: "
                        "Found corresponding config entry: key='" + e->key + "'");
                }

                m_params.removeAt(i);

                if (i < m_params.count())
                {
                    const QString& val = m_params[i];
                    e->getVar().sets(val);
                    m_params.removeAt(i);
                }
                else
                {
                    AVLogger->Write(LOG_ERROR, "** ERROR: option -" +
                                    opt + " specified without argument");
                    rc = false;
                }

                --i;
            }
            else
            {
                AVLogger->Write(LOG_DEBUG1, "AVConfigBase:parseParams: "
                    "Could NOT find corresponding config entry!");
            }
        }

        ++i;
    }

    if (check4help) checkForHelp();
    if (check_for_invalid_params)
        if (!checkForInvalidParameters()) rc = false;
    if (rc && option_save && dosave) saveAndExit();
    if (!rc) exitApp(EXIT_FAILURE);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::saveAndExit()
{
    writeConfig();
    AVLogger->Write(LOG_INFO, "configuration saved to " + cfg_fname + ":" + paraset_name);
    exitApp(EXIT_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::checkForInvalidParameters()
{
    bool rc = true;
    for (int i = 1; i < m_params.count(); i++)
    {
        // See avconsole script for details about the av_bb_alias parameter.
        if (m_params[i].startsWith("-") && m_params[i] != "--av_bb_alias")
        {
            AVLogger->Write(LOG_ERROR, "** ERROR: invalid option " + m_params[i] + " specified!");
            rc = false;
        }
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::resetEntriesToDefault()
{
    QList<AVConfigEntry*>::iterator it = (*elist).begin();
    QList<AVConfigEntry*>::const_iterator it_end = (*elist).end();
    for (; it != it_end; ++it)
    {
        (*it)->setValueToDefault();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::parseConfigFileMappings(const QString& fileName, const QString& suffix)
{
    m_useConfigFileMappings = true;

    QString path = AVEnvironment::getApplicationConfig();
    if (path.isEmpty()) {
        path = QDir::homePath();
        if (path.isEmpty()) path = ".";
    }
    AVASSERT(!path.startsWith("#"));
    QString fullPath = path + "/" + fileName;
    if (!fileName.endsWith(suffix)) fullPath += suffix;
    AVLogger->Write(LOG_INFO, "AVConfigBase: readConfigFileMappings %s", qPrintable(fullPath));
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        AVLogger->Write(LOG_FATAL, "AVConfigBase::parseConfigFileMappings: "
                "Failed to read mapping file %s", qPrintable(fileName));
    QString defaultMapping;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        if (line.startsWith("@")) {
            parseConfigFileMappings(line.mid(1), suffix);
            continue;
        }
        QString key, value;
        QStringList pair = line.split(':');
        int count = pair.count();
        if (count == 1) {

            // Allow mappings of the form "filename" to be used. The DEFAULT mapping is
            // applied in this case. If the default mapping is "DEFAULT:dir/*",
            // then "filename" is equivalent to "filename:dir/filename".

            key = pair[0].trimmed();
            if (defaultMapping.isEmpty())
                AVLogger->Write(LOG_FATAL, "AVConfigBase::parseConfigFileMappings: "
                        "no DEFAULT mapping specified for %s", qPrintable(key));
            value = defaultMapping;
        } else if (count == 2) {
            key = pair[0].trimmed();
            value = pair[1].trimmed();
        } else
            AVLogger->Write(LOG_FATAL, "AVConfigBase::parseConfigFileMappings: "
                    "invalid mapping %s", qPrintable(line));

        // Check for DEFAULT mapping

        if (key == "DEFAULT") {
            defaultMapping = value;
            AVASSERT(defaultMapping.contains('*') == 1);
            continue;
        }

        // Allow mappings of the form "filename:dir/*".
        // "filename:dir/*" is equivalent to "filename:dir/filename".

        AVASSERT(key.count('*') == 0);
        AVASSERT(value.count('*') <= 1);
        value.replace('*', key);

        if (m_configFileMappings.contains(key))
            AVLogger->Write(LOG_WARNING, "duplicate key %s", qPrintable(key));
        m_configFileMappings[key] = value;
    }
    file.close();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::parseSpecialParams()
{
    QString options ("-help:-version:-save:-cfg:-defps:-debug:-ssenabled:-ssfullscreen:-suppress_transient_problems");
    QStringList optionList = options.split(":");
    int i = 1;
    while (i < m_params.count()) {
        QString option = m_params[i];
        if (optionList.contains(option)) {
            m_params.removeAt(i);
            if (option == "-help") option_help = true;
            else if (option == "-version") option_version = true;
            else if (option == "-save") option_save = true;
            else if (option == "-suppress_transient_problems") {
                AVLog::setExecutionPhase(AVLog::EP_STARTING_UP);
            } else {
                // options with mandatory parameter
                if (i < m_params.count()) {
                    if (option == "-cfg") option_cfg = m_params[i];
                    else if (option == "-defps") option_defps = m_params[i];
//TODO_QT3 AVApplication fehlt
//TODO_QT3                    else if (option == "-ssenabled" && avApp != 0) {
//TODO_QT3                        bool b = (bool) m_params[i].toInt();
//TODO_QT3                        avApp->setScreenShotsEnabled(b, avApp->screenShotsFullScreen());
//TODO_QT3                        AVLogger->Write(LOG_INFO, "built-in screen saver %s",
//TODO_QT3                                        b ? "enabled" : "disabled");
//TODO_QT3                    }
//TODO_QT3                    else if (option == "-ssfullscreen" && avApp != 0) {
//TODO_QT3                        bool b = (bool) m_params[i].toInt();
//TODO_QT3                        avApp->setScreenShotsEnabled(avApp->screenShotsEnabled(), b);
//TODO_QT3                        AVLogger->Write(LOG_INFO, "built-in screen saver in %s",
//TODO_QT3                                        b ? "full-screen mode" : "window mode");
//TODO_QT3                    }
                    else if (option == "-debug") {
                        const QString& levelString = m_params[i];
                        bool ok;
                        int level = levelString.toInt(&ok);
                        if (!ok) {
                            AVLogger->Write(LOG_ERROR,"** parameter " + levelString
                                    + " to option debug invalid");
                            exit(EXIT_FAILURE);
                        }
                        option_debug = level;

                        // this also sets the logging debug level
                        if (option_debug == 1)
                            AVLogger->setMinLevel(AVLog::LOG__DEBUG);
                        if (option_debug == 2)
                            AVLogger->setMinLevel(AVLog::LOG__DEBUG1);
                        if (option_debug >= 3)
                            AVLogger->setMinLevel(AVLog::LOG__DEBUG2);
                    } // -an is ignored
                    m_params.removeAt(i);
                } else {
                    AVLogger->Write(LOG_ERROR, "** option " + option + " missing parameter");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else
            ++i; // continue to next parameter
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::findParaSet(const QString &psname, QTextStream &s,
                               QString &l)
{

    while (true) {
        QString line = s.readLine();
        if (line.isNull()) break;
        if (compareParaSet(line, psname)) {
            if (!l.isNull()) {  //QT4PORT remark: string is empty at start
                l = line.mid(1, line.length() - 2).trimmed();
            }
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfigBase::compareParaSet(const QString &line, const QString &psname)
{
    if (line.isNull() || line.isEmpty()) return false;
    if (line.startsWith("[")) {
        QString sline = line.trimmed();
        int pc = sline.indexOf("]");
        if (pc == sline.length() - 1) {
            if (psname.isEmpty()) return true;
            if (sline.mid(1, sline.length() - 2) == psname) return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

int AVConfigBase::findParam(const QString& parm, QString &key)
{
    for(int i = 0; i < elist->count(); i++) {
        AVConfigEntry *e = elist->at(i);
        if (e != 0 && e->option == parm) {
            key = e->key;
            return i;
        }
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::usage(const QString& prog) const
{
    static bool first = true;
    if (first) {
        QString p("program");
        if (!prog.isNull()) p = prog;
        cout << "\n";
        cout << "Usage: " << qPrintable(p) << " [options]\n";
        cout << "\n";
        cout << "General Options:\n\n";
        cout << "         -cfgdir d .... use this config directory\n";
        cout << "         -cfg c ....... use this config file or server\n";
        cout << "         -defps ps .... use another default parameter set\n";
        cout << "         -save ........ save the used parameters\n";
        cout << "         -debug n ..... set the debug level to n "
                                         "(default=0)\n";
        cout << "         -an app ...... set application to app (not supported "
                                         "by all progs.)\n";
        cout << "         -help ........ this help text\n";
        cout << "         -version ..... show version information\n";
//TODO_QT3 AVApplication fehlt
//TODO_QT3        if (avApp != 0) {
//TODO_QT3            cout << "         -widgetcount . print debug messages about "
//TODO_QT3                    "leftover widgets\n";
//TODO_QT3            cout << "         -ssenabled n . enable build-in screenshot function\n";
//TODO_QT3            cout << "         -ssfullscreen n ... screenshots fullscreen or mainwindow\n";
//TODO_QT3        }
//TODO_QT3#if defined(Q_WS_X11)
//TODO_QT3        if (avApp != 0) {
//TODO_QT3            cout << "\n";
//TODO_QT3            cout << "X11 Options (not all work for all applications):\n\n";
//TODO_QT3            cout << "         -display d ... use the specified display\n";
//TODO_QT3            cout << "         -geometry g .. use the specified geometry\n";
//TODO_QT3            cout << "         -fn f ........ use the specified font\n";
//TODO_QT3            cout << "         -bg c ........ use the specified background color\n";
//TODO_QT3            cout << "         -fg c ........ use the specified foreground color\n";
//TODO_QT3            cout << "         -btn c ....... use the specified button color\n";
//TODO_QT3            cout << "         -name n ...... set the application name\n";
//TODO_QT3            cout << "         -title t ..... set the application title (caption)\n";
//TODO_QT3            cout << "         -visual v .... use the specified visual\n";
//TODO_QT3            cout << "         -ncols n ..... limit # of allocated colors "
//TODO_QT3                    "for 8-bit visuals\n";
//TODO_QT3            cout << "         -cmap ........ use a private colormap on 8-bit visuals\n";
//TODO_QT3        }
//TODO_QT3#endif
        first = false;
    }

    cout << "\nProgram specific options:\n\n";
    printHelpOfEntries();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::printHelpOfEntries() const
{
    for(int i = 0; i < elist->count(); i++)
    {
        AVConfigEntry *e = elist->at(i);
        if (e == 0 || e->option.isEmpty()) continue;

        cout << "         ";
        int ndots = 10 - e->option.length();
        QString dots;
        dots.fill('.', ndots);

        cout << "-" << qPrintable(e->option) << " x " << qPrintable(dots) << " ";
        QStringList words = e->desc.split(" ");
        int len = 24;

        for (int j = 0; j < words.count(); j++)
        {
            int lw = words[j].length();
            if (len + lw >= 79)
            {
                cout << "\n                        ";
                len = 24;
            }

            cout << qPrintable(words[j]) << " ";
            len += (lw + 1);
        }

        cout << endl;
        // current values
        AVCVariant &var = e->getVar();
        QString val = var.toString();

        if (!val.isEmpty())
        {
            cout << "                        current value: "
                 << qPrintable(val) << "\n";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::version(const QString& prog) const
{
    static bool first = true;
    if (first) {
        QString p("program");
        if (!prog.isNull()) p = prog;
        cout << endl;
        cout << "About " << qPrintable(p) << ":" << endl;
        cout << qPrintable(AVEnvironment::getApplicationName()) << endl;
        cout << qPrintable(AVPackageInfo::getVersion()) << endl;
        cout << "Build " << qPrintable(AVPackageInfo::getBuildKey()) << endl;
        cout << qPrintable(AVEnvironment::getApplicationInfo()) << endl;
        first = false;
    }
}

///////////////////////////////////////////////////////////////////////////////

const QStringList &AVConfigBase::getParaSetList(const QString &un)
{
    // prepare I/O parameters

    QString tmp = user_name;
    prepareIO(paraset_name, un);
    user_name = tmp;

    // allocate a new string list if not yet done

    if (paraset_list) paraset_list->clear();
    else paraset_list = new QStringList();

    // open the input file

    QFile f(cfg_fname);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {

        // read all parameter sets

        QTextStream s(&f);
        QString psn("");
        while (findParaSet(QString(), s, psn)) {
            if (!psn.startsWith("."))         // skip hidden parameter-sets
                paraset_list->append(psn);
        }
        f.close();
    }

    // if the list is empty -> add a default entry

    if (paraset_list->isEmpty()) paraset_list->append("default");
    return *paraset_list;
}

///////////////////////////////////////////////////////////////////////////////

const QStringList &AVConfigBase::getUserList() const
{
    AVConfigBase *that = const_cast<AVConfigBase*>(this);      // get rid of const

    // prepare I/O parameters

    that->prepareIO(paraset_name, user_name);

    // prepare the user list, if not yet done

    if (user_list == 0) that->user_list = new (LOG_HERE) QStringList();
    else                user_list->clear();

    // open the directory and lookup all config files there

    QDir d(ioname, QString("*_") + app_name + ".cfg");
    d.setFilter(QDir::Files);

    // copy file names to our own list

    for (uint i = 0; i < d.count(); i++) {
        QString f = d[i];
        int p = f.indexOf("_");
        user_list->append(f.left(p));
    }
    return *user_list;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::setAppName(const QString &s)
{
    app_name = s;
    if (m_useConfigFileMappings) app_name = configFileMapping(s);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::writeConfigIOMessage(const QString& action)
{
    QString f = cfg_fname;
    if (!cfg_fname_override.isEmpty()) f = cfg_fname_override;
    if (AVLogger != 0) {
        AVLogger->Write(LOG_DEBUG, action + " config file " + f + ", paraset " + paraset_name);
    } else {
        qDebug() << "** INFO: " << action << " config file " << f << ", paraset " << paraset_name;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigBase::setConfigFileName(const QString &fname)
{
    cfg_fname = fname;
    if (cfg_fname.right(4).toLower() != ".cfg") {
        cfg_fname = cfg_fname + ".cfg";
    }
    cfg_fname_override = cfg_fname;
}

///////////////////////////////////////////////////////////////////////////////

int AVConfigBase::indexOfConfigEntry(const QString & compareKey)
{
    if (elist == 0)
        return -1;

    if (compareKey.isEmpty())
        return -1;

    for (int i = 0; i < elist->size(); ++i)
    {
       if (elist->at(i)->key == compareKey)
         return i;
    }
    return -1;
}

// end of file
