#ifndef XRAYPROTOCOL_H
#define XRAYPROTOCOL_H

#include "QProcess"

#include "core/ipcclient.h"
#include "vpnprotocol.h"
#include "settings.h"
#include <QtCore/qsharedpointer.h>

class QNetworkAccessManager;

class XrayProtocol : public VpnProtocol
{
public:
    XrayProtocol(const QJsonObject &configuration, QObject *parent = nullptr);
    virtual ~XrayProtocol() override;

    ErrorCode start() override;
    void stop() override;

private:
    ErrorCode setupRouting();
    ErrorCode startTun2Socks();
    void probeProxy();

    QJsonObject m_xrayConfig;
    Settings::RouteMode m_routeMode;
    QList<QHostAddress> m_dnsServers;
    QString m_remoteAddress;
    bool m_proxyMode = false;
    int m_proxyModePort = 0;
    bool m_proxyXrayRunning = false;
    QNetworkAccessManager *m_probeNam = nullptr;

    QSharedPointer<IpcProcessInterfaceReplica> m_tun2socksProcess;
};

#endif // XRAYPROTOCOL_H
