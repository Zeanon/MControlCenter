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
    constexpr uint8_t shiftModeBattery = 0xC2;
    constexpr uint8_t fanModeAddress = 0xD4;
    constexpr uint8_t fanModeAuto = 0x0D;
    constexpr uint8_t fanModeSilent = 0x1D;
    constexpr uint8_t fanModeBasic = 0x4D;
    constexpr uint8_t fanModeAdvanced = 0x8D;

    QByteArray ecData() {
        return cachedEcData;
    }

    uint8_t readValue(const QByteArray &data, const uint8_t address) {
        return address < data.size() ? static_cast<uint8_t>(data.at(address)) : 0;
    }

    QList<int> readValueList(const QByteArray &data, uint8_t start, int count) {
        QList<int> values;
        if (data.size() < static_cast<int>(start) + count)
            return values;
        for (int i = 0; i < count; ++i) {
            values.append(readValue(data, static_cast<uint8_t>(start + i)));
        }
        return values;
    }

    void writeBit(ReadWrite &rw, const uint8_t address, const uint8_t bit, const bool enabled) {
        const QByteArray data = rw.readFromFile();
        const uint8_t current = readValue(data, address);
        rw.writeToFile(address, enabled ? current | bit : current & static_cast<uint8_t>(~bit));
    }

    void writeSettings(ReadWrite &rw, const QList<int> &values, const int expectedSize,
                    const uint8_t startAddress) {
        if (values.size() != expectedSize || std::any_of(values.cbegin(), values.cend(),
                [](int value) { return value < 0 || value > 255; }))
            return;
        for (int i = 0; i < values.size(); ++i)
            rw.writeToFile(static_cast<uint8_t>(startAddress + i), static_cast<uint8_t>(values.at(i)));
    }
}

bool Helper::updateData() const {
    cachedEcData = rw.readFromFile();
    return cachedEcData.size() == 256;
}

QVariantMap Helper::getStatusSnapshot() const {
    QVariantMap result;
    const bool valid = updateData();

    result.insert(QStringLiteral("valid"), valid);
    if (!valid) {
        return result;
    }

    const auto toVariantList = [](const QList<int> &values) {
        QVariantList list;
        list.reserve(values.size());
        for (const int value : values) list.append(value);
        return list;
    };

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

    const auto toVariantList = [](const QList<int> &values) {
        QVariantList list;
        for (const int value : values)
            list.append(value);
        return list;
    };

    result.insert("fan1SpeedSettings", toVariantList(getFan1SpeedSettings()));
    result.insert("fan2SpeedSettings", toVariantList(getFan2SpeedSettings()));
    result.insert("fan1TempSettings", toVariantList(getFan1TempSettings()));
    result.insert("fan2TempSettings", toVariantList(getFan2TempSettings()));

    return result;
}

QString Helper::getEcVersion() const {
    return QString::fromLatin1(ecData().mid(0xA0, 12));
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
    const auto d = ecData();
    return gpuTempAddress < d.size() ? readValue(d, gpuTempAddress) : -1;
}

int Helper::getFan1Speed() const {
    const auto d = ecData();
    const int value = (readValue(d, fan1Address - 1) << 8) | readValue(d, fan1Address);
    return value > 0 ? 480000 / value : value;
}

int Helper::getFan2Speed() const {
    const auto d = ecData();
    if (d.size() <= fan2Address) {
        return -1;
    }
    const int value = (readValue(d, fan2Address - 1) << 8) | readValue(d, fan2Address);
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
    const auto d = ecData();
    switch (readValue(d, shiftModeAddress)) {
        case shiftModeTurbo:
            return 4;
        case shiftModePerformance:
            return 0;
        case shiftModeBalanced:
            return readValue(d, fanModeAddress) == fanModeSilent ? 2 : 1;
        case shiftModeBattery:
            return 3;
        default:
            return -1;
    }
}
int Helper::getFanMode() const {
    switch (readValue(ecData(), fanModeAddress)) {
        case fanModeAuto:
            return 0;
        case fanModeSilent:
            return 1;
        case fanModeBasic:
            return 2;
        case fanModeAdvanced:
            return 3;
        default:
            return -1;
    }
}

void Helper::setBatteryThreshold(const int value) {
    if (value < 0 || value > 100)
        return;
    const uint8_t current = readValue(rw.readFromFile(), batteryThresholdAddress) & 0x7F;
    if (current != value)
        rw.writeToFile(batteryThresholdAddress, static_cast<uint8_t>(value | 0x80));
    checkStatusChanges();
}

