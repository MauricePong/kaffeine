/*
 * dvbdevice.h
 *
 * Copyright (C) 2007 Christoph Pfister <christophpfister@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef DVBDEVICE_H
#define DVBDEVICE_H

#include <QTimer>
#include <Solid/Device>

class DvbConfig;
class DvbDeviceThread;
class DvbTransponder;

class DvbPidFilter
{
public:
	DvbPidFilter() { }
	virtual ~DvbPidFilter() { }

	virtual void processData(const char data[188]) = 0;
};

class DvbDevice : public QObject
{
	Q_OBJECT
public:
	enum TransmissionType
	{
		DvbC = (1 << 0),
		DvbS = (1 << 1),
		DvbT = (1 << 2),
		Atsc = (1 << 3)
	};

	Q_DECLARE_FLAGS(TransmissionTypes, TransmissionType)

	enum DeviceState
	{
		DeviceNotReady,
		DeviceIdle,
		DeviceRotorMoving,
		DeviceTuning,
		DeviceTuned
	};

	DvbDevice(int deviceId_);
	~DvbDevice();

	int getId() const
	{
		return deviceId;
	}

	void componentAdded(const Solid::Device &component);
	bool componentRemoved(const QString &udi);

	DeviceState getDeviceState() const
	{
		return deviceState;
	}

	TransmissionTypes getTransmissionTypes() const
	{
		Q_ASSERT(deviceState != DeviceNotReady);
		return transmissionTypes;
	}

	QString getFrontendName() const
	{
		Q_ASSERT(deviceState != DeviceNotReady);
		return frontendName;
	}

	void tuneDevice(const DvbTransponder *transponder, const DvbConfig *config);
	void stopDevice();

	/*
	 * signal and SNR are scaled from 0 to 100
	 * the device has to be tuned / tuning when you call one of these functions
	 */

	int getSignal();
	int getSnr();
	bool isTuned();

	/*
	 * you can use the same filter object for different pids
	 * all filters will be removed when the device becomes idle
	 */

	void addPidFilter(int pid, DvbPidFilter *filter);
	void removePidFilter(int pid, DvbPidFilter *filter);

signals:
	void stateChanged();

private slots:
	void frontendEvent();

private:
	Q_DISABLE_COPY(DvbDevice)

	enum stateFlag {
		CaPresent	= (1 << 0),
		DemuxPresent	= (1 << 1),
		DvrPresent	= (1 << 2),
		FrontendPresent	= (1 << 3),

		DevicePresent	= (DemuxPresent | DvrPresent | FrontendPresent)
	};

	Q_DECLARE_FLAGS(stateFlags, stateFlag)

	void setInternalState(stateFlags newState);
	void setDeviceState(DeviceState newState);
	bool identifyDevice();

	Solid::Device caComponent;
	Solid::Device demuxComponent;
	Solid::Device dvrComponent;
	Solid::Device frontendComponent;

	QString caPath;
	QString demuxPath;
	QString dvrPath;
	QString frontendPath;

	int deviceId;
	stateFlags internalState;
	DeviceState deviceState;
	TransmissionTypes transmissionTypes;
	QString frontendName;

	int frontendFd;
	int frontendTimeout;
	QTimer frontendTimer;

	DvbDeviceThread *thread;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DvbDevice::TransmissionTypes)

class DvbDeviceManager : public QObject
{
	Q_OBJECT
public:
	explicit DvbDeviceManager(QObject *parent);
	~DvbDeviceManager();

	QList<DvbDevice *> getDeviceList() const
	{
		return devices;
	}

private slots:
	void componentAdded(const QString &udi);
	void componentRemoved(const QString &udi);

private:
	void componentAdded(const Solid::Device &component);

	QList<DvbDevice *> devices;
};

#endif /* DVBDEVICE_H */
