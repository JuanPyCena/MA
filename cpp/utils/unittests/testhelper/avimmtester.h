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
#include "../../utils/avimmconfigparser.h"

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
    static Vector zeroSmallElements(const Vector &M);
    
    DEFINE_GET(ResultingStates, QVector<Vector>, m_resulting_states);
    DEFINE_GET(MeasurementData, QVector<Vector>, m_measurement_data);
    DEFINE_GET(ErrorToMeasurementData, QVector<Vector>, m_error_to_measurement);
    DEFINE_GET(MeasurementDataSingleStepCalculation, QVector<Vector>, m_measurement_data_single_step_calc);
    
    template<typename T1, typename T2>
    static std::pair<bool, double> getMatricesEqual(T1 m1, T2 m2)
    {
        int m1_rows = m1.rows();
        int m1_cols = m1.cols();
        int m2_rows = m2.rows();
        int m2_cols = m2.cols();
        
        // If not same dimensions return false
        if (m1_rows != m2_rows || m1_cols != m2_cols)
            return std::make_pair(false,-1);
        
        // Accept if distance between matrices is below e-6
        static double PROXIMITY = exp(-6);
        return std::make_pair(((m1 - m2).norm() < PROXIMITY), (m1 - m2).norm());
    };
    
private:
    Vector load_test_data(const QString& test_data_file);
    std::vector<double> split(const std::string& str, const std::string& delim);
    
    QDateTime m_start_time;
    QString m_test_data_file;
    QVector<QDateTime> m_time_stamps;
    QVector<QDateTime> m_time_stamps_single_step_calc;;
    QVector<Vector> m_resulting_states;
    QVector<Vector> m_error_to_measurement;
    QVector<Vector> m_measurement_data;
    QVector<Vector> m_measurement_data_single_step_calc;
    AVIMMEstimator *m_avimm_estimator;
};
//--------------------------------------------------------------------------

Vector AVIMMTester::zeroSmallElements(const Vector &M)
{
    // Find all elements below a certain threshold and set them to zero to gain numeical stability
    Vector new_M = M;
    for (int i = 0; i < M.size(); i++) {
        if (M(i) <= MIN_THRESHOLD) {
            new_M(i) = 0.0;
        }
    }
    return new_M;
}
//--------------------------------------------------------------------------

AVIMMTester::AVIMMTester(const QString& test_data_file)
        : m_test_data_file(test_data_file)
{
    m_start_time = QDateTime::currentDateTimeUtc();
    Vector initial_state = this->load_test_data(m_test_data_file);
    m_measurement_data_single_step_calc = m_measurement_data;
    m_avimm_estimator = new AVIMMEstimator(initial_state);
    m_avimm_estimator->m_test_run = true;
    m_avimm_estimator->m_last_calculation= m_start_time;
}

//--------------------------------------------------------------------------

AVIMMTester::~AVIMMTester()
{
    delete m_avimm_estimator;
}

//--------------------------------------------------------------------------

bool AVIMMTester::dump_results(const QString& imm_data_file)
{
    std::ofstream file;
    file.open(imm_data_file.toStdString());
    
    if (!file.is_open())
        return false;
    
    auto time_iterator = m_time_stamps.constBegin();
    for (auto &result : m_resulting_states)
    {
        QString result_string;
        result_string += time_iterator->toString("hh:mm:ss.zzz");
        for (int i = 0; i < result.size(); i++)
        {
            result_string += ", ";
            result_string += AVToString(result.coeffRef(i));
        }
        result_string += "\n";
        time_iterator++;
        
        file << result_string.toStdString();
    }
    
    file.close();
    return true;
}

//--------------------------------------------------------------------------

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
    // Iterate through each line and split the content using delimiter
    bool first_data_point = true;
    if (file.is_open())
    {
        while (getline(file, line)) {
            std::vector<double> vec = split(line, DELIMITER);
            // Special handling for first data pint, use this as initial state for setting up IMMEstimator
            if (first_data_point) {
                // Remove timestamp
                vec.erase(vec.begin());
                initial_state.resize(vec.size(), 1);
                int i = 0;
                for (auto &elem : vec) {
                    initial_state(i) = elem;
                    i++;
                }
                first_data_point = false;
                continue;
            }
            double mseconds_since_start = vec.front();
            QDateTime time_stamp = m_start_time.addMSecs(mseconds_since_start);
            m_time_stamps.push_back(time_stamp);
            // Remove timestamp
            vec.erase(vec.begin());
        
            int i = 0;
            Vector measurement(vec.size(), 1);;
            for (auto &elem : vec) {
                measurement(i) = elem;
                i++;
            }
            m_measurement_data.push_back(measurement);
        }
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
        m_error_to_measurement.push_back(zeroSmallElements(m_avimm_estimator->getData().x - measurement));
        time_iterator++;
    }
}

//--------------------------------------------------------------------------

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
