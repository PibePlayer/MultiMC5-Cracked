/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "CapeCache.h"
#include <Application.h>
#include <FileSystem.h>

namespace {
QString fileName = "cache/capes.dat";
}

CapeCache::CapeCache(QObject* parent) : QObject(parent)
{
    connect(&m_saveTimer, &QTimer::timeout, this, &CapeCache::saveNow);
    load();
}

CapeCache::~CapeCache() noexcept
{
    if(m_saveTimer.isActive())
    {
        m_saveTimer.stop();
        saveNow();
    }
}

void CapeCache::load()
{
    if(!QFile::exists(fileName))
    {
        qDebug() << "No cape cache file to load";
        return;
    }

    QFile capesData(fileName);
    if(!capesData.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open " << fileName << " for reading";
        return;
    }
    QDataStream in(&capesData);
    in.setVersion(QDataStream::Qt_5_4);
    int32_t count;
    in >> count;
    if(in.status() != QDataStream::Ok)
    {
        qWarning() << "Could not read count of capes from " << fileName;
        return;
    }
    for(int i = 0; i < count; i++)
    {
        QString uuid;
        QByteArray data;
        in >> uuid >> data;
        if(in.status() != QDataStream::Ok)
        {
            qWarning() << "Could not read cape " << i << " out of " << count << " from " << fileName;
            return;
        }
        QPixmap cape;
        if(!cape.loadFromData(data, "PNG"))
        {
            qWarning() << "Failed to load cape id " << uuid << " from " << fileName << " : Could not read the PNG data.";
        }
        m_index[uuid] = {cape.toImage(), data};
    }
}

void CapeCache::saveLater()
{
    m_saveTimer.stop();
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(5000);
    m_saveTimer.start();
}

void CapeCache::saveNow()
{
    if(!FS::ensureFilePathExists(fileName))
    {
        qWarning() << "Could not create directory for cape cache";
        return;
    }
    QSaveFile capesData(fileName);
    if(!capesData.open(QIODevice::WriteOnly))
    {
        qWarning() << "Could not open " << fileName << " for writing";
        return;
    }
    QDataStream out(&capesData);   // we will serialize the data into the file
    out.setVersion(QDataStream::Qt_5_4);
    out << int32_t(m_index.size());
    for (auto it = m_index.begin(); it != m_index.end(); ++it) {
        out << it.key() << it.value().rawData;
    }
    if(!capesData.commit())
    {
        qWarning() << "Could not write into " << fileName;
        return;
    }
}

void CapeCache::addCapeImage(const QString& uuid, const QString& url)
{
    // If we already have it, we skip it
    if(m_index.contains(uuid))
    {
        return;
    }

    // Queue up retrieval of this cape if we do not have it
    m_requests.enqueue({uuid, url});
    getNext();
}

void CapeCache::getNext()
{
    // if we are downloading already, don't start another one
    if(m_downloadJob)
    {
        return;
    }

    // make sure we do not duplicate work in case the queue is filled with duplicate cape requests
    // pull out of the queue until we have a valid request to process
GO_AGAIN:
    if(m_requests.isEmpty())
    {
        return;
    }
    CapeRequest& request = m_requests.head();
    if(m_index.contains(request.uuid))
    {
        m_requests.dequeue();
        goto GO_AGAIN;
    }

    auto *netJob = new NetJob("CapeCache::Request", APPLICATION->network());
    netJob->addNetAction(Net::Download::makeByteArray(QUrl(request.url), &m_response));
    m_downloadJob = netJob;
    m_downloadJob->start();

    QObject::connect(netJob, &NetJob::succeeded, this, &CapeCache::requestFinished);
    QObject::connect(netJob, &NetJob::failed, this, &CapeCache::requestFailed);
}

void CapeCache::requestFinished()
{
    disposeDownloadJob();

    CapeRequest request = m_requests.dequeue();
    QPixmap cape;
    if(!cape.loadFromData(m_response, "PNG"))
    {
        qWarning() << "Failed to load cape id " << request.uuid << " from " << request.url << " : Could not read the PNG data.";
    }
    m_index[request.uuid] = {cape.toImage(), m_response};
    m_response.clear();

    saveLater();
    emit capeReady(request.uuid);

    getNext();
}

void CapeCache::requestFailed(QString reason)
{
    CapeRequest request = m_requests.dequeue();

    disposeDownloadJob();

    qWarning() << "Failed to download cape id " << request.uuid << " from " << request.url << " : " << reason;

    getNext();
}

void CapeCache::disposeDownloadJob()
{
    disconnect(m_downloadJob.get());
    m_downloadJob->deleteLater();
    m_downloadJob = nullptr;
}


const QImage& CapeCache::getCapeImage(const QString& uuid) const
{
    auto iter = m_index.constFind(uuid);
    if(iter == m_index.constEnd())
    {
        return m_placeholder;
    }
    return iter.value().image;
}
