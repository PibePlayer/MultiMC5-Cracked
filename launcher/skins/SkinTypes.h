/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */
#pragma once

#include <QImage>
#include <QString>
#include <QByteArray>
#include <nonstd/optional>

namespace Skins {

enum class Model
{
    Classic,
    Slim,
};

class SkinData
{
public:
    SkinData() = default;
    SkinData(const QByteArray& data, const QImage& texture, const QString& textureID): data(data), texture(texture), textureID(textureID) {}

    QByteArray data;
    QImage texture;
    QString textureID;
    const QImage& getListTexture(Model model) const;
private:
    mutable QImage preview;
};

struct SkinEntry
{
    // From a free-standing texture
    SkinEntry(const QString& name, const QString& path, const QImage& image, const QString& textureID, const QByteArray data);
    // From internal resources
    SkinEntry(const QString& name, const QString& pathSlim, const QString& pathClassic);
    SkinEntry() {};

    bool matchesId(const QString& textureID) const;
    bool isNull() const
    {
        if(internal)
            return !slimVariant && !classicVariant;
        return !fileVariant;
    }

    const QImage& getListTexture() const;
    const QImage& getTextureFor(Model model) const;
    QString getTextureIDFor(Model model) const;
    QByteArray getTextureDataFor(Model model) const;

    bool internal = false;
    QString name;
    QString filename;
    nonstd::optional<SkinData> slimVariant;
    nonstd::optional<SkinData> classicVariant;
    nonstd::optional<SkinData> fileVariant;
};

struct CapeEntry
{
    QString uuid;
    QString alias;
    QImage preview;
};

class RenderContext;

enum class Texture: int
{
    Skin = 0,
    Cape = 1,
};

struct Rectangle {
    Rectangle(float x, float y, float w, float h, bool flipY = false, bool flipX = false) : x(x), y(y), w(w), h(h), flipY(flipY), flipX(flipX) {};

    float x;
    float y;
    float w;
    float h;
    bool flipY = false;
    bool flipX = false;
};

struct TextureMapping {
    // texture mappings
    Rectangle left;
    Rectangle right;
    Rectangle top;
    Rectangle bottom;
    Rectangle front;
    Rectangle back;
    Texture material;
    bool transparent;
};

}
