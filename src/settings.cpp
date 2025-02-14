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
    Processing settings getters/setters implementation.
*/

#include <sstream>
#include <wx/xml/xml.h>

#include "num_formatter.h"
#include "settings.h"

const int XML_INDENT = 4;
const unsigned FLOAT_PREC = 4;

/// Names of XML elements in a settings file
namespace XmlName
{
    const char* root = "imppg";

    const char* lucyRichardson = "lucy-richardson";
    const char* lrSigma = "sigma";
    const char* lrIters = "iterations";
    const char* lrDeringing = "deringing";

    const char* unshMask = "unsharp_mask";
    const char* unshAdaptive = "adaptive";
    const char* unshSigma = "sigma";
    const char* unshAmountMin = "amount_min";
    const char* unshAmountMax = "amount_max";
    const char* unshThreshold = "amount_threshold";
    const char* unshWidth = "amount_width";

    const char* tcurve = "tone_curve";
    const char* tcSmooth = "smooth";
    const char* tcIsGamma = "is_gamma";
    const char* tcGamma = "gamma";

    const char* normalization = "normalization";
    const char* normEnabled = "enabled";
    const char* normMin = "min";
    const char* normMax = "max";
}

const char* trueStr = "true";
const char* falseStr = "false";

bool CreateAndSaveDocument(wxString filePath, wxXmlNode* root)
{
    wxXmlDocument xdoc;
    xdoc.SetVersion("1.0");
    xdoc.SetFileEncoding("UTF-8");
    xdoc.SetRoot(root);
    return xdoc.Save(filePath, XML_INDENT);
}

wxXmlNode* CreateToneCurveSettingsNode(const c_ToneCurve& toneCurve)
{
    wxXmlNode *result = new wxXmlNode(wxXML_ELEMENT_NODE, XmlName::tcurve);
    result->AddAttribute(XmlName::tcSmooth, toneCurve.GetSmooth() ? trueStr : falseStr);
    result->AddAttribute(XmlName::tcIsGamma, toneCurve.IsGammaMode() ? trueStr : falseStr);
    if (toneCurve.IsGammaMode())
         result->AddAttribute(XmlName::tcGamma, NumFormatter::Format(toneCurve.GetGamma(), FLOAT_PREC));
    wxString pointsStr;
    for (int i = 0; i < toneCurve.GetNumPoints(); i++)
        pointsStr += NumFormatter::Format(toneCurve.GetPoint(i).x, FLOAT_PREC) + ";"+
                     NumFormatter::Format(toneCurve.GetPoint(i).y, FLOAT_PREC) + ";";

    result->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString, pointsStr));

    return result;
}

wxXmlNode* CreateLucyRichardsonSettingsNode(float lrSigma, int lrIters, bool lrDeringing)
{
    wxXmlNode* result = new wxXmlNode(wxXML_ELEMENT_NODE, XmlName::lucyRichardson);
    result->AddAttribute(XmlName::lrSigma, NumFormatter::Format(lrSigma, FLOAT_PREC));
    result->AddAttribute(XmlName::lrIters, wxString::Format("%d", lrIters));
    result->AddAttribute(XmlName::lrDeringing, lrDeringing ? trueStr : falseStr);
    return result;
}

wxXmlNode* CreateUnsharpMaskingSettingsNode(bool adaptive, float sigma, float amountMin, float amountMax, float threshold, float width)
{
    wxXmlNode* result = new wxXmlNode(wxXML_ELEMENT_NODE, XmlName::unshMask);

    result->AddAttribute(XmlName::unshAdaptive, adaptive ? trueStr : falseStr);
    result->AddAttribute(XmlName::unshSigma, NumFormatter::Format(sigma, FLOAT_PREC));
    result->AddAttribute(XmlName::unshAmountMin, NumFormatter::Format(amountMin, FLOAT_PREC));
    result->AddAttribute(XmlName::unshAmountMax, NumFormatter::Format(amountMax, FLOAT_PREC));
    result->AddAttribute(XmlName::unshThreshold, NumFormatter::Format(threshold, FLOAT_PREC));
    result->AddAttribute(XmlName::unshWidth, NumFormatter::Format(width, FLOAT_PREC));
    return result;
}

wxXmlNode* CreateNormalizationSettingsNode(bool normalizationEnabled, float normMin, float normMax)
{
    wxXmlNode* result = new wxXmlNode(wxXML_ELEMENT_NODE, XmlName::normalization);
    result->AddAttribute(XmlName::normEnabled, normalizationEnabled ? trueStr : falseStr);
    result->AddAttribute(XmlName::normMin, NumFormatter::Format(normMin, FLOAT_PREC));
    result->AddAttribute(XmlName::normMax, NumFormatter::Format(normMax, FLOAT_PREC));
    return result;
}

bool SaveSettings(wxString filePath, const ProcessingSettings& settings)
{
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, XmlName::root);

    root->AddChild(CreateLucyRichardsonSettingsNode(
        settings.LucyRichardson.sigma,
        settings.LucyRichardson.iterations,
        settings.LucyRichardson.deringing.enabled
    ));
    root->AddChild(CreateUnsharpMaskingSettingsNode(
        settings.unsharpMasking.adaptive,
        settings.unsharpMasking.sigma,
        settings.unsharpMasking.amountMin,
        settings.unsharpMasking.amountMax,
        settings.unsharpMasking.threshold,
        settings.unsharpMasking.width
    ));
    root->AddChild(CreateToneCurveSettingsNode(settings.toneCurve));
    root->AddChild(CreateNormalizationSettingsNode(
        settings.normalization.enabled,
        settings.normalization.min,
        settings.normalization.max
    ));

    return CreateAndSaveDocument(filePath, root);
}

