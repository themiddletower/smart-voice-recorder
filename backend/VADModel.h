#pragma once

#include <vector>
#include <array>
#include <string>

class VADModel
{
public:

    bool load(const std::string& path);
    bool loadNorm(const std::string& path);

    float forward(const std::array<float, 5>& input);

private:

    float relu(float x);
    float sigmoid(float x);

    std::vector<float> mean;
    std::vector<float> std;

    std::vector<std::vector<float>> w1;
    std::vector<float> b1;

    std::vector<std::vector<float>> w2;
    std::vector<float> b2;

    std::vector<std::vector<float>> w3;
    std::vector<float> b3;

    std::vector<std::vector<float>> w4;
    std::vector<float> b4;
};
