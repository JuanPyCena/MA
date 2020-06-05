//
// Created by felix on 6/4/20.
//

#ifndef QT3_SVN_AVIMMTESTER_H
#define QT3_SVN_AVIMMTESTER_H

#include <avenvironment.h>
#include <avmacros.h>
#include <fstream>
#include <list>

#include "../../filterlib/avimmextendedkalmanfilter.cpp"
#include "../../filterlib/avimmkalmanfilter.cpp"
#include "../../filterlib/avimmestimator.cpp"
#include "../../utils/avimmconfig.cpp"

#include "avimmlibunittesthelperlib_export.h"

#define DELIMITER ","

class AVIMMLIBUNITTESTHELPERLIB_EXPORT AVIMMTester
{
public:
    explicit AVIMMTester(const QString& test_data_file);
    virtual ~AVIMMTester();
    
    void run_sim();
    void perform_single_calculation_step();
    bool dump_results(const QString& imm_data_file);
    
    DEFINE_GET(ResultingStates, QVector<Vector>, m_resulting_states);
    DEFINE_GET(MeasurementData, QVector<Vector>, m_measurement_data);
    DEFINE_GET(MeasurementDataSingleStepCalculation, QVector<Vector>, m_measurement_data_single_step_calc);
    
private:
    Vector load_test_data(const QString& test_data_file);
    std::vector<double> split(const std::string& str, const std::string& delim);
    
    QDateTime m_start_time;
    QString m_test_data_file;
    QVector<QDateTime> m_time_stamps;
    QVector<QDateTime> m_time_stamps_single_step_calc;;
    QVector<Vector> m_resulting_states;
    QVector<Vector> m_measurement_data;
    QVector<Vector> m_measurement_data_single_step_calc;
    AVIMMEstimator *m_avimm_estimator;
};

//--------------------------------------------------------------------------

AVIMMTester::AVIMMTester(const QString& test_data_file)
        : m_test_data_file(test_data_file)
{
    m_start_time = QDateTime::currentDateTime();
    Vector initial_state = load_test_data(m_test_data_file);
    m_measurement_data_single_step_calc = m_measurement_data;
    m_avimm_estimator = new AVIMMEstimator(initial_state);
    m_avimm_estimator->m_test_run = true;
}

//--------------------------------------------------------------------------

AVIMMTester::~AVIMMTester()
{
    delete m_avimm_estimator;
}

//--------------------------------------------------------------------------

bool AVIMMTester::dump_results(const QString& imm_data_file)
{
    Q_UNUSED(imm_data_file)
    //Todo: implement writing of states in csv
    return false;
}

std::vector<double> AVIMMTester::split(const std::string &str, const std::string &delim)
{
    std::vector<double> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(std::stod(token));
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

//--------------------------------------------------------------------------

Vector AVIMMTester::load_test_data(const QString &test_data_file)
{
    Vector initial_state;
    std::ifstream file(test_data_file.toStdString());
    std::string line = "";
    // Iterate through each line and split the content using delimeter
    bool first_data_point = true;
    while (getline(file, line))
    {
        std::vector<double> vec = split(line, DELIMITER);
        // Special handling for first data pint, use this as initial state for setting up IMMEstimator
        if (first_data_point)
        {
            int num_elem = vec.size();
            for (auto& elem : vec)
                initial_state << elem;
            initial_state.resize(num_elem, 1);
            first_data_point = false;
            continue;
        }
        double mseconds_since_start = vec.front();
        QDateTime time_stamp = m_start_time.addMSecs(mseconds_since_start);
        m_time_stamps.push_back(time_stamp);
        // Remove timestamp
        vec.erase(vec.begin());
        
        Vector measurement;
        for (auto& elem : vec)
            measurement << elem;
        
        m_measurement_data.push_back(measurement);
    }
    // Close the File
    file.close();
    
    return initial_state;
}

//--------------------------------------------------------------------------

void AVIMMTester::run_sim()
{
    auto time_iterator = m_time_stamps.constBegin();
    for (const auto &measurement : m_measurement_data)
    {
        // Manipulate time here manually to avoid having "real time" problems
        m_avimm_estimator->m_now = *time_iterator;
        m_avimm_estimator->predictAndUpdate(measurement);
        m_resulting_states.push_back(m_avimm_estimator->getData().x);
        time_iterator++;
    }
}

void AVIMMTester::perform_single_calculation_step()
{
    // Take measurement and time from seperate array
    Vector z = m_measurement_data_single_step_calc.front();
    QDateTime now = m_time_stamps_single_step_calc.front();
    m_measurement_data_single_step_calc.pop_front();
    m_time_stamps_single_step_calc.pop_front();
    
    m_avimm_estimator->m_now = now;
    m_avimm_estimator->predictAndUpdate(z);
}

#endif //QT3_SVN_AVIMMTESTER_H
