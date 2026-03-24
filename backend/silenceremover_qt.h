// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SILENCEREMOVER_QT_H
#define SILENCEREMOVER_QT_H

#include <QString>
#include <QVariantList>

struct SilenceRemoveResult
{
    QString outputPath;
    QVariantList newAnnotations;
    QString error;
};

class SilenceRemoverQt
{
public:
    struct Interval { qint64 t1; qint64 t2; };

    SilenceRemoveResult removeSilenceToWav(const QString &inputPath,
                                           const QVariantList &annotations,
                                           int silenceType,
                                           const QString &outputWavPath);

private:
    static QList<Interval> collectAndMergeSilences(const QVariantList &annotations, int silenceType);
    static QList<Interval> buildKeepIntervals(const QList<Interval> &silences);

    static qint64 removedBefore(const QList<Interval> &silences, qint64 t);
    static QVariantList recalcAnnotations(const QVariantList &annotations,
                                          const QList<Interval> &silences,
                                          int silenceType);

    static bool cutAndWriteWav(const QString &inputPath,
                               const QList<Interval> &keeps,
                               const QString &outputWavPath,
                               QString *error);

    static void writeWavHeader(QDataStream &out, quint32 dataSize, int sampleRate, int channels, int sampleSize);
};

#endif // SILENCEREMOVER_QT_H
