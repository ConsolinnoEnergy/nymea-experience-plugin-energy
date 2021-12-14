#include "energylogs.h"

#include <QVariant>

EnergyLogs::EnergyLogs(QObject *parent): QObject(parent)
{

}

PowerBalanceLogEntry::PowerBalanceLogEntry()
{

}

PowerBalanceLogEntry::PowerBalanceLogEntry(const QDateTime &timestamp, double consumption, double production, double acquisition, double storage, double totalConsumption, double totalProduction, double totalAcquisition, double totalReturn):
    m_timestamp(timestamp),
    m_consumption(consumption),
    m_production(production),
    m_acquisition(acquisition),
    m_storage(storage),
    m_totalConsumption(totalConsumption),
    m_totalProduction(totalProduction),
    m_totalAcquisition(totalAcquisition),
    m_totalReturn(totalReturn)
{

}

QDateTime PowerBalanceLogEntry::timestamp() const
{
    return m_timestamp;
}

double PowerBalanceLogEntry::consumption() const
{
    return m_consumption;
}

double PowerBalanceLogEntry::production() const
{
    return m_production;
}

double PowerBalanceLogEntry::acquisition() const
{
    return m_acquisition;
}

double PowerBalanceLogEntry::storage() const
{
    return m_storage;
}

double PowerBalanceLogEntry::totalConsumption() const
{
    return m_totalConsumption;
}

double PowerBalanceLogEntry::totalProduction() const
{
    return m_totalProduction;
}

double PowerBalanceLogEntry::totalAcquisition() const
{
    return m_totalAcquisition;
}

double PowerBalanceLogEntry::totalReturn() const
{
    return m_totalReturn;
}

QVariant PowerBalanceLogEntries::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void PowerBalanceLogEntries::put(const QVariant &variant)
{
    append(variant.value<PowerBalanceLogEntry>());
}

ThingPowerLogEntry::ThingPowerLogEntry()
{

}

ThingPowerLogEntry::ThingPowerLogEntry(const QDateTime &timestamp, const ThingId &thingId, double currentPower, double totalConsumption, double totalProduction):
    m_timestamp(timestamp),
    m_thingId(thingId),
    m_currentPower(currentPower),
    m_totalConsumption(totalConsumption),
    m_totalProduction(totalProduction)
{

}

QDateTime ThingPowerLogEntry::timestamp() const
{
    return m_timestamp;
}

ThingId ThingPowerLogEntry::thingId() const
{
    return m_thingId;
}

double ThingPowerLogEntry::currentPower() const
{
    return m_currentPower;
}

double ThingPowerLogEntry::totalConsumption() const
{
    return m_totalConsumption;
}

double ThingPowerLogEntry::totalProduction() const
{
    return m_totalProduction;
}

QVariant ThingPowerLogEntries::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ThingPowerLogEntries::put(const QVariant &variant)
{
    append(variant.value<ThingPowerLogEntry>());
}
