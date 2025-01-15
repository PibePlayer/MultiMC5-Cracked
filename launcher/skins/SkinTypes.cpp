/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */
#include "SkinTypes.h"
#include "SkinUtils.h"
#include "TextureMappings.h"

#include <QFile>
#include <QPainter>

namespace{
const QImage nullImage;
}

namespace Skins {
SkinEntry::SkinEntry(const QString& name, const QString& path, const QImage& image, const QString& textureID, const QByteArray data) : name(name), filename(path)
{
    internal = false;
    fileVariant = SkinData{data, image, textureID};
}

SkinEntry::SkinEntry(const QString& name, const QString& pathSlim, const QString& pathClassic): name(name)
{
    internal = true;
    {
        QFile file(pathSlim);
        file.open(QIODevice::ReadOnly);
        auto data = file.readAll();
        file.close();
        QImage image;
        QString textureID;
        readSkinFromData(data, image, textureID);
        slimVariant = SkinData(data, image, textureID);
    }
    {
        QFile file(pathClassic);
        file.open(QIODevice::ReadOnly);
        auto data = file.readAll();
        file.close();
        QImage image;
        QString textureID;
        readSkinFromData(data, image, textureID);
        classicVariant = SkinData(data, image, textureID);
    }
}

const QImage& SkinEntry::getTextureFor(Model model) const
{
    if(internal)
    {
        switch(model)
        {
            case Model::Slim:
                return slimVariant->texture;
            case Model::Classic:
                return classicVariant->texture;
        }
    }

    if(fileVariant)
    {
        return fileVariant->texture;
    }
    return nullImage;
}

QString Skins::SkinEntry::getTextureIDFor(Model model) const
{
    if(internal)
    {
        switch(model)
        {
            case Model::Slim:
                return slimVariant->textureID;
            case Model::Classic:
                return classicVariant->textureID;
        }
    }

    if(fileVariant)
    {
        return fileVariant->textureID;
    }
    return QString();
}

QByteArray Skins::SkinEntry::getTextureDataFor(Model model) const
{
    if(internal)
    {
        switch(model)
        {
            case Model::Slim:
                return slimVariant->data;
            case Model::Classic:
                return classicVariant->data;
        }
    }

    if(fileVariant)
        return fileVariant->data;
    return QByteArray();
}

const QImage & SkinData::getListTexture(Model model) const
{
    if(preview.isNull())
    {
        QImage temp(36, 36, QImage::Format::Format_ARGB32);
        temp.fill(Qt::transparent);
        QPainter painter(&temp);
        int version = texture.height() == 64 ? 1: 0;

        auto paintFront = [&](int x, int y, const Skins::TextureMapping& part)
        {
            auto partTexture = texture.copy(part.front.x, part.front.y, part.front.w, part.front.h);
            if(!part.transparent)
            {
                painter.fillRect(x, y, part.front.w, part.front.w, Qt::black);
            }
            painter.drawImage(x, y, partTexture.mirrored(part.front.flipX, part.front.flipY));
        };
        auto paintBack = [&](int x, int y, const Skins::TextureMapping& part)
        {
            auto partTexture = texture.copy(part.back.x, part.back.y, part.back.w, part.back.h);
            if(!part.transparent)
            {
                painter.fillRect(x, y, part.back.w, part.back.w, Qt::black);
            }
            painter.drawImage(x, y, partTexture.mirrored(part.back.flipX, part.back.flipY));
        };
        paintFront(4, 2, head);
        paintFront(4, 2, head_cover);
        paintFront(4, 10, torso);
        paintFront(4, 22, right_leg);

        if(version == 0)
        {
            if(model == Model::Classic)
            {
                paintFront(12, 10, left_arm_old_classic);
                paintFront(0, 10, right_arm_old_classic);
            }
            else
            {
                paintFront(12, 10, left_arm_old_slim);
                paintFront(1, 10, right_arm_old_slim);
            }

            paintFront(8, 22, left_leg_old);
        }
        else if(version == 1)
        {
            paintFront(4, 10, torso_cover);

            if(model == Model::Classic)
            {
                paintFront(12, 10, left_arm_classic);
                paintFront(12, 10, left_arm_cover_classic);

                paintFront(0, 10, right_arm_classic);
                paintFront(0, 10, right_arm_cover_classic);
            }
            else
            {
                paintFront(12, 10, left_arm_slim);
                paintFront(12, 10, left_arm_cover_slim);

                paintFront(1, 10, right_arm_slim);
                paintFront(1, 10, right_arm_cover_slim);
            }

            paintFront(8, 22, left_leg);
            paintFront(8, 22, left_leg_cover);
            paintFront(4, 22, right_leg_cover);
        }

        paintBack(24, 2, head);
        paintBack(24, 2, head_cover);
        paintBack(24, 10, torso);
        paintBack(28, 22, right_leg);

        if(version == 0)
        {
            if(model == Model::Classic)
            {
                paintBack(20, 10, left_arm_old_classic);
                paintBack(32, 10, right_arm_old_classic);
            }
            else
            {
                paintBack(21, 10, left_arm_old_slim);
                paintBack(32, 10, right_arm_old_slim);
            }

            paintBack(24, 22, left_leg_old);
        }
        else if(version == 1)
        {
            paintBack(24, 10, torso_cover);

            if(model == Model::Classic)
            {
                paintBack(20, 10, left_arm_classic);
                paintBack(20, 10, left_arm_cover_classic);

                paintBack(32, 10, right_arm_classic);
                paintBack(32, 10, right_arm_cover_classic);
            }
            else
            {
                paintBack(21, 10, left_arm_slim);
                paintBack(21, 10, left_arm_cover_slim);

                paintBack(32, 10, right_arm_slim);
                paintBack(32, 10, right_arm_cover_slim);
            }

            paintBack(24, 22, left_leg);
            paintBack(24, 22, left_leg_cover);
            paintBack(28, 22, right_leg_cover);
        }
        preview = temp.scaled(72, 72);
    }
    return preview;
}

const QImage & SkinEntry::getListTexture() const
{
    if(internal)
    {
        return slimVariant->getListTexture(Model::Slim);
    }

    if(fileVariant)
    {
        return fileVariant->getListTexture(Model::Classic);
    }
    return nullImage;
}

bool SkinEntry::matchesId(const QString& textureID) const
{
    if(internal)
    {
        if(slimVariant && slimVariant->textureID == textureID)
        {
            return true;
        }
        if(classicVariant && classicVariant->textureID == textureID)
        {
            return true;
        }
        return false;
    }

    if(fileVariant && fileVariant->textureID == textureID)
    {
        return true;
    }
    return false;
}

}

