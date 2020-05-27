///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Stefan Kunz, s.kunz@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Definition of miscellaneous macros.
*/

#ifndef __AVMACROS_H__
#define __AVMACROS_H__

#include <limits.h>

#include <QCoreApplication>
#include <QMetaType>
#include <QStringList>
#include <QMap>

#include "avlib_export.h"
#include "avfromtostring.h"

template<typename T>
class avRegisterMetaTypeHelper;

//! This macro will register a type in Qt's meta type framework
//! on application startup using qRegisterMetaType<T>()
/*
 * This is needed when using custom types in signals/slots, QVariant or Qml
 * as it requires the type to be registered.
 *
 * This macro should be placed at the end of the .cpp file of a class
 * Some types also require a Q_DECLARE_METATYPE in your .h file (non-pointers)
 * The advantage of using this macro instead of placing the qRegisterMetaType
 * call in every application's main (or even worse: in every test case)
 * is that it happens automatically when linking the library with the class to
 * the application. No need to think about it when a new application uses the
 * same library again.
 *
 * __VA_ARGS__ was needed for templated types with comma in it
 * as this results in splitting into multiple macro args!
 */
#define AV_REGISTER_METATYPE(...)                                                                       \
    template<>                                                                                          \
    class avRegisterMetaTypeHelper<__VA_ARGS__ >                                                        \
    {                                                                                                   \
        explicit avRegisterMetaTypeHelper()                                                             \
        {                                                                                               \
            /* qAddPreRoutine is qt-internal but needed to register after QApplication instantiation */ \
            qAddPreRoutine([] { qRegisterMetaType<__VA_ARGS__>(); });                                   \
        }                                                                                               \
                                                                                                        \
        static const avRegisterMetaTypeHelper<__VA_ARGS__ > staticInstance;                             \
    };                                                                                                  \
                                                                                                        \
    /* This is almost the same as Q_COREAPP_STARTUP_FUNCTION but using typename template args */        \
    /* to avoid problems when the type contains special characters like * <> ,   */                     \
    const avRegisterMetaTypeHelper<__VA_ARGS__ > avRegisterMetaTypeHelper<__VA_ARGS__ >::staticInstance;

#define AV_STARTUP_FUNCTION_PRIVATE(FUNC, ...)       \
namespace {                                          \
    void FUNC() {                                    \
        __VA_ARGS__;                                 \
    }                                                \
    Q_COREAPP_STARTUP_FUNCTION(FUNC)                 \
}

#define AV_CAT(a, b) AV_CAT_PRIVATE(a, b)
#define AV_CAT_PRIVATE(a, b) a ## b

//! Macro to put the arguments into a function (named by the caller) as part of an anonymous namespace and register the
//! function to be called at start-up of QApplicationCore.
//! Attention: This macro is not suitable for use in library code that is then statically linked into
//! an application since the function may not be called at all due to being eliminated by the linker.
#define AV_STARTUP_FUNCTION(...) AV_STARTUP_FUNCTION_PRIVATE(AV_CAT(called_on_start_up,__COUNTER__), __VA_ARGS__);

//! Template-function to ensure that a given QMetaType::registerEqualsComparator call for a given type is done only once.
template<typename T>
void avRegisterEqualsComparator();

//! Macro to globally register the equals-comparators for a given type in the Qt's meta type framework.
//! E.g.: Use compare operator for a given type when putting this type into a QVariant.
#define AV_REGISTER_METATYPE_EQUALS_COMPARATOR(TYPE) \
    template<>                                       \
    void avRegisterEqualsComparator<TYPE>(){}        \
    AV_STARTUP_FUNCTION(AVASSERT(QMetaType::registerEqualsComparator<TYPE>()));

///////////////////////////////////////////////////////////////////////////////
//! enum factory for simpler enum usage

// expansion macro for enum value definition
#define ENUM_VALUE(name, ...) name __VA_ARGS__,

// expansion macro for enum to string conversion
#define ENUM_CASE(name, ...) case AVCurrentEnum::name: return #name;

// expansion macro for enum from integer conversion
#define ENUM_INTCMP(name, ...) case AVCurrentEnum::name: { target = AVCurrentEnum::name; return true; }

// expansion macro for enum to string conversion
#define ENUM_STRCMP(name, ...) if (str == #name) { target = AVCurrentEnum::name; return true; }

