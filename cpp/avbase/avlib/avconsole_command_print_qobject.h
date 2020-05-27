//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVConsoleCommandPrintQObject header
 */

#ifndef AVCONSOLECOMMANDPRINTQOBJECT_INCLUDED
#define AVCONSOLECOMMANDPRINTQOBJECT_INCLUDED

// Qt includes
#include <QList>
#include <QPointer>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QStringBuilder>
#include <QStringList>
#include <QVariant>

// local includes
#include "avlib_export.h"
#include "avconsole_command.h"

// Forward declarations

//-----------------------------------------------------------------------------
/**
 * The class AVConsoleCommandPrintQObject provides a base class for printing properties of a given QObject.
 *
 * The method doExecute needs to be implemented by the child class and hand-over
 * a QObject and a respective hierarchical property identifier to the protected method printProperty
 * to print the value of the property to the console connection.
 * The class also provides support for the implementation of the auto-completion: Therefore
 * implement doComplete and hand-over the qobject and the current hierarchical property identifier
 * to the protected method completePropertyIdentifiers
 *
 * Hierachical Property Identifier:
 * String separated by '.' to provide
 *
 * AVConsolePropertyObject:
 * If the QProperty is of type AVConsolePropertyObject, the property is casted and the result of
 * AVConsolePropertyObject::toString is printed to the console connection.
 *
 * QMetaEnums:
 * If the QProperty is of type QMetaEnum, the string representation of the enum-value
 * is printed to the console connection.
 *
*/
class AVLIB_EXPORT AVConsoleCommandPrintQObject : public AVConsoleCommand
{

public:
    AVConsoleCommandPrintQObject(const QString& name,
                                 const QString& help_text);
    ~AVConsoleCommandPrintQObject() override;

protected:
    //! prints the property of the object to the avconsole \sa printProperty
    bool printProperty(const QString &property_id, const QObject *obj);

    //! Loops over all properties of the handed over root_object and adds those to completions for which READ
    //! method is specified for the property - we use a qset to avoid duplicates already at the time of insert
    void completePropertyIdentifiers(QSet<QString> &completions, const QString &curr_property_arg, const QObject *root_object) const;

    //! this method behaves the same way than the previous - the values of the resulting qset are assigned to completions
    //! and the list is sorted afterwards
    void completePropertyIdentifiers(QStringList &completions, const QString& curr_property_arg, const QObject *root_object) const;

    //! returns the string representation of the given property variable
    //! if the property is a QMetaEnum, the string representation of the enum value is returned
    //! if the QVariant of the property does not provide a 'toString'-method (result will be empty), it will be checked
    //! if the QVariant can be converted to a QObject and casted to a AVConsolePropertyObject
    QString propertyVariableToString(const QVariant& property_var, const QString &property_id, bool *ok) const;

    //! expands the given hierarchical property string and based on the root_object returns the Loops over all properties of the handed over root_object and adds those to completions for which READ
    QString propertyHierarchyToString(const QStringList &property_hierarchy, const QObject *root_object) const;

    //! Helper class containing the value and property filter
    class FilterArgument : public QPair<QString, QString>
    {
    public:
        static const QChar HIERARCHY_SEPARATOR;
    public:
        FilterArgument();
        explicit FilterArgument(const QString& property_filter, const QString &value_filter);
        virtual ~FilterArgument();
        FilterArgument(const FilterArgument &other);
        FilterArgument &operator=(const FilterArgument &other);

        const QString& propertyFilter() const { return first; }
        QStringList propertyFilterHierarchy() const { return m_property_hierarchy; }
        const QString& valueFilter() const { return second; }
    private:
        //! cached QObject property hierarchy
        QStringList m_property_hierarchy;
    };
    //! Helper class for handling the filters to read/write from/to string
    //! The basic format for a filter is {property-id}={value},{property-id}={value}
    class QObjectPropertyFilter : public QList<FilterArgument>
    {
    public:
        static const QChar FILTER_ARGUMENT_SEPARATOR;
        static const QChar FILTER_PROPERTY_VALUE_SEPARATOR;

    public:
        QObjectPropertyFilter();
        virtual ~QObjectPropertyFilter();

        // converts a string to a list of FilterAgrguments
        bool fromString(const QString& filter_string);
        // conterts this filter to a filter string
        QString toString() const;
    };

    //! converts the given string into a QObjectPropertyFilter and adds all
    //! If the user is looking for the property argument all properties available candidates are added to
    //! completions, otherwise all values available for the properties are added to completions.
    void completeFilter(QStringList &completions, const QString &filter, const QList<QPointer<QObject>> &candidates) const;

    //! applies a given filter string (\sa parseFilterFromString) on the given candidates list
    //! \return list of objects that match the filter
    QList<QPointer<QObject>> applyFilter(const QString &filter_string, const QList<QPointer<QObject>> &candidates) const;

private:
    // ----------------------------------------------------
    // methods used for printing the property

    //! returns a QVariant containing a property by looping over property_id_hierarchy. result is invalid if property is not found.
    //! supported property types are: QObject*, QVariantList and QVariantMap
    QVariant findParentPropertyVariable(const QStringList &property_id_hierarchy, const QObject *root_object) const;

    // ----------------------------------------------------
    // methods used for auto-completion

    //! returns a list of names of all registered properties (including the super-classes) of the given
    //! meta object
    QStringList getPropertyIdentifiers(const QMetaObject *meta_object) const;

    Q_DISABLE_COPY(AVConsoleCommandPrintQObject)
};

#endif // AVCONSOLECOMMANDPRINTQOBJECT_INCLUDED

// End of file
