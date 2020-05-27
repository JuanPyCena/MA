#pragma once

#include "avauthlib_export.h"

#include <QSet>
#include <QString>

namespace av
{
namespace auth
{
using StringUSet = QSet<QString>;

/**
 * @brief The IdToken struct is a public subset of the ID token, and is exposed via the Authenticator API.
 */
struct AVAUTHLIB_EXPORT IdToken
{
    QString    preferred_username;
    QString    given_name;
    QString    family_name;
    QString    email;
    StringUSet roles;
};  // struct IdToken

}  // namespace auth
}  // namespace av
