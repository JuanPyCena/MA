///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Alexander Wemmer, a.wemmer@avitbit.com
  \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
  \brief   Provides a method which emits a signal when called
*/

#if !defined(AVSIGNALPUBLISHER_H_INCLUDED)
#define AVSIGNALPUBLISHER_H_INCLUDED

#include <qobject.h>

// forward declarations
class QString;

///////////////////////////////////////////////////////////////////////////////
//! Provides a method which emits the signalPublish() when called
class AVSignalPublisher : public QObject
{
    Q_OBJECT
    
public:

    //! Constructor
    AVSignalPublisher() {}

    //! Destructor
    virtual ~AVSignalPublisher() {}

    //! emits thesignalPublish() without a value
    void publish()
    {
        emit signalPublish();
    }

    //! emits the signalPublish() with an integer value
    void publish(int value)
    {
        emit signalPublish(value);
    }

    //! emits the signalPublish() with a const QString& parameter
    void publish(const QString& str)
    {
        emit signalPublish(str);
    }

    //! emits the signalPublish() with two const QString& parameter
    void publish(const QString& str1, const QString& str2)
    {
        emit signalPublish(str1, str2);
    }

    //! emits a signalPublich() with three const QString& and an integer parameter
    void publish(const QString& str1, const QString& str2, const QString& str3, int value)
    {
        emit signalPublish(str1, str2, str3, value);
    }

    //! emits the signalPublish() with two const QString& and one int parameter
    void publish(const QString& str1, const QString& str2, int value)
    {
        emit signalPublish(str1, str2, value);
    }

signals:

    //! emitted when publish() is called
    void signalPublish();

    //! emitted when publish(int) is called
    void signalPublish(int);

    //! emitted when publish(const QString&) is called
    void signalPublish(const QString&);
 
    //! emitted when publish(const QString&, const QString&) is called
    void signalPublish(const QString&, const QString&);

    //! emitted when publish(const QString&, const QString&, const QString&, int) is called
    void signalPublish(const QString&, const QString&, const QString&, int);

    //! emitted when publish(const QString&, const QString&) is called
    void signalPublish(const QString&, const QString&, int);

private:

    //! Dummy copy constructor
    AVSignalPublisher(const AVSignalPublisher&);

    //! Dummy assignment operator
    AVSignalPublisher& operator=(const AVSignalPublisher&);
};

#endif

/////////////////////////////////////////////////////////////////////////////

// End of file
