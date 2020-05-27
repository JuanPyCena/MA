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
#include "avconfig.h" // TODO CM remove after QTP-10 has been resolved
#include "avcvariant.h"
#include "avlog.h"
#include "avmacros.h"

///////////////////////////////////////////////////////////////////////////////

void AVCVariant::copy(const AVCVariant &s)
{
    if (s.addr() == 0)
    {
        AVLogger->Write(LOG_DEBUG2, "AVCVariant:copy: did not copy - addr == 0");
        return;
    }

    if (s.type() == Undef)
    {
        AVLogger->Write(LOG_DEBUG2, "AVCVariant:copy: did not copy - typ undef");
        return;
    }

    switch(s.type()) {
        case Int:         set(s.asIntConst());        break;
        case UInt:        set(s.asUIntConst());       break;
        case Double:      set(s.asDoubleConst());     break;
        case Bool:        set(s.asBoolConst());       break;
        case String:      set(s.asStringConst());     break;
        case Rect:        set(s.asRectConst());       break;
        case Color:       set(s.asColorConst());      break;
        case Size:        set(s.asSizeConst());       break;
        case Point:       set(s.asPointConst());      break;
        case StringList:  set(s.asStringListConst()); break;
        case IntList:     set(s.asIntListConst());    break;
        case Font:        set(s.asFontConst());       break;
        case Directory:   set(s.asDirectoryConst(),Directory);  break;
        case File:        set(s.asFileConst(),File);  break;
        case Undef:       AVASSERT(0); break;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::compare(const AVCVariant &s) const
{
    if (type() != s.type()) return false;
    if ((addr() == 0) && (s.addr() == 0)) return true;

    bool res = true;
    switch(s.type()) {
        case Int:         res = (asIntConst()        == s.asIntConst());        break;
        case UInt:        res = (asUIntConst()       == s.asUIntConst());       break;
        case Double:      res = (asDoubleConst()     == s.asDoubleConst());     break;
        case Bool:        res = (asBoolConst()       == s.asBoolConst());       break;
        case String:      res = (asStringConst()     == s.asStringConst());     break;
        case Rect:        res = (asRectConst()       == s.asRectConst());       break;
        case Color:       res = (asColorConst()      == s.asColorConst());      break;
        case Size:        res = (asSizeConst()       == s.asSizeConst());       break;
        case Point:       res = (asPointConst()      == s.asPointConst());      break;
        case StringList:  res = (asStringListConst() == s.asStringListConst()); break;
        case IntList:     res = (asIntListConst()    == s.asIntListConst());    break;
        case Font:        res = (asFontConst()       == s.asFontConst());       break;
        case Directory:   res = (asDirectoryConst()  == s.asDirectoryConst());  break;
        case File:        res = (asFileConst()       == s.asFileConst());       break;
        case Undef:       res = true;                                           break;
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void AVCVariant::sets(const QString &v) {
    int v11, v22, v33, v44;
    switch(t) {
        case Int:         set(v.toInt());        break;
        case UInt:        set(v.toUInt());       break;
        case Double:      set(v.toDouble());     break;
        case Bool:        set(v.toInt() != 0); break;
        case String:      set(v);                break;
        case Rect:        parseInt(v, &v11, &v22, &v33, &v44);
                          set(QRect(v11, v22, v33, v44));
                                                 break;
        case Color:       set(QColor(v));        break;
        case Size:        parseInt(v, &v11, &v22);
                          set(QSize(v11, v22));    break;
        case Point:       parseInt(v, &v11, &v22);
                          set(QPoint(v11, v22));   break;
        case StringList:  set(v.split(","));
                                                 break;
        case IntList:    { AVIntList il;
                           parseIntList(v, il);
                           set(il);        }     break;
        case Font:       { QFont f;
                           if (!f.fromString(v))
                           f = QFont("Helvetica");
                           set(f);
                         }                       break;
        case Directory:   set(v,Directory);      break;
        case File:        set(v,File);           break;
        case Undef:       break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVCVariant::delObject()
{
    if (va == 0) return;
    switch(t) {
        case Int:        delete static_cast<int*>(va);         break;
        case UInt:       delete static_cast<uint*>(va);        break;
        case Double:     delete static_cast<double*>(va);      break;
        case Bool:       delete static_cast<bool*>(va);        break;
        case String:     delete static_cast<QString*>(va);     break;
        case Rect:       delete static_cast<QRect*>(va);       break;
        case Color:      delete static_cast<QColor*>(va);      break;
        case Size:       delete static_cast<QSize*>(va);       break;
        case Point:      delete static_cast<QPoint*>(va);      break;
        case StringList: delete static_cast<QStringList*>(va); break;
        case IntList:    delete static_cast<AVIntList*>(va);   break;
        case Font:       delete static_cast<QFont*>(va);       break;
        case Directory:  delete static_cast<QString*>(va);     break;
        case File:       delete static_cast<QString*>(va);     break;
        case Undef:      break;
    }
    va = 0;
}

///////////////////////////////////////////////////////////////////////////////

void AVCVariant::parseInt(const QString &v, int *v11, int *v22, int *v33, int *v44)
{
    if (v33 != 0) {
        sscanf(qPrintable(v), "%d,%d,%d,%d", v11, v22, v33, v44);
    } else {
        sscanf(qPrintable(v), "%d,%d", v11, v22);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVCVariant::parseIntList(const QString &v, AVIntList &il)
{
    if (v.isEmpty())
    {
        return;
    }

    QByteArray bytearray = v.toLatin1();
    const char *p = bytearray.constData();
    while (p != 0) {
        int i = atoi(p);
        il.append(i);
        p = strchr(p, static_cast<int>(','));
        if (p != 0) p++;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::checkType(Type tt)
{

    if ((va != 0) && tt != t)
    {
        AVLogWarning << "AVCVariant:checkType: type mismatch (have "
                     << t << "(" << typeName(t) << ")"
                     << " <-> got "
                     << tt << "(" << typeName(tt) << "))";
        return false;
    }

    if (va == 0) {          // allocate type if not yet there
        ah = true;          // set "allocated here flag
        t  = tt;            // save the type
        switch (t) {
            case Int:        va = new int;           AVASSERT(va != 0); break;
            case UInt:       va = new uint;          AVASSERT(va != 0); break;
            case Double:     va = new double;        AVASSERT(va != 0); break;
            case Bool:       va = new bool;          AVASSERT(va != 0); break;
            case String:     va = new QString();     AVASSERT(va != 0); break;
            case Rect:       va = new QRect();       AVASSERT(va != 0); break;
            case Color:      va = new QColor();      AVASSERT(va != 0); break;
            case Size:       va = new QSize();       AVASSERT(va != 0); break;
            case Point:      va = new QPoint();      AVASSERT(va != 0); break;
            case StringList: va = new QStringList(); AVASSERT(va != 0); break;
            case IntList:    va = new AVIntList();   AVASSERT(va != 0); break;
            case Font:       va = new QFont();       AVASSERT(va != 0); break;
            case Directory:  va = new QString();     AVASSERT(va != 0); break;
            case File:       va = new QString();     AVASSERT(va != 0); break;
            case Undef:      break;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::write(QDataStream &s) const
{
    s << static_cast<qint8>(t);

    switch (t) {
        case Int:        s << asIntConst();           break;
        case UInt:       s << asUIntConst();          break;
        case Double:     s << asDoubleConst();        break;
        case Bool:       s << static_cast<qint8>(asBoolConst()); break;
        case String:     s << asStringConst();        break;
        case Rect:       s << asRectConst();          break;
        case Color:      s << asColorConst();         break;
        case Size:       s << asSizeConst();          break;
        case Point:      s << asPointConst();         break;
        case StringList: s << asStringListConst();    break;
        case IntList:    s << asIntListConst();       break;
        case Font:       s << asFontConst();          break;
        case Directory:  s << asDirectoryConst();     break;
        case File:       s << asFileConst();          break;
        default:         return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::read(QDataStream &s)
{
    delObject();
    quint8 c;
    s >> c; t = static_cast<Type>(c);
    switch (t) {
        case Int:        { s >> asInt(); }         break;
        case UInt:       { s >> asUInt(); }        break;
        case Double:     { s >> asDouble(); }      break;
        case Bool:       { quint8 v;
                           s >> v; set(static_cast<bool>(v)); } break;
        case String:     { s >> asString(); }      break;
        case Rect:       { s >> asRect(); }        break;
        case Color:      { s >> asColor(); }       break;
        case Size:       { s >> asSize(); }        break;
        case Point:      { s >> asPoint(); }       break;
        case StringList: { s >> asStringList(); }  break;
        case IntList:    { s >> asIntList(); }     break;
        case Font:       { s >> asFont(); }        break;
        case Directory:  { s >> asDirectory(); }   break;
        case File:       { s >> asFile(); }        break;
        default:         return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::write(QTextStream &s, bool skipType) const
{
    if (!skipType) s << static_cast<int>(type()) << ",";
    switch (type()) {
        case AVCVariant::Int:    s << asIntConst();
                                 break;
        case AVCVariant::UInt:   s << asUIntConst();
                                 break;
        case AVCVariant::Double: s.setRealNumberPrecision(12);
                                 s << asDoubleConst();
                                 break;
        case AVCVariant::Bool:   s << static_cast<int>(asBoolConst());
                                 break;
        case AVCVariant::String: s << asStringConst();
                                 break;
        case AVCVariant::Rect:   {
                                   const QRect& v = asRectConst();
                                   s << v.left()  << "," << v.top() << ","
                                     << v.right() << "," << v.bottom();
                                 }
                                 break;
        case AVCVariant::Color:  {
                                    const QColor& c = asColorConst();
                                    if (c.isValid()) s << c.name();
                                    else             s << QString();

                                 }
                                 break;
        case AVCVariant::Size:   s << asSizeConst().width() << ","
                                   << asSizeConst().height();
                                 break;
        case AVCVariant::Point:  s << asPointConst().x() << ","
                                   << asPointConst().y();
                                 break;
        case AVCVariant::StringList:
            {
                const QStringList& l = asStringListConst();
                s << l.count() << "\n";
                QStringList::ConstIterator iter = l.begin();
                QStringList::ConstIterator en = l.end();
                for (; iter != en; ++iter) {
                    s << *iter << "\n";
                }
            }
            break;
        case AVCVariant::IntList:
            {
                const AVIntList& l = asIntListConst();
                const uint cnt = l.count();
                for (uint i = 0; i < cnt; ++i) {
                    s << l[i];
                    if (i < cnt - 1) s << ",";
                }
            }
            break;
        case AVCVariant::Font:       s << asFontConst().toString(); break;
        case AVCVariant::Directory:  s << asDirectoryConst();
                                     break;
        case AVCVariant::File:       s << asFileConst();
                                     break;
        default:                 return false;
    }
    s << "\n";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::write(QTextStream &s, AVCVariant &def, AVCVariant &min,
                       AVCVariant &max,bool &defaultIsValid, QString &help,
                       const QString &reference)
{
    //if (!skipType)
    s << (static_cast<int>(type()) | 0x80) << ",";
    switch (type()) {
        case AVCVariant::Int:
        {
            if (reference.isNull()) s << asInt();
            else s << "@" << reference;
            s << "," << min.asInt() << "," << max.asInt() << ",";
            if(defaultIsValid)
                s << def.asInt();
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::UInt:
        {
            if (reference.isNull()) s << asUInt();
            else s << "@" << reference;
            s << "," << min.asUInt() << "," << max.asUInt() << ",";
            if(defaultIsValid)
                 s << def.asUInt();
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Double:
        {
            s.setRealNumberPrecision(12);
            if (reference.isNull()) s << asDouble();
            else s << "@" << reference;
            s << "," << min.asDouble() << "," << max.asDouble() << ",";
            if(defaultIsValid)
                s << def.asDouble();
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Bool:
        {
            if (reference.isNull()) s << static_cast<int>(asBool());
            else s << "@" << reference;
            s << ",";
            if(defaultIsValid)
                 s << def.asBool();
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::String:
        {
            if (reference.isNull()) s << "\"" << asString() << "\"" ;
            else s << "@" << reference;
            s << "," << max.asInt() << ",";
            if(defaultIsValid)
                s << "\"" << def.asString() << "\"";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Rect:
        {
            QRect v;
            if (reference.isNull())
            {
                v = asRect();
                s << v.left()  << "," << v.top() << ","
                  << v.right() << "," << v.bottom() << ",";
            } else s << "@" << reference << ",0,0,0,";
            v=min.asRect();
            s << v.left()  << "," << v.top() << ","
              << v.right() << "," << v.bottom() << ",";
            v=max.asRect();
            s << v.left()  << "," << v.top() << ","
              << v.right() << "," << v.bottom() << ",";
            if(defaultIsValid)
            {
                v=def.asRect();
                s << v.left()  << "," << v.top() << ","
                  << v.right() << "," << v.bottom();
            }
            else
                s <<",,,";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Color:
        {
            if (reference.isNull()) s << asColor().name();
            else s << "@" << reference;
            s << ",";
            if(defaultIsValid)
                s  << def.asColor().name();
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Size:
        {
            if (reference.isNull())
            {
                s << asSize().width() << "," << asSize().height() << ",";
            } else s << "@" << reference << ",0,";
            s << min.asSize().width() << "," << min.asSize().height() << ","
            << max.asSize().width() << "," << max.asSize().height() << ",";
            if(defaultIsValid)
                s << def.asSize().width() << "," << def.asSize().height();
            else
                s << ",";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Point:
        {
            if (reference.isNull())
            {
                s << asPoint().x() << "," << asPoint().y() << ",";
            } else s << "@" << reference << ",0,";
            s << min.asPoint().x() << "," << min.asPoint().y() << ","
            << max.asPoint().x() << "," << max.asPoint().y() << ",";
            if(defaultIsValid)
                s << def.asPoint().x() << "," << def.asPoint().y();
            else
                s << ",";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::StringList:
        {
            if (reference.isNull()) s << asStringList().count();
            else s << "@" << reference;
            s << "," << max.asInt() << ",helpText:";
            if(!help.isEmpty())
                s << help;
            s << "\n";
            QStringList::Iterator iter = asStringList().begin();
            QStringList::Iterator en = asStringList().end();
            for (; iter != en; ++iter) {
                s << "\"" << *iter << "\"\n";
            }
        }
        break;
        case AVCVariant::IntList:
        {
            if (reference.isNull())
            {
                s << min.asInt() << "," << max.asInt() << ",";
                uint cnt = asIntList().count();
                for (uint i = 0; i < cnt; i++) {
                    s << asIntList()[i];
                    if (i < cnt - 1) s << ",";
                }
            } else s << "@" << reference << ",0";

            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Font:
        {
            if (reference.isNull()) s << asFont().toString();
            else s << "@" << reference;
            s << ",";
            if(defaultIsValid)
                s << def.asFont().toString();
            else
                s << ",,,,,,,,,";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::Directory:
        {
            if (reference.isNull()) s << "\"" << asDirectory() << "\"";
            else s << "@" << reference;
            s << "," << max.asInt() << ",";
            if(defaultIsValid)
                s << "\"" << def.asDirectory() << "\"";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        case AVCVariant::File:
        {
            if (reference.isNull()) s << "\"" << asFile() << "\"" ;
            else s << "@" << reference;
            s << "," << max.asInt() << "," ;
            if(defaultIsValid)
                 s << "\"" << def.asFile() << "\"";
            s << ",helpText:";
            if(!help.isEmpty())
                s << help;
        }
        break;
        default:
            return false;
    }
    s << "\n";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVCVariant::toString() const
{
    QString s;
    QTextStream textstream(&s, QIODevice::WriteOnly);
    write(textstream,true);
    return s.left(s.length() - 1);       // skip trailing \n
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::read(QTextStream &s, AVCVariant &def, AVCVariant &min,
                      AVCVariant &max, bool &defaultIsValid, QString &help,
                      QString &reference)
{
    QString line;
    bool newfmt;
    if (!readLine(s, line)) return false;
    int pc = line.indexOf(",");
    if (pc < 0) return false;                // invalid line, need TYPE,VALUE
    int type = line.left(pc).toInt();
    if((type & 0x80)!=0)
    {
        newfmt=true;
        type &= ~0x80;
    }
    else
    {
        newfmt=false;
    }

    QString value = line.mid(pc + 1);

    // depending on the type, we construct the proper variant here

    AVCVariant::Type ttt = static_cast<AVCVariant::Type>(type);
    if(newfmt)
        switch (ttt) {
            case AVCVariant::Int:
            {
                QString v,vdef,vmin,vmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                vmin=value.section(",",1,1).trimmed();
                vmax=value.section(",",2,2).trimmed();
                vdef=value.section(",",3,3).trimmed();
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(v.toInt());
                }
                min.set(vmin.toInt());
                max.set(vmax.toInt());
                if(vdef!="")
                {
                    def.set(vdef.toInt());
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
                break;
            }
            case AVCVariant::UInt:
            {
                QString v,vdef,vmin,vmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                vmin=value.section(",",1,1).trimmed();
                vmax=value.section(",",2,2).trimmed();
                vdef=value.section(",",3,3).trimmed();
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(v.toUInt());
                }
                min.set(vmin.toUInt());
                max.set(vmax.toUInt());
                if(vdef!="")
                {
                    def.set(vdef.toUInt());
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Double:
            {
                QString v,vdef,vmin,vmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                vmin=value.section(",",1,1).trimmed();
                vmax=value.section(",",2,2).trimmed();
                vdef=value.section(",",3,3).trimmed();
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(v.toDouble());
                }
                min.set(vmin.toDouble());
                max.set(vmax.toDouble());
                if(vdef!="")
                {
                    def.set(vdef.toDouble());
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Bool:
            {
                QString v,vdef;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                vdef=value.section(",",1,1).trimmed();
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(static_cast<bool>(v.toInt()));
                }
                if(vdef!="")
                {
                    def.set(static_cast<bool>(vdef.toInt()));
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::String:
            {
                QString v,vdef,vmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                if(v.startsWith("\""))
                {
                    int startpos,endpos;
                    startpos=value.indexOf("\"")+1;
                    endpos=value.indexOf("\"",startpos);
                    v=value.mid(startpos,endpos-startpos).trimmed();
                    startpos=value.indexOf(",",endpos+1)+1;
                    endpos=value.indexOf(",",startpos);
                    vmax=value.mid(startpos,endpos-startpos).trimmed();
                    startpos=value.indexOf("\"",endpos+1)+1;
                    endpos=value.indexOf("\"",startpos);
                    vdef=value.mid(startpos,endpos-startpos).trimmed();
                }
                else
                {
                    vmax=value.section(",",1,1).trimmed();
                    vdef=value.section(",",2,2).trimmed();
                    if (vdef.length() > 0)
                    {
                        if (vdef.length() == 1     ||
                            !vdef.startsWith("\"") ||
                            !vdef.endsWith("\""))
                        {
                            AVLogger->Write(LOG_FATAL, "AVCVariant::read: "
                                    "Specify string default value enclosed in \" (%s)",
                                    qPrintable(vdef));
                        }
                        vdef = vdef.mid(1, vdef.length()-2);
                    }
                }
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(v);
                }
                max.set(vmax.toInt());
                if(vdef!="")
                {
                    def.set(vdef);
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Rect:
            {
                QString l,top,b,r,ldef,tdef,bdef,rdef;
                QString lmin,tmin,bmin,rmin,lmax,tmax,bmax,rmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                l=value.section(",",0,0).trimmed();
                top=value.section(",",1,1).trimmed();
                b=value.section(",",2,2).trimmed();
                r=value.section(",",3,3).trimmed();
                lmin=value.section(",",4,4).trimmed();
                tmin=value.section(",",5,5).trimmed();
                bmin=value.section(",",6,6).trimmed();
                rmin=value.section(",",7,7).trimmed();
                lmax=value.section(",",8,8).trimmed();
                tmax=value.section(",",9,9).trimmed();
                bmax=value.section(",",10,10).trimmed();
                rmax=value.section(",",11,11).trimmed();
                ldef=value.section(",",12,12).trimmed();
                tdef=value.section(",",13,13).trimmed();
                bdef=value.section(",",14,14).trimmed();
                rdef=value.section(",",15,15).trimmed();
                if (l.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = l.right(l.length()-1);
                } else
                {
                    set(QRect(QPoint(l.toInt(),top.toInt()),
                              QPoint(b.toInt(),r.toInt())));
                }
                min.set(QRect(QPoint(lmin.toInt(),tmin.toInt()),
                              QPoint(bmin.toInt(),rmin.toInt())));
                max.set(QRect(QPoint(lmax.toInt(),tmax.toInt()),
                              QPoint(bmax.toInt(),rmax.toInt())));
                if(ldef!="" && tdef!="" && bdef!="" && rdef!="")
                {
                    def.set(QRect(QPoint(ldef.toInt(),tdef.toInt()),
                                  QPoint(bdef.toInt(),rdef.toInt())));
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Color:
            {
                QString v,vdef;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                vdef=value.section(",",1,1).trimmed();
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(QColor(v));
                }
                if(vdef!="")
                {
                    def.set(QColor(vdef));
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Size:
            {
                QString w,h,wdef,hdef,wmin,hmin,wmax,hmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                w=value.section(",",0,0).trimmed();
                h=value.section(",",1,1).trimmed();
                wmin=value.section(",",2,2).trimmed();
                hmin=value.section(",",3,3).trimmed();
                wmax=value.section(",",4,4).trimmed();
                hmax=value.section(",",5,5).trimmed();
                wdef=value.section(",",6,6).trimmed();
                hdef=value.section(",",7,7).trimmed();
                if (w.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = w.right(w.length()-1);
                } else
                {
                    set(QSize(w.toInt(),h.toInt()));
                }
                min.set(QSize(wmin.toInt(),hmin.toInt()));
                max.set(QSize(wmax.toInt(),hmax.toInt()));
                if(wdef!="" && hdef!="")
                {
                    def.set(QSize(wdef.toInt(),hdef.toInt()));
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Point:
            {
                QString x,y,xdef,ydef,xmin,ymin,xmax,ymax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                x=value.section(",",0,0).trimmed();
                y=value.section(",",1,1).trimmed();
                xmin=value.section(",",2,2).trimmed();
                ymin=value.section(",",3,3).trimmed();
                xmax=value.section(",",4,4).trimmed();
                ymax=value.section(",",5,5).trimmed();
                xdef=value.section(",",6,6).trimmed();
                ydef=value.section(",",7,7).trimmed();
                if (x.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = x.right(x.length()-1);
                } else
                {
                    set(QPoint(x.toInt(),y.toInt()));
                }
                min.set(QPoint(xmin.toInt(),ymin.toInt()));
                max.set(QPoint(xmax.toInt(),ymax.toInt()));
                if(xdef!="" && ydef!="")
                {
                    def.set(QPoint(xdef.toInt(),ydef.toInt()));
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::StringList:
            {
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();

                if (value.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    AVASSERT(value.indexOf(",") != -1);
                    reference = value.mid(1, value.indexOf(",")-1);
                } else
                {
                    int n,vmax;
                    sscanf(qPrintable(value),"%d,%d",&n,&vmax);
                    max.set(vmax);

                    QStringList sl;
                    for (int i = 0; i < n; i++)
                    {
                        QString v;
                        if (!readLine(s, v)) break;
                        if(v.startsWith("\""))
                        {
                            int startpos,endpos;
                        startpos=v.indexOf("\"")+1;
                        endpos=v.indexOf("\"",startpos);
                        v=v.mid(startpos,endpos-startpos).trimmed();
                        }
                        sl.append(v);
                    }
                    set(sl);
                }
                break;
            }
            case AVCVariant::IntList:
            {
                if (!checkType(IntList))
                    AVLogger->Write(LOG_ERROR, "AVCVariant:read: "
                                    "failed in line (" + line + ")");

                int vmin,vmax;
                QString minmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                pc=value.indexOf(",",value.indexOf(",")+1);
                minmax=value.left(pc);
                if (minmax.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    AVASSERT(minmax.indexOf(",") != -1);
                    reference = minmax.mid(1, value.indexOf(",")-1);
                } else
                {
                    sscanf(qPrintable(minmax),"%d,%d",&vmin,&vmax);
                    min.set(vmin);
                    max.set(vmax);
                    value=value.mid(pc+1);
                    sets(value);
                }
                break;
            }
            case AVCVariant::Font:
            {
                if (!checkType(Font))
                    AVLogger->Write(LOG_ERROR, "AVCVariant:read: "
                                    "failed in line (" + line + ")");

                QString v,vdef;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,9).trimmed();
                vdef=value.section(",",10,19).trimmed();
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    sets(v);
                }
                if(vdef.section(",",0,0)!="")
                {
                    def.checkType(Font);
                    def.sets(vdef);
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::Directory:
            {
                QString v,vdef,vmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                if(v.startsWith("\""))
                {
                    int startpos,endpos;
                    startpos=value.indexOf("\"")+1;
                    endpos=value.indexOf("\"",startpos);
                    v=value.mid(startpos,endpos-startpos).trimmed();
                    startpos=value.indexOf(",",endpos+1)+1;
                    endpos=value.indexOf(",",startpos);
                    vmax=value.mid(startpos,endpos-startpos).trimmed();
                    startpos=value.indexOf("\"",endpos+1)+1;
                    endpos=value.indexOf("\"",startpos);
                    vdef=value.mid(startpos,endpos-startpos).trimmed();
                }
                else
                {
                    vmax=value.section(",",1,1).trimmed();
                    vdef=value.section(",",2,2).trimmed();
                }
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(v,Directory);
                }
                max.set(vmax.toInt());
                if(vdef!="")
                {
                    def.set(vdef,Directory);
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            case AVCVariant::File:
            {
                QString v,vdef,vmax;
                help=value.section(",helpText:",1).trimmed();
                value=value.section(",helpText:",0,0).trimmed();
                v=value.section(",",0,0).trimmed();
                if(v.startsWith("\""))
                {
                    int startpos,endpos;
                    startpos=value.indexOf("\"")+1;
                    endpos=value.indexOf("\"",startpos);
                    v=value.mid(startpos,endpos-startpos).trimmed();
                    startpos=value.indexOf(",",endpos+1)+1;
                    endpos=value.indexOf(",",startpos);
                    vmax=value.mid(startpos,endpos-startpos).trimmed();
                    startpos=value.indexOf("\"",endpos+1)+1;
                    endpos=value.indexOf("\"",startpos);
                    vdef=value.mid(startpos,endpos-startpos).trimmed();
                }
                else
                {
                    vmax=value.section(",",1,1).trimmed();
                    vdef=value.section(",",2,2).trimmed();
                }
                if (v.startsWith("@"))
                {
                    AVASSERT(AVConfigBase::process_uses_avconfig2);
                    reference = v.right(v.length()-1);
                } else
                {
                    set(v,File);
                }
                max.set(vmax.toInt());
                if(vdef!="")
                {
                    def.set(vdef,File);
                    defaultIsValid=true;
                }
                else
                    defaultIsValid=false;
            }
            break;
            default:
                qWarning("AVEnvironment::read: unknown config type");
                return false;
        }
    else
        switch (ttt) {
            case AVCVariant::Int:
            {
                set(value.toInt());
                min.set(-2147483647);
                max.set(2147483647);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::UInt:
            {
                set(value.toUInt());
                min.set(0U);
                max.set(2147483647U);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Double:
            {
                set(value.toDouble());
                min.set(-10.0E38);
                max.set(10.0E38);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Bool:
            {
                set(static_cast<bool>(value.toInt()));
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::String:
            {
                set(value);
                max.set(AVConfigEntry::STRING_DEFAULT_MAX_LENGTH);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Rect:
            {
                int l, ttt, b, r;
                sscanf(qPrintable(value), "%d,%d,%d,%d", &l, &ttt, &b, &r);
                set(QRect(QPoint(l,ttt), QPoint(b,r)));
                min.set(QRect(QPoint(-65535,-65535),QPoint(-65535,-65535)));
                max.set(QRect(QPoint(65535,65535),QPoint(65535,65535)));
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Color:
            {
                set(QColor(value));
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Size:
            {
                int w, h;
                sscanf(qPrintable(value), "%d,%d", &w, &h);
                set(QSize(w, h));
                min.set(QSize(-65535,-65535));
                max.set(QSize(65535,65535));
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Point:
            {
                int x, y;
                QByteArray bytearray = value.toLatin1();
                const char *vvv = bytearray.data();
                sscanf(vvv, "%d,%d", &x, &y);
                set(QPoint(x, y));
                min.set(QPoint(-65535,-65535));
                max.set(QPoint(65535,65535));
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::StringList:
            {
                int n; QString v; QStringList sl;
                n = value.toInt();
                for (int i = 0; i < n; i++)
                {
                    if (!readLine(s, v)) break;
                    sl.append(v);
                }
                set(sl);
                max.set(AVConfigEntry::STRING_DEFAULT_MAX_LENGTH);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::IntList:
            {
                if (!checkType(IntList))
                    AVLogger->Write(LOG_ERROR, "AVCVariant:read: "
                                    "failed in line (" + line + ")");

                sets(value);
                min.set(-AVConfigEntry::INT_DEFAULT_MAXMIN);
                max.set(AVConfigEntry::INT_DEFAULT_MAXMIN);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Font:
            {
                if (!checkType(Font))
                    AVLogger->Write(LOG_ERROR, "AVCVariant:read: "
                                    "failed in line (" + line + ")");
                sets(value);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::Directory:
            {
                set(value,Directory);
                max.set(AVConfigEntry::STRING_DEFAULT_MAX_LENGTH);
                defaultIsValid=false;
                help="";
            }
            break;
            case AVCVariant::File:
            {
                set(value,File);
                max.set(AVConfigEntry::STRING_DEFAULT_MAX_LENGTH);
                defaultIsValid=false;
                help="";
            }
            break;
            default:
                qWarning("AVEnvironment::read: unknown config type");
                return false;
        }

    return checkType(ttt);
}

///////////////////////////////////////////////////////////////////////////////

bool AVCVariant::readLine(QTextStream &s, QString &l)
{
    do {
        l = s.readLine();
        if (l.isNull()) return false;    // at eof
        if (l.isEmpty()) continue;       // empty line
    } while(l.startsWith("#"));          // comment lines
    return true;
}

///////////////////////////////////////////////////////////////////////////////

const char *AVCVariant::typeName(Type tt)
{
    switch (tt)
    {
        case Undef:      return "Undefined";
        case Int:        return "Int";
        case UInt:       return "UInt";
        case Double:     return "Double";
        case Bool:       return "Bool";
        case String:     return "String";
        case Rect:       return "Rect";
        case Color:      return "Color";
        case Size:       return "Size";
        case Point:      return "Point";
        case StringList: return "StringList";
        case IntList:    return "IntList";
        case Font:       return "Font";
        case Directory:  return "Directory";
        case File:       return "File";

        // no default so any new type can be detected by compiler warning
    }

    AVASSERT(false);
    return 0;
}

// end of file
