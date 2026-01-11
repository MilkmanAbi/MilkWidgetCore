/**
 * MilkWidgetCore - Additional API Implementations
 * NetworkMonitor, BatteryMonitor, WeatherAPI, MediaPlayer, NotificationAPI
 */

#include "milk/APIs.h"
#include "milk/Utils.h"

#include <QFile>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QTimer>

namespace Milk {

// ============================================================================
// NETWORK MONITOR
// ============================================================================

NetworkMonitor* NetworkMonitor::s_instance = nullptr;

NetworkMonitor* NetworkMonitor::instance() {
    if (!s_instance) s_instance = new NetworkMonitor();
    return s_instance;
}

NetworkMonitor::NetworkMonitor() : QObject() {
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &NetworkMonitor::update);
    m_updateTimer->start(1000);
    update();
}

void NetworkMonitor::update() {
    // Read /proc/net/dev for network stats
    QFile file("/proc/net/dev");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    
    qint64 totalRx = 0, totalTx = 0;
    
    QTextStream in(&file);
    in.readLine(); // Header 1
    in.readLine(); // Header 2
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 10) continue;
        
        QString iface = parts[0].remove(':');
        if (iface == "lo") continue; // Skip loopback
        
        qint64 rx = parts[1].toLongLong();
        qint64 tx = parts[9].toLongLong();
        
        // Calculate speed
        if (m_lastRx.contains(iface)) {
            qint64 rxDiff = rx - m_lastRx[iface];
            qint64 txDiff = tx - m_lastTx[iface];
            m_rxSpeed[iface] = rxDiff;
            m_txSpeed[iface] = txDiff;
        }
        
        m_lastRx[iface] = rx;
        m_lastTx[iface] = tx;
        m_rxTotal[iface] = rx;
        m_txTotal[iface] = tx;
        
        totalRx += rx;
        totalTx += tx;
        
        if (!m_interfaces.contains(iface)) {
            m_interfaces.append(iface);
        }
    }
    
    m_totalRx = totalRx;
    m_totalTx = totalTx;
    
    emit updated();
}

qint64 NetworkMonitor::downloadSpeed(const QString& iface) const {
    if (iface.isEmpty()) {
        qint64 total = 0;
        for (auto it = m_rxSpeed.begin(); it != m_rxSpeed.end(); ++it)
            total += it.value();
        return total;
    }
    return m_rxSpeed.value(iface, 0);
}

qint64 NetworkMonitor::uploadSpeed(const QString& iface) const {
    if (iface.isEmpty()) {
        qint64 total = 0;
        for (auto it = m_txSpeed.begin(); it != m_txSpeed.end(); ++it)
            total += it.value();
        return total;
    }
    return m_txSpeed.value(iface, 0);
}

QString NetworkMonitor::downloadSpeedStr(const QString& iface) const {
    return String::formatBytes(downloadSpeed(iface)) + "/s";
}

QString NetworkMonitor::uploadSpeedStr(const QString& iface) const {
    return String::formatBytes(uploadSpeed(iface)) + "/s";
}

qint64 NetworkMonitor::totalDownload(const QString& iface) const {
    if (iface.isEmpty()) return m_totalRx;
    return m_rxTotal.value(iface, 0);
}

qint64 NetworkMonitor::totalUpload(const QString& iface) const {
    if (iface.isEmpty()) return m_totalTx;
    return m_txTotal.value(iface, 0);
}

QString NetworkMonitor::totalDownloadStr(const QString& iface) const {
    return String::formatBytes(totalDownload(iface));
}

QString NetworkMonitor::totalUploadStr(const QString& iface) const {
    return String::formatBytes(totalUpload(iface));
}

QStringList NetworkMonitor::interfaces() const { return m_interfaces; }
QString NetworkMonitor::activeInterface() const { return m_interfaces.isEmpty() ? "" : m_interfaces.first(); }
bool NetworkMonitor::isConnected() const { return !m_interfaces.isEmpty(); }
QString NetworkMonitor::ipAddress(const QString&) const { return ""; } // Would need socket API
QString NetworkMonitor::publicIP() const { return m_publicIP; }

void NetworkMonitor::fetchPublicIP() {
    QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
    QNetworkReply* reply = mgr->get(QNetworkRequest(QUrl("https://api.ipify.org")));
    connect(reply, &QNetworkReply::finished, [this, reply, mgr]() {
        if (reply->error() == QNetworkReply::NoError) {
            m_publicIP = QString::fromUtf8(reply->readAll()).trimmed();
            emit publicIPChanged(m_publicIP);
        }
        reply->deleteLater();
        mgr->deleteLater();
    });
}

