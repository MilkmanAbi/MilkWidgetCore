/**
 * MilkWidgetCore - System Monitor Implementation
 */

#include "milk/APIs.h"
#include "milk/Utils.h"

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QProcess>
#include <QStorageInfo>

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <pwd.h>
#endif

namespace Milk {

SystemMonitor* SystemMonitor::s_instance = nullptr;

SystemMonitor* SystemMonitor::instance() {
    if (!s_instance) {
        s_instance = new SystemMonitor();
    }
    return s_instance;
}

void SystemMonitor::cleanup() {
    delete s_instance;
    s_instance = nullptr;
}

SystemMonitor::SystemMonitor(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SystemMonitor::updateSystemInfo);
    m_timer->start(m_updateInterval);
    
    // Get static info
#ifdef Q_OS_LINUX
    m_cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
    m_lastCoreStats.resize(m_cpuCores);
    
    // Hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        m_hostname = QString::fromLocal8Bit(hostname);
    }
    
    // Username
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        m_username = QString::fromLocal8Bit(pw->pw_name);
    }
    
    // CPU model
    QFile cpuinfo("/proc/cpuinfo");
    if (cpuinfo.open(QIODevice::ReadOnly)) {
        QTextStream stream(&cpuinfo);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("model name")) {
                int idx = line.indexOf(':');
                if (idx >= 0) {
                    m_cpuModel = line.mid(idx + 1).trimmed();
                    break;
                }
            }
        }
        cpuinfo.close();
    }
    
    // OS info
    QFile osRelease("/etc/os-release");
    if (osRelease.open(QIODevice::ReadOnly)) {
        QTextStream stream(&osRelease);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("NAME=")) {
                m_osName = line.mid(5).remove('"');
            } else if (line.startsWith("VERSION=")) {
                m_osVersion = line.mid(8).remove('"');
            }
        }
        osRelease.close();
    }
    
    // Kernel version
    QFile version("/proc/version");
    if (version.open(QIODevice::ReadOnly)) {
        QString line = QString::fromLocal8Bit(version.readLine());
        QStringList parts = line.split(' ');
        if (parts.size() >= 3) {
            m_kernelVersion = parts[2];
        }
        version.close();
    }
#endif
    
    // Initial update
    updateSystemInfo();
}

SystemMonitor::~SystemMonitor() {
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
    }
}

void SystemMonitor::setUpdateInterval(int ms) {
    m_updateInterval = ms;
    m_timer->setInterval(ms);
}

void SystemMonitor::updateSystemInfo() {
    QMutexLocker locker(&m_mutex);
    
    readCpuInfo();
    readMemInfo();
    readDiskInfo();
    readTempInfo();
    readProcessInfo();
    
    emit updated();
}

void SystemMonitor::readCpuInfo() {
#ifdef Q_OS_LINUX
    QFile stat("/proc/stat");
    if (!stat.open(QIODevice::ReadOnly)) return;
    
    QTextStream stream(&stat);
    QString line = stream.readLine();
    QStringList values = line.split(' ', Qt::SkipEmptyParts);
    
    if (values.size() >= 8 && values[0] == "cpu") {
        quint64 user = values[1].toULongLong();
        quint64 nice = values[2].toULongLong();
        quint64 system = values[3].toULongLong();
        quint64 idle = values[4].toULongLong();
        quint64 iowait = values[5].toULongLong();
        quint64 irq = values[6].toULongLong();
        quint64 softirq = values[7].toULongLong();
        
        quint64 total = user + nice + system + idle + iowait + irq + softirq;
        
        if (m_lastCpuTotal > 0) {
            quint64 totalDiff = total - m_lastCpuTotal;
            quint64 idleDiff = idle - m_lastCpuIdle;
            
            if (totalDiff > 0) {
                m_info.cpuUsage = 100.0 * (totalDiff - idleDiff) / totalDiff;
            }
        }
        
        m_lastCpuTotal = total;
        m_lastCpuIdle = idle;
    }
    
    stat.close();
#endif
}

