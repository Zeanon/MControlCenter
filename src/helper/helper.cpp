/* Copyright (C) 2022  Dmitry Serov
 *
 * This file is part of MControlCenter.
 *
 * MControlCenter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * MControlCenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MControlCenter. If not, see <https://www.gnu.org/licenses/>.
 */
#include "helper.h"

#include "readwrite.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QProcess>
#include <QFile>

#include <algorithm>
#include <string>

ReadWrite rw;
static QByteArray cachedEcData;

void Helper::quit() const {
    QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit);
}

namespace {
    constexpr uint8_t cpuTempAddress = 0x68;
    constexpr uint8_t gpuTempAddress = 0x80;
    constexpr uint8_t batteryChargeAddress = 0x42;
    constexpr uint8_t batteryThresholdAddress = 0xD7;
    constexpr uint8_t batteryChargingStatusAddress = 0x31;
    constexpr uint8_t batteryNotCharging = 0x01;
    constexpr uint8_t batteryCharging = 0x03;
    constexpr uint8_t batteryDischarging = 0x05;
    constexpr uint8_t batteryFullyCharged = 0x09;
    constexpr uint8_t batteryFullyChargedNoPower = 0x0D;
    constexpr uint8_t keyboardBacklightModeAddress = 0x2C;
    constexpr uint8_t keyboardBacklightAutoTurnOffBit = 0x08;
    constexpr uint8_t keyboardBacklightAddress = 0xD3;
    constexpr uint8_t keyboardBacklightBaseState = 0x80;
    constexpr uint8_t keyboardBacklightMaxState = 3;
    constexpr uint8_t usbPowerShareAddress = 0xBF;
    constexpr uint8_t usbPowerShareBit = 0x20;
    constexpr uint8_t coolerBoostAddress = 0x98;
    constexpr uint8_t coolerBoostBit = 0x80;
    constexpr uint8_t fnMetaSwapAddress = 0xE8;
    constexpr uint8_t fnMetaSwapBit = 0x10;
    constexpr uint8_t fan1Address = 0xC9;
    constexpr uint8_t fan2Address = 0xCB;
    constexpr uint8_t fan1SpeedSettingStartAddress = 0x72;
    constexpr uint8_t fan2SpeedSettingStartAddress = 0x8A;
    constexpr uint8_t fan1TempSettingStartAddress = 0x6A;
    constexpr uint8_t fan2TempSettingStartAddress = 0x82;
    constexpr uint8_t fanSpeedSettingsCount = 7;
    constexpr uint8_t fanTempSettingsCount = 6;
    constexpr uint8_t shiftModeAddress = 0xD2;
    constexpr uint8_t shiftModeTurbo = 0xC5;
    constexpr uint8_t shiftModePerformance = 0xC4;
    constexpr uint8_t shiftModeBalanced = 0xC1;
    constexpr uint8_t shiftModeEco = 0xC2;
    constexpr uint8_t fanModeAddress = 0xD4;
    constexpr uint8_t fanModeAuto = 0x0D;
    constexpr uint8_t fanModeSilent = 0x1D;
    constexpr uint8_t fanModeBasic = 0x4D;
    constexpr uint8_t fanModeAdvanced = 0x8D;

    inline QByteArray ecData() {
        return cachedEcData;
    }

    static inline uint8_t readValue(const QByteArray &data, const uint8_t address) {
        return address < data.size() ? static_cast<uint8_t>(data.at(address)) : 0;
    }

    static inline QList<int> readValueList(const QByteArray &data, uint8_t start, int count) {
        QList<int> values;
        if (data.size() < static_cast<int>(start) + count)
            return values;
        for (int i = 0; i < count; ++i) {
            values.append(readValue(data, static_cast<uint8_t>(start + i)));
        }
        return values;
    }

    static inline bool writeBit(ReadWrite &rw, const uint8_t address, const uint8_t bit, const bool enabled) {
        const uint8_t current = readValue(ecData(), address);
        const uint8_t new_value = enabled ? current | bit : current & ~bit;
        if (new_value != current) {
            rw.writeToFile(address, new_value);
            return true;
        }
        return false;
    }

    static bool isValidSettings(
        const QList<int> &values,
        const int expectedSize
    ) {
        return values.size() == expectedSize &&
            std::all_of(
                values.cbegin(),
                values.cend(),
                [](const int value) {
                    return value >= 0 && value <= 255;
                }
            );
    }

