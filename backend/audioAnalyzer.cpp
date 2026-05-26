#include "AudioAnalyzer.h"
#include "VADModel.h"

#include <fstream>
#include <vector>
#include <cmath>
#include <QVariantMap>
#include <QUrl>
#include <QDebug>
#include <array>

// ================= CONFIG (MATCH PYTHON) =================

static constexpr float SR = 16000.0f;
static constexpr int FRAME_MS = 50;
static constexpr int FRAME_SIZE = (int)(SR * FRAME_MS / 1000.0f);

// ================= WAV FFT (NO WINDOW, MATCH NUMPY) =================

static void computeSpectrum(const std::vector<float>& x,
                            std::vector<float>& spectrum)
{
    int N = (int)x.size();
    int K = N / 2 + 1;

    spectrum.assign(K, 0.0f);

    const float TWO_PI = 2.0f * M_PI;

    for (int k = 0; k < K; ++k)
    {
        float real = 0.0f;
        float imag = 0.0f;

        for (int n = 0; n < N; ++n)
        {
            float phase = TWO_PI * k * n / N;
            real += x[n] * cosf(phase);
            imag -= x[n] * sinf(phase);
        }

        spectrum[k] = sqrtf(real * real + imag * imag);
    }
}

// ================= FEATURES (MATCH PYTHON EXACTLY) =================

static std::array<float, 5> extractFeatures(const std::vector<float>& audio)
{
    const int N = (int)audio.size();

    // ---------- RMS ----------
    float rms = 0.0f;
    for (float v : audio)
        rms += v * v;
    rms = sqrtf(rms / N);

    // ---------- ZCR (np.mean(audio[:-1] * audio[1:] < 0)) ----------
    int zcr_count = 0;
    for (int i = 1; i < N; ++i)
    {
        if (audio[i - 1] * audio[i] < 0.0f)
            zcr_count++;
    }
    float zcr = (float)zcr_count / (float)(N - 1);

    // ---------- FFT ----------
    std::vector<float> spectrum;
    computeSpectrum(audio, spectrum);

    int K = (int)spectrum.size();

    // ---------- Spectral centroid ----------
    float num = 0.0f;
    float den = 0.0f;

    // ---------- flatness ----------
    float log_sum = 0.0f;
    float mean_sum = 0.0f;

    // ---------- band ratio ----------
    float speech_energy = 0.0f;
    float total_energy = 0.0f;

    for (int k = 0; k < K; ++k)
    {
        float freq = (float)k * SR / N;
        float mag = spectrum[k];

        num += freq * mag;
        den += mag;

        log_sum += logf(mag + 1e-8f);
        mean_sum += mag;

        if (freq >= 300.0f && freq <= 3000.0f)
            speech_energy += mag;

        total_energy += mag;
    }

    float centroid = num / (den + 1e-8f);

    float flatness =
        expf(log_sum / K) /
        (mean_sum / K + 1e-8f);

    float band_ratio =
        speech_energy / (total_energy + 1e-8f);

    return { rms, zcr, centroid, flatness, band_ratio };
}

// ================= WAV READER =================

static std::vector<float> readMonoFrame(std::ifstream& file,
                                        int frameSamples,
                                        int channels)
{
    std::vector<int16_t> buffer(frameSamples * channels);

    if (!file.read(reinterpret_cast<char*>(buffer.data()),
                   buffer.size() * sizeof(int16_t)))
    {
        return {};
    }

    std::vector<float> mono(frameSamples);

    for (int i = 0; i < frameSamples; i++)
    {
        int32_t sum = 0;

        for (int c = 0; c < channels; c++)
            sum += buffer[i * channels + c];

        mono[i] = (float(sum / channels) / 32768.0f);
    }

    return mono;
}

// ================= MAIN =================

QVariantList AudioAnalyzer::analyzeFile(const QString &filePath)
{
    VADModel model;

    if (!model.load(":/backend/weights.json"))
        return {};

    if (!model.loadNorm(":/backend/norm.json"))
        return {};

    QString path = filePath;
    if (path.startsWith("file://"))
        path = QUrl(path).toLocalFile();

    std::ifstream file(path.toStdString(), std::ios::binary);
    if (!file) return {};

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::string(header.riff, 4) != "RIFF" ||
        header.audioFormat != 1 ||
        header.bitsPerSample != 16)
        return {};

    const int sampleRate = header.sampleRate;
    const int channels = header.numChannels;

    const int frameSamples = FRAME_SIZE; // 30ms FIXED

    const double frameDurationMs = 1000.0 * frameSamples / sampleRate;

    std::vector<int> labels;
    std::vector<float> probHistory;

    const int smoothWindow = 10;

    // ================= FRAME LOOP =================

    while (true)
    {
        auto mono = readMonoFrame(file, frameSamples, channels);
        if (mono.empty())
            break;

        auto feats = extractFeatures(mono);

        float prob = model.forward(feats);

        probHistory.push_back(prob);
        if (probHistory.size() > smoothWindow)
            probHistory.erase(probHistory.begin());

        float avg = 0.0f;
        for (float p : probHistory)
            avg += p;
        avg /= probHistory.size();

        labels.push_back(avg > 0.5f ? 0 : 1);
    }

    if (labels.empty())
        return {};

    // ================= SMOOTHING =================

    const int window = 40;
    std::vector<int> smooth = labels;

    for (size_t i = 0; i < labels.size(); ++i)
    {
        int count[2] = {0, 0};

        for (int k = -window; k <= window; k++)
        {
            int idx = (int)i + k;
            if (idx >= 0 && idx < (int)labels.size())
                count[labels[idx]]++;
        }

        smooth[i] = (count[1] > count[0]) ? 1 : 0;
    }

    // ================= SEGMENTS =================

    QVariantList result;

    int current = smooth[0];
    double t1 = 0.0;

    for (size_t i = 1; i < smooth.size(); i++)
    {
        if (smooth[i] != current)
        {
            double t2 = i * frameDurationMs;

            QVariantMap seg;
            seg["t1"] = t1;
            seg["t2"] = t2;
            seg["type"] = current;

            result.append(seg);

            t1 = t2;
            current = smooth[i];
        }
    }

    QVariantMap last;
    last["t1"] = t1;
    last["t2"] = smooth.size() * frameDurationMs;
    last["type"] = current;

    result.append(last);

    // ================= PADDING =================

    const double pad = 400.0;

    for (int i = 0; i < result.size(); i++)
    {
        QVariantMap seg = result[i].toMap();

        if (seg["type"].toInt() == 1)
        {
            double t1 = seg["t1"].toDouble() - pad;
            double t2 = seg["t2"].toDouble() + pad;

            seg["t1"] = std::max(0.0, t1);
            seg["t2"] = t2;

            result[i] = seg;
        }
    }

    return result;
}
