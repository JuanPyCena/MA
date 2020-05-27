///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief
*/

#if !defined(AVLOG_METADATA_H_INCLUDED)
#define AVLOG_METADATA_H_INCLUDED

#include <QString>
#include <QSharedPointer>

#include "avlib_export.h"

/////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVLogMetadataBase
{
public:
    AVLogMetadataBase(const QString& name);
    virtual ~AVLogMetadataBase();

    QString getName() const;
    virtual QString getValueAsString() const = 0;

private:
    QString m_name;
};

/////////////////////////////////////////////////////////////////////////////

template <typename T>
class AVLogMetadataTyped : public AVLogMetadataBase
{
public:
    AVLogMetadataTyped() {}
    AVLogMetadataTyped(QString name, T value);

    QString getValueAsString() const override;

    T m_value;
};

/////////////////////////////////////////////////////////////////////////////

template <typename T>
AVLogMetadataTyped<T>::AVLogMetadataTyped(QString name, T value) :
    AVLogMetadataBase(name),
    m_value(value)
{
}

/////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVLogMetadataTyped<T>::getValueAsString() const
{
    return AVToString(m_value);
}


/////////////////////////////////////////////////////////////////////////////
/**
 * @brief The AVLogMetadata class implements value semantics for arbitrarily typed log metadata.
 */
class AVLIB_EXPORT AVLogMetadata
{
public:

    AVLogMetadata();
    //! Claims ownership of the passed pointer.
    template <typename T>
    AVLogMetadata(const QString& name, T value);
    AVLogMetadata& operator=(const AVLogMetadata& other);

    QString getName() const;
    QString getValueAsString() const;

private:
    QSharedPointer<const AVLogMetadataBase> m_metadata_ptr;
};

/////////////////////////////////////////////////////////////////////////////

template <typename T>
AVLogMetadata::AVLogMetadata(const QString& name, T value) :
    m_metadata_ptr(new AVLogMetadataTyped<T>(name, value))
{
}

#endif

// End of file