    static inline void writeSettings(ReadWrite &rw, const QList<int> &values, const int expectedSize,
                    const uint8_t startAddress) {
        if (
            values.size() != expectedSize ||
            std::any_of(
                values.cbegin(),
                values.cend(),
                [](int value) {
                    return value < 0 || value > 255;
                }
            )
        ) {
            throw QDBusError(
                QDBusError::Failed,
                QStringLiteral("Value out of range (0-255)")
            );
        }
        for (int i = 0; i < values.size(); ++i) {
            const uint8_t current = readValue(ecData(), startAddress + i);
            const uint8_t new_value = static_cast<uint8_t>(values.at(i));
            if (current != new_value)
                rw.writeToFile(startAddress + i, new_value);
        }
    }
}

bool Helper::updateData() const {
    cachedEcData = rw.readFromFile();
    return cachedEcData.size() == 256;
}

static inline QVariantList toVariantList(const QList<int> &values) {
    QVariantList list;
    list.reserve(values.size());
    for (const int value : values) {
        list.append(value);
    }
    return list;
}

QVariantMap Helper::getStatusSnapshot() const {
    QVariantMap result;
    const bool valid = updateData();

    result.insert(QStringLiteral("valid"), valid);
    if (!valid)
        return result;

    result.insert(QStringLiteral("ecVersion"), getEcVersion());
    result.insert(QStringLiteral("ecBuild"), getEcBuild());
    result.insert(QStringLiteral("modelName"), getModelName());
    result.insert(QStringLiteral("batteryCharge"), getBatteryCharge());
    result.insert(QStringLiteral("batteryThreshold"), getBatteryThreshold());
    result.insert(QStringLiteral("chargingStatus"), getChargingStatus());
    result.insert(QStringLiteral("cpuTemp"), getCpuTemp());
    result.insert(QStringLiteral("gpuTemp"), getGpuTemp());
    result.insert(QStringLiteral("fan1Speed"), getFan1Speed());
    result.insert(QStringLiteral("fan2Speed"), getFan2Speed());
    result.insert(QStringLiteral("fan1SpeedSettings"), toVariantList(getFan1SpeedSettings()));
    result.insert(QStringLiteral("fan2SpeedSettings"), toVariantList(getFan2SpeedSettings()));
    result.insert(QStringLiteral("fan1TempSettings"), toVariantList(getFan1TempSettings()));
    result.insert(QStringLiteral("fan2TempSettings"), toVariantList(getFan2TempSettings()));
    result.insert(QStringLiteral("keyboardBacklightMode"), getKeyboardBacklightMode());
    result.insert(QStringLiteral("keyboardBrightness"), getKeyboardBrightness());
    result.insert(QStringLiteral("usbPowerShareState"), getUsbPowerShareState());
    result.insert(QStringLiteral("fnMetaSwapState"), getFnMetaSwapState());
    result.insert(QStringLiteral("coolerBoostState"), getCoolerBoostState());
    result.insert(QStringLiteral("userMode"), getUserMode());
    result.insert(QStringLiteral("fanMode"), getFanMode());
    result.insert(QStringLiteral("ecModuleLoaded"), isEcModuleLoaded());
    result.insert(QStringLiteral("isAcpiEc"), isAcpiEc());
    result.insert(QStringLiteral("isEcSys"), isEcSys());
    return result;
}

QVariantMap Helper::getMetricsSnapshot() const
{
    QVariantMap result;

    result.insert("batteryCharge", getBatteryCharge());
    result.insert("chargingStatus", getChargingStatus());
    result.insert("cpuTemp", getCpuTemp());
    result.insert("gpuTemp", getGpuTemp());
    result.insert("fan1Speed", getFan1Speed());
    result.insert("fan2Speed", getFan2Speed());
    return result;
}

QVariantMap Helper::getConfigurationSnapshot() const
{
    QVariantMap result;

    result.insert("batteryThreshold", getBatteryThreshold());
    result.insert("keyboardBacklightMode", getKeyboardBacklightMode());
    result.insert("keyboardBrightness", getKeyboardBrightness());
    result.insert("usbPowerShareState", getUsbPowerShareState());
    result.insert("fnMetaSwapState", getFnMetaSwapState());
    result.insert("coolerBoostState", getCoolerBoostState());
    result.insert("userMode", getUserMode());
    result.insert("fanMode", getFanMode());
    result.insert("fan1SpeedSettings", toVariantList(getFan1SpeedSettings()));
    result.insert("fan2SpeedSettings", toVariantList(getFan2SpeedSettings()));
    result.insert("fan1TempSettings", toVariantList(getFan1TempSettings()));
    result.insert("fan2TempSettings", toVariantList(getFan2TempSettings()));
    return result;
}