void SystemMonitor::readMemInfo() {
#ifdef Q_OS_LINUX
    QFile mem("/proc/meminfo");
    if (!mem.open(QIODevice::ReadOnly)) return;
    
    qint64 memTotal = 0;
    qint64 memFree = 0;
    qint64 memAvailable = 0;
    qint64 buffers = 0;
    qint64 cached = 0;
    
    QTextStream stream(&mem);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        
        if (parts.size() >= 2) {
            QString key = parts[0].remove(':');
            qint64 value = parts[1].toLongLong() * 1024;  // kB to bytes
            
            if (key == "MemTotal") memTotal = value;
            else if (key == "MemFree") memFree = value;
            else if (key == "MemAvailable") memAvailable = value;
            else if (key == "Buffers") buffers = value;
            else if (key == "Cached") cached = value;
        }
    }
    mem.close();
    
    if (memAvailable == 0) {
        memAvailable = memFree + buffers + cached;
    }
    
    qint64 memUsed = memTotal - memAvailable;
    
    if (memTotal > 0) {
        m_info.memoryUsage = 100.0 * memUsed / memTotal;
    }
#endif
}

void SystemMonitor::readDiskInfo() {
    QStorageInfo storage("/");
    if (storage.isValid()) {
        qint64 total = storage.bytesTotal();
        qint64 free = storage.bytesFree();
        
        if (total > 0) {
            m_info.diskUsage = 100.0 * (total - free) / total;
        }
    }
}

void SystemMonitor::readTempInfo() {
#ifdef Q_OS_LINUX
    // Try various temperature sources
    QStringList tempPaths = {
        "/sys/class/thermal/thermal_zone0/temp",
        "/sys/class/hwmon/hwmon0/temp1_input",
        "/sys/class/hwmon/hwmon1/temp1_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp1_input"
    };
    
    for (const QString& path : tempPaths) {
        QFile temp(path);
        if (temp.open(QIODevice::ReadOnly)) {
            QString value = QString::fromLocal8Bit(temp.readAll()).trimmed();
            double tempVal = value.toDouble();
            
            // Temperature in millidegrees Celsius
            if (tempVal > 1000) {
                tempVal /= 1000.0;
            }
            
            m_info.temperature = tempVal;
            temp.close();
            break;
        }
    }
#endif
}

void SystemMonitor::readProcessInfo() {
#ifdef Q_OS_LINUX
    QDir procDir("/proc");
    int count = 0;
    
    for (const QString& entry : procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        bool ok;
        entry.toInt(&ok);
        if (ok) count++;
    }
    
    m_info.processCount = count;
    
    // Uptime
    QFile uptime("/proc/uptime");
    if (uptime.open(QIODevice::ReadOnly)) {
        QString line = QString::fromLocal8Bit(uptime.readLine());
        QStringList parts = line.split(' ');
        if (!parts.isEmpty()) {
            double seconds = parts[0].toDouble();
            int days = static_cast<int>(seconds / 86400);
            int hours = static_cast<int>((seconds - days * 86400) / 3600);
            int minutes = static_cast<int>((seconds - days * 86400 - hours * 3600) / 60);
            
            if (days > 0) {
                m_info.uptime = QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
            } else if (hours > 0) {
                m_info.uptime = QString("%1h %2m").arg(hours).arg(minutes);
            } else {
                m_info.uptime = QString("%1m").arg(minutes);
            }
        }
        uptime.close();
    }
#endif
}

// ============================================================================
// GETTERS
// ============================================================================

double SystemMonitor::cpu() {
    QMutexLocker locker(&m_mutex);
    return m_info.cpuUsage;
}

double SystemMonitor::cpuCore(int core) {
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(core)
    // Per-core implementation would go here
    return m_info.cpuUsage;
}

int SystemMonitor::cpuCores() {
    return m_cpuCores;
}

QString SystemMonitor::cpuModel() {
    return m_cpuModel;
}

double SystemMonitor::cpuFrequency() {
#ifdef Q_OS_LINUX
    QFile freq("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (freq.open(QIODevice::ReadOnly)) {
        QString value = QString::fromLocal8Bit(freq.readAll()).trimmed();
        freq.close();
        return value.toDouble() / 1000.0;  // kHz to MHz
    }
#endif
    return 0;
}

