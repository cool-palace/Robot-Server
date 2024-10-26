#include "Robot.h"

DH_Matrix::DH_Matrix(double theta_, double a, double d, double alpha_) {
    double theta = theta_ * M_PI / 180.0;
    double alpha = alpha_ * M_PI / 180.0;

    matrix[0][0] = cos(theta);
    matrix[0][1] = -sin(theta) * cos(alpha);
    matrix[0][2] = sin(theta) * sin(alpha);
    matrix[0][3] = a * cos(theta);

    matrix[1][0] = sin(theta);
    matrix[1][1] = cos(theta) * cos(alpha);
    matrix[1][2] = -cos(theta) * sin(alpha);
    matrix[1][3] = a * sin(theta);

    matrix[2][0] = 0;
    matrix[2][1] = sin(alpha);
    matrix[2][2] = cos(alpha);
    matrix[2][3] = d;

    matrix[3][0] = 0;
    matrix[3][1] = 0;
    matrix[3][2] = 0;
    matrix[3][3] = 1;
}

DH_Matrix DH_Matrix::operator*(const DH_Matrix &other) const {
    DH_Matrix result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.matrix[i][j] = 0;
            for (int k = 0; k < 4; ++k) {
                result.matrix[i][j] += matrix[i][k] * other.matrix[k][j];
            }
        }
    }
    return result;
}

DH_Matrix &DH_Matrix::operator*=(const DH_Matrix &other) {
    auto result = (*this) * other;
    std::memcpy(matrix, result.matrix, 4 * 4 * sizeof(double));
    return *this;
}

Robot::Robot() {
    // Default values for tests
    a_ = {0, -0.8, -0.59, 0, 0, 0};
    d_ = {0.213, 0.19, -0.16, 0.25, 0.28, 0.25};
    alpha_ = {90, 0, 0, 90, -90};
    theta_ = {15, -50, -60, 95, 50, 0};
}

Robot::Robot(const std::vector<double>& a, const std::vector<double>& d, const std::vector<double>& alpha)
        : a_(a), d_(d), alpha_(alpha), theta_(a.size(), 0) {
}

std::string Robot::current_position() const {
    DH_Matrix result = matrix(0);
    for (int i = 1; i < 6; ++i) {
        result *= matrix(i);
    }
    Point p = result.coordinates();
    std::ostringstream oss;
    oss << "x: " << p.x() << ", y: " << p.y() << ", z: " << p.z();
    return oss.str();
}

DH_Matrix Robot::matrix(int joint) const {
    return DH_Matrix(theta_[joint], a_[joint], d_[joint], alpha_[joint]);
}

void Robot::turn(int joint, double angle) {
    theta_[joint] += angle;
    if (theta_[joint] > 180) {
        theta_[joint] -= 360;
    } else if (theta_[joint] < -180) {
        theta_[joint] += 360;
    }
}
