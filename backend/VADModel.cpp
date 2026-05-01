#include "VADModel.h"
#include <cmath>
#include <json.hpp>
#include <QFile>
#include <QByteArray>

using json = nlohmann::json;

float VADModel::relu(float x) {
    return x > 0 ? x : 0;
}

float VADModel::sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

bool VADModel::loadNorm(const std::string& path) {
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    json j = json::parse(data.constData());

    mean = j["mean"].get<std::vector<float>>();
    std  = j["std"].get<std::vector<float>>();

    if (mean.size() != 3 || std.size() != 3) {
        return false;
    }

    return true;
}

bool VADModel::load(const std::string& path) {
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll();

    json j = json::parse(data.constData());

    w1 = j["net.0.weight"].get<std::vector<std::vector<float>>>();
    b1 = j["net.0.bias"].get<std::vector<float>>();

    w2 = j["net.2.weight"].get<std::vector<std::vector<float>>>();
    b2 = j["net.2.bias"].get<std::vector<float>>();

    w3 = j["net.4.weight"].get<std::vector<std::vector<float>>>();
    b3 = j["net.4.bias"].get<std::vector<float>>();

    return true;
}

float VADModel::forward(const std::array<float, 3>& input) {
    std::array<float, 3> x;

    for (int i = 0; i < 3; i++) {
        x[i] = (input[i] - mean[i]) / std[i];
    }

    // layer1
    std::vector<float> h1(16);
    for (int i = 0; i < 16; i++) {
        float sum = b1[i];
        for (int j = 0; j < 3; j++)
            sum += w1[i][j] * x[j];
        h1[i] = relu(sum);
    }

    // layer2
    std::vector<float> h2(8);
    for (int i = 0; i < 8; i++) {
        float sum = b2[i];
        for (int j = 0; j < 16; j++)
            sum += w2[i][j] * h1[j];
        h2[i] = relu(sum);
    }

    // output
    float out = b3[0];
    for (int j = 0; j < 8; j++)
        out += w3[0][j] * h2[j];

    return sigmoid(out);
}
