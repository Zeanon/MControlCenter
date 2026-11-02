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

#ifndef HELPER_H
#define HELPER_H

#include <QtCore/QObject>
#include <QtDBus/QDBusInterface>
#include <QVector>
#include <QByteArray>
#include <QVariantMap>
#include <QVariantList>
#include <optional>
#include <cstdint>

class Helper : public QObject {
    Q_OBJECT

public:
    Helper();

    bool isEcModuleLoaded() const;
    bool loadEcModule() const;
    bool isAcpiEc() const;
    bool isEcSys() const;
    QString getModelName() const;
    bool updateData() const;
    void updateDataAsync();
    QString getEcVersion() const;
    QString getEcBuild() const;
    uint8_t getBatteryCharge() const;
    uint8_t getBatteryThreshold() const;
    int getChargingStatus() const;
    uint8_t getCpuTemp() const;
    std::optional<uint8_t> getGpuTemp() const;
    int getFan1Speed() const;
    std::optional<int> getFan2Speed() const;
    QVector<int> getFan1SpeedSettings() const;
    QVector<int> getFan2SpeedSettings() const;
    QVector<int> getFan1TempSettings() const;
    QVector<int> getFan2TempSettings() const;
    uint8_t getKeyboardBacklightMode() const;
    uint8_t getKeyboardBrightness() const;
    bool getUsbPowerShareState() const;
    bool getFnMetaSwapState() const;
    bool getCoolerBoostState() const;
    bool getTurboMode() const;
    int getUserMode() const;
    int getFanMode() const;
    void setBatteryThreshold(uint8_t value) const;
    void setKeyboardBacklightMode(bool enabled) const;
    void setKeyboardBrightness(uint8_t value) const;
    void setUsbPowerShareState(bool enabled) const;
    void setFnMetaSwapState(bool enabled) const;
    void setCoolerBoostState(bool enabled) const;
    void setTurboMode(bool enabled) const;
    void setUserMode(int mode) const;
    void setFan1SpeedSettings(const QVector<int> &values) const;
    void setFan2SpeedSettings(const QVector<int> &values) const;
    void setFan1TempSettings(const QVector<int> &values) const;
    void setFan2TempSettings(const QVector<int> &values) const;
    void setFanModeAdvanced(bool enabled, int userMode) const;
    void quit() const;
    QDBusInterface *iface = nullptr;

signals:
    void dataUpdated(bool valid);

private:
    mutable QVariantMap statusSnapshot;
    QVariant value(const char *key, const QVariant &fallback = QVariant()) const;
    QVector<int> intList(const char *key) const;
    void printError(const QDBusError &error) const;
    bool updatePending = false;
};

#endif // HELPER_H