void Helper::setKeyboardBacklightMode(const bool enabled) {
    writeBit(rw, keyboardBacklightModeAddress, keyboardBacklightAutoTurnOffBit, enabled);
    checkStatusChanges();
}

bool Helper::toggleKeyboardBacklightMode(const bool forceUpdate) {
    if (forceUpdate && !updateData())
        return false;
    const bool enable = !getKeyboardBacklightMode();
    writeBit(rw, keyboardBacklightModeAddress, keyboardBacklightAutoTurnOffBit, enable);
    checkStatusChanges();
    return enable;
}

void Helper::setKeyboardBrightness(const int value) {
    if (value < 0 || value > keyboardBacklightMaxState)
        return;
    rw.writeToFile(keyboardBacklightAddress, static_cast<uint8_t>(keyboardBacklightBaseState | value));
    checkStatusChanges();
}

void Helper::setUsbPowerShareState(const bool enabled) {
    writeBit(rw, usbPowerShareAddress, usbPowerShareBit, enabled);
    checkStatusChanges();
}

void Helper::setFnMetaSwapState(const bool enabled) {
    writeBit(rw, fnMetaSwapAddress, fnMetaSwapBit, enabled);
    checkStatusChanges();
}

void Helper::setCoolerBoostState(const bool enabled) {
    writeBit(rw, coolerBoostAddress, coolerBoostBit, enabled);
    checkStatusChanges();
}

void Helper::setTurboMode(const bool enabled) {
    const uint8_t current = readValue(rw.readFromFile(), shiftModeAddress);
    if (enabled && current == shiftModePerformance)
        rw.writeToFile(shiftModeAddress, shiftModeTurbo);
    else if (!enabled && current == shiftModeTurbo)
        rw.writeToFile(shiftModeAddress, shiftModePerformance);
    checkStatusChanges();
}

void Helper::setUserMode(const int mode) {
    uint8_t shiftMode = 0;
    uint8_t fanMode = fanModeAuto;
    switch (mode) {
        case 0: // performance
            shiftMode = shiftModePerformance;
            break;
        case 1: // balanced
            shiftMode = shiftModeBalanced;
            break;
        case 2: // silent
            shiftMode = shiftModeBalanced;
            fanMode = fanModeSilent;
            break;
        case 3: // eco silent
            shiftMode = shiftModeBattery;
            fanMode = fanModeSilent;
            break;
        case 4: // turbo
            shiftMode = shiftModeTurbo;
            break;
        default:
            return;
    }
    rw.writeToFile(shiftModeAddress, shiftMode);
    const uint8_t currentFanMode = readValue(rw.readFromFile(), fanModeAddress);
    if (currentFanMode != fanModeAdvanced)
        rw.writeToFile(fanModeAddress, fanMode);
    checkStatusChanges();
}

void Helper::setFan1SpeedSettings(const QList<int> &values) {
    writeSettings(rw, values, fanSpeedSettingsCount, fan1SpeedSettingStartAddress);
    checkStatusChanges();
}
void Helper::setFan2SpeedSettings(const QList<int> &values) {
    writeSettings(rw, values, fanSpeedSettingsCount, fan2SpeedSettingStartAddress);
    checkStatusChanges();
}
void Helper::setFan1TempSettings(const QList<int> &values) {
    writeSettings(rw, values, fanTempSettingsCount, fan1TempSettingStartAddress);
    checkStatusChanges();
}
void Helper::setFan2TempSettings(const QList<int> &values) {
    writeSettings(rw, values, fanTempSettingsCount, fan2TempSettingStartAddress);
    checkStatusChanges();
}

void Helper::setFanModeAdvanced(const bool enabled, const int userMode) {
    if (enabled) {
        rw.writeToFile(fanModeAddress, fanModeAdvanced);
    } else {
        const uint8_t fanMode = (userMode == 2 || userMode == 3) ? fanModeSilent : fanModeAuto;
        rw.writeToFile(fanModeAddress, fanMode);
    }
    checkStatusChanges();
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
    fprintf(stderr, "%s\n", qPrintable("Trying to load the ec_sys kernel module"));
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

    helper->startMonitoring();

    return QCoreApplication::exec();
}
