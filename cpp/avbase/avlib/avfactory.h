///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Michael Gebetsroither, m.gebetsroither@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVFactory_H__
#define __AVFactory_H__

// Qt includes
#include <QObject>
#include <QMap>
#include <QTextStream>

// avcommon library
#include "avlib_export.h"
#include "avlog.h"
#include "avshare.h"

// local includes

//! This is the default error policy of AVFactory
/*! It tries to get a string representation of the Identifier through operator<< of QTextOStream
 *  and writes a log entry through AVLogger with priority LOG_WARNING.
 */
template<typename Identifier, typename BaseClass>
struct AVFactoryDefaultErrorPolicy
{
    static BaseClass* errorUnknownType(const Identifier &src)
    {
        QString tmp;
        QTextStream stream(&tmp);
        stream << src;
        AVLogger->Write(LOG_WARNING, "AVFactory::get(): No Class with identifier \"%s\" found",
                qPrintable(tmp));
        return 0;
    }
};

/////////////////////////////////////////////////////////////////////////////

//! This is the universal error policy of AVFactory
/*! Use this if your Identifier does not implement operator<< and you don't want to implement one.
 *  Or if you don't need the exact name of the Identifier which was not found.
 *  This error policy simply prints the address of the unknown Identifier.
 */
template<typename Identifier, typename BaseClass>
struct AVFactoryUniversalErrorPolicy
{
    static BaseClass* errorUnknownType(const Identifier &src)
    {
        AVLogger->Write(LOG_WARNING, "AVFactory::get(): No Class with this identifier found "
               "(addr=%p)", &src);
        return 0;
    }
};

/////////////////////////////////////////////////////////////////////////////

//! Does not give any output, you should check with supports() befor get()
template<typename Identifier, typename BaseClass>
struct AVFactoryQuietErrorPolicy
{
    static BaseClass* errorUnknownType(const Identifier&)
    {
        return 0;
    }
};

/////////////////////////////////////////////////////////////////////////////
//! Abstract functor creating objects of base type \c BaseClass
template<typename BaseClass>
class AVFactoryFunctor
{
public:
    AVFactoryFunctor() {}
    virtual ~AVFactoryFunctor(){}

    virtual BaseClass* operator()() const = 0;
};

///////////////////////////////////////////////////////////////////////////////
//! Creates objects of base type \c BaseClass using polymorphic functors
//! TODO: The current functor implementation has to been seen as the first step on the way to a
//! generic functor. Template idioms like TypeList and Tuple<TypeList> have to implemented in order
//! to provide a generic functor.
template<typename BaseClass>
class AVFactoryCreator
{
public:
    typedef AVFactoryFunctor<BaseClass> FunctorImpl;

    AVFactoryCreator(){}

    explicit AVFactoryCreator(FunctorImpl* creatorImpl)
    {
       AVASSERT(creatorImpl != 0);
       m_creatorImpl = QSharedPointer<FunctorImpl>(creatorImpl);
    }

    virtual ~AVFactoryCreator(){}

    virtual BaseClass* operator()() const
    {
        AVASSERT(!m_creatorImpl.isNull());
        return m_creatorImpl->operator()();
    }
private:
    QSharedPointer<FunctorImpl> m_creatorImpl;
};

/////////////////////////////////////////////////////////////////////////////
/*! Functor creating objects of class \c DerivedClass inheriting from class \c BaseClass.
*   The objects are created using the standard constructor of \c DerivedClass.
*/
template<typename BaseClass, typename DerivedClass>
class AVFactoryFunctorStdCtor : public AVFactoryFunctor<BaseClass>
{
public:
    AVFactoryFunctorStdCtor() {}
    virtual ~AVFactoryFunctorStdCtor() {}
    virtual BaseClass* operator()() const
    {
        BaseClass* instance = new DerivedClass;
        AVASSERT(instance != 0);
        return instance;
    }
};

/////////////////////////////////////////////////////////////////////////////
/*! Functor creating objects of class \c DerivedClass, inheriting from class \c BaseClass
 *  For the instantiation of classes with one constructor parameter.
 */
template<typename BaseClass, typename DerivedClass,
         typename CtorParam1>
