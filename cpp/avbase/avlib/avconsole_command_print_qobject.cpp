//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVConsoleCommandPrintQObject implementation
 */

// local includes
#include "avconsole_command_print_qobject.h"
#include "avconsolepropertyobject.h"

// Qt includes

//-----------------------------------------------------------------------------

const QChar AVConsoleCommandPrintQObject::FilterArgument::HIERARCHY_SEPARATOR  = QChar('.');

const QChar AVConsoleCommandPrintQObject::QObjectPropertyFilter::FILTER_ARGUMENT_SEPARATOR       = QChar(',');
const QChar AVConsoleCommandPrintQObject::QObjectPropertyFilter::FILTER_PROPERTY_VALUE_SEPARATOR = QChar('=');

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::AVConsoleCommandPrintQObject(const QString &name, const QString &help_text) :
    AVConsoleCommand(name, help_text)
{
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::~AVConsoleCommandPrintQObject()
{
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::FilterArgument::FilterArgument()
{
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::FilterArgument::FilterArgument(const QString &property, const QString &value) :
    QPair<QString, QString>(property, value), m_property_hierarchy(property.split(HIERARCHY_SEPARATOR))
{
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::FilterArgument::~FilterArgument()
{
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::FilterArgument::FilterArgument(const AVConsoleCommandPrintQObject::FilterArgument &other) :
    QPair<QString, QString>(other)
{
    m_property_hierarchy = other.propertyFilterHierarchy();
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::FilterArgument &AVConsoleCommandPrintQObject::FilterArgument::operator=(const AVConsoleCommandPrintQObject::FilterArgument &other)
{
    QPair<QString, QString>::operator=(other);
    m_property_hierarchy = other.propertyFilterHierarchy();
    return *this;
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::QObjectPropertyFilter::QObjectPropertyFilter() :
    QList<FilterArgument>()
{
}

//-----------------------------------------------------------------------------

AVConsoleCommandPrintQObject::QObjectPropertyFilter::~QObjectPropertyFilter()
{
}

//-----------------------------------------------------------------------------

bool AVConsoleCommandPrintQObject::QObjectPropertyFilter::fromString(const QString &filter_string)
{
    QStringList raw_filter_arguments = filter_string.split(FILTER_ARGUMENT_SEPARATOR);
    for(const QString& raw_filter_arg : raw_filter_arguments)
    {
        QStringList filter_key_value_pair = raw_filter_arg.split(FILTER_PROPERTY_VALUE_SEPARATOR);
        QString property_filter_arg = filter_key_value_pair[0];
        QString value_filter_arg;
        if(filter_key_value_pair.count() == 2)
            value_filter_arg = filter_key_value_pair[1];
        else if(raw_filter_arg.contains(FILTER_PROPERTY_VALUE_SEPARATOR))
            value_filter_arg = EmptyQString;  // the value filter is specified - and empty
        else
            value_filter_arg = QString::null; // the value filter is not specified (we use this for auto-completion)

        append(FilterArgument(property_filter_arg, value_filter_arg));
    }
    return true;
}

//-----------------------------------------------------------------------------

QString AVConsoleCommandPrintQObject::QObjectPropertyFilter::toString() const
{
    QStringList result;
    for(const FilterArgument& filter_arg : *this)
    {
        QString filter_str = filter_arg.propertyFilter();
        if(!filter_arg.valueFilter().isEmpty())
            filter_str += QObjectPropertyFilter::FILTER_PROPERTY_VALUE_SEPARATOR + filter_arg.valueFilter();
        result.append(filter_str);
    }
    return result.join(FILTER_ARGUMENT_SEPARATOR);
}

//-----------------------------------------------------------------------------

bool AVConsoleCommandPrintQObject::printProperty(const QString& hierarchical_property_id, const QObject* root_object)
{
    QStringList property_hierarchy = hierarchical_property_id.split(FilterArgument::HIERARCHY_SEPARATOR);
    QString property_id = property_hierarchy.last();

    // parent_object -> the object containing the property we are looking for
    QVariant parent_property_var = findParentPropertyVariable(property_hierarchy, root_object);
    if(!parent_property_var.isValid())
    {
        error(QStringLiteral("Property does not exist: '%1'").arg(hierarchical_property_id));
        return false;
    }
    bool ok = false;
    QString response = propertyVariableToString(parent_property_var, property_id, &ok);
    if(!ok)
        error(response);
    else
        print(response);
    return ok;
}

//-----------------------------------------------------------------------------

QVariant AVConsoleCommandPrintQObject::findParentPropertyVariable(const QStringList& property_id_hierarchy, const QObject* root_object) const
{
    const QObject* curr_object = root_object;
    QVariantList curr_object_list;
    QVariantMap curr_object_map;

    for(int i = 0; i < property_id_hierarchy.count() - 1; ++i)
    {
        QVariant property_var;
        const QString& curr_property_id = property_id_hierarchy[i];

        // find matching child of current variable
        if (curr_object != nullptr)
            property_var = curr_object->property(curr_property_id.toLatin1().constData());
        else if (!curr_object_list.isEmpty())
        {
            bool ok = false;
            int list_idx = curr_property_id.toInt(&ok);
            if (!ok || list_idx < 0 || list_idx >= curr_object_list.size())
                return QVariant();
            property_var = curr_object_list.at(list_idx);
        }
        else if (!curr_object_map.isEmpty())
        {
            if (!curr_object_map.contains(curr_property_id))
                return QVariant();
            property_var = curr_object_map[curr_property_id];
        }

        // we have reached an invalid property: abort
        if(!property_var.isValid())
            return QVariant();

        // store current variable for next iteration
        if (property_var.canConvert<QObject*>())
        {
            curr_object = property_var.value<QObject*>();
            if(curr_object == nullptr) // the pointer can still be null even if the property was valid
                return QVariant();

            curr_object_list.clear();
            curr_object_map.clear();
        }
        else if (property_var.canConvert<QVariantList>())
        {
            curr_object = nullptr;
            curr_object_list = property_var.value<QVariantList>();
            curr_object_map.clear();
        }
        else if (property_var.canConvert<QVariantMap>())
        {
            curr_object = nullptr;
            curr_object_list.clear();
            curr_object_map = property_var.toMap();
        }
        else
            return QVariant();
    }

    // final step: return requested property
    if (curr_object != nullptr)
        return QVariant::fromValue<QObject*>(const_cast<QObject*>(curr_object));
    else if (!curr_object_list.isEmpty())
        return curr_object_list;
    else if (!curr_object_map.isEmpty())
        return curr_object_map;
    return QVariant();
}

//-----------------------------------------------------------------------------

QString AVConsoleCommandPrintQObject::propertyVariableToString(const QVariant& property_var, const QString& property_id, bool *ok) const
{
    AVASSERT(property_var.isValid());

    QVariant value;
    QString response;

    // step 1: fetch property

    // property of type QVariantList: value = list[property_id]
    if (property_var.canConvert<QVariantList>())
    {
        QVariantList curr_list = property_var.value<QVariantList>();
        bool to_int_ok = false;
        int list_idx = property_id.toInt(&to_int_ok);
        if (!to_int_ok || list_idx < 0 || list_idx >= curr_list.size())
        {
            if(ok != 0) *ok = false;
            return QStringLiteral("Property '%1' not registered in list").arg(property_id);
        }
        value = curr_list.at(list_idx);
    }
    // property of type QVariantMap: value = map[property_id]
    else if (property_var.canConvert<QVariantMap>())
    {
        QVariantMap curr_map = property_var.toMap();
        if (!curr_map.contains(property_id))
        {
            if(ok != 0) *ok = false;
            return QStringLiteral("Property '%1' not registered in map").arg(property_id);
        }
        value = curr_map[property_id];
    }
    // property of type QObject*: value = object.property_id
    else if (property_var.canConvert<QObject*>())
    {
        QObject* object = property_var.value<QObject*>();
        AVASSERT(object != 0);

        const QMetaObject *meta_object = object->metaObject();
        AVASSERT(meta_object != 0); // according to Qt this can never been null

        int property_index = meta_object->indexOfProperty(property_id.toLatin1().constData());
        if(property_index == -1)
        {
            if(ok != 0) *ok = false;
            return QStringLiteral("Property '%1' not registered in class '%2'").arg(property_id).arg(meta_object->className());
        }
        QMetaProperty meta_property = meta_object->property(property_index);
        value = meta_property.read(object);

        // convert enum indices to enum keys
        if(meta_property.isEnumType())
        {
            QMetaEnum meta_enum = meta_property.enumerator();
            value = meta_enum.valueToKey(value.toInt());
        }
    }
    // generic property
    else
    {
        value = property_var;
    }

    // step 2: generate string representation of property

    // basic types (e.g. QString)
    if (response.isEmpty())
    {
        // try simple toString operation ...
        response = AVToString(value);
    }
    // if response is still empty this property might be a special type that has AVStringifiable implemented
    if(response.isEmpty() && value.canConvert<QObject*>())
    {
        QObject* value_qobject = value.value<QObject*>();
        if (value_qobject == nullptr)
            value = QVariant();
        else
        {
            AVConsolePropertyObject* printable = dynamic_cast<AVConsolePropertyObject*>(value_qobject);
            if(printable != nullptr)
                response = printable->toString();
        }
    }
    // unable to convert to string
    if(response.isEmpty() && value.type() == QVariant::UserType)
    {
        if(ok != 0) *ok = false;
        return QStringLiteral("Cannot convert property '%1' to string").arg(property_id);
    }
    if(ok != 0) *ok = true;
    return response.isEmpty() ? QStringLiteral("{untouched}") : response;
}

//-----------------------------------------------------------------------------

void AVConsoleCommandPrintQObject::completePropertyIdentifiers(
        QStringList& completions, const QString& curr_property_arg, const QObject *root_object) const
{
    QSet<QString> completions_set;
    completePropertyIdentifiers(completions_set, curr_property_arg, root_object);
    completions += completions_set.values();
    completions.sort();
}

//-----------------------------------------------------------------------------

void AVConsoleCommandPrintQObject::completePropertyIdentifiers(
        QSet<QString>& completions, const QString& curr_property_arg, const QObject *root_object) const
{
    QStringList property_hierarchy = curr_property_arg.split(FilterArgument::HIERARCHY_SEPARATOR);
    QVariant parent_property_var = findParentPropertyVariable(property_hierarchy, root_object);
    if(!parent_property_var.isValid())
        return;

    // calculate prefix string: hierarchy without last incomplete property name
    int separator_pos = curr_property_arg.lastIndexOf(FilterArgument::HIERARCHY_SEPARATOR);
    QString property_prefix = (separator_pos != -1) ? curr_property_arg.mid(0, separator_pos + 1) : QString();

    // auto completion for QObject
    if (parent_property_var.canConvert<QObject*>())
    {
        QObject *curr_object = parent_property_var.value<QObject*>();
        if (curr_object != nullptr)
        {
            const QMetaObject *meta_object = curr_object->metaObject();
            for(const QString& property_id : getPropertyIdentifiers(meta_object))
            {
                completions.insert(property_prefix + property_id);
            }
        }
    }
    // auto completion for QVariantList
    else if (parent_property_var.canConvert<QVariantList>())
    {
        QVariantList curr_list = parent_property_var.value<QVariantList>();
        for(int idx = 0; idx < curr_list.size(); ++idx)
        {
            completions.insert(property_prefix + QString::number(idx));
        }
    }
    // auto completion for QVariantMap
    else if (parent_property_var.canConvert<QVariantMap>())
    {
        QVariantMap curr_map = parent_property_var.toMap();
        for (const QString &key : curr_map.keys())
        {
            completions.insert(property_prefix + key);
        }
    }
}

//-----------------------------------------------------------------------------

QString AVConsoleCommandPrintQObject::propertyHierarchyToString(const QStringList& property_hierarchy, const QObject *root_object) const
{
    QVariant parent_property_var = findParentPropertyVariable(property_hierarchy, root_object);
    if(!parent_property_var.isValid())
        return QString::null;

    bool ok = false;
    QString property_id = property_hierarchy.last();
    QString property_value = propertyVariableToString(parent_property_var, property_id, &ok);
    if(ok && !property_value.isEmpty())
        return property_value;
    return QString::null;
}

//-----------------------------------------------------------------------------

QStringList AVConsoleCommandPrintQObject::getPropertyIdentifiers(const QMetaObject* meta_object) const
{
    QStringList result;
    const QMetaObject* current_meta_object = meta_object;
    while(current_meta_object != 0)
    {
        for(int i = current_meta_object->propertyOffset(); i < current_meta_object->propertyCount(); ++i)
        {
            QMetaProperty meta_property = current_meta_object->property(i);
            if(meta_property.isReadable())
                result << meta_property.name();
        }
        current_meta_object = current_meta_object->superClass();
    }
    return result;
}

//-----------------------------------------------------------------------------

QList<QPointer<QObject>> AVConsoleCommandPrintQObject::applyFilter(const QString &filter_string, const QList<QPointer<QObject>> &candidates) const
{
    QList<QPointer<QObject>> matching_objects;

    QObjectPropertyFilter filter;
    filter.fromString(filter_string);

    for(const QPointer<QObject> &curr_candidate : candidates)
    {
        if(curr_candidate.isNull())
            continue;
        bool filter_match = true;
        for(const FilterArgument& curr_filter_criteria : filter)
        {
            QString property_value = propertyHierarchyToString(curr_filter_criteria.propertyFilterHierarchy(), curr_candidate.data());
            QString value_filter   = curr_filter_criteria.valueFilter();
            if(property_value != value_filter)
            {
                filter_match = false;
                break;
            }
        }
        if(filter_match)
            matching_objects.append(curr_candidate);
    }
    return matching_objects;
}

//-----------------------------------------------------------------------------

void AVConsoleCommandPrintQObject::completeFilter(QStringList &completions, const QString &filter_string, const QList<QPointer<QObject>> &candidates) const
{
    QObjectPropertyFilter filter;
    filter.fromString(filter_string);
    FilterArgument last_filter_arg = filter.last();

    QSet<QString> current_completions;
    for(const QPointer<QObject> &curr_candidate : candidates)
    {
        AVASSERT(!curr_candidate.isNull());
        // we are looking for the VALUE argument of the filter:
        if(last_filter_arg.valueFilter().isNull())
        {
            completePropertyIdentifiers(current_completions, last_filter_arg.propertyFilter(), curr_candidate.data());
        }
        else
        {
            QString property_value = propertyHierarchyToString(last_filter_arg.propertyFilterHierarchy(), curr_candidate.data());
            if(!property_value.isEmpty())
                current_completions.insert(property_value);
        }
    }

    filter.removeLast();
    QString base = filter.toString();
    if(!base.isEmpty())
        base += QObjectPropertyFilter::FILTER_ARGUMENT_SEPARATOR;
    if(!last_filter_arg.valueFilter().isNull())
        base += last_filter_arg.propertyFilter() + QObjectPropertyFilter::FILTER_PROPERTY_VALUE_SEPARATOR;
    for(const QString& completion : current_completions)
    {
        completions.append(base + completion);
    }
    completions.sort();
}

// End of file
