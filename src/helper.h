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

class Helper : public QObject {
Q_OBJECT
public:
    Helper();

    bool isEcModuleLoaded() const;
    bool loadEcModule() const;
    bool isAcpiEc() const;
    bool isEcSys() const;
    bool updateData() const;
    void updateDataAsync();
    std::optional<uint8_t> getOptionalValue(uint8_t address) const;
    uint8_t getValue(uint8_t address) const;
    QByteArray getValues(uint8_t startAddress, uint8_t size) const;
    void putValue(uint8_t address, uint8_t value) const;
    void quit() const;
    QDBusInterface *iface;
private:
    void printError(const QDBusError &error) const;
private slots:
    void callFinishedSlot(QDBusPendingCallWatcher *call) const;
};

#endif // HELPER_H
