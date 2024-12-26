/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QImage>
#include <QQueue>
#include <QTimer>

#include "net/NetJob.h"

class CapeCache: public QObject
{
    struct CapeRequest
    {
        QString uuid;
        QString url;
    };
    struct IndexEntry
    {
        QImage image;
        QByteArray rawData;
    };
    Q_OBJECT
public:
    explicit CapeCache(QObject* parent = nullptr);
    virtual ~CapeCache() noexcept;

    void load();

    const QImage& getCapeImage(const QString& uuid) const;
    void addCapeImage(const QString& uuid, const QString& url);

signals:
    void capeReady(QString uuid);

private:
    void saveLater();
    void getNext();
    void disposeDownloadJob();

private slots:
    void requestFinished();
    void requestFailed(QString reason);
    void saveNow();

private:
    QMap<QString, IndexEntry> m_index;
    QImage m_placeholder;

    NetJob::Ptr m_downloadJob;
    QByteArray m_response;
    QQueue<CapeRequest> m_requests;
    QTimer m_saveTimer;
};
