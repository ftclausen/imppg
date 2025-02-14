/*
ImPPG (Image Post-Processor) - common operations for astronomical stacks and other images
Copyright (C) 2016-2019 Filip Szczerek <ga.software@yahoo.com>

This file is part of ImPPG.

ImPPG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ImPPG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ImPPG.  If not, see <http://www.gnu.org/licenses/>.

File description:
    Processing settings structure.
*/

#ifndef IMPGG_PROCESSING_SETTINGS_HEADER
#define IMPGG_PROCESSING_SETTINGS_HEADER

#include "common/tcrv.h"

struct ProcessingSettings
{
    /// Normalization is performed prior to all other processing steps.
    struct
    {
        bool enabled{false};
        float min, max;
    } normalization;

    struct
    {
        float sigma; ///< Lucy-Richardson deconvolution kernel sigma
        int iterations{0}; ///< Number of Lucy-Richardson deconvolution iterations.
        struct
        {
            bool enabled{false}; ///< Experimantal; enables deringing along edges of overexposed areas (see c_LucyRichardsonThread::DoWork()).
        } deringing;
    } LucyRichardson;

    struct
    {
        bool adaptive{false}; ///If true, adaptive unsharp masking is used.
        float sigma; ///< Gaussian kernel sigma.
        float amountMin{1.0}; ///< Amount (weight) of the unsharped layer; <1.0 blurs, >1.0 sharpens; if adaptive=true, used as the min amount.
        float amountMax{1.0}; ///< Max amount.
        float threshold; ///< Threshold of input image brightness where the min-max amount transition occurs.
        float width; ///< Width of the transition interval.

        bool IsEffective() const
        {
            return !adaptive && amountMax != 1.0f ||
                adaptive && (amountMin != 1.0f || amountMax != 1.0f);
        }
    } unsharpMasking;

    c_ToneCurve toneCurve;
};

#endif // IMPGG_PROCESSING_SETTINGS_HEADER

