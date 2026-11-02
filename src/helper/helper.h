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

#include "service.h"

#include <QtCore/QObject>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusVariant>
#include <QList>
#include <QString>
#include <QVariantMap>

class Helper : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", INTERFACE_NAME)
public:
    explicit Helper(QObject *obj) : QDBusAbstractAdaptor(obj) {}

signals:
    void aboutToQuit();

public slots:
    Q_NOREPLY void quit() const;
    [[nodiscard]] bool updateData() const;
    [[nodiscard]] QVariantMap getStatusSnapshot() const;
    [[nodiscard]] QString getEcVersion() const;
    [[nodiscard]] QString getEcBuild() const;
    [[nodiscard]] QString getModelName() const;
    [[nodiscard]] int getBatteryCharge() const;
    [[nodiscard]] int getBatteryThreshold() const;
    [[nodiscard]] int getChargingStatus() const;
    [[nodiscard]] int getCpuTemp() const;
    [[nodiscard]] int getGpuTemp() const;
    [[nodiscard]] int getFan1Speed() const;
    [[nodiscard]] int getFan2Speed() const;
    [[nodiscard]] QList<int> getFan1SpeedSettings() const;
    [[nodiscard]] QList<int> getFan2SpeedSettings() const;
    [[nodiscard]] QList<int> getFan1TempSettings() const;
    [[nodiscard]] QList<int> getFan2TempSettings() const;
    [[nodiscard]] int getKeyboardBacklightMode() const;
    [[nodiscard]] int getKeyboardBrightness() const;
    [[nodiscard]] bool getUsbPowerShareState() const;
    [[nodiscard]] bool getFnMetaSwapState() const;
    [[nodiscard]] bool getCoolerBoostState() const;
    [[nodiscard]] int getUserMode() const;
    [[nodiscard]] int getFanMode() const;
    // Typed EC operations. Raw EC addresses are intentionally not exposed over D-Bus.
    Q_NOREPLY void setBatteryThreshold(int value) const;
    Q_NOREPLY void setKeyboardBacklightMode(bool enabled) const;
    [[nodiscard]] bool toggleKeyboardBacklightMode() const;
    Q_NOREPLY void setKeyboardBrightness(int value) const;
    Q_NOREPLY void setUsbPowerShareState(bool enabled) const;
    Q_NOREPLY void setFnMetaSwapState(bool enabled) const;
    Q_NOREPLY void setCoolerBoostState(bool enabled) const;
    Q_NOREPLY void setTurboMode(bool enabled) const;
    Q_NOREPLY void setUserMode(int mode) const;
    Q_NOREPLY void setFan1SpeedSettings(const QList<int> &values) const;
    Q_NOREPLY void setFan2SpeedSettings(const QList<int> &values) const;
    Q_NOREPLY void setFan1TempSettings(const QList<int> &values) const;
    Q_NOREPLY void setFan2TempSettings(const QList<int> &values) const;
    Q_NOREPLY void setFanModeAdvanced(bool enabled, int userMode) const;
    [[nodiscard]] bool isEcModuleLoaded() const;
    [[nodiscard]] bool isAcpiEc() const;
    [[nodiscard]] bool isEcSys() const;
    [[nodiscard]] bool loadEcModule() const;
};

#endif // HELPER_H
