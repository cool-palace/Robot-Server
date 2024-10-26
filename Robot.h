#ifndef SERVER_ROBOT_H
#define SERVER_ROBOT_H
#include <cmath>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>

class Point {
private:
    double x_, y_, z_;

public:
    Point(double x = 0, double y = 0, double z = 0) : x_(x), y_(y), z_(z) {}
    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }
};

class DH_Matrix {
private:
    double matrix[4][4];

public:
    DH_Matrix(double theta = 0, double a = 0, double d = 0, double alpha = 0);
    DH_Matrix operator*(const DH_Matrix& other) const;
    DH_Matrix& operator*=(const DH_Matrix& other);
    Point coordinates() const { return Point(matrix[0][3], matrix[1][3], matrix[2][3]); }
};

class Robot {
public:
    Robot();
    Robot(const std::vector<double>& a, const std::vector<double>& d, const std::vector<double>& alpha);
    std::string current_position() const;
    void turn(int joint, double angle, double speed = 180);

private:
    DH_Matrix matrix(int joint) const;
    std::vector<double> theta_;
    std::vector<double> a_;
    std::vector<double> d_;
    std::vector<double> alpha_;
};


#endif //SERVER_ROBOT_H