// expansion macro for all enums to a list list
#define ENUM_LIST(name, ...) << AVCurrentEnum::name

// expansion macro for all enums to string list
#define ENUM_STRLIST(name, ...) << #name

// expansion macro that returns all set flags as string
#define ENUM_STRFLAGS(name, ...) if (static_cast<std::underlying_type<AVCurrentEnum>::type>(flags) &                    \
                                     static_cast<std::underlying_type<AVCurrentEnum>::type>(AVCurrentEnum::name))       \
                                     flags_string.append(#name);

//! This is a lightweight enum definition without struct; it *must* be used inside classes.
//! Usage of this enum is discouraged, use DEFINE_TYPESAFE_ENUM instead!
/*
 * can be used like followed:
 *
 * class AVExampleClass
 * {
 * public:
 *
 * ...
 *
 * #define EnumValues1(enum_value) \
 *         enum_value(Value1,  =0x1)   \
 *         enum_value(Value2,  =0x2)   \
 *         enum_value(Value3,  =0x4)
 *
 * #define EnumValues2(enum_value) \
 *         enum_value(Value5,  =0x1)   \
 *         enum_value(Value6,  =0x2)   \
 *         enum_value(Value7,  =0x4)
 *
 *     DEFINE_ENUM(Enum1, EnumValues1)
 *     DEFINE_ENUM(Enum2, EnumValues2)
 *	   DEFINE_ENUM(Enum3, EnumValues2)	// That's not allowed, in such a case use DEFINE_TYPESAFE_ENUM
 * ...
 *
 * public:
 *
 * 	   void printEnum(){
 * 	   		AVLogInfo << "AVExampleClass::printEnum: " << AVExampleClass::enumToString(AVExampleClass::Value1);
 * 	   }
 * };
 *
 */
#define DEFINE_ENUM(Enum,ENUM_DEF)                                                                \
    enum Enum                                                                                     \
    {                                                                                             \
            ENUM_DEF(ENUM_VALUE)                                                                  \
    };                                                                                            \
    ENUM_FUNCTIONS_INTERNAL(Enum, ENUM_DEF, static, friend)
//! This is a lightweight typesafe enum definition without struct; it *must* be used inside classes
/*
 * Note: type safe enums are not implicit int, so they have to be cast to int if necessary (see unit tests)
 *
 * can be used like followed:
 *
 * class AVExampleClass
 * {
 * public:
 *
 * ...
 *
 * #define EnumValues(enum_value) \
 *         enum_value(Value1,  =0x1)   \
 *         enum_value(Value2,  =0x2)   \
 *         enum_value(Value3,  =0x4)
 *
 *     DEFINE_TYPESAFE_ENUM(Enum1, EnumValues)
 *     DEFINE_TYPESAFE_ENUM(Enum2, EnumValues)
 *
 * ...
 *
 * public:
 *
 * 	   void printEnum(){
 * 	   		AVLogInfo << "AVExampleClass::printEnum:" << Enum1::enumToString(Enum1::Value1);
 * 	   }
 * };
 *
 */
#define DEFINE_TYPESAFE_ENUM(Enum,ENUM_DEF)                                                       \
    enum class Enum                                                                               \
    {                                                                                             \
            ENUM_DEF(ENUM_VALUE)                                                                  \
    };                                                                                            \
    ENUM_FUNCTIONS_INTERNAL(Enum, ENUM_DEF, static, friend)

//! This macro allows to define a typesafe enum inside a namespace.
/*
 * Using a namespace enum inside of an AVFieldEnum is possible by using the generated type Enum##ToFromStringHelper
 * as the TOFROMSTRING_BASE template parameter.
 * Example:
 * AVFieldEnum<some_namespace::EnumType, some_namespace::EnumTypeToFromStringHelper>
 */
#define DEFINE_TYPESAFE_ENUM_IN_NAMESPACE(Enum,ENUM_DEF)                                          \
    enum class Enum                                                                               \
    {                                                                                             \
            ENUM_DEF(ENUM_VALUE)                                                                  \
    };                                                                                            \
    ENUM_FUNCTIONS_INTERNAL(Enum, ENUM_DEF, inline, inline)                                       \
    static bool internalFromStringBounce(const QString& str, Enum &target) { return enumFromString(str, target); } \
    static QString internalToStringBounce(Enum value, const QString& errorString) { return enumToString(value, errorString); } \
    struct Enum##ToFromStringHelper {                                                             \
        static bool enumFromString(const QString& str, Enum &target)                              \
        {                                                                                         \
            return internalFromStringBounce(str, target);                                         \
        }                                                                                         \
        static QString enumToString(Enum value, const QString& errorString=QString::null)         \
        {                                                                                         \
            return internalToStringBounce(value, errorString);                                    \
        }                                                                                         \
    };

