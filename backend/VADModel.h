#ifndef VADMODEL_H
#define VADMODEL_H

#pragma once
#include <vector>
#include <array>
#include <string>

class VADModel {
public:
    bool load(const std::string& path);

    float forward(const std::array<float, 3>& x);

    bool loadNorm(const std::string& path);

private:
    // веса
    std::vector<std::vector<float>> w1, w2, w3;
    std::vector<float> b1, b2, b3;

    float relu(float x);
    float sigmoid(float x);

    std::vector<float> mean;
    std::vector<float> std;
};

#endif // VADMODEL_H
