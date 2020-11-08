/*
 * Copyright (C) 2015 Niek Linnenbank
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DeviceServer.h"

DeviceServer::DeviceServer(const char *path)
    : FileSystemServer(new Directory(1), path)
{
    m_interrupts.fill(ZERO);
}

DeviceServer::~DeviceServer()
{
}

FileSystem::Result DeviceServer::initialize()
{
    // Initialize all devices
    for (Size i = 0; i < m_devices.count(); i++)
    {
        Device *dev = m_devices[i];
        if (dev != ZERO)
        {
            const FileSystem::Result result = dev->initialize();
            if (result != FileSystem::Success)
            {
                ERROR("failed to initialize device " << (*dev->getIdentifier()) <<
                      ": result = " << (int)result);
                return result;
            }
        }
    }

    // Mount on the root file system
    const FileSystem::Result result = mount();
    if (result != FileSystem::Success)
    {
        ERROR("failed to mount to path " << m_mountPath << ": result = " << (int)result);
        return result;
    }

    return FileSystem::Success;
}

void DeviceServer::registerDevice(Device *dev, const char *path)
{
    FileSystemServer::registerFile(dev, path);

    // Add to the list of Devices
    m_devices.insert(dev);
}

void DeviceServer::registerInterrupt(Device *dev, Size vector)
{
    if (!m_interrupts[vector])
    {
        m_interrupts.insert(vector, new List<Device *>);
    }
    m_interrupts[vector]->append(dev);

    // Register to kernel
    ProcessCtl(SELF, WatchIRQ, vector);
    ProcessCtl(SELF, EnableIRQ, vector);

    // Register interrupt handler
    addIRQHandler(vector, (IRQHandlerFunction) &DeviceServer::interruptHandler);
}

void DeviceServer::interruptHandler(Size vector)
{
    List<Device *> *lst = m_interrupts.at(vector);

    // Do we have any Devices with this interrupt vector?
    if (lst)
    {
        // Loop all Devices of interest. Invoke callback.
        for (ListIterator<Device *> i(lst); i.hasCurrent(); i++)
        {
            i.current()->interrupt(vector);
        }
    }

    // Keep retrying any pending requests, if any
    retryAllRequests();
}