class AVFactoryFunctor1Param : public AVFactoryFunctor<BaseClass>
{
public:
    explicit AVFactoryFunctor1Param(const CtorParam1& param1) : m_param1(param1)
    {}

    virtual ~AVFactoryFunctor1Param() {}

    virtual BaseClass* operator()() const
    {
        BaseClass* instance = new DerivedClass(m_param1);
        AVASSERT(instance != 0);
        return instance;
    }

    CtorParam1 m_param1;
};

/////////////////////////////////////////////////////////////////////////////
/*! Functor creating objects of class \c DerivedClass inheriting from class \c BaseClass.
 *  Intended for the instantiation of classes with two constructor parameters.
 */
template<typename BaseClass, typename DerivedClass,
         typename CtorParam1, typename CtorParam2>
class AVFactoryFunctor2Param : public AVFactoryFunctor<BaseClass>
{
public:
    AVFactoryFunctor2Param(const CtorParam1& param1, const CtorParam2& param2)
        : m_param1(param1), m_param2(param2)
    {}

    virtual ~AVFactoryFunctor2Param() {}

    virtual BaseClass* operator()() const
    {
        BaseClass* instance = new DerivedClass(m_param1, m_param2);
        AVASSERT(instance != 0);
        return instance;
    }

    CtorParam1 m_param1;
    CtorParam2 m_param2;
};

/////////////////////////////////////////////////////////////////////////////
/*! Functor creating objects of class \c DerivedClass inheriting from class \c BaseClass.
 *  Intended for the instantiation of classes with three constructor parameters.
 */
template<typename BaseClass, typename DerivedClass,
         typename CtorParam1, typename CtorParam2, typename CtorParam3>
class AVFactoryFunctor3Param : public AVFactoryFunctor<BaseClass>
{
public:
    AVFactoryFunctor3Param(const CtorParam1& param1, const CtorParam2& param2,
                           const CtorParam3& param3)
        : m_param1(param1), m_param2(param2), m_param3(param3)
    {}

    virtual ~AVFactoryFunctor3Param() {}

    virtual BaseClass* operator()() const
    {
        BaseClass* instance = new DerivedClass(m_param1, m_param2, m_param3);
        AVASSERT(instance != 0);
        return instance;
    }

    CtorParam1 m_param1;
    CtorParam2 m_param2;
    CtorParam3 m_param3;
};

/////////////////////////////////////////////////////////////////////////////
/*! Functor creating objects of class \c DerivedClass inheriting from class \c BaseClass.
 *  Intended for the instantiation of classes with four constructor parameters.
 */
template<typename BaseClass, typename DerivedClass,
         typename CtorParam1, typename CtorParam2, typename CtorParam3, typename CtorParam4>
class AVFactoryFunctor4Param : public AVFactoryFunctor<BaseClass>
{
public:
    AVFactoryFunctor4Param(const CtorParam1& param1, const CtorParam2& param2,
                           const CtorParam3& param3, const CtorParam4& param4)
        : m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4)
    {}

    virtual ~AVFactoryFunctor4Param() {}

    virtual BaseClass* operator()() const
    {
        BaseClass* instance = new DerivedClass(m_param1, m_param2, m_param3, m_param4);
        AVASSERT(instance != 0);
        return instance;
    }

    CtorParam1 m_param1;
    CtorParam2 m_param2;
    CtorParam3 m_param3;
    CtorParam4 m_param4;
};

/////////////////////////////////////////////////////////////////////////////
/*! Functor creating objects of class \c DerivedClass inheriting from class \c BaseClass.
 *  Intended for the instantiation of classes with five constructor parameters.
 */
template<typename BaseClass, typename DerivedClass,
         typename CtorParam1, typename CtorParam2,
         typename CtorParam3, typename CtorParam4,
         typename CtorParam5>
class AVFactoryFunctor5Param : public AVFactoryFunctor<BaseClass>
{
public:
    AVFactoryFunctor5Param(const CtorParam1& param1, const CtorParam2& param2,
                           const CtorParam3& param3, const CtorParam4& param4,
                           const CtorParam5& param5)
        : m_param1(param1), m_param2(param2),
          m_param3(param3), m_param4(param4),
          m_param5(param5)
    {}

