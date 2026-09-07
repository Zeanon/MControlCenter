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

#include "readwrite.h"
#include <fstream>

#include <QFile>

const QString acpi_ec_file = "/dev/ec";
const QString ec_file = "/sys/kernel/debug/ec/ec0/io";
QString ioFile = "";

ReadWrite::ReadWrite() = default;

QByteArray ReadWrite::readFromFile() const {
    QFile file(ioFile);
    if (file.open(QIODevice::ReadOnly))
        return file.readAll();
    return {};
}

void ReadWrite::writeToFile(const uint8_t pos, const uint8_t value) const {
    std::ofstream file(ioFile.toStdString(), std::ios::in | std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.seekp(pos);
        file << value;
    }
}

bool ReadWrite::useAcpiEc() const {
    if (QFile::exists(acpi_ec_file)) {
        ioFile = acpi_ec_file;
        return true;
    }
    return false;
}

bool ReadWrite::useEcSys() const {
    if (QFile::exists(ec_file)) {
        ioFile = ec_file;
        return true;
    }
    return false;
}

bool ReadWrite::isAcpiEc() const {
    return acpi_ec_file == ioFile;
}

bool ReadWrite::isEcSys() const {
    return ec_file == ioFile;
}
