///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
    \file
    \author    DI Wemmer Alexander <a.wemmer@avibit.com>
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Matrix of arbitrary types
*/

#if !defined(AVMATRIX_H_INCLUDED)
#define AVMATRIX_H_INCLUDED

// QT includes
#include <QMap>
#include <QStringList>
#include <QTextStream>
#include <QBuffer>

#include "avlib_export.h"
#include "avfromtostring.h"
#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////
//! A Matrix template class, holding a 2-dimensional matrix of arbitrary types.
template <class T>
class AVMatrix
{
public:

    //! Constructor.
    //! The init_value is used to initialize all allocated matrix fields and
    //! should not be stored as a normal value into the matrix.
    AVMatrix(int rows, int columns, const T& init_value)
    {
        m_rows = rows;
        m_columns = columns;
        m_init_value = init_value;
        m_matrix = 0;

        createMatrix();
        AVASSERT(m_matrix);
    }

    //! Default Constructor
    AVMatrix()
    {
        m_rows = 0;
        m_columns = 0;
        m_init_value = T();
        m_matrix = 0;

        createMatrix();
        AVASSERT(m_matrix);
    }

    //! Copy Constructor
    AVMatrix(const AVMatrix& other)
    {
        m_matrix = 0;
        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_init_value = other.m_init_value;
        m_row_descriptions = other.m_row_descriptions;
        m_column_descriptions = other.m_column_descriptions;
        createMatrix();
        copyMatrix(other);
    }

    //! Destructor
    virtual ~AVMatrix()
    {
        deleteMatrix();
    }

    //! Assignment operator
    AVMatrix& operator=(const AVMatrix& other)
    {
        if (&other == this) return *this;

        deleteMatrix();
        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_init_value = other.m_init_value;
        m_row_descriptions = other.m_row_descriptions;
        m_column_descriptions = other.m_column_descriptions;
        createMatrix();
        copyMatrix(other);

        return *this;
    }

    //! Equality operator
    bool operator==(const AVMatrix& other) const
    {
        if (&other == this) return true;

        if (other.m_rows != m_rows) return false;
        if (other.m_columns != m_columns) return false;
        if (other.m_init_value != m_init_value) return false;

        // TODO
//         if (other.m_row_descriptions != m_row_descriptions) return false;
//         if (other.m_column_descriptions != m_column_descriptions) return false;

        for(int row_count = 0; row_count < m_rows; ++row_count)
            for(int col_count = 0; col_count < m_columns; ++col_count)
                if (m_matrix[row_count][col_count] != other.m_matrix[row_count][col_count])
                    return false;

        return true;
    }

    //! Inequality operator
    bool operator!=(const AVMatrix& other) const
    {
        return !operator==(other);
    }

    //! Returns true if the given row description is known, false otherwise
    bool hasRowDescription(const QString& row_description) const
    { return m_row_descriptions.contains(row_description); }

    //! Returns true if the given column description is known, false otherwise
    bool hasColumnDescription(const QString& column_description) const
    { return m_column_descriptions.contains(column_description); }

    //! returns the requested field
    const T& get(int row, int column) const
    {
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        return m_matrix[row][column];
    }

    //! returns the requested field based on the row number and col description
    const T& get(int row, const QString& column_description) const
    {
        QMap<QString, int>::const_iterator itCol = m_column_descriptions.find(column_description);
        AVASSERT(itCol != m_column_descriptions.constEnd());
        int column = *itCol;
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        return m_matrix[row][column];
    }

    //! returns the requested field based on the row description and col number
    const T& get(const QString& row_description, int column) const
    {
        QMap<QString, int>::const_iterator itRow = m_row_descriptions.find(row_description);
        AVASSERT(itRow != m_row_descriptions.constEnd());
        int row = *itRow;
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        return m_matrix[row][column];
    }

    //! returns the requested field based on the row/col descriptions
    const T& get(const QString& row_description,
                 const QString& column_description) const
    {
        QMap<QString, int>::const_iterator itRow = m_row_descriptions.find(row_description);
        AVASSERT(itRow != m_row_descriptions.constEnd());
        QMap<QString, int>::const_iterator itCol = m_column_descriptions.find(column_description);
        AVASSERT(itCol != m_column_descriptions.constEnd());
        int row = *itRow;
        int column = *itCol;
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        return m_matrix[row][column];
    }

    //! sets the requested field
    void set(int row, int column, const T& value)
    {
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        m_matrix[row][column] = value;
    }