QString Helper::getEcVersion() const {
    return QString::fromLocal8Bit(ecData().mid(0xA0, 12));
}

QString Helper::getEcBuild() const {
    const std::string build = ecData().mid(0xAC, 16).toStdString();
    if (build.size() < 16) return QString::fromStdString(build);
    return QString::fromStdString(build.substr(0, 2) + "/" + build.substr(2, 2) + "/" + build.substr(4, 4) + " " + build.substr(8, 8));
}

QString Helper::getModelName() const {
    QFile file("/sys/devices/virtual/dmi/id/product_name");
    if (file.open(QIODevice::ReadOnly)) return QString::fromLocal8Bit(file.readAll()).trimmed();
    return {};
}

int Helper::getBatteryCharge() const {
    return static_cast<int>(readValue(ecData(), batteryChargeAddress)) - 1;
}

int Helper::getBatteryThreshold() const {
    return readValue(ecData(), batteryThresholdAddress) & 0x7F;
}

int Helper::getChargingStatus() const {
    switch (readValue(ecData(), batteryChargingStatusAddress)) {
        case batteryCharging:
            return 0;
        case batteryDischarging:
            return 1;
        case batteryNotCharging:
            return 2;
        case batteryFullyCharged:
            return 3;
        case batteryFullyChargedNoPower:
            return 4;
        default:
            return 5;
    }
}

int Helper::getCpuTemp() const {
    return readValue(ecData(), cpuTempAddress);
}

int Helper::getGpuTemp() const {
    const auto data = ecData();
    return gpuTempAddress < data.size() ? readValue(data, gpuTempAddress) : -1;
}

int Helper::getFan1Speed() const {
    const auto data = ecData();
    const int value = (readValue(data, fan1Address - 1) << 8) | readValue(data, fan1Address);
    return value > 0 ? 480000 / value : value;
}

int Helper::getFan2Speed() const {
    const auto data = ecData();
    if (data.size() <= fan2Address) {
        return -1;
    }
    const int value = (readValue(data, fan2Address - 1) << 8) | readValue(data, fan2Address);
    return value > 0 ? 480000 / value : value;
}

QList<int> Helper::getFan1SpeedSettings() const {
    return readValueList(ecData(), fan1SpeedSettingStartAddress, fanSpeedSettingsCount);
}

QList<int> Helper::getFan2SpeedSettings() const {
    return readValueList(ecData(), fan2SpeedSettingStartAddress, fanSpeedSettingsCount);
}

QList<int> Helper::getFan1TempSettings() const {
    return readValueList(ecData(), fan1TempSettingStartAddress, fanTempSettingsCount);
}

QList<int> Helper::getFan2TempSettings() const {
    return readValueList(ecData(), fan2TempSettingStartAddress, fanTempSettingsCount);
}

int Helper::getKeyboardBacklightMode() const {
    return (readValue(ecData(), keyboardBacklightModeAddress) & keyboardBacklightAutoTurnOffBit) ? 1 : 0;
}

int Helper::getKeyboardBrightness() const {
    return readValue(ecData(), keyboardBacklightAddress) & 0x7F;
}

bool Helper::getUsbPowerShareState() const {
    return readValue(ecData(), usbPowerShareAddress) & usbPowerShareBit;
}

bool Helper::getFnMetaSwapState() const {
    return readValue(ecData(), fnMetaSwapAddress) & fnMetaSwapBit;
}

bool Helper::getCoolerBoostState() const {
    return readValue(ecData(), coolerBoostAddress) & coolerBoostBit;
}

int Helper::getUserMode() const {
    const auto data = ecData();
    switch (readValue(data, shiftModeAddress)) {
        case shiftModeTurbo:
            return static_cast<int>(UserMode::Turbo);
        case shiftModePerformance:
            return static_cast<int>(UserMode::Performance);
        case shiftModeBalanced:
            return static_cast<int>(readValue(data, fanModeAddress) == fanModeSilent ? UserMode::Silent : UserMode::Balanced);
        case shiftModeEco:
            return static_cast<int>(UserMode::Eco);
        default:
            return static_cast<int>(UserMode::Unknown);
    }
}

