//
// Created by Felix on 26.05.2020.
//

#ifndef CPP_TYPEDEFS_H
#define CPP_TYPEDEFS_H

#include <Eigen/Dense>
#include <memory>

typedef Eigen::MatrixXd Matrix;
typedef Eigen::VectorXd Vector;

const Vector DEFAULT_VECTOR = Vector();
const Matrix DEFAULT_MATRIX = Matrix();

enum FilterType {
    KalmanFilter,
    ExtendedKalmanFilter
};

// Template function usufull for cleaning up std::list and std::vector
template<typename Container>
void cleanup_unique_ptr(Container& c) { while(!c.empty()) c.back().reset(), c.pop_back(); }

#endif //CPP_TYPEDEFS_H
