/**
 * MilkWidgetCore - System APIs
 * 
 * System monitoring, weather, media player, etc.
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <memory>

#include "Types.h"

namespace Milk {

// ============================================================================
// SYSTEM MONITOR
// ============================================================================
class SystemMonitor : public QObject {
    Q_OBJECT
    
public:
    static SystemMonitor* instance();
    static void cleanup();
    
    // CPU
    double cpu();
    double cpuCore(int core);
    int cpuCores();
    QString cpuModel();
    double cpuFrequency();
    
    // Memory
    double memory();
    qint64 memoryUsed();
    qint64 memoryTotal();
    qint64 memoryAvailable();
    QString memoryUsedStr();
    QString memoryTotalStr();
    
    // Swap
    double swap();
    qint64 swapUsed();
    qint64 swapTotal();
    
    // Disk
    double disk(const QString& path = "/");
    qint64 diskUsed(const QString& path = "/");
    qint64 diskTotal(const QString& path = "/");
    qint64 diskFree(const QString& path = "/");
    QStringList mountPoints();
    
    // Temperature
    double temperature();
    double cpuTemperature();
    double gpuTemperature();
    QMap<QString, double> temperatures();
    
    // System
    QString uptime();
    qint64 uptimeSeconds();
    int processes();
    QString hostname();
    QString username();
    QString osName();
    QString osVersion();
    QString kernelVersion();
    
    // GPU
    double gpuUsage();
    double gpuMemory();
    QString gpuModel();
    
    // Update interval
    void setUpdateInterval(int ms);
    int updateInterval() const { return m_updateInterval; }
    
    // Get all info at once
    SystemInfo info();
    
signals:
    void updated();
    void cpuChanged(double usage);
    void memoryChanged(double usage);
    void temperatureChanged(double temp);
    
private:
    explicit SystemMonitor(QObject* parent = nullptr);
    ~SystemMonitor();
    
    void updateSystemInfo();
    void readCpuInfo();
    void readMemInfo();
    void readDiskInfo();
    void readTempInfo();
    void readProcessInfo();
    
private:
    static SystemMonitor* s_instance;
    
    QTimer* m_timer;
    QMutex m_mutex;
    int m_updateInterval = 1000;
    
    // Cached data
    SystemInfo m_info;
    
    // CPU calculation state
    quint64 m_lastCpuIdle = 0;
    quint64 m_lastCpuTotal = 0;
    QList<QPair<quint64, quint64>> m_lastCoreStats;
    
    // Static info
    QString m_cpuModel;
    int m_cpuCores = 0;
    QString m_hostname;
    QString m_username;
    QString m_osName;
    QString m_osVersion;
    QString m_kernelVersion;
};

// ============================================================================
// NETWORK MONITOR
// ============================================================================
class NetworkMonitor : public QObject {
    Q_OBJECT
    
public:
    static NetworkMonitor* instance();
    static void cleanup();
    
    // Speeds (bytes per second)
    double downloadSpeed();
    double uploadSpeed();
    double downloadSpeedKB();
    double uploadSpeedKB();
    double downloadSpeedMB();
    double uploadSpeedMB();
    
    // Formatted strings
    QString downloadSpeedStr();
    QString uploadSpeedStr();
    
    // Totals
    qint64 totalDownloaded();
    qint64 totalUploaded();
    QString totalDownloadedStr();
    QString totalUploadedStr();
    
    // Interface info
    QString activeInterface();
    QStringList interfaces();
    QString ipAddress(const QString& iface = "");
    QString macAddress(const QString& iface = "");
    bool isConnected();
    bool isWifi();
    QString wifiSSID();
    int wifiSignal();
    
    // Public IP
    QString publicIP();
    void refreshPublicIP();
    
    // Update interval
    void setUpdateInterval(int ms);
    void setInterface(const QString& iface);
    
signals:
    void updated();
    void speedChanged(double download, double upload);
    void connectionChanged(bool connected);
    void publicIPChanged(const QString& ip);
    
private:
    explicit NetworkMonitor(QObject* parent = nullptr);
    ~NetworkMonitor();
    
    void updateNetworkInfo();
    void readNetworkStats();
    
private:
    static NetworkMonitor* s_instance;
    
    QTimer* m_timer;
    QMutex m_mutex;
    QNetworkAccessManager* m_network;
    
    QString m_interface;
    double m_downloadSpeed = 0;
    double m_uploadSpeed = 0;
    qint64 m_totalDownload = 0;
    qint64 m_totalUpload = 0;
    qint64 m_lastRxBytes = 0;
    qint64 m_lastTxBytes = 0;
    QString m_publicIP;
    
    int m_updateInterval = 1000;
};

// ============================================================================
// BATTERY MONITOR
// ============================================================================
class BatteryMonitor : public QObject {
    Q_OBJECT
    
public:
    static BatteryMonitor* instance();
    static void cleanup();
    
    // Status
    int percent();
    bool isCharging();
    bool isPluggedIn();
    bool hasBattery();
    
    // Details
    QString status();  // "Charging", "Discharging", "Full", "Not charging"
    QString timeRemaining();
    int timeRemainingMinutes();
    
    // Health
    double health();
    int cycleCount();
    double voltage();
    double current();
    double power();
    QString technology();
    
    // Thresholds
    void setLowThreshold(int percent);
    void setCriticalThreshold(int percent);
    
signals:
    void updated();
    void percentChanged(int percent);
    void chargingChanged(bool charging);
    void lowBattery(int percent);
    void criticalBattery(int percent);
    
private:
    explicit BatteryMonitor(QObject* parent = nullptr);
    ~BatteryMonitor();
    
    void updateBatteryInfo();
    void findBattery();
    
private:
    static BatteryMonitor* s_instance;
    
    QTimer* m_timer;
    QString m_batteryPath;
    
    int m_percent = 100;
    bool m_charging = false;
    bool m_pluggedIn = false;
    bool m_hasBattery = false;
    QString m_status;
    int m_lowThreshold = 20;
    int m_criticalThreshold = 10;
};

// ============================================================================
// WEATHER API
// ============================================================================
class WeatherAPI : public QObject {
    Q_OBJECT
    
public:
    static WeatherAPI* instance();
    static void cleanup();
    
    // Setup
    void setApiKey(const QString& key);
    void setCity(const QString& city);
    void setCoordinates(double lat, double lon);
    void setUnits(const QString& units);  // "metric", "imperial"
    void setUpdateInterval(int minutes);
    
    // Current weather
    QString temperature();
    int temperatureInt();
    double temperatureDouble();
    QString feelsLike();
    QString condition();
    QString description();
    QString icon();
    QString iconUrl();
    int humidity();
    QString windSpeed();
    QString windDirection();
    double pressure();
    int visibility();
    int cloudiness();
    
    // Sun
    QDateTime sunrise();
    QDateTime sunset();
    bool isDaytime();
    
    // Conditions
    bool isRaining();
    bool isSnowing();
    bool isCloudy();
    bool isSunny();
    bool isStormy();
    
    // Location
    QString city();
    QString country();
    
    // Update
    void refresh();
    QDateTime lastUpdate();
    bool isValid();
    
    // Get all info at once
    WeatherInfo info();
    
signals:
    void updated();
    void error(const QString& message);
    
private:
    explicit WeatherAPI(QObject* parent = nullptr);
    ~WeatherAPI();
    
    void fetchWeather();
    void parseWeatherData(const QJsonObject& data);
    
private slots:
    void onWeatherReply();
    
private:
    static WeatherAPI* s_instance;
    
    QNetworkAccessManager* m_network;
    QTimer* m_timer;
    
    QString m_apiKey;
    QString m_city;
    double m_lat = 0;
    double m_lon = 0;
    QString m_units = "metric";
    
    WeatherInfo m_info;
    QJsonObject m_rawData;
    QDateTime m_lastUpdate;
    bool m_valid = false;
};

// ============================================================================
// MEDIA PLAYER
// ============================================================================
class MediaPlayer : public QObject {
    Q_OBJECT
    
public:
    static MediaPlayer* instance();
    static void cleanup();
    
    // Playback info
    QString title();
    QString artist();
    QString album();
    QString artUrl();
    QImage artImage();
    int duration();
    int position();
    double progress();  // 0.0 - 1.0
    QString durationStr();
    QString positionStr();
    bool isPlaying();
    bool isPaused();
    bool isStopped();
    double volume();
    bool isMuted();
    
    // Player info
    QString playerName();
    QStringList availablePlayers();
    void setPlayer(const QString& player);
    
    // Controls
    void play();
    void pause();
    void playPause();
    void stop();
    void next();
    void previous();
    void seek(int position);
    void seekPercent(double percent);
    void setVolume(double volume);
    void mute();
    void unmute();
    void toggleMute();
    
    // Get all info at once
    MediaInfo info();
    
signals:
    void updated();
    void playbackChanged(bool playing);
    void trackChanged(const QString& title, const QString& artist);
    void positionChanged(int position);
    void volumeChanged(double volume);
    
private:
    explicit MediaPlayer(QObject* parent = nullptr);
    ~MediaPlayer();
    
    void updateMediaInfo();
    void connectToPlayer();
    
private:
    static MediaPlayer* s_instance;
    
    QTimer* m_timer;
    QString m_playerName;
    MediaInfo m_info;
};

// ============================================================================
// NOTIFICATION API
// ============================================================================
class NotificationAPI : public QObject {
    Q_OBJECT
    
public:
    static NotificationAPI* instance();
    static void cleanup();
    
    // Send notification
    void notify(const QString& title, const QString& message);
    void notify(const QString& title, const QString& message, const QString& icon);
    void notify(const QString& title, const QString& message, int timeout);
    
    // Options
    void setDefaultTimeout(int ms);
    void setDefaultIcon(const QString& icon);
    void setAppName(const QString& name);
    
    // Notification history
    struct Notification {
        QString title;
        QString message;
        QString icon;
        QDateTime timestamp;
    };
    
    QList<Notification> history();
    void clearHistory();
    
signals:
    void notificationClicked(int id);
    void notificationClosed(int id);
    
private:
    explicit NotificationAPI(QObject* parent = nullptr);
    ~NotificationAPI();
    
private:
    static NotificationAPI* s_instance;
    
    QString m_appName = "MilkWidget";
    QString m_defaultIcon;
    int m_defaultTimeout = 5000;
    QList<Notification> m_history;
};

// ============================================================================
// GLOBAL ACCESSORS
// ============================================================================

SystemMonitor* sys();
NetworkMonitor* net();
BatteryMonitor* battery();
WeatherAPI* weather();
MediaPlayer* media();
NotificationAPI* notify();

// Global cleanup
void cleanupAPIs();

} // namespace Milk