    //! sets the requested field based on the row number and col description
    void set(int row, const QString& column_description, const T& value)
    {
        QMap<QString, int>::const_iterator itCol = m_column_descriptions.find(column_description);
        AVASSERT(itCol != m_column_descriptions.constEnd());
        int column = *itCol;
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        m_matrix[row][column] = value;
    }

    //! sets the requested field based on the row description and row number
    void set(const QString& row_description, int column, const T& value)
    {
        QMap<QString, int>::const_iterator itRow = m_row_descriptions.find(row_description);
        AVASSERT(itRow != m_row_descriptions.constEnd());
        int row = *itRow;
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        m_matrix[row][column] = value;
    }

    //! sets the requested field based on the row/col descriptions
    void set(const QString& row_description,
             const QString& column_description,
             const T& value)
    {
        QMap<QString, int>::const_iterator itRow = m_row_descriptions.find(row_description);
        AVASSERT(itRow != m_row_descriptions.constEnd());
        QMap<QString, int>::const_iterator itCol = m_column_descriptions.find(column_description);
        AVASSERT(itCol != m_column_descriptions.constEnd());
        int row = *itRow;
        int column = *itCol;
        AVASSERT(row < m_rows);
        AVASSERT(column < m_columns);
        m_matrix[row][column] = value;
    }

    //! returns the init_value which is used to initialize all allocated matrix fields
    const T& getInitVal() const { return m_init_value; }

    //! sets the row descriptions
    bool setRowDescriptions(const QMap<QString, int> row_descriptions)
    {
        if (row_descriptions.count() != m_rows) return false;
        m_row_descriptions = row_descriptions;
        return true;
    }

    //! sets the column descriptions
    bool setColumnDescriptions(const QMap<QString, int> column_descriptions)
    {
        if (column_descriptions.count() != m_columns) return false;
        m_column_descriptions = column_descriptions;
        return true;
    }

    //! returns whether row descriptions are set
    inline bool hasRowDescriptions() const
    {
        return (m_row_descriptions.count() != 0);
    }

    //! returns whether column descriptions are set
    inline bool hasColumnDescriptions() const
    {
        return (m_column_descriptions.count() != 0);
    }

    //! returns the row descriptions
    const QMap<QString, int>& getRowDescriptions() const
    {
        return m_row_descriptions;
    }

    //! returns the column descriptions
    const QMap<QString, int>& getColumnDescriptions() const
    {
        return m_column_descriptions;
    }

    //! change description of a row
    void setRowDescription(int row, const QString &description)
    {
        m_row_descriptions[description] = row;
    }

    //! change description of a column
    void setColumnDescription(int column, const QString &description)
    {
        m_column_descriptions[description] = column;
    }

    //! returns the description for the given row index (O(n))
    QString getRowDescription(int row) const
    {
        QMap<QString, int>::const_iterator iter = m_row_descriptions.begin();
        while (iter != m_row_descriptions.end())
        {
            if (iter.value() == row) return iter.key();
            ++iter;
        }
        return QString::null;
    }

    //! returns the description for the given column index (O(n))
    QString getColumnDescription(int column) const
    {
        QMap<QString, int>::const_iterator iter = m_column_descriptions.begin();
        while (iter != m_column_descriptions.end())
        {
            if (iter.value() == column) return iter.key();
            ++iter;
        }
        return QString::null;
    }

    //! deletes the matrix's contents and rebiulds the matrix memory with the new size
    /*! row and column descriptions are also cleared.
     */
    void reset(int rows, int columns)
    {
        deleteMatrix();
        m_rows = rows;
        m_columns = columns;
        m_row_descriptions.clear();
        m_column_descriptions.clear();
        createMatrix();
        AVASSERT(m_matrix);
    }

    //! deletes the matrix's contents and rebiulds the matrix memory with the new size
    /*! row and column descriptions are also cleared.
     */
    void reset(int rows, int columns, const T& init_value)
    {
        m_init_value = init_value;
        reset(rows, columns);
    }

    //! returns the number of rows of the matrix
    int getRows() const { return m_rows; }

    //! returns the number of columns of the matrix
    int getColumns() const { return m_columns; }

