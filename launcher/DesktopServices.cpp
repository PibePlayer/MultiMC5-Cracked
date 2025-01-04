#include "DesktopServices.h"
#include <QDir>
#include <QDesktopServices>
#include <QProcess>
#include <QDebug>
#include "FileSystem.h"

namespace DesktopServices {
bool openDirectory(const QString &path)
{
    QUrl url = QUrl::fromLocalFile(path);
    QString urlString = url.toString(QUrl::FullyEncoded);

    qDebug() << "Opening directory" << path << "url" << urlString;

    if(!FS::ensureFolderPathExists(path))
    {
        qDebug() << "Failed to create directory for opening:" << path << "url" << urlString;
        return false;
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    return QProcess::startDetached("xdg-open", QStringList() << urlString);
#else
    return QDesktopServices::openUrl(url);
#endif
}

bool openFile(const QString &path)
{
    QUrl url = QUrl::fromLocalFile(path);
    QString urlString = url.toString(QUrl::FullyEncoded);
    qDebug() << "Opening file" << path << " url " << urlString;

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    return QProcess::startDetached("xdg-open", QStringList() << urlString);
#else
    return QDesktopServices::openUrl(url);
#endif
}

bool openFile(const QString &application, const QString &path, const QString &workingDirectory, qint64 *pid)
{
    qDebug() << "Opening file" << path << "using" << application;
    return QProcess::startDetached(application, QStringList() << path, workingDirectory, pid);
}

bool run(const QString &application, const QStringList &args, const QString &workingDirectory, qint64 *pid)
{
    qDebug() << "Running" << application << "with args" << args.join(' ');
    return QProcess::startDetached(application, args, workingDirectory, pid);
}

bool openUrl(const QUrl &url)
{
    QString urlString = url.toString(QUrl::FullyEncoded);
    qDebug() << "Opening URL" << urlString;
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    return QProcess::startDetached("xdg-open", QStringList() << urlString);
#else
    return QDesktopServices::openUrl(url);
#endif
}

}