bool ParseLucyRichardsonSettings(const wxXmlNode* node, float& sigma, int& iterations, bool& deringing)
{
    if (!NumFormatter::Parse(node->GetAttribute(XmlName::lrSigma), sigma))
    {
        return false;
    }

    unsigned long value{};
    if (!node->GetAttribute(XmlName::lrIters).ToULong(&value))
    {
        return false;
    }
    else
    {
        iterations = static_cast<int>(value);
    }

    if (node->GetAttribute(XmlName::lrDeringing) == trueStr)
        deringing = true;
    else if (node->GetAttribute(XmlName::lrDeringing) == falseStr)
        deringing = false;
    else
        return false;

    return true;
}

bool ParseUnsharpMaskingSettings(const wxXmlNode* node, bool& adaptive, float& sigma, float& amountMin, float& amountMax, float& threshold, float& width)
{
    std::stringstream parser;

    if (node->GetAttribute(XmlName::unshAdaptive) == trueStr)
        adaptive = true;
    else if (node->GetAttribute(XmlName::unshAdaptive) == falseStr)
        adaptive = false;
    else
        return false;

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::unshSigma), sigma))
    {
        return false;
    }

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::unshAmountMin), amountMin))
    {
        return false;
    }

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::unshAmountMax), amountMax))
    {
        return false;
    }

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::unshThreshold), threshold))
    {
        return false;
    }

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::unshWidth), width))
    {
        return false;
    }

    return true;
}

bool ParseNormalizationSetings(const wxXmlNode* node, bool& normalizationEnabled, float& normMin, float& normMax)
{
    if (node->GetAttribute(XmlName::normEnabled) == "true")
        normalizationEnabled = true;
    else if (node->GetAttribute(XmlName::normEnabled) == "false")
        normalizationEnabled = false;
    else
        return false;

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::normMin), normMin))
    {
        return false;
    }

    if (!NumFormatter::Parse(node->GetAttribute(XmlName::normMax), normMax))
    {
        return false;
    }

    return true;
}

bool ParseToneCurveSettings(const wxXmlNode* node, c_ToneCurve& tcurve)
{
    wxString boolStr = node->GetAttribute(XmlName::tcSmooth);
    if (boolStr == "true")
        tcurve.SetSmooth(true);
    else if (boolStr == "false")
        tcurve.SetSmooth(false);
    else
        return false;

    boolStr = node->GetAttribute(XmlName::tcIsGamma);
    if (boolStr == "true")
    {
        tcurve.SetGammaMode(true);

        float gamma{};
        if (!NumFormatter::Parse(node->GetAttribute(XmlName::tcGamma), gamma))
        {
            return false;
        }
        else
        {
            tcurve.SetGamma(gamma);
        }
    }
    else if (boolStr == "false")
        tcurve.SetGammaMode(false);
    else
        return false;

    std::vector<float> points_xy;
    if (!NumFormatter::ParseList(node->GetNodeContent(), points_xy, ';'))
    {
        return false;
    }
    if (points_xy.size() % 2 != 0)
    {
        return false;
    }

    tcurve.ClearPoints();
    for (size_t i = 0; i < points_xy.size(); i += 2)
    {
        tcurve.AddPoint(points_xy[i], points_xy[i + 1]);
    }

    if (tcurve.GetNumPoints() < 2)
        return false;
    else
        return true;
}

bool LoadSettings(
    wxString filePath,
    ProcessingSettings& settings,
    bool* loadedLR,
    bool* loadedUnsh,
    bool* loadedTCurve
)
{
    if (loadedLR)
        *loadedLR = false;
    if (loadedUnsh)
        *loadedUnsh = false;
    if (loadedTCurve)
        *loadedTCurve = false;

    settings.LucyRichardson.deringing.enabled = false;
    settings.normalization.enabled = false;

    wxXmlDocument xdoc;
    if (!xdoc.Load(filePath))
        return false;

    wxXmlNode* root = xdoc.GetRoot();
    if (!root)
        return false;

    wxXmlNode* child = root->GetChildren();
    while (child)
    {
        if (child->GetName() == XmlName::lucyRichardson)
        {
            float sigma;
            int iters;
            bool deringing;

            if (!ParseLucyRichardsonSettings(child, sigma, iters, deringing))
                return false;

            settings.LucyRichardson.sigma = sigma;
            settings.LucyRichardson.iterations = iters;
            settings.LucyRichardson.deringing.enabled = deringing;

            if (loadedLR)
                *loadedLR = true;
        }
        else if (child->GetName() == XmlName::unshMask)
        {
            bool adaptive;
            float sigma, amountMin, amountMax, threshold, width;

            if (!ParseUnsharpMaskingSettings(child,adaptive, sigma, amountMin, amountMax, threshold, width))
                return false;

            settings.unsharpMasking.adaptive = adaptive;
            settings.unsharpMasking.sigma = sigma;
            settings.unsharpMasking.amountMin = amountMin;
            settings.unsharpMasking.amountMax = amountMax;
            settings.unsharpMasking.threshold = threshold;
            settings.unsharpMasking.width = width;

            if (loadedUnsh)
                *loadedUnsh = true;
        }
        else if (child->GetName() == XmlName::tcurve)
        {
            c_ToneCurve tcurve;
            if (!ParseToneCurveSettings(child, tcurve))
                return false;

            settings.toneCurve = tcurve;

            if (loadedTCurve)
                *loadedTCurve = true;
        }
        else if (child->GetName() == XmlName::normalization)
        {
            bool enabled;
            float nmin, nmax;

            if (!ParseNormalizationSetings(child, enabled, nmin, nmax))
                return false;
            settings.normalization.enabled = enabled;
            settings.normalization.min = nmin;
            settings.normalization.max = nmax;
        }

        child = child->GetNext();
    }

    return true;
}
