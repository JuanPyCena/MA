//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_MAKROS_H
#define CPP_MAKROS_H

#define DEFINE_ACCESSORS_REF(NAME, TYPE, MEMBER)              \
    const TYPE& get##NAME() const { return MEMBER; };          \
    void set##NAME(const TYPE& value) { MEMBER = value; };

#define DEFINE_ACCESSORS_VAL(NAME, TYPE, MEMBER)              \
    TYPE get##NAME() const { return MEMBER; };                \
    void set##NAME(TYPE value) { MEMBER = value; };

#define DEFINE_GET(NAME, TYPE, MEMBER)                        \
    const TYPE& get##NAME() const { return MEMBER; };         \
    TYPE& get##NAME() { return MEMBER; };

#endif //CPP_MAKROS_H
