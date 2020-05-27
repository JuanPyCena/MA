///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \brief    TODO
*/

#ifndef AVEXPRESSIONMETADATA_H
#define AVEXPRESSIONMETADATA_H

// Qt includes
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

// AviBit common includes
#include "avlib_export.h"
#include "avexplicitsingleton.h"

class AVGenericComputableMetadata;

///////////////////////////////////////////////////////////////////////////////

//! This class provides a reflection API for AVComputables.

class AVLIB_EXPORT AVComputableMetadata
{
public:
    //! Constructor
    explicit AVComputableMetadata(const QString& name) : m_name(name) { }
    //! Destructor
    virtual ~AVComputableMetadata() { }
    //! Answer the name of the AVComputable class that this instance describes
    QString name() const { return m_name; }
    //! Answer the vocabulary that valueOfLHS understands. These strings do not contain a ".".
    //! E.g. if valueOfLHS understands "callSign", this string is contained in the result.
    virtual QStringList valueOfLHSStrings() const;
    //! Answer the composed vocabulary that valueOfLHS understands.
    //! In expressions, these strings are followed with "." and a remaining part.
    //! E.g. if valueOfLHS understands "remark.exists", the string "remark" is contained in the result.
    virtual QStringList valueOfLHSFirstStrings() const;
    //! Answer a metadata instance that describes the remaining part for each string
    //! returned from valueOfLHSFirstStrings().
    //! E.g. if valueOfLHS understands "remark.exists", valueOfLHSRemainingMetadata("remark") answers
    //! a metadata which valueOfLHSStrings() answers "exists".
    virtual AVComputableMetadata *valueOfLHSRemainingMetadata(const QString& first) const;
    //! Answer the vocabulary that assignValue understands. These strings do not contain a ".".
    //! E.g. if assignValue understands "approachType", this string is contained in the result.
    virtual QStringList assignValueStrings() const;
    //! Answer the composed vocabulary that assignValue understands.
    //! In expressions, these strings are followed with "." and a remaining part.
    //! E.g. if assignValue understands "menu.hidden", the string "menu" is contained in the result.
    virtual QStringList assignValueFirstStrings() const;
    //! Answer a metadata instance that describes the remaining part for each string
    //! returned from assignValueFirstStrings().
    //! E.g. if assignValue understands "menu.hidden", assignValueRemainingMetadata("menu") answers
    //! a metadata which valueOfLHSStrings() answers "hidden".
    virtual AVComputableMetadata *assignValueRemainingMetadata(const QString& first) const;
    //! Checks whether this particular instance is registered. This is used to avoid recursive log output.
    bool isRegistered() const;
    //! Write the valueOfLHS vocabulary to the log output
    void logMetadataValueOfLHS(const QString& prefix) const;
    //! Write the assignValue vocabulary to the log output
    void logMetadataAssignValue(const QString& prefix) const;
protected:
    //! Answer the metadata for the specified name
    AVComputableMetadata *getMetadata(const QString& name) const;
    //! Answer the metadata for the specified name
    AVComputableMetadata& accessMetadata(const QString& name) const;
private:
    //! Copy-constructor: defined but not implemented
    AVComputableMetadata(const AVComputableMetadata& rhs);
    //! Assignment operator: defined but not implemented
    AVComputableMetadata& operator=(const AVComputableMetadata& rhs);
private:
    QString m_name;
};

///////////////////////////////////////////////////////////////////////////////

//! This class provides a registry for AVComputableMetadatas

class AVLIB_EXPORT AVComputableMetadataRegistry : public AVExplicitSingleton<AVComputableMetadataRegistry>
{
public:
    //! Initialize the singleton
    static AVComputableMetadataRegistry& initializeSingleton();
public:
    //! Constructor
    AVComputableMetadataRegistry() { }
    //! Destructor
    ~AVComputableMetadataRegistry() override;
    //! Answer the name of this class
    virtual const char *className() const { return "AVComputableMetadataRegistry"; }
    //! Register a metadata instance. The registry takes ownership of the instance.
    void registerMetadata(AVComputableMetadata *metadata);
    //! Answer a list of all registered metadata names
    QStringList getMetadataNames() const;
    //! Answer the metadata for the specified name
    AVComputableMetadata *getMetadata(const QString& name) const;
    //! Answer the metadata for the specified name
    AVComputableMetadata& accessMetadata(const QString& name) const;
    //! Write the contents of the registry to the log output
    void logAllMetadata() const;
private:
    //! Copy-constructor: defined but not implemented
    AVComputableMetadataRegistry(const AVComputableMetadataRegistry& rhs);
    //! Assignment operator: defined but not implemented
    AVComputableMetadataRegistry& operator=(const AVComputableMetadataRegistry& rhs);
private:
    QHash<QString, AVComputableMetadata*> m_metadataDict;
};

///////////////////////////////////////////////////////////////////////////////

//! This helper class is used to implement nested metadata

class AVLIB_EXPORT AVGenericComputableMetadata : public AVComputableMetadata
{
public:
    explicit AVGenericComputableMetadata(const QString& name);
    ~AVGenericComputableMetadata() override;
    void setValueOfLHSStrings(const QStringList& list);
    void setValueOfLHSStrings(const QString& str) { setValueOfLHSStrings(QStringList(str)); }
    void setValueOfLHSFirstStrings(const QStringList& list);
    void setValueOfLHSFirstStrings(const QString& str) { setValueOfLHSFirstStrings(QStringList(str)); }
    void setAssignValueStrings(const QStringList& list);
    void setAssignValueStrings(const QString& str) { setAssignValueStrings(QStringList(str)); }
    void setAssignValueFirstStrings(const QStringList& list);
    void setAssignValueFirstStrings(const QString& str) { setAssignValueFirstStrings(QStringList(str)); }
    void addSuccessor(AVComputableMetadata *successor) const;
    QStringList           valueOfLHSStrings() const override;
    QStringList           valueOfLHSFirstStrings() const override;
    AVComputableMetadata* valueOfLHSRemainingMetadata(const QString& first) const override;
    QStringList           assignValueStrings() const override;
    QStringList           assignValueFirstStrings() const override;
    AVComputableMetadata* assignValueRemainingMetadata(const QString& first) const override;

protected:
    virtual AVComputableMetadata *getValueOfLHSRemainingMetadata(const QString& first) const;
    virtual AVComputableMetadata *getAssignValueRemainingMetadata(const QString& first) const;
    virtual void addSuccessors() const { };
    void addSuccessorsOnce() const;
private:
    //! Copy-constructor: defined but not implemented
    AVGenericComputableMetadata(const AVGenericComputableMetadata& rhs);
    //! Assignment operator: defined but not implemented
    AVGenericComputableMetadata& operator=(const AVGenericComputableMetadata& rhs);
protected:
    QStringList                                m_valueOfLHSStrings;
    QStringList                                m_valueOfLHSFirstStrings;
    mutable QHash<QString, AVGenericComputableMetadata*> m_valueOfLHSRemainingMetadataDict;
    QStringList                                m_assignValueStrings;
    QStringList                                m_assignValueFirstStrings;
    mutable QHash<QString, AVGenericComputableMetadata*> m_assignValueRemainingMetadataDict;
    mutable bool                               m_successorsAdded;
    mutable QList<AVComputableMetadata*>       m_successors;
};

#endif // AVEXPRESSIONMETADATA_H

// End of file