double SystemMonitor::memory() {
    QMutexLocker locker(&m_mutex);
    return m_info.memoryUsage;
}

qint64 SystemMonitor::memoryUsed() {
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (info.totalram - info.freeram) * info.mem_unit;
    }
#endif
    return 0;
}

qint64 SystemMonitor::memoryTotal() {
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalram * info.mem_unit;
    }
#endif
    return 0;
}

qint64 SystemMonitor::memoryAvailable() {
    return memoryTotal() - memoryUsed();
}

QString SystemMonitor::memoryUsedStr() {
    return String::formatBytes(memoryUsed());
}

QString SystemMonitor::memoryTotalStr() {
    return String::formatBytes(memoryTotal());
}

double SystemMonitor::swap() {
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0 && info.totalswap > 0) {
        return 100.0 * (info.totalswap - info.freeswap) / info.totalswap;
    }
#endif
    return 0;
}

qint64 SystemMonitor::swapUsed() {
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (info.totalswap - info.freeswap) * info.mem_unit;
    }
#endif
    return 0;
}

qint64 SystemMonitor::swapTotal() {
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalswap * info.mem_unit;
    }
#endif
    return 0;
}

double SystemMonitor::disk(const QString& path) {
    QStorageInfo storage(path);
    if (storage.isValid() && storage.bytesTotal() > 0) {
        return 100.0 * (storage.bytesTotal() - storage.bytesFree()) / storage.bytesTotal();
    }
    return 0;
}

qint64 SystemMonitor::diskUsed(const QString& path) {
    QStorageInfo storage(path);
    return storage.bytesTotal() - storage.bytesFree();
}

qint64 SystemMonitor::diskTotal(const QString& path) {
    QStorageInfo storage(path);
    return storage.bytesTotal();
}

qint64 SystemMonitor::diskFree(const QString& path) {
    QStorageInfo storage(path);
    return storage.bytesFree();
}

QStringList SystemMonitor::mountPoints() {
    QStringList points;
    for (const QStorageInfo& storage : QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            points << storage.rootPath();
        }
    }
    return points;
}

double SystemMonitor::temperature() {
    QMutexLocker locker(&m_mutex);
    return m_info.temperature;
}

double SystemMonitor::cpuTemperature() {
    return temperature();
}

double SystemMonitor::gpuTemperature() {
    // Would need NVIDIA/AMD specific implementation
    return 0;
}

QMap<QString, double> SystemMonitor::temperatures() {
    QMap<QString, double> temps;
    temps["cpu"] = cpuTemperature();
    temps["gpu"] = gpuTemperature();
    return temps;
}

QString SystemMonitor::uptime() {
    QMutexLocker locker(&m_mutex);
    return m_info.uptime;
}

qint64 SystemMonitor::uptimeSeconds() {
#ifdef Q_OS_LINUX
    QFile uptime("/proc/uptime");
    if (uptime.open(QIODevice::ReadOnly)) {
        QString line = QString::fromLocal8Bit(uptime.readLine());
        QStringList parts = line.split(' ');
        if (!parts.isEmpty()) {
            uptime.close();
            return static_cast<qint64>(parts[0].toDouble());
        }
        uptime.close();
    }
#endif
    return 0;
}

int SystemMonitor::processes() {
    QMutexLocker locker(&m_mutex);
    return m_info.processCount;
}

QString SystemMonitor::hostname() {
    return m_hostname;
}

QString SystemMonitor::username() {
    return m_username;
}

QString SystemMonitor::osName() {
    return m_osName;
}

QString SystemMonitor::osVersion() {
    return m_osVersion;
}

QString SystemMonitor::kernelVersion() {
    return m_kernelVersion;
}

double SystemMonitor::gpuUsage() {
    // Would need NVIDIA/AMD specific implementation
    return 0;
}

double SystemMonitor::gpuMemory() {
    return 0;
}

QString SystemMonitor::gpuModel() {
    return QString();
}

SystemInfo SystemMonitor::info() {
    QMutexLocker locker(&m_mutex);
    return m_info;
}

// ============================================================================
// GLOBAL ACCESSOR
// ============================================================================

SystemMonitor* sys() {
    return SystemMonitor::instance();
}

} // namespace Milk
