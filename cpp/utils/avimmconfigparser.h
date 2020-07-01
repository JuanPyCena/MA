//
// Created by felix on 6/3/20.
//

#ifndef AVIMMCONFIGPARSER_H
#define AVIMMCONFIGPARSER_H

#include "avimmtypedefs.h"
#include "exprtk.hpp"

// AviBit common includes
#include "avconfig2.h"
#include "avexplicitsingleton.h"

typedef float T; // numeric type (float, double, mpfr etc...)
typedef exprtk::symbol_table<T> symbol_table_t;
typedef exprtk::expression<T> expression_t;
typedef exprtk::parser<T> parser_t;

class AVIMMConfigParser : public AVExplicitSingleton<AVIMMConfigParser>
{
public:
    explicit AVIMMConfigParser() {};
    virtual ~AVIMMConfigParser() = default;
    
    //! Initialise the global configuration data instance
    static AVIMMConfigParser& initializeSingleton()
    { return setSingleton(new AVIMMConfigParser()); }
    
    Matrix calculateTimeDependentMatrices(const AVMatrix<QString> &M, float time_delta=0.0, float variance=1.0)
    {
        int rows = M.getRows();
        int cols = M.getColumns();
        Matrix calculated_matrix(rows, cols);
        
        T dt    = T(time_delta);
        T sigma = T(variance);
        
        symbol_table_t symbol_table;
        symbol_table.add_variable("dt", dt);
        symbol_table.add_variable("sigma", sigma);
        
        expression_t expression;
        expression.register_symbol_table(symbol_table);
        
        // Convert Data element wise
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
            {
                std::string expression_string = M.get(i,j).toStdString();
                if (!m_parser.compile(expression_string, expression))
                {
                    assert(("Could not compile expression: \"" + expression_string + "\"!", false));
                }
                calculated_matrix(i,j) = expression.value();
            }
        return calculated_matrix;
    }

private:
    parser_t m_parser;
};


#endif //AVIMMCONFIGPARSER_H
