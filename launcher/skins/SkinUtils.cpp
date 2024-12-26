/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "SkinUtils.h"
#include <QFileInfo>
#include <QCryptographicHash>
#include <FileSystem.h>

namespace Skins {

// We will refuse to load 'skins' larger than what a raw bitmap of a skin would take up to prevent abuse of the system
constexpr qint64 maxFileSize = 64 * 64 * 4;

QString hashSkin(const QImage& image)
{
    QCryptographicHash checksum(QCryptographicHash::Algorithm::Sha256);
    char nothing[5] = "\0\0\0\0";
    for(int x = 0; x < image.width(); x++)
    {
        for(int y = 0; y < image.height(); y++)
        {
            QRgb pixel = image.pixel(x, y);
            char color[4];
            color[0] = qRed(pixel);
            color[1] = qGreen(pixel);
            color[2] = qBlue(pixel);
            color[3] = qAlpha(pixel);
            if(color[3] == 0)
            {
                checksum.addData(nothing, 4);
            }
            else
            {
                checksum.addData(color, 4);
            }
        }
    }
    return checksum.result().toHex();
}
bool readSkinFromFile(const QString& path, QByteArray& dataOut, QImage& imageOut, QString& keyOut, QString& textureIDOut)
{
    QFileInfo info(path);
    keyOut = info.baseName();
    if(!info.isFile())
    {
        return false;
    }
    if(info.suffix().toLower() != "png")
    {
        return false;
    }
    if(info.size() >= maxFileSize)
    {
        return false;
    }
    try
    {
        dataOut = FS::read(path);
    }
    catch (const Exception& e)
    {
        qWarning() << "Failed to read skin file:" << path << "Error:" << e.cause();
        return false;
    }
    return readSkinFromData(dataOut, imageOut, textureIDOut);
}

bool readSkinFromData(const QByteArray& data, QImage& imageOut, QString& textureIDOut)
{
    if(data.size() >= maxFileSize)
    {
        return false;
    }
    QImage img = QImage::fromData(data, "PNG");
    if(img.width() != 64)
    {
        return false;
    }
    int height = img.height();
    if(height != 32 && height != 64)
    {
        return false;
    }

    imageOut = img;
    if(img.hasAlphaChannel())
    {
        textureIDOut = hashSkin(imageOut);
        return true;
    }
    // No alpha channel -> take top left pixel and replace all matching pixels with transparency
    auto alphaChannel = imageOut.createMaskFromColor(imageOut.pixel(0,0), Qt::MaskMode::MaskOutColor);
    imageOut.setAlphaChannel(alphaChannel);

    textureIDOut = hashSkin(imageOut);
    return true;
}

}