    //! reads a matrix from the given string list
    /*! Each entry in the string list represents a row. The elements in each
        row are separated by colSep. If allowEmptyEntries is set to true,
        the empty entries will be filled with init-val. Reading supports all
        types T that are supported by QTextStream operator>>.
        \return false if one or more of the following conditions are met:
                1. string list matrix is empty,
                2. empty entries found although not allowed,
                3. conversion of an entry to specific type T failed,
                4. number of row descriptions does not match number of rows,
                5. number of col descriptions does not match number of cols.
                return true otherwise.
        \warning The string list must not contain white spaces
        \sa getInitVal()
     */
    bool read(const QStringList& matrix, const QString& colSep,
              bool allowEmptyEntries = false, bool matrix_hasRowDescriptions = false,
              bool matrix_hasColumnDescriptions = false)
    {
        AVASSERT(! colSep.isEmpty());
        deleteMatrix();
        m_matrix = 0;
        QStringList modifiedMatrix = matrix;
        if (! parseAndRemoveDescriptions(
                modifiedMatrix, colSep, matrix_hasRowDescriptions, matrix_hasColumnDescriptions))
        {
            AVLogger->Write(LOG_WARNING, "AVMatrix::read(): could not read "
                            "descriptions");
            return false;
        }

        m_rows = modifiedMatrix.count();
        if (m_rows == 0)
        {
            AVLogger->Write(LOG_WARNING, "AVMatrix::read(): empty matrix");
            return false;
        }
        m_columns = modifiedMatrix[0].split(colSep,  QString::KeepEmptyParts).count();
        createMatrix();

        QStringList row;
        for (int row_count = 0; row_count < m_rows; ++row_count)
        {
            row = modifiedMatrix[row_count].split(colSep, QString::KeepEmptyParts);
            if (row.count() != m_columns && ! allowEmptyEntries)
            {
                AVLogger->Write(LOG_WARNING, "AVMatrix::read(): row %d from %d "
                                "has %d cols instead of %d. Row='%s'",
                                row_count, m_rows, row.count(), m_columns,
                                qPrintable(modifiedMatrix[row_count]));
                return false;
            }
            for (int col_count = 0; col_count < m_columns; ++col_count)
            {
                QString entry = row[col_count];
                // remove spaces because otherwise stream device status
                // can't be used this way
                entry.remove(' ');

                T value=m_init_value;

                if (entry.isEmpty())
                {
                    if(!allowEmptyEntries)
                    {
                        AVLogger->Write(LOG_WARNING, "AVMatrix::read(): empty col "
                                    "%d in row %d Row='%s'",
                                    col_count, row_count,
                                    qPrintable(modifiedMatrix[row_count]));
                        return false;
                    }
                }
                else
                {
                    QTextStream stream(&entry, QIODevice::ReadOnly);
                    stream >> value;
                    // a read-error implies that everything could be read from the
                    // stream and therefore the stream is empty after the operation
                    if (stream.status() != QTextStream::Ok || !stream.atEnd())
                    {
                        AVLogger->Write(LOG_WARNING, "AVMatrix::read(): Could not "
                                        "convert string '%s' to specific type",
                                        qPrintable(entry));
                        return false;
                    }
                }
                m_matrix[row_count][col_count] = value;
            }
        }

        return true;
    }

    //! writes the matrix to the given string list
    /*! The string list will not be cleared. Writing supports all
        types T that are supported by QTextStream operator<<.

        \todo Add support for writing descriptions
     */
    bool write(QStringList& matrix, const QString& colSep, bool includeDescription = false,
               int fieldWidth = 0, QTextStream::FieldAlignment alignment = QTextStream::AlignLeft,
               QChar padChar = QChar(' ')) const
    {
        AVASSERT(! colSep.isEmpty());

        // TODO: add support for writing descriptions

        if (includeDescription)
        {
            QString row;
            QTextStream stream(&row, QIODevice::WriteOnly);
            stream.setFieldAlignment(alignment);
            stream.setFieldWidth(fieldWidth);
            stream.setPadChar(padChar);
            int maxLength = 0;
            foreach (const QString& rowDescription, this->getRowDescriptions().keys())
                maxLength = AVmax(maxLength, rowDescription.length());
            QString fillString;
            fillString.fill(' ', maxLength);
            stream << fillString << colSep;
            for (int col_count = 0; col_count < m_columns; ++col_count)
            {
                stream << this->getColumnDescriptions().key(col_count);
                if (col_count != m_columns-1) stream << colSep;
            }
             matrix.push_back(row);
        }

        for (int row_count = 0; row_count < m_rows; ++row_count)
        {
            QString row;
            QTextStream stream(&row, QIODevice::WriteOnly);
            stream.setFieldWidth(fieldWidth);
            stream.setFieldAlignment(alignment);
            stream.setPadChar(padChar);
            if (includeDescription)
            {
                QString rowDesc = this->getRowDescriptions().key(row_count) ;
                stream << rowDesc  << colSep;
            }
            for (int col_count = 0; col_count < m_columns; ++col_count)
            {
                stream << m_matrix[row_count][col_count];
                if (col_count != m_columns-1) stream << colSep;
            }
            matrix.push_back(row);
        }
        return true;
    }

