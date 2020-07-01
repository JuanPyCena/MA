//
// Created by Felix on 26.05.2020.
//

#ifndef AVIMMTYPEDEFS_H
#define AVIMMTYPEDEFS_H

#include <Eigen/Dense>
#include <memory>
#include <string>
#include <list>
#include <vector>

typedef Eigen::MatrixXd Matrix;
typedef Eigen::Matrix<double, 6, 6> Matrix6d;
typedef Eigen::Matrix<double, 2, 2> Matrix2d;
typedef Eigen::Matrix<double, 4, 4> Matrix4d;
typedef Eigen::Matrix<double, 6, 2> Matrix62d;
typedef Eigen::Matrix<double, 4, 6> Matrix46d;
typedef Eigen::Matrix<double, 6, 1> Vector6d;
typedef Eigen::Matrix<double, 4, 1> Vector4d;
typedef Eigen::Matrix<double, 2, 1> Vector2d;
typedef Eigen::VectorXd Vector;

const Vector DEFAULT_VECTOR = Vector();
const Matrix DEFAULT_MATRIX = Matrix();

enum FilterType {
    KalmanFilter,
    ExtendedKalmanFilter
};

// Template function usufull for cleaning up std::list and std::vector
template<typename Container>
void cleanup_ptr_container(Container& c) { while(!c.empty()) delete c.back(), c.pop_back(); }

#endif //AVIMMTYPEDEFS_H