    virtual ~AVFactoryFunctor5Param() {}

    virtual BaseClass* operator()() const
    {
        BaseClass* instance = new DerivedClass(m_param1, m_param2, m_param3, m_param4, m_param5);
        AVASSERT(instance != 0);
        return instance;
    }

    CtorParam1 m_param1;
    CtorParam2 m_param2;
    CtorParam3 m_param3;
    CtorParam4 m_param4;
    CtorParam5 m_param5;
};

/////////////////////////////////////////////////////////////////////////////

/*! \example tests/unit/avlib/avfactorytest.cpp
 * This should provide further usage examples.
 */

//! AVFactory should be a generic factory
/*! NOTES:
 *      - To create objects with other constructors as the default constructor use a functor.
 *      - Identifier has to implement operator<< with AVFactoryDefaultErrorPolicy. If the
 *          Identifier does not implement operator<< create your own error policy or use
 *          AVFactoryUniversalErrorPolicy.
 *      - Through customizing the error policy used by the factory it's possible to
 *          construct a default object on method get if the identifier is not found.
 *          (write your own error policy and return a pointer to a newly created
 *          default object).
 *
 * Sample:
 * \code
 * BaseMessage *msg;
 * DerivedMessage *msg_derived;
 * AVFactory<BaseMessage, QString> factory;
 *
 * // Solaris linker fails to resolve symbol if only the method is templated
 * template<typename T>
 * class Creator
 * {
 * public:
 *     static BaseMessage* create()
 *     {
 *         T* t = new T;
 *         AVASSERT(t != 0);
 *         return t;
 *     }
 * };
 *
 * factory.add("base", Creator<BaseMessage>::create);
 * factory.add("derived", Creator<DerivedMessage>::create);
 *
 * msg = factory.get("base");
 * msg_derived = factory.get("derived");
 * \endcode
 */
template<typename BaseClass,
         typename Identifier = QString,
         typename Creator = BaseClass*(*)(),
         template<typename, typename>
             class FactoryErrorPolicy = AVFactoryDefaultErrorPolicy>
class AVFactory : public FactoryErrorPolicy<Identifier, BaseClass>
{
public:
    AVFactory() {}
    virtual ~AVFactory() {}

    typedef QMap<Identifier, Creator>   CallbackMap;

    //! Method to insert objects with your own Creator
    /*! \param  ident    Object identifier
     *  \param  creator  Custom creator for this object
     *  \return true/false if the insertion took place
     */
    bool add(const Identifier &ident, const Creator &creator)
    {
        return insert(ident, creator);
    }

    //! Check if the factory is able to create an object for this identifier
    /*! \param  ident    Object identifier
     *  \return true/false
     */
    bool supports(const Identifier &ident) const
    {
        return (m_callbacks.find(ident) != m_callbacks.end());
    }

    //! Deletes a pair (identifier, creator) from the callback map
    /*! \param  ident    Object identifier
     */
    void del(const Identifier &ident)
    {
        m_callbacks.erase(ident);
    }

    //! Returns the object created by the creator of the given identifier (or 0)
    /*! \param  ident    Object identifier
     *  \return Pointer to the newly created object or 0 if identifier not found
     */
    BaseClass* get(const Identifier &ident) const
    {
        CallbackMapConstIterator iter = m_callbacks.find(ident);
        if(iter != m_callbacks.end()) {
            return (*iter)();
        } else {
            return this->errorUnknownType(ident);
        }
    }

    //! Returns a const reference to the intern callback map
    const CallbackMap& getCallbackMap() const
    {
        return m_callbacks;
    }

protected:  // members
    typedef typename CallbackMap::const_iterator    CallbackMapConstIterator;

    //! Holds the callbacks for object creation
    CallbackMap                                     m_callbacks;

protected:  // methods

    //! Intern method to insert creators into the callback map
    inline bool insert(const Identifier &ident, const Creator &creator)
    {
        if(!m_callbacks.contains(ident)) {
            m_callbacks.insert(ident, creator);
            return true;
        }
        return false;
    }

private:

    AVFactory(AVFactory const &src);
    AVFactory &operator=(const AVFactory &src);
};

#endif
// vim:foldmethod=marker
// End of file