    //! writes the matrix to the given string list with entries sorted by keys
    /*! The string list will not be cleared. Writing supports all
        types T that are supported by QTextStream operator<<.

        \todo Add support for writing descriptions
     */
    bool writeKeySorted(QStringList& matrix, const QString& colSep, bool includeDescription = false,
               int fieldWidth = 0, QTextStream::FieldAlignment alignment = QTextStream::AlignLeft,
               QChar padChar = QChar(' ')) const
    {
        AVASSERT(! colSep.isEmpty());

        // TODO: add support for writing descriptions

        if (includeDescription)
        {
            QString row;
            QTextStream stream(&row, QIODevice::WriteOnly);
            stream.setFieldAlignment(alignment);
            stream.setFieldWidth(fieldWidth);
            stream.setPadChar(padChar);
            int maxLength = 0;
            foreach (const QString& rowDescription, this->getRowDescriptions().keys())
                maxLength = AVmax(maxLength, rowDescription.length());
            QString fillString;
            fillString.fill(' ', maxLength);
            stream << fillString << colSep;
            QStringList col_keys = m_column_descriptions.keys();
            int counter = 0;
            foreach (const QString& col_key, col_keys)
            {
                stream << col_key;
                if (counter != m_columns-1) stream << colSep;
                counter++;
            }
             matrix.push_back(row);
        }
#if defined(Q_CC_GNU)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
        foreach (const QString row_key, m_row_descriptions.keys())
        {
            QString row;
            QTextStream stream(&row, QIODevice::WriteOnly);
            stream.setFieldWidth(fieldWidth);
            stream.setFieldAlignment(alignment);
            stream.setPadChar(padChar);
            if (includeDescription)
            {
                stream << row_key << colSep;
            }
            int counter = 0;
            foreach (const QString& col_key, m_column_descriptions.keys())
            {
                stream << this->get(row_key, col_key);
                if (counter != m_columns-1) stream << colSep;
                counter++;
            }
            matrix.push_back(row);
        }
#if defined(Q_CC_GNU)
#pragma GCC diagnostic pop
#endif
        return true;
    }

protected:

    //! builds up the matrix memory
    void createMatrix()
    {
        AVASSERT(m_matrix == 0);

        m_matrix = new T*[m_rows];
        AVASSERT(m_matrix);

        for(int row_count = 0; row_count < m_rows; ++row_count)
        {
            m_matrix[row_count] = new T[m_columns];
            AVASSERT(m_matrix[row_count]);

            for( int col_count = 0; col_count < m_columns; ++col_count)
                m_matrix[row_count][col_count] = m_init_value;
        }
    }

    //! frees the matrix memory
    void deleteMatrix()
    {
        if (m_matrix == 0) return;

        for( int row_count = 0; row_count < m_rows; ++row_count)
            delete[] m_matrix[row_count];
        delete[] m_matrix;

        m_matrix = 0;
    }

    //! copies the values from the given matrix
    void copyMatrix(const AVMatrix& other)
    {
        AVASSERT(other.m_rows == m_rows);
        AVASSERT(other.m_columns == m_columns);

        //FIXXME copy memory block ??
        for(int row_count = 0; row_count < m_rows; ++row_count)
            for(int col_count = 0; col_count < m_columns; ++col_count)
                m_matrix[row_count][col_count] = other.m_matrix[row_count][col_count];
    }

