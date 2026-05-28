#include "VADModel.h"

#include <cmath>
#include <json.hpp>
#include <QFile>
#include <QByteArray>
#include <array>

using json = nlohmann::json;

float VADModel::relu(float x)
{
    return x > 0.0f ? x : 0.0f;
}

float VADModel::sigmoid(float x)
{
    return 1.0f / (1.0f + std::exp(-x));
}

bool VADModel::loadNorm(const std::string& path)
{
    QFile file(QString::fromStdString(path));

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();

    json j = json::parse(data.constData());

    mean = j["mean"].get<std::vector<float>>();
    std  = j["std"].get<std::vector<float>>();

    if (mean.size() != 5 || std.size() != 5)
        return false;

    return true;
}

bool VADModel::load(const std::string& path)
{
    QFile file(QString::fromStdString(path));

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();

    json j = json::parse(data.constData());

    // layer1
    w1 = j["net.0.weight"].get<std::vector<std::vector<float>>>();
    b1 = j["net.0.bias"].get<std::vector<float>>();

    // layer2
    w2 = j["net.3.weight"].get<std::vector<std::vector<float>>>();
    b2 = j["net.3.bias"].get<std::vector<float>>();

    // layer3
    w3 = j["net.6.weight"].get<std::vector<std::vector<float>>>();
    b3 = j["net.6.bias"].get<std::vector<float>>();

    // output
    w4 = j["net.8.weight"].get<std::vector<std::vector<float>>>();
    b4 = j["net.8.bias"].get<std::vector<float>>();

    return true;
}

static std::vector<float> linear(
    const std::vector<std::vector<float>>& W,
    const std::vector<float>& B,
    const std::vector<float>& X)
{
    std::vector<float> out(W.size());

    for (size_t i = 0; i < W.size(); ++i) {

        float sum = B[i];

        for (size_t j = 0; j < X.size(); ++j)
            sum += W[i][j] * X[j];

        out[i] = sum;
    }

    return out;
}

float VADModel::forward(const std::array<float, 5>& input)
{
    std::vector<float> x(5);

    for (int i = 0; i < 5; ++i)
        x[i] = (input[i] - mean[i]) / std[i];

    auto h1 = linear(w1, b1, x);

    for (float& v : h1)
        v = relu(v);

    auto h2 = linear(w2, b2, h1);

    for (float& v : h2)
        v = relu(v);

    auto h3 = linear(w3, b3, h2);

    for (float& v : h3)
        v = relu(v);

    auto out = linear(w4, b4, h3);

    return sigmoid(out[0]);
}