int Helper::getFanMode() const {
    switch (readValue(ecData(), fanModeAddress)) {
        case fanModeAuto:
            return static_cast<int>(FanMode::Auto);
        case fanModeSilent:
            return static_cast<int>(FanMode::Silent);
        case fanModeBasic:
            return static_cast<int>(FanMode::Basic);
        case fanModeAdvanced:
            return static_cast<int>(FanMode::Advanced);
        default:
            return static_cast<int>(FanMode::Unknown);
    }
}

void Helper::setBatteryThreshold(const int value) {
    if (value < 0 || value > 100)
        throw QDBusError(
            QDBusError::Failed,
            QStringLiteral("Value out of range (0-100)")
        );
    const uint8_t new_value = value | 0x80;
    if (new_value != readValue(ecData(), batteryThresholdAddress)) {
        rw.writeToFile(batteryThresholdAddress, new_value);
        checkStatusChanges();
    }
}

void Helper::setKeyboardBacklightMode(const bool enabled) {
    if (writeBit(rw, keyboardBacklightModeAddress, keyboardBacklightAutoTurnOffBit, enabled))
        checkStatusChanges();
}

bool Helper::toggleKeyboardBacklightMode(const bool forceUpdate) {
    if (forceUpdate && !updateData())
        throw QDBusError(
            QDBusError::Failed,
            QStringLiteral("Data could not be updated")
        );
    const bool enable = !getKeyboardBacklightMode();
    writeBit(rw, keyboardBacklightModeAddress, keyboardBacklightAutoTurnOffBit, enable);
    checkStatusChanges();
    return enable;
}

void Helper::setKeyboardBrightness(const int value) {
    if (value < 0 || value > keyboardBacklightMaxState)
        throw QDBusError(
            QDBusError::Failed,
            QStringLiteral("Value out of range (0-3)")
        );
    const uint8_t new_value = keyboardBacklightBaseState | value;
    if (new_value != readValue(ecData(), keyboardBacklightAddress)) {
        rw.writeToFile(keyboardBacklightAddress, new_value);
        checkStatusChanges();
    }
}

void Helper::setUsbPowerShareState(const bool enabled) {
    if (writeBit(rw, usbPowerShareAddress, usbPowerShareBit, enabled))
        checkStatusChanges();
}

void Helper::setFnMetaSwapState(const bool enabled) {
    if (writeBit(rw, fnMetaSwapAddress, fnMetaSwapBit, enabled))
        checkStatusChanges();
}

void Helper::setCoolerBoostState(const bool enabled) {
    if (writeBit(rw, coolerBoostAddress, coolerBoostBit, enabled))
        checkStatusChanges();
}

void Helper::setTurboMode(const bool enabled) {
    const uint8_t current = readValue(ecData(), shiftModeAddress);
    if (enabled && current == shiftModePerformance) {
        rw.writeToFile(shiftModeAddress, shiftModeTurbo);
        checkStatusChanges();
    } else if (!enabled && current == shiftModeTurbo) {
        rw.writeToFile(shiftModeAddress, shiftModePerformance);
        checkStatusChanges();
    }
}

void Helper::setUserMode(const int mode) {
    uint8_t shiftMode = 0;
    uint8_t fanMode = fanModeAuto;
    switch (static_cast<UserMode>(mode)) {
        case UserMode::Turbo: // turbo
            shiftMode = shiftModeTurbo;
            break;
        case UserMode::Performance: // performance
            shiftMode = shiftModePerformance;
            break;
        case UserMode::Balanced: // balanced
            shiftMode = shiftModeBalanced;
            break;
        case UserMode::Silent: // silent
            shiftMode = shiftModeBalanced;
            fanMode = fanModeSilent;
            break;
        case UserMode::Eco: // eco silent
            shiftMode = shiftModeEco;
            fanMode = fanModeSilent;
            break;
        default:
            return;
    }
    rw.writeToFile(shiftModeAddress, shiftMode);
    if (readValue(rw.readFromFile(), fanModeAddress) != fanModeAdvanced)
        rw.writeToFile(fanModeAddress, fanMode);
    checkStatusChanges();
}

