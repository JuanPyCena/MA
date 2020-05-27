///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
 */

// Qt includes
#include <QColor>
#include <QRect>

#include "avconfig2adapters.h"

#include "avconfig2.h"
#include "avfromtostring.h"
#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////

AVConfig2XmlAdapter::AVConfig2XmlAdapter(const AVConfig2Container& container) :
    m_container(container)
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2XmlAdapter::replaceLoadedParameters(const QString& text) const
{
    QString result = text;
    int last_pos = 0;
    for (;;)
    {
        static const QString start_pattern("{config:");
        static int start_len = start_pattern.length();

        int start_pos = result.indexOf(start_pattern, last_pos);
        if (start_pos == -1) break;
        int end_pos = result.indexOf('}', start_pos + start_len);
        if (end_pos == -1) break;
        QString name = result.mid(start_pos + start_len, end_pos - start_pos - start_len);
        const AVConfig2ImporterClient::LoadedParameter *param = m_container.getLoadedParameter(name);
        if (param == 0)
        {
            AVLogWarning << "parameter not found: " << name;
            last_pos = end_pos + 1;
            continue;
        }
        QString value = getCanonicalString(param->getValue(), param->getMetadata());
        if (value.isNull())
        {
            last_pos = end_pos + 1;
            continue;
        }
        AVLogDebug << "AVConfig2XmlAdapter::replaceLoadedParameters: replace " << name << " with " << value;
        result.replace(start_pos, end_pos - start_pos + 1, value);
        // If the replacement contains nested parameters, these will also be replaced.
        // Please note that this may lead to an infinite loop.
        last_pos = start_pos;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2XmlAdapter::getCanonicalString(const QString& value, const AVConfig2Metadata& metadata)
{
    QString type = metadata.m_type;
    QString result;
    if (type == "QString")
    {
        AVFromString(value, result);
        if (result.isNull()) result = "null"; // this is the standard representation for rules
    } else if (type == "int")
    {
        int int_value;
        if (!AVFromString(value, int_value))
        {
            AVLogError << "AVConfig2XmlAdapter::getCanonicalString: parameter "
                       << metadata.m_name << " cannot be parsed: " << value;
            return QString::null;
        }
        result = QString::number(int_value);
    } else if (type == "uint")
    {
        uint uint_value;
        if (!AVFromString(value, uint_value))
        {
            AVLogError << "AVConfig2XmlAdapter::getCanonicalString: parameter "
                       << metadata.m_name << " cannot be parsed: " << value;
            return QString::null;
        }
        result = QString::number(uint_value);
    } else if (type == "bool")
    {
        bool bool_value;
        if (!AVFromString(value, bool_value))
        {
            AVLogError << "AVConfig2XmlAdapter::getCanonicalString: parameter "
                       << metadata.m_name << " cannot be parsed: " << value;
            return QString::null;
        }
        result = (bool_value ? "true" : "false");
    } else
    {
        AVLogError << "AVConfig2XmlAdapter::getCanonicalString: parameter "
                   << metadata.m_name << " type " << type << " not supported";
        return QString::null;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2QVariantAdapter::hasParamValue(const QString& param_name)
{
    return (AVConfig2Global::singleton().getLoadedParameter(param_name) != 0);
}

///////////////////////////////////////////////////////////////////////////////

QVariant AVConfig2QVariantAdapter::getParamValue(const QString& param_name)
{
    const AVConfig2ImporterClient::LoadedParameter* param = AVConfig2Global::singleton().getLoadedParameter(param_name);
    if(!param)
        AVLogFatal << "AVConfig2QVariantAdapter: Param not available: " << param_name;

    QString  type    = param->getMetadata().m_type;
    QString  value   = param->getValue();
    QVariant result;
    bool     success = false;

    if(type == AVConfig2Types::getTypeName<bool>())
        success = convertParam<bool>(value, result);
    else if(type == AVConfig2Types::getTypeName<int>())
        success = convertParam<int>(value, result);
    else if(type == AVConfig2Types::getTypeName<uint>())
        success = convertParam<uint>(value, result);
    else if(type == AVConfig2Types::getTypeName<float>())
        success = convertParam<float>(value, result);
    else if(type == AVConfig2Types::getTypeName<double>())
        success = convertParam<double>(value, result);
    else if(type == AVConfig2Types::getTypeName<int>())
        success = convertParam<int>(value, result);
    else if(type == AVConfig2Types::getTypeName<QString>())
        success = convertParam<QString>(value, result);
    else if(type == AVConfig2Types::getTypeName<QStringList>())
        success = convertParam<QStringList>(value, result);
    else if(type == AVConfig2Types::getTypeName<QColor>())
        success = convertParam<QColor>(value, result);
    else if(type == AVConfig2Types::getTypeName<QPoint>())
        success = convertParam<QPoint>(value, result);
    else if(type == AVConfig2Types::getTypeName<QRect>())
        success = convertParam<QRect>(value, result);
    else if(type == AVConfig2Types::getTypeName<AVPath>())
    {
        AVPath path;
        success = AVFromString(value, path);
        if (success)
        {
            result = path.expandPath();
        }
    }
    else if(type == AVConfig2Types::getTypeName<QList<QMap<QString, QString> > >())
    {
        QList<QMap<QString, QString> > typed_result;
        success = AVFromString(value, typed_result);

        if(success)
        {
            QList<QVariant> result_list;

            QList<QMap<QString, QString> >::const_iterator list_entry = typed_result.begin();
            for(; list_entry != typed_result.end(); ++list_entry)
            {
                QMap<QString, QVariant> current_map;
                QMap<QString, QString>:: const_iterator map_entry = list_entry->begin();
                for(; map_entry != list_entry->end(); ++map_entry)
                {
                    current_map.insert(map_entry.key(), QVariant(map_entry.value()));
                }
                result_list.append(current_map);
            }

            result = QVariant(result_list);
        }
    }
    else if(type == AVConfig2Types::getTypeName<QMap<QString, QStringList> >())
    {
        QMap<QString, QStringList> typed_result;
        success = AVFromString(value, typed_result);

        if(success)
        {
            QMap<QString, QVariant> result_map;

            QMap<QString, QStringList>::const_iterator map_iterator = typed_result.begin();
            for(; map_iterator != typed_result.end(); ++map_iterator)
            {
                result_map.insert(map_iterator.key(), map_iterator.value());
            }

            result = QVariant(result_map);
        }
    }
    else
    {
        AVLogFatal << "AVConfig2QVariantAdapter: Type not found: " << type;
    }
    if(!success)
        AVLogFatal << "AVConfig2QVariantAdapter: Could not parse " << type << " value: " << value;

    return result;
}

// End of file
