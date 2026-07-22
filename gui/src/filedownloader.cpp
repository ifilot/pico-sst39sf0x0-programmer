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
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public license      *
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 *                                                                          *
 ****************************************************************************/

#include "filedownloader.h"

/**
 * @brief Construct a downloader for a single URL.
 * @param url source URL
 * @param parent parent object
 */
FileDownloader::FileDownloader(QUrl url, QObject *parent) :
    QObject(parent) {

    // add connection
    connect(
        &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
        this, SLOT (fileDownloaded(QNetworkReply*))
    );

    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::supportsSsl();
    qDebug() << QSslSocket::sslLibraryVersionString();

    QNetworkRequest request(url);
    qDebug() << request.url();
    m_WebCtrl.get(request);
}

/**
 * @brief Destroy the downloader.
 */
FileDownloader::~FileDownloader() { }

/**
 * @brief Handle a completed network reply.
 * @param pReply reply object
 */
void FileDownloader::fileDownloaded(QNetworkReply* pReply) {

    if(pReply->error() != QNetworkReply::NoError) {
        m_Success = false;
        m_ErrorMessage = pReply->errorString();
        pReply->deleteLater();
        emit downloaded();
        return;
    }

    int statuscode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HttpStatusCode: " << statuscode;

    QVariant redirectTarget = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (statuscode >= 300 && statuscode < 400 && redirectTarget.isValid()) {
        m_RedirectCount++;
        if(m_RedirectCount > 5) {
            m_Success = false;
            m_ErrorMessage = "Too many HTTP redirects.";
            pReply->deleteLater();
            emit downloaded();
            return;
        }
        QUrl redirectUrl = pReply->url().resolved(redirectTarget.toUrl());
        QNetworkRequest request(redirectUrl);
        qDebug() << request.url();
        pReply->deleteLater();
        m_WebCtrl.get(request);
        return;
    }

    if(statuscode < 200 || statuscode >= 300) {
        m_Success = false;
        m_ErrorMessage = QString("Unexpected HTTP status code %1.").arg(statuscode);
        pReply->deleteLater();
        emit downloaded();
        return;
    }

    m_DownloadedData = pReply->readAll();
    m_Success = true;

    //emit a signal
    pReply->deleteLater();
    emit downloaded();
}

/**
 * @brief Get the downloaded payload.
 * @return response body
 */
QByteArray FileDownloader::downloadedData() const {
    return m_DownloadedData;
}

/**
 * @brief Check whether the download completed successfully.
 * @return true on success
 */
bool FileDownloader::isSuccessful() const {
    return m_Success;
}

/**
 * @brief Get the last download error message.
 * @return error message
 */
QString FileDownloader::errorMessage() const {
    return m_ErrorMessage;
}
