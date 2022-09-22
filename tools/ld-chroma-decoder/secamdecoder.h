/************************************************************************

    monodecoder.h

    ld-chroma-decoder - Colourisation filter for ld-decode
    Copyright (C) 2019-2021 Adam Sampson

    This file is part of ld-decode-tools.

    ld-chroma-decoder is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************/

#ifndef SECAMDECODER_H
#define SECAMDECODER_H

#include <QObject>
#include <QAtomicInt>
#include <QThread>
#include <QDebug>

#include "componentframe.h"
#include "lddecodemetadata.h"
#include "sourcevideo.h"

#include "comb.h"
#include "decoder.h"
#include "firfilter.h"
#include "sourcefield.h"

class DecoderPool;

// SÉCAM chroma decoder
class SecamDecoder : public Decoder {
public:
    SecamDecoder();
    bool configure(const LdDecodeMetaData::VideoParameters &videoParameters) override;
    QThread *makeThread(QAtomicInt& abort, DecoderPool& decoderPool) override;

private:
    Configuration config;
    const FIRFilter<std::array<double, 17>> dbFilter;
    const FIRFilter<std::array<double, 17>> drFilter;
};

class SecamThread : public DecoderThread
{
    Q_OBJECT
public:
    explicit SecamThread(QAtomicInt &abort, DecoderPool &decoderPool,
                       const SecamDecoder::Configuration &config,
                       QObject *parent = nullptr);

protected:
    void decodeFrames(const QVector<SourceField> &inputFields, qint32 startIndex, qint32 endIndex,
                      QVector<ComponentFrame> &componentFrames) override;

private:
    void decodeFrame(const SourceField &firstField, const SourceField &secondField, ComponentFrame &componentFrame);

    // Settings
    const SecamDecoder::Configuration &config;
};

#endif // SECAMDECODER_H