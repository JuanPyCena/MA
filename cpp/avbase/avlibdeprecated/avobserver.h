///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2008
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Declaration of Observer and Observable
*/

#if !defined(AVOBSERVER_H_INCLUDED)
#define AVOBSERVER_H_INCLUDED

// local includes
#include "avmacros.h"
#include "avobservableevent.h"
#include "avlibdeprecated_export.h"
// forward declarations
class AVObservable;

//! The observer base class of the well-known observer pattern
/*!
 *  \note
 *  The observer pattern was implemented due to the intrinsic threading issues of the SIGNAL/SLOT
 *  facility in Qt3, see also  PTS1658 and PTS3639. The AVObserver and AVObservable shall only
 *  be used to replace non-thread-safe SIGNAL/SLOT connections in multi-threaded code. It
 *  shall *NOT* be used in new code.
 *
 *  \attention
 *  The observer will be automatically unsubscribed from all its AVObservables at destruction time,
 *  similar to the auto-disconnect feature of QObject. In multi-threaded code, it must be
 *  guaranteed, that, while the observer is going to be destroyed, AVObserver::gotNotifcation()
 *  cannot be called by another thread. Otherwise, the program will most likely crash in
 *  AVObserver::gotNotification(). Thus, AVObserver::unsubscribe() shall always be called prior
 *  the destruction of the observer!
 *
 *  Concrete observer implementation using switch/case statement with downcast:
 *
 *  \code
 *  class MyObserver : public AVObserver
 *  {
 *  public:
 *      virtual void gotNotification(const AVObservableEvent& event)
 *      {
 *          switch(event.type())
 *          {
 *              case AVObservableEvent::Error:
 *              {
 *                  doSomething();
 *                  break;
 *              }
 *              case AVObservableEvent::HostConnected:
 *              {
 *                  const AVObservableEventHostConnected& concrete_event =
 *                       static_cast<const AVObservableEventHostConnected&>(event);
 *                  doSomething(concrete_event.addr(), concrete_event.port());
 *                  break;
 *              }
 *              default: break;
 *          }
 *      }
 *  };
 *  \endcode
 *
 *  Concrete observer using visitors:
 *
 *  \code
 *  class MyObserver
 *      : public AVObserver,
 *        public AVObservableEventVisitor<AVObservableEventError>,
 *        public AVObservableEventVisitor<AVObservableEventHostConnected>
 *  {
 *  public:
 *      virtual void gotNotification(const AVObservableEvent& event)
 *      {
 *          event.accept(*this);
 *      }
 *
 *      virtual bool visit(const AVObservableEventError& event)
 *      {
 *          // Do Something
 *          return true;
 *      }
 *
 *      virtual bool visit(const AVObservableEventHostConnected& event)
 *      {
 *          // Do Something
 *          return true;
 *      }
 *  };
 *
 */
class AVLIBDEPRECATED_EXPORT AVObserver
{
    AVDISABLECOPY(AVObserver);
    friend class AVObserverTest;
    friend class AVObservable;
public:
    AVObserver();

    virtual ~AVObserver();

    //! Handle events from AVObservables the observer has registered for
    /*! \sa AVObservable::subscribe()
     *  \param event subclass of AVObservableEvent, providing the reason
     *         for the notification. May also provide additional context information.
     */
    virtual void gotNotification(const AVObservableEvent& event) = 0;
private:
    class Data;
    Data*   m_d;
};

///////////////////////////////////////////////////////////////////////////////
//! The observable base class of the well-known observer pattern
class AVLIBDEPRECATED_EXPORT AVObservable
{
    AVDISABLECOPY(AVObservable);
    friend class AVObservableTest;
public:
    AVObservable();

    virtual ~AVObservable();

    //! Subscribes the \c observer to receive notifications from the observable
    void subscribe(AVObserver* observer);

    //! Unsubscribes the \c observer. The observer will no longer get
    //! notifications from the observable
    void unsubscribe(AVObserver* observer);

    void unsubscribeAll();

    //! Returns the number of subscribed observers
    int countObservers() const;

protected:
    //! Called by subclasses to notify subscribed observers
    //! \param event subclass of AVObservableEvent, providing the reason
    //! for the notification. May also provide additional context information.
    void notifyObservers(const AVObservableEvent& event);

private:
    class Data;
    Data*   m_d;
};

///////////////////////////////////////////////////////////////////////////////

#endif

// End of file