// This macro is deprecated. Don't use it in new code.
// Define your enums in namespaces, classes or structs instead.
#define ENUM_FUNCTIONS(Enum, ENUM_DEF)                                                       	  \
    ENUM_FUNCTIONS_INTERNAL(Enum, ENUM_DEF, static, friend)

// ENUM Macro function definition
#define ENUM_FUNCTIONS_INTERNAL(Enum, ENUM_DEF, INTERNAL_KEYWORD, EXTERNAL_KEYWORD)               \
    INTERNAL_KEYWORD QString enumToString(Enum value, const QString& error_string=QString::null)  \
    {                                                                                             \
        using AVCurrentEnum = Enum;                                                               \
        switch(value)                                                                             \
        {                                                                                         \
            ENUM_DEF(ENUM_CASE)                                                                   \
        }                                                                                         \
        return error_string;                                                                      \
    }                                                                                             \
    INTERNAL_KEYWORD bool enumFromString(const QString& str, Enum &target)                        \
    {                                                                                             \
        using AVCurrentEnum = Enum;                                                               \
        ENUM_DEF(ENUM_STRCMP)                                                                     \
        return false; /* handle input error */                                                    \
    }                                                                                             \
    INTERNAL_KEYWORD bool enumFromInt(int value, Enum &target)                                    \
    {                                                                                             \
        using AVCurrentEnum = Enum;                                                               \
        switch(static_cast<AVCurrentEnum>(value))                                                 \
        {                                                                                         \
            ENUM_DEF(ENUM_INTCMP)                                                                 \
        }                                                                                         \
        return false; /* handle input error */                                                    \
    }                                                                                             \
    /* returns all available enums as string in a string list*/                                   \
    INTERNAL_KEYWORD QStringList get##Enum##Strings()                                             \
    {                                                                                             \
        static const QStringList enums = (QStringList() ENUM_DEF(ENUM_STRLIST));                  \
        return enums;                                                                             \
    }                                                                                             \
    /* returns all available enums in a list*/                                                    \
    INTERNAL_KEYWORD QList<Enum> getAll##Enum##Values()                                           \
    {                                                                                             \
        using AVCurrentEnum = Enum;                                                               \
        static const QList<Enum> enums = (QList<Enum>() ENUM_DEF(ENUM_LIST));                     \
        return enums;                                                                             \
    }                                                                                             \
    /* returns all set flags in flags as string in a string list */                               \
    INTERNAL_KEYWORD QStringList get##Enum##FlagStrings(int flags)                                \
    {                                                                                             \
        using AVCurrentEnum = Enum;                                                               \
        QStringList flags_string;                                                                 \
        ENUM_DEF(ENUM_STRFLAGS)                                                                   \
        return flags_string;                                                                      \
    }                                                                                             \
    /* returns all set flags in flags as string in a string list */                               \
    INTERNAL_KEYWORD QStringList getFlagStrings(int flags, Enum)                                  \
    {                                                                                             \
        using AVCurrentEnum = Enum;                                                               \
        QStringList flags_string;                                                                 \
        ENUM_DEF(ENUM_STRFLAGS)                                                                   \
        return flags_string;                                                                      \
    }                                                                                             \
    EXTERNAL_KEYWORD QString AVToString(Enum arg, bool escape = false)                            \
    {                                                                                             \
        return AVToString(enumToString(arg), escape);                                             \
    }                                                                                             \
    EXTERNAL_KEYWORD bool AVFromString(const QString& str, Enum& arg)                             \
    {                                                                                             \
        QString temp;                                                                             \
        return AVFromString(str, temp) && enumFromString(temp, arg);                              \
    }                                                                                             \
    EXTERNAL_KEYWORD QList<Enum> AVGetEnumValues(Enum = Enum{}) { return getAll##Enum##Values(); }


#define DEFINE_ENUM_IN_STRUCT(EnumStruct, Enum, ENUM_DEF) \
    struct EnumStruct \
    { \
        DEFINE_ENUM(Enum, ENUM_DEF) \
    };

//! Helper function for AVASSERT to reduce compile time depencencies.
/*! \note Has no 'av' prefix to not show up in auto completion.
 */
void AVLIB_EXPORT helper_avassert(int line, const char *file, const char *message);

//! special AVLib assertion macro which is always compiled in.
/*! It writes the given message to the logfile in case that the
    assertion fails and terminates the program.
    If the logger is not initialized yet, it writes the message to stderr.
*/
#define AVASSERTMSG(x, msg)                                                  \
    do {                                                                     \
        if (!(x)) {                                                          \
            helper_avassert(__LINE__, __FILE__, (msg));                      \
        }                                                                    \
    } while (false)

//! special AVLib assertion macro which is always compiled in.
/*! It writes a message to the logfile in case that the assertion fails
    and terminates the program.
    If the logger is not initialized yet, it uses qFatal.
*/
#define AVASSERT(x) AVASSERTMSG(x, #x)

//! Special assertion macro which is used by AVShare and always compiled in.
/*! It writes a message to stderr. This macro is needed since AVShare is a
    template (only header file) and AVLog class has AVShare's as members.
    Therefore a cyclic dependency (AVLog->AVShare->AVLog) would occur when
    using the AVASSERT macro.
*/
#define AVASSERTNOLOGGER(x)                                                  \
    do {                                                                     \
        if (!(x)) {                                                          \
            qFatal("ASSERTION FAILED: \"%s\" in file %s, line %d",           \
                   #x, __FILE__, __LINE__);                                  \
        }                                                                    \
    } while (false)

//! Static (compile-time) assertion.
/*! Use this to verify assumptions which can be already tested at compile time (e.g. sizes of
    constant arrays which must match, etc.)
*/
#define AV_STATIC_ASSERT(B) AV_STATIC_ASSERT_IMPL <B>()
//! Static assertion helper function.
template <bool B>
inline void AV_STATIC_ASSERT_IMPL()
{
    // B will be true or false, which will implictly convert to 1 or 0
    char AV_STATIC_ASSERT_FAILURE[B] = {0};
    Q_UNUSED(AV_STATIC_ASSERT_FAILURE);
}

//! asserted signal-slot connection
/*! Example code:
    \code
    AVSIGNALSLOTDIRECTCONNECT(&m_timer, timeout(),
    this,     slotTimeout());
    \endcode
    is essentially the same as:
    \code
    AVASSERT(QObject::connect(&m_timer, SIGNAL(timeout()),
    this,     SLOT(slotTimeout())));
    \endcode
    */
#define AVSIGNALSLOTDIRECTCONNECT(sender, signal, receiver, method)     \
    AVASSERTMSG((QObject::connect((sender), SIGNAL(signal),             \
                    (receiver), SLOT(method), Qt::DirectConnection)),   \
                 "AVSIGNALSLOTDIRECTCONNECT failed")

#define AVSIGNALSLOTCONNECT(sender, signal, receiver, method,...)		\
    {																	\
        bool connected = QObject::connect((sender), SIGNAL(signal),		\
                    (receiver), SLOT(method), ## __VA_ARGS__);			\
        AVASSERTMSG(connected, "AVSIGNALSLOTCONNECT failed");			\
    }

//! asserted connection of signal-signal or signal-slot
#define AVDIRECTCONNECT(sender, signal, receiver, method)       \
    AVASSERTMSG((QObject::connect((sender), signal,             \
                    (receiver), method, Qt::DirectConnection)), \
                "AVDIRECTCONNECT failed")

//! asserted connection of signal-signal or signal-slot
#define AVCONNECT(sender, signal, receiver, method,...)			\
    {															\
        bool connected = QObject::connect((sender), signal,		\
            (receiver), method, ## __VA_ARGS__);				\
        AVASSERTMSG(connected, "AVCONNECT failed");				\
    }

//! asserted disconnection of signal-signal or signal-slot
#define AVDISCONNECT(a,b,c,d) \
    AVASSERT(QObject::disconnect((a), b, (c), d))

//! Creates a new field of type BASE with name VAR with constructor CLASS
#define AVNEW(BASE, VAR, CLASS) \
    BASE* VAR = new CLASS;      \
    AVASSERT(VAR != 0);

//! Call delete for the given pointer and set the pointer to 0.
#define AVDEL(a)    \
    do {            \
        delete a;   \
        a = 0;      \
    } while (false)

//! ensure that a qapp->quit() really quits
//! post an pseudo event because otherwise qApp may not get the quit correctly
//!
//! *ATTENTION* this macro does not set the AVProcessTerminated flag (why??)
//!
//! *DEPRECATED*, don't use in new code: this macro probably is a workaround for signal handler problems, see SWE-1255.
//! Instead, refactor AVSignalHandler to provide a threadsafe way to terminate the application.
#define AVAPPQUIT() \
    do { \
        qApp->quit();                                            \
        QEvent *e = new QEvent(QEvent::None);                    \
        AVASSERT(e != 0);                                        \
        QApplication::postEvent(qApp, e);                        \
    } while (false)

//! ensure that a qapp->quit() really quits
//! post an pseudo event because otherwise qApp may not get the quit correctly.
//! Equal to AVAPPQUIT, but also sets AVProcessTerminated to true.
//!
//! *DEPRECATED*, don't use in new code: this macro probably is a workaround for signal handler problems, see SWE-1255.
//! Instead, refactor AVSignalHandler to provide a threadsafe way to terminate the application.
#define AVAPPDAEMONQUIT() \
    do { \
        qApp->quit();                                            \
        QEvent *e = new QEvent(QEvent::None);                    \
        AVASSERT(e != 0);                                        \
        QApplication::postEvent(qApp, e);                        \
        AVProcessTerminated = true;                              \
    } while (false)

//! Disables/hides copy constructor and assignment operator
/*! Classes which have dynamically allocated members are
    required to have a copy constructor and assignment operator
    (checked by the Parasoft C++ Test suite).
    If no copying is required, it can just be disabled by declaring
    the two as private. This macro does exactly that.

    Example:
    \code
    class MyClass
    {
        AVDISABLECOPY(MyClass);

    public:
        MyClass();

    private:
        SomePointer* m_pointer;
    };
    \endcode
*/
#define AVDISABLECOPY(ClassName)            \
private:                                    \
    /*! Hidden copy constructor */          \
    ClassName(const ClassName&);            \
    /*! Hidden assignment operator */       \
    ClassName& operator=(const ClassName&)

//! Concatenates the two given parameters
#define __AVCONCATENATE2(a, b)  a##b
#define AVCONCATENATE2(a, b)    __AVCONCATENATE2(a,b)

//! 64 bit max and min values. Take the available system define in limits.h
#if defined(ULONG_LONG_MAX)
#define AV_UINT64_MAX ULONG_LONG_MAX
#else
#define AV_UINT64_MAX ULLONG_MAX
#endif

#if defined(LONG_LONG_MAX)
#define AV_INT64_MAX LONG_LONG_MAX
#else
#define AV_INT64_MAX LLONG_MAX
#endif

#if defined(LONG_LONG_MIN)
#define AV_INT64_MIN LONG_LONG_MIN
#else
#define AV_INT64_MIN LLONG_MIN
#endif

//! Functions to convert between QENUM values and their string representations
#define QENUM_STRING_CONVERSION(enum_name)\
static const char* enum##enum_name##ToString(const enum_name type) \
{ \
    static QMetaEnum meta_enum = \
            staticMetaObject.enumerator( \
                staticMetaObject.indexOfEnumerator(#enum_name)); \
    return meta_enum.valueToKey(static_cast<int>(type)); \
} \
static enum_name enum##enum_name##FromString(const QString& type, bool* ok = 0) \
{ \
    static QMetaEnum meta_enum = \
            staticMetaObject.enumerator( \
                staticMetaObject.indexOfEnumerator(#enum_name)); \
    return static_cast<enum_name>(meta_enum.keyToValue(qPrintable(type),ok)); \
}


// explicit instantiation of QMap for MSVC
#if defined(Q_OS_WIN)
AVLIB_TEMPLATE_EXPORT template class AVLIB_EXPORT QMap < QString, QString >;
#endif

// export macros to for CRTP base macros to work on linux
#if defined(Q_OS_WIN)
    #define AV_CLASS_TEMPLATE_EXPORT
#else
    #define AV_CLASS_TEMPLATE_EXPORT __attribute__((visibility("default")))
#endif

#endif /* __AVMACROS_H__ */

// End of file
