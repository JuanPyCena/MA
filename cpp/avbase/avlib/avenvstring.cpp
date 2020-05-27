///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Implementation of the EnvString class
*/

// local includes
#include "avenvironment.h"
#include "avenvstring.h"


///////////////////////////////////////////////////////////////////////////////

AVEnvString::AVEnvString()
{
    m_reg_exp_env_name = QRegExp("\\$\\([a-zA-Z](_*(\\d|[a-zA-Z]))*\\)");
}

///////////////////////////////////////////////////////////////////////////////

AVEnvString::AVEnvString(const QString& env_string)
{
    m_reg_exp_env_name = QRegExp("\\$\\([a-zA-Z](_*(\\d|[a-zA-Z]))*\\)");
    setEnvString(env_string);
}

///////////////////////////////////////////////////////////////////////////////

bool AVEnvString::isEmpty() const
{
    return m_env_string.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////

void AVEnvString::setEnvString(const QString &new_env_string)
{
    m_env_string = new_env_string;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvString::envString() const
{
    return m_env_string;
}

///////////////////////////////////////////////////////////////////////////////

QString AVEnvString::expandEnvString() const
{
    if(m_env_string.isEmpty()) return QString::null;

    int env_pos    = 0;
    int pos_in_string = 0;

    QString exp_env_string, env_var;
    exp_env_string.reserve(m_env_string.capacity());

    for(;;)
    {
        env_pos = m_reg_exp_env_name.indexIn(m_env_string, pos_in_string, QRegExp::CaretAtOffset);
        if(env_pos == -1)
        {
            exp_env_string += m_env_string.mid(pos_in_string);
            return exp_env_string;
        }

        // copy envString from last pos til the beginning of the environment variable exp
        exp_env_string += m_env_string.mid(pos_in_string, env_pos - pos_in_string);
        pos_in_string = env_pos;

        QString env_name = m_reg_exp_env_name.cap();
        pos_in_string += env_name.length();

        // remove "$(", ")" from env var name
        env_name = env_name.mid(2, env_name.length() - 3);

        if(!getEnv(env_name, env_var))
        {
            AVLogError << "AVEnvString::expandEnvString: "
                       << "reading environment variable ("
                       << env_name
                       << ") failed when expanding EnvString("
                       << m_env_string
                       << ").";
            return m_env_string;
        }

        exp_env_string += env_var;
    }
    return QString();
}

///////////////////////////////////////////////////////////////////////////////

bool AVEnvString::getEnv(const QString& name, QString& value) const
{

    value = AVEnvironment::getEnv(name, true, AVEnvironment::NoPrefix);
    bool rc = !value.isNull();

    return rc;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const AVEnvString& arg, bool enable_escape)
{
    return AVToString(arg.envString(), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, AVEnvString& arg)
{
    QString env_string;
    if (!AVFromString(str, env_string)) return false;
    arg.setEnvString(env_string);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AV_REGISTER_METATYPE(AVEnvString)

///////////////////////////////////////////////////////////////////////////////

// End of file
