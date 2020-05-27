//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2020
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Daniel Brunner, daniel.brunner@adbsafegate.com
    \brief   AVTypesafeSignalSpy header
 */

#pragma once

// Qt includes
#include <QObject>

// common includes
#include "avmacros.h"

template<typename ...Targs>
using AVTypesafeSignalSpyContainer = QVector<std::tuple<Targs...>>;

//! Similar to QSignalSpy, but doesn't use QVariants to store signal parameters
//! and therefore also works with types like QSet<QString>. This signal spy does
//! not require all signal parameters to be registers with QMetaType system.
//!
//! Example Usage:
//! \code
//! SomeClass obj;
//! AVTypesafeSignalSpy<SomeClass, QSet<QString>, QStringList> spy(&obj, &SomeClass:someSignal);
//! QCOMPARE(spy.calls(), {});
//! emit obj.someSignal({}, {});
//! emit obj.someSignal({"a"}, {"b"});
//! const decltype(spy)::Container expected{
//!     { {}, {} },
//!     { {"a"}, {"b"} }
//! };
//! QCOMPARE(spy.calls(), expected);
//! \endcode
template<typename SenderClass, typename ...Targs>
class AVTypesafeSignalSpy : public AVTypesafeSignalSpyContainer<Targs...>
{
public:
    using Container = AVTypesafeSignalSpyContainer<Targs...>;

    explicit AVTypesafeSignalSpy(const SenderClass *obj, void (SenderClass::*signal)(Targs ...));
    AVTypesafeSignalSpy(const AVTypesafeSignalSpy<SenderClass, Targs...> &other);
    AVTypesafeSignalSpy<SenderClass, Targs...> &operator=(const AVTypesafeSignalSpy<SenderClass, Targs...> &other);
    ~AVTypesafeSignalSpy();

private:
    void setupConnection();
    void destroyConnection();

    const SenderClass *m_obj;
    void (SenderClass::*m_signal)(Targs...);
    QMetaObject::Connection m_connection;
};

//-----------------------------------------------------------------------------

template<typename SenderClass, typename ...Targs>
AVTypesafeSignalSpy<SenderClass, Targs...>::AVTypesafeSignalSpy(const SenderClass *obj, void (SenderClass::*signal)(Targs...)) :
    AVTypesafeSignalSpyContainer<Targs...>{},
    m_obj{obj},
    m_signal{signal}
{
    setupConnection();
}

//-----------------------------------------------------------------------------

template<typename SenderClass, typename ...Targs>
AVTypesafeSignalSpy<SenderClass, Targs...>::AVTypesafeSignalSpy(const AVTypesafeSignalSpy<SenderClass, Targs...> &other) :
    AVTypesafeSignalSpyContainer<Targs...>{other},
    m_obj{other.m_obj},
    m_signal{other.m_signal}
{
    setupConnection();
}

//-----------------------------------------------------------------------------

template<typename SenderClass, typename ...Targs>
AVTypesafeSignalSpy<SenderClass, Targs...> &AVTypesafeSignalSpy<SenderClass, Targs...>::operator=(const AVTypesafeSignalSpy<SenderClass, Targs...> &other)
{
    AVTypesafeSignalSpyContainer<Targs...>::operator=(other);

    destroyConnection();

    m_obj = other.m_obj;
    m_signal = other.m_signal;

    setupConnection();

    return *this;
}

//-----------------------------------------------------------------------------

template<typename SenderClass, typename ...Targs>
AVTypesafeSignalSpy<SenderClass, Targs...>::~AVTypesafeSignalSpy()
{
    destroyConnection();
}

//-----------------------------------------------------------------------------

template<typename SenderClass, typename ...Targs>
void AVTypesafeSignalSpy<SenderClass, Targs...>::setupConnection()
{
    m_connection = QObject::connect(m_obj, m_signal, [this](Targs...args){
        this->append({args...});
    });
}

//-----------------------------------------------------------------------------

template<typename SenderClass, typename ...Targs>
void AVTypesafeSignalSpy<SenderClass, Targs...>::destroyConnection()
{
    QObject::disconnect(m_connection);
    m_connection = {};
}

//-----------------------------------------------------------------------------

//! Helper utility to construct a AVTypesafeSignalSpy
//!
//! //! Example Usage:
//! \code
//! SomeClass obj;
//! auto spy = avSpyFor(&obj, &SomeClass:someSignal);
//! QCOMPARE(spy.calls(), {});
//! emit obj.someSignal({}, {});
//! emit obj.someSignal({"a"}, {"b"});
//! const decltype(spy)::Container expected{
//!     { {}, {} },
//!     { {"a"}, {"b"} }
//! };
//! QCOMPARE(spy.calls(), expected);
//! \endcode
template<typename SenderClass, typename ...Targs>
AVTypesafeSignalSpy<SenderClass, Targs...> avSpyFor(SenderClass *ptr, void (SenderClass::*signal)(Targs ...))
{
    return AVTypesafeSignalSpy<SenderClass, Targs...>{ptr, signal};
}
