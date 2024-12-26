/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QWidget>
#include <QFrame>
#include <QMouseEvent>
#include <QPoint>
#include <QMatrix4x4>
#include <QVector3D>

#include "SkinTypes.h"

class SkinWindow: public QOpenGLWindow
{
public:
    SkinWindow();
    virtual ~SkinWindow() noexcept;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void setAll(Skins::Model model, const QImage& skinImage, const QImage& capeImage);
    void setCapeImage(const QImage& capeImage);
    void setSkinImage(const QImage& skinImage);
    void setModel(Skins::Model model);
    void setBackgroundColor(QColor color);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void updateMatrix();

private:
    QMatrix4x4 m_projection;

    QPoint m_lastPos;
    double m_spinAngle;
    double m_liftAngle;
    double m_distance;

    QMatrix4x4 m_worldToView;

    QImage m_skinImage;
    QImage m_capeImage;
    Skins::Model m_model;
    QVector3D m_backgroundColor;
    bool m_skinDirty = false;
    int m_width = 0;
    int m_height = 0;

    Skins::RenderContext *m_context = nullptr;
    QOpenGLFunctions GL;
};

class SkinWidget : public QFrame
{
    Q_OBJECT
public:
    SkinWidget(QWidget *parent = nullptr);
    virtual ~SkinWidget() = default;

    void setAll(Skins::Model model, const QImage& skinImage, const QImage& capeImage);
    void setCapeImage(const QImage& capeImage);
    void setSkinImage(const QImage& skinImage);
    void setModel(Skins::Model model);
    void setBackgroundColor(QColor color);

    bool event(QEvent * event) override;
private:
    SkinWindow* m_window = nullptr;
};