bool Helper::setFanSettings(
    const QList<int> &fan1Speed,
    const QList<int> &fan2Speed,
    const QList<int> &fan1Temp,
    const QList<int> &fan2Temp
) {
    // Alle vier Kurven vor dem Schreiben validieren.
    if (!isValidSettings(fan1Speed, fanSpeedSettingsCount) ||
        !isValidSettings(fan2Speed, fanSpeedSettingsCount) ||
        !isValidSettings(fan1Temp, fanTempSettingsCount) ||
        !isValidSettings(fan2Temp, fanTempSettingsCount)) {
        return false;
    }

    // Alle vier Kurven schreiben.
    writeSettings(rw, fan1Speed,
                  fanSpeedSettingsCount,
                  fan1SpeedSettingStartAddress);

    writeSettings(rw, fan2Speed,
                  fanSpeedSettingsCount,
                  fan2SpeedSettingStartAddress);

    writeSettings(rw, fan1Temp,
                  fanTempSettingsCount,
                  fan1TempSettingStartAddress);

    writeSettings(rw, fan2Temp,
                  fanTempSettingsCount,
                  fan2TempSettingStartAddress);

    // Status nur einmal aktualisieren.
    checkStatusChanges();

    return true;
}

void Helper::setFanModeAdvanced(const bool enabled) {
    const uint8_t current = readValue(ecData(), fanModeAddress);
    if (enabled && current != fanModeAdvanced) {
        rw.writeToFile(fanModeAddress, fanModeAdvanced);
        checkStatusChanges();
    } else {
        UserMode userMode = static_cast<UserMode>(getUserMode());
        const uint8_t new_value = (userMode == UserMode::Silent || userMode == UserMode::Eco) ? fanModeSilent : fanModeAuto;
        if (current != new_value) {
            rw.writeToFile(fanModeAddress, new_value);
            checkStatusChanges();
        }
    }
}

bool Helper::isEcModuleLoaded() const {
    const bool isAcpiEc = rw.isAcpiEc();
    if (rw.useAcpiEc()) {
        if (!isAcpiEc)
            fprintf(stderr, "%s\n", qPrintable("The acpi_ec kernel module is loaded"));
        return true;
    }
    const bool isEcSys = rw.isEcSys();
    if (rw.useEcSys()) {
        if (!isEcSys)
            fprintf(stderr, "%s\n", qPrintable("The ec_sys kernel module is loaded"));
        return true;
    }
    if (isAcpiEc || isEcSys)
        fprintf(stderr, "%s\n", qPrintable("The acpi_ec and ec_sys kernel module are not loaded"));
    return false;
}

bool Helper::isAcpiEc() const {
    return rw.isAcpiEc();
}

bool Helper::isEcSys() const {
    return rw.isEcSys();
}

bool Helper::loadEcModule() {
    auto *process = new QProcess();
    process->start("sh", QStringList() << "-c" << "/usr/sbin/modprobe ec_sys write_support=1 2>&1");
    process->waitForFinished(1000);
    if (QByteArray output = process->readAllStandardOutput(); output != "")
        fprintf(stderr, "%s", qPrintable(output));
    if (isEcModuleLoaded())
        return true;
    return false;
}

void Helper::startMonitoring() {
    if (statusTimer)
        return;

    if (!updateData())
        return;

    lastMetrics = getMetricsSnapshot();
    lastConfiguration = getConfigurationSnapshot();

    statusTimer = new QTimer(this);
    statusTimer->setInterval(500);

    connect(statusTimer, &QTimer::timeout,
            this, &Helper::checkStatusChanges);

    statusTimer->start();
}

void Helper::stopMonitoring() {
    if (!statusTimer)
        return;
    
    statusTimer->stop();
    statusTimer = nullptr;
}

void Helper::checkStatusChanges() {
    if (!updateData())
        return;

    const QVariantMap metrics = getMetricsSnapshot();
    const QVariantMap configuration = getConfigurationSnapshot();

    if (metrics != lastMetrics) {
        lastMetrics = metrics;
        emit MetricsChanged(metrics);
    }

    if (configuration != lastConfiguration) {
        lastConfiguration = configuration;
        emit ConfigurationChanged(configuration);
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QObject obj;
    auto *helper = new Helper(&obj);

    QObject::connect(&a, &QCoreApplication::aboutToQuit, helper, &Helper::aboutToQuit);

    if (!QDBusConnection::systemBus().registerObject("/", &obj, QDBusConnection::ExportAdaptors)) {
        fprintf(stderr, "%s\n", qPrintable(QDBusConnection::systemBus().lastError().message()));
        return 1;
    }

    if (!QDBusConnection::systemBus().registerService(SERVICE_NAME)) {
        fprintf(stderr, "%s\n", qPrintable(QDBusConnection::systemBus().lastError().message()));
        return 1;
    }

    //helper->startMonitoring();
    if (!helper->loadEcModule()) {
        fprintf(stderr, "%s\n", "Could not load the EC module");
        return 1;
    }

    return QCoreApplication::exec();
}