void NetworkMonitor::setUpdateInterval(int ms) { m_updateTimer->setInterval(ms); }

// ============================================================================
// BATTERY MONITOR
// ============================================================================

BatteryMonitor* BatteryMonitor::s_instance = nullptr;

BatteryMonitor* BatteryMonitor::instance() {
    if (!s_instance) s_instance = new BatteryMonitor();
    return s_instance;
}

BatteryMonitor::BatteryMonitor() : QObject() {
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &BatteryMonitor::update);
    m_updateTimer->start(5000);
    findBattery();
    update();
}

void BatteryMonitor::findBattery() {
    QDir powerSupply("/sys/class/power_supply");
    for (const QString& name : powerSupply.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString typePath = powerSupply.filePath(name + "/type");
        QFile typeFile(typePath);
        if (typeFile.open(QIODevice::ReadOnly)) {
            QString type = QString::fromUtf8(typeFile.readAll()).trimmed();
            if (type == "Battery") {
                m_batteryPath = powerSupply.filePath(name);
                m_hasBattery = true;
                return;
            }
        }
    }
    m_hasBattery = false;
}

void BatteryMonitor::update() {
    if (!m_hasBattery || m_batteryPath.isEmpty()) return;
    
    // Read capacity
    QFile capFile(m_batteryPath + "/capacity");
    if (capFile.open(QIODevice::ReadOnly)) {
        m_level = QString::fromUtf8(capFile.readAll()).trimmed().toInt();
    }
    
    // Read status
    QFile statusFile(m_batteryPath + "/status");
    if (statusFile.open(QIODevice::ReadOnly)) {
        QString status = QString::fromUtf8(statusFile.readAll()).trimmed();
        m_charging = (status == "Charging");
        m_pluggedIn = (status == "Charging" || status == "Full" || status == "Not charging");
    }
    
    emit updated();
    emit levelChanged(m_level);
    if (m_level <= 10 && !m_charging) {
        emit lowBattery();
    }
}

int BatteryMonitor::level() const { return m_level; }
bool BatteryMonitor::isCharging() const { return m_charging; }
bool BatteryMonitor::isPluggedIn() const { return m_pluggedIn; }
bool BatteryMonitor::hasBattery() const { return m_hasBattery; }
QString BatteryMonitor::status() const {
    if (!m_hasBattery) return "No Battery";
    if (m_charging) return "Charging";
    if (m_pluggedIn) return "Plugged In";
    return "Discharging";
}
int BatteryMonitor::timeRemaining() const { return -1; } // Would need ACPI
QString BatteryMonitor::timeRemainingStr() const { return "Unknown"; }
void BatteryMonitor::setUpdateInterval(int ms) { m_updateTimer->setInterval(ms); }

// ============================================================================
// WEATHER API
// ============================================================================

WeatherAPI* WeatherAPI::s_instance = nullptr;

WeatherAPI* WeatherAPI::instance() {
    if (!s_instance) s_instance = new WeatherAPI();
    return s_instance;
}

WeatherAPI::WeatherAPI() : QObject(), m_networkMgr(new QNetworkAccessManager(this)) {
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &WeatherAPI::refresh);
}

void WeatherAPI::setApiKey(const QString& key) { m_apiKey = key; }
void WeatherAPI::setLocation(const QString& location) { m_location = location; }
void WeatherAPI::setCoordinates(double lat, double lon) { m_lat = lat; m_lon = lon; m_useCoords = true; }
void WeatherAPI::setUnits(const QString& units) { m_units = units; }
void WeatherAPI::setUpdateInterval(int ms) { m_updateTimer->setInterval(ms); if (ms > 0) m_updateTimer->start(); }

void WeatherAPI::refresh() {
    if (m_apiKey.isEmpty()) {
        emit error("API key not set");
        return;
    }
    
    QString url = QString("https://api.openweathermap.org/data/2.5/weather?appid=%1&units=%2")
        .arg(m_apiKey).arg(m_units);
    
    if (m_useCoords) {
        url += QString("&lat=%1&lon=%2").arg(m_lat).arg(m_lon);
    } else if (!m_location.isEmpty()) {
        url += "&q=" + m_location;
    } else {
        emit error("Location not set");
        return;
    }
    
    QNetworkReply* reply = m_networkMgr->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            parseResponse(reply->readAll());
        } else {
            emit error(reply->errorString());
        }
        reply->deleteLater();
    });
}