    //! parses the descriptions and removes them from the given matrix
    bool parseAndRemoveDescriptions(QStringList& matrix, const QString& colSep,
                                    bool matrix_hasRowDescriptions,
                                    bool matrix_hasColumnDescriptions)
    {
        // nothing to do
        if (! matrix_hasRowDescriptions && ! matrix_hasColumnDescriptions) return true;

        int rows = matrix.count();
        if (rows < 1) return false;
        if (rows < 2 && matrix_hasColumnDescriptions) return false;

        QStringList row;
        for (int row_count = 0; row_count < rows; ++row_count)
        {
            QString& rowStr = matrix[row_count];
            row = rowStr.split(colSep, QString::KeepEmptyParts);
            if (row.count() < 1 ||
                (row.count() < 2 && matrix_hasRowDescriptions)) return false;

            if (row_count == 0 && matrix_hasColumnDescriptions)
            {
                for (int col_count = matrix_hasRowDescriptions ? 1 : 0;
                     col_count < row.count(); ++col_count)
                {
                    m_column_descriptions[row[col_count]] =
                        col_count - (matrix_hasRowDescriptions ? 1 : 0);
                }
                continue;
            }
            if (! matrix_hasRowDescriptions) break;
            m_row_descriptions[row[0]] =
                row_count - (matrix_hasColumnDescriptions ? 1 : 0);
            // remove row description
            row.pop_front();
            rowStr = row.join(colSep);
        }

        // remove col descriptions
        if (matrix_hasColumnDescriptions) matrix.pop_front();
        return true;
    }

    //-----

    int m_rows;
    int m_columns;
    T m_init_value;
    T **m_matrix;
    QMap<QString, int> m_row_descriptions;
    QMap<QString, int> m_column_descriptions;
};

/////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVToString(const AVMatrix<T>& arg, bool enable_escape = false)
{
    Q_UNUSED(enable_escape);
    int rows = arg.getRows(), columns = arg.getColumns();
    QStringList rows_strings;

    // add column descriptions
    if (columns > 0)
    {
        QStringList column_description_values;
        column_description_values.append("");
        for (int col_count = 0; col_count < columns; ++col_count)
        {
            QString column_description = arg.getColumnDescription(col_count);
            if (!column_description.isEmpty())
                column_description_values.append(AVToString(column_description, true));
            else
                column_description_values.append("");
        }
        rows_strings.append(avfromtostring::DELIMITER_START +
                            column_description_values.join(QString(avfromtostring::SEPARATOR) + " ") +
                            avfromtostring::DELIMITER_END);
    }

    // add rows and row descriptions
    for (int row_count = 0; row_count < rows; ++row_count)
    {
        QStringList row_values;
        QString row_description = arg.getRowDescription(row_count);
        if (!row_description.isEmpty())
            row_values.append(AVToString(row_description, true));
        else
            row_values.append("");
        for (int col_count = 0; col_count < columns; ++col_count)
        {
            QString value_str = AVToString(arg.get(row_count, col_count), true);
            if (value_str != "\"\"")
                row_values.append(value_str);
            else
                row_values.append("");
        }
        rows_strings.append(avfromtostring::DELIMITER_START +
                            row_values.join(QString(avfromtostring::SEPARATOR) + " ") +
                            avfromtostring::DELIMITER_END);
    }
    return avfromtostring::DELIMITER_START +
           rows_strings.join(QString(avfromtostring::SEPARATOR) + " ") +
           avfromtostring::DELIMITER_END;
}

/////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVFromString(const QString& str, AVMatrix<T>& arg)
{
    QStringList row_strings;
    if (!avfromtostring::nestedSplit(row_strings, avfromtostring::SEPARATOR, str, true))
        return false;
    // empty matrix
    if (row_strings.isEmpty())
    {
        arg.reset(0, 0);
        return true;
    }

    // fetch column descriptions
    QStringList column_descriptions;
    if (!avfromtostring::nestedSplit(column_descriptions, avfromtostring::SEPARATOR, row_strings[0], true))
        return false;
    if (column_descriptions.count() < 1)
        return false;

    // init and resize matrix
    int rows = row_strings.count() - 1, columns = column_descriptions.count() - 1;
    arg.reset(rows, columns);

    // apply column descriptions
    for (int col_count = 1; col_count < column_descriptions.count(); ++col_count)
    {
        QString column_description;
        if (!AVFromString(column_descriptions[col_count], column_description))
            return false;
        arg.setColumnDescription(col_count - 1, column_description);
    }
    row_strings.pop_front();

    // fetch data and row descriptions
    for (int row_count = 0; row_count < rows; ++row_count)
    {
        QStringList values;
        if (!avfromtostring::nestedSplit(values, avfromtostring::SEPARATOR, row_strings[row_count], true))
            return false;
        if (values.count() != (columns + 1))
            return false;
        QString row_description;
        if (!AVFromString(values[0], row_description))
            return false;
        arg.setRowDescription(row_count, row_description);
        for (int col_count = 1; col_count < values.count(); ++col_count)
        {
            T value;
            if (!AVFromString(values[col_count], value))
                return false;
            arg.set(row_count, col_count - 1, value);
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////

// End of file
