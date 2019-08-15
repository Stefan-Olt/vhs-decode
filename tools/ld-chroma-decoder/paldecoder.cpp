/************************************************************************

    paldecoder.cpp

    ld-chroma-decoder - Colourisation filter for ld-decode
    Copyright (C) 2018-2019 Simon Inns
    Copyright (C) 2019 Adam Sampson

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

#include "paldecoder.h"

#include "decoderpool.h"

PalDecoder::PalDecoder(bool _blackAndWhite, bool _useTransformFilter, double _transformThreshold)
{
    config.blackAndWhite = _blackAndWhite;
    config.pal.useTransformFilter = _useTransformFilter;
    config.pal.transformThreshold = _transformThreshold;
}

bool PalDecoder::configure(const LdDecodeMetaData::VideoParameters &videoParameters) {
    // Ensure the source video is PAL
    if (!videoParameters.isSourcePal) {
        qCritical() << "This decoder is for PAL video sources only";
        return false;
    }

    // Compute cropping parameters
    setVideoParameters(config, videoParameters, config.pal.firstActiveLine, config.pal.lastActiveLine);

    return true;
}

QThread *PalDecoder::makeThread(QAtomicInt& abort, DecoderPool& decoderPool) {
    return new PalThread(abort, decoderPool, config);
}

PalThread::PalThread(QAtomicInt& _abort, DecoderPool& _decoderPool,
                     const PalDecoder::Configuration &_config, QObject *parent)
    : QThread(parent), abort(_abort), decoderPool(_decoderPool), config(_config)
{
    // Configure PALcolour
    palColour.updateConfiguration(config.videoParameters, config.pal);
}

void PalThread::run()
{
    qint32 frameNumber;

    // Input data buffers
    QByteArray firstFieldData;
    QByteArray secondFieldData;

    // Frame metadata
    qint32 firstFieldPhaseID; // not used in PAL
    qint32 secondFieldPhaseID; // not used in PAL
    qreal burstMedianIre;

    while(!abort) {
        // Get the next frame to process from the input file
        if (!decoderPool.getInputFrame(frameNumber, firstFieldData, secondFieldData,
                                       firstFieldPhaseID, secondFieldPhaseID, burstMedianIre)) {
            // No more input frames -- exit
            break;
        }

        // Calculate the saturation level from the burst median IRE
        // Note: This code works as a temporary MTF compensator whilst ld-decode gets
        // real MTF compensation added to it.
        // PAL burst is 300 mV p-p (about 43 IRE, as 100 IRE = 700 mV)
        qreal nominalBurstIre = 300 * (100.0 / 700) / 2;
        qreal tSaturation = 100 * (nominalBurstIre / burstMedianIre);

        if (config.blackAndWhite) {
            tSaturation = 0;
        }

        // Perform the PALcolour filtering
        QByteArray outputData = palColour.performDecode(firstFieldData, secondFieldData, 100, static_cast<qint32>(tSaturation));

        // PALcolour outputs the whole frame; crop it to the active area
        QByteArray croppedData = PalDecoder::cropOutputFrame(config, outputData);

        // Write the result to the output file
        if (!decoderPool.putOutputFrame(frameNumber, croppedData)) {
            abort = true;
            break;
        }
    }
}
