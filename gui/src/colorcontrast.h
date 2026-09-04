#ifndef COLORCONTRAST_H
#define COLORCONTRAST_H

#include <QColor>
#include <QString>

#include <algorithm>
#include <cmath>

inline double relativeLuminance(const QColor& color)
{
    const auto linearChannel = [](int channel) {
        const double value = channel / 255.0;
        return value <= 0.04045
            ? value / 12.92
            : std::pow((value + 0.055) / 1.055, 2.4);
    };

    return 0.2126 * linearChannel(color.red())
         + 0.7152 * linearChannel(color.green())
         + 0.0722 * linearChannel(color.blue());
}

inline double colorContrastRatio(const QColor& first, const QColor& second)
{
    const double firstLuminance = relativeLuminance(first);
    const double secondLuminance = relativeLuminance(second);
    const double lighter = std::max(firstLuminance, secondLuminance);
    const double darker = std::min(firstLuminance, secondLuminance);
    return (lighter + 0.05) / (darker + 0.05);
}

/**
 * @brief Return black or white, whichever has the greater WCAG contrast.
 */
inline QColor contrastingTextColor(const QColor& background)
{
    const double luminance = relativeLuminance(background);
    const double contrastWithBlack = (luminance + 0.05) / 0.05;
    const double contrastWithWhite = 1.05 / (luminance + 0.05);

    return contrastWithBlack >= contrastWithWhite ? QColor(Qt::black) : QColor(Qt::white);
}

inline QString colorSwatchStyleSheet(const QColor& background)
{
    const QColor foreground = contrastingTextColor(background);
    return QStringLiteral(
        "QPushButton {"
        " background-color: %1;"
        " color: %2;"
        " border: 1px solid %2;"
        " padding: 2px 4px;"
        "}"
        "QPushButton:focus { border-width: 2px; }"
    ).arg(background.name(QColor::HexRgb), foreground.name(QColor::HexRgb));
}

#endif // COLORCONTRAST_H
