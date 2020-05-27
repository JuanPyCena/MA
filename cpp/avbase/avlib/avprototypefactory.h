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
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVPROTOTYPEFACTORY_H__
#define __AVPROTOTYPEFACTORY_H__

#include "avlog.h"

#include "avprototype.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////// prototype factory ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*!
  Factory template for AVProtoType derived classes.
  The template class must implement the AVProtoType interface.

  \code

  // add prototypes to the factory

  class SomeBaseClass : public AVPrototype
  {
      ...
      AVProtoType* clone() const { ... };
      ...
  };

  clase SomeDerivedClass : public SomeBaseClass
  {
      ...
      AVProtoType* clone() const { ... };
      ...
  };

  class MyFactory : public AVProtoTypeFactory<SomeBaseClass>
  {
      ...
  };

  ...

  MyFactory factory;
  SomeBaseClass* prototype = new SomeBaseClass;
  AVASSERT(prototype != 0);
  factory.addPrototype(BASE_TYPE, prototype);
  prototype = new SomeDerivedClass;
  AVASSERT(prototype != 0);
  factory.addPrototype(DERIVED_TYPE, prototype);

  ...

  SomeBaseClass* clone = factory.getPrototype(DERIVED_TYPE);
  AVASSERT(clone != 0);

  ...

  delete clone;
  clone = 0;

  ...

  factory.clear();

  SomeBaseClass* clone = factory.getPrototype(TYPE);
  // ATTENTION: (clone == 0) -> because of deleted prototypes

  \endcode
*/
template <class PROTOTYPE> class AVProtoTypeFactory
{
public:
    //! Standard Constructor
    AVProtoTypeFactory()
    {
    }

    //! Destructor
    virtual ~AVProtoTypeFactory()
    {
        clear();
    }

    //! Adds the given prototype the given type.
    //! Any existing prototype for this_type will be deleted.
    //! Returns true if a prototype already existed, false otherwise.
    virtual bool addPrototype(const QString& type, PROTOTYPE* prototype)
    {
        AVASSERT(!type.isEmpty());
        AVASSERT(prototype != 0);

        PROTOTYPE* existing = m_prototype_dict.value(type, 0);
        if(existing)
        {
            delete existing;
        }

        m_prototype_dict.insert(type, prototype);
        return existing;
    }

    //! Removes any prototype of the given type.
    //! Returns true if such a type existed, false otherwise.
    virtual bool removePrototype(const QString& type)
    {
        AVASSERT(!type.isEmpty());

        typename QHash<QString, PROTOTYPE*>::iterator iter = m_prototype_dict.find(type);
        if(iter != m_prototype_dict.end())
        {
            delete iter.value();
            m_prototype_dict.erase(iter);
            return true;
        }
        return false;
    }

    //! Removes all prototypes.
    virtual void clear()
    {
        qDeleteAll(m_prototype_dict);
        m_prototype_dict.clear();
    }

    //! Returns a (maybe NULL on error) pointer to an allocated prototype of the given type.
    //! ATTENTION: The calling method is resonsible to delete the returned pointer!
    virtual PROTOTYPE* getPrototype(const QString& type)
    {
        AVASSERT(!type.isEmpty());

        PROTOTYPE* prototype = m_prototype_dict.value(type, 0);
        if (!prototype)
        {
            AVLogger->Write(LOG_ERROR, "AVProtoTypeFactory:getPrototype: "
                            "unknown field with type (%s)", qPrintable(type));
            return 0;
        }

        PROTOTYPE* copied_instance = prototype->clone();
        if (!copied_instance)
        {
            AVLogger->Write(LOG_ERROR, "AVProtoTypeFactory:getPrototype: "
                            "could not clone prototype with type (%s)", qPrintable(type));
            return 0;
        }

        return copied_instance;
    }

protected:

    //! dict holding the prototypes we are able to create indexed by their type
    QHash<QString, PROTOTYPE*> m_prototype_dict;

private:
    //! Hidden copy-constructor
    AVProtoTypeFactory(const AVProtoTypeFactory&);
    //! Hidden assignment operator
    const AVProtoTypeFactory& operator= (const AVProtoTypeFactory&);
};

#endif /* __AVPROTOTYPEFACTORY_H__ */

// End of file
