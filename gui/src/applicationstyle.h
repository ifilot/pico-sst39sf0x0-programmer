/****************************************************************************
 *                                                                          *
 *   PICO-SST39SF0x0-FLASHER                                                *
 *   Copyright (C) 2023 Ivo Filot <ivo@ivofilot.nl>                         *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Lesser General Public License as         *
 *   published by the Free Software Foundation, either version 3 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 ****************************************************************************/

#ifndef APPLICATIONSTYLE_H
#define APPLICATIONSTYLE_H

#include <QApplication>
#include <QStyle>
#include <QStyleFactory>

/**
 * @brief Select the deliberately compact Windows appearance.
 * @return The object name of the style that is active after configuration.
 *
 * Qt's Windows Vista style is supplied as a plugin. Keep a fallback for
 * developer builds that do not have that optional plugin installed; other
 * platforms retain their native default style.
 */
inline QString configureApplicationStyle(QApplication& application)
{
#ifdef Q_OS_WIN
    if (QStyle* vistaStyle = QStyleFactory::create(QStringLiteral("windowsvista"))) {
        application.setStyle(vistaStyle);
    } else if (QStyle* windowsStyle = QStyleFactory::create(QStringLiteral("Windows"))) {
        application.setStyle(windowsStyle);
    }
#else
    Q_UNUSED(application);
#endif

    return QApplication::style()->objectName();
}

#endif // APPLICATIONSTYLE_H
