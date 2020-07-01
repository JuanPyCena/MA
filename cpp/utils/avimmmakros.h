//
// Created by Felix on 25.05.2020.
//

#ifndef AVIMMMAKROS_H
#define AVIMMMAKROS_H

#define DEFINE_ACCESSORS_REF(NAME, TYPE, MEMBER)               \
    const TYPE& get##NAME() const { return MEMBER; };          \
    void set##NAME(const TYPE& value) { MEMBER = value; };     \

#define DEFINE_ACCESSORS_VAL(NAME, TYPE, MEMBER)              \
    TYPE get##NAME() const { return MEMBER; };                \
    void set##NAME(TYPE value) { MEMBER = value; };           \

#define DEFINE_GET(NAME, TYPE, MEMBER)                        \
    const TYPE& get##NAME() const { return MEMBER; };         \
    TYPE& get##NAME() { return MEMBER; };                     \

// https://de.wikibooks.org/wiki/C%2B%2B-Programmierung:_Entwurfsmuster:_Singleton
#define DEF_SINGLETON(CLASS)                    \
 public:                                        \
    static CLASS& instance()                    \
    {                                           \
       static CLASS _instance;                  \
       return _instance;                        \
    }                                           \
 private:                                       \
    CLASS( const CLASS& ) = delete;             \

#endif //AVIMMMAKROS_H
