/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QString>
#include <QByteArray>
#include <QImage>

namespace Skins {
bool readSkinFromFile(const QString& path, QByteArray& dataOut, QImage& imageOut, QString& keyOut, QString& textureIDOut);
bool readSkinFromData(const QByteArray& data, QImage& imageOut, QString& textureIDOut);

QString hashSkin(const QImage& image);
}