void WeatherAPI::parseResponse(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    m_info.location = obj["name"].toString();
    
    QJsonObject main = obj["main"].toObject();
    m_info.temperature = main["temp"].toDouble();
    m_info.feelsLike = main["feels_like"].toDouble();
    m_info.humidity = main["humidity"].toInt();
    m_info.pressure = main["pressure"].toInt();
    
    QJsonArray weather = obj["weather"].toArray();
    if (!weather.isEmpty()) {
        QJsonObject w = weather[0].toObject();
        m_info.condition = w["main"].toString();
        m_info.description = w["description"].toString();
        m_info.icon = w["icon"].toString();
    }
    
    QJsonObject wind = obj["wind"].toObject();
    m_info.windSpeed = wind["speed"].toDouble();
    m_info.windDirection = wind["deg"].toInt();
    
    emit updated();
}

WeatherInfo WeatherAPI::current() const { return m_info; }
double WeatherAPI::temperature() const { return m_info.temperature; }
double WeatherAPI::feelsLike() const { return m_info.feelsLike; }
int WeatherAPI::humidity() const { return m_info.humidity; }
QString WeatherAPI::condition() const { return m_info.condition; }
QString WeatherAPI::description() const { return m_info.description; }
QString WeatherAPI::icon() const { return m_info.icon; }
double WeatherAPI::windSpeed() const { return m_info.windSpeed; }
int WeatherAPI::windDirection() const { return m_info.windDirection; }
QString WeatherAPI::locationName() const { return m_info.location; }

// ============================================================================
// MEDIA PLAYER (MPRIS)
// ============================================================================

MediaPlayer* MediaPlayer::s_instance = nullptr;

MediaPlayer* MediaPlayer::instance() {
    if (!s_instance) s_instance = new MediaPlayer();
    return s_instance;
}

MediaPlayer::MediaPlayer() : QObject() {
    // MPRIS D-Bus integration would go here
    // For now, stub implementation
}

void MediaPlayer::play() { emit playbackChanged(); }
void MediaPlayer::pause() { emit playbackChanged(); }
void MediaPlayer::playPause() { emit playbackChanged(); }
void MediaPlayer::stop() { emit playbackChanged(); }
void MediaPlayer::next() { emit trackChanged(); }
void MediaPlayer::previous() { emit trackChanged(); }
void MediaPlayer::setVolume(int) {}
void MediaPlayer::seek(int) {}

bool MediaPlayer::isPlaying() const { return m_playing; }
bool MediaPlayer::hasPlayer() const { return m_hasPlayer; }
QString MediaPlayer::playerName() const { return m_playerName; }
MediaInfo MediaPlayer::currentTrack() const { return m_info; }
QString MediaPlayer::title() const { return m_info.title; }
QString MediaPlayer::artist() const { return m_info.artist; }
QString MediaPlayer::album() const { return m_info.album; }
QString MediaPlayer::artUrl() const { return m_info.artUrl; }
int MediaPlayer::duration() const { return m_info.duration; }
int MediaPlayer::position() const { return m_info.position; }
int MediaPlayer::volume() const { return m_volume; }
QStringList MediaPlayer::availablePlayers() const { return {}; }
void MediaPlayer::setActivePlayer(const QString&) {}

// ============================================================================
// NOTIFICATION API
// ============================================================================

NotificationAPI* NotificationAPI::s_instance = nullptr;

NotificationAPI* NotificationAPI::instance() {
    if (!s_instance) s_instance = new NotificationAPI();
    return s_instance;
}

NotificationAPI::NotificationAPI() : QObject() {}

void NotificationAPI::send(const QString& title, const QString& body, const QString& icon, int timeout) {
    // Use notify-send or D-Bus
    QProcess::startDetached("notify-send", {
        "-t", QString::number(timeout),
        "-i", icon,
        title, body
    });
    emit notificationSent(title, body);
}

void NotificationAPI::send(const QString& title, const QString& body) {
    send(title, body, "dialog-information", 5000);
}

// ============================================================================
// CLEANUP
// ============================================================================

void cleanupAPIs() {
    if (SystemMonitor::s_instance) { delete SystemMonitor::s_instance; SystemMonitor::s_instance = nullptr; }
    if (NetworkMonitor::s_instance) { delete NetworkMonitor::s_instance; NetworkMonitor::s_instance = nullptr; }
    if (BatteryMonitor::s_instance) { delete BatteryMonitor::s_instance; BatteryMonitor::s_instance = nullptr; }
    if (WeatherAPI::s_instance) { delete WeatherAPI::s_instance; WeatherAPI::s_instance = nullptr; }
    if (MediaPlayer::s_instance) { delete MediaPlayer::s_instance; MediaPlayer::s_instance = nullptr; }
    if (NotificationAPI::s_instance) { delete NotificationAPI::s_instance; NotificationAPI::s_instance = nullptr; }
}

} // namespace Milk
