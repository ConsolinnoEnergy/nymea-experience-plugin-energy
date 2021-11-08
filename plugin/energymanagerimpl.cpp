#include "energymanagerimpl.h"
#include "nymeasettings.h"

#include <qmath.h>

Q_DECLARE_LOGGING_CATEGORY(dcEnergyExperience)

EnergyManagerImpl::EnergyManagerImpl(ThingManager *thingManager, QObject *parent):
    EnergyManager(parent),
    m_thingManager(thingManager)
{
    // Most of the time we get a bunch of signals at the same time (root meter, producers, consumers etc)
    // In order to decrease some load on the system, we'll wait for wee bit until we actually update to
    // accumulate those changes and calculate the change in one go.
    m_balanceUpdateTimer.setInterval(50);
    m_balanceUpdateTimer.setSingleShot(true);
    connect(&m_balanceUpdateTimer, &QTimer::timeout, this, &EnergyManagerImpl::updatePowerBalance);

    QSettings settings(NymeaSettings::settingsPath() + "/energy.conf", QSettings::IniFormat);
    ThingId rootMeterThingId = settings.value("rootMeterThingId").toUuid();
    EnergyManagerImpl::setRootMeter(rootMeterThingId);
    qCDebug(dcEnergyExperience()) << "Loaded root meter" << rootMeterThingId;

    foreach (Thing *thing, m_thingManager->configuredThings()) {
        watchThing(thing);
    }
    connect(thingManager, &ThingManager::thingAdded, this, &EnergyManagerImpl::watchThing);
    connect(thingManager, &ThingManager::thingRemoved, this, &EnergyManagerImpl::unwatchThing);
}

Thing *EnergyManagerImpl::rootMeter() const
{
    return m_rootMeter;
}

EnergyManager::EnergyError EnergyManagerImpl::setRootMeter(const ThingId &rootMeterId)
{
    Thing *rootMeter = m_thingManager->findConfiguredThing(rootMeterId);
    if (!rootMeter || !rootMeter->thingClass().interfaces().contains("energymeter")) {
        return EnergyErrorInvalidParameter;
    }

    if (m_rootMeter != rootMeter) {
        qCDebug(dcEnergyExperience()) << "Setting root meter to" << rootMeter->name();
        m_rootMeter = rootMeter;

        QSettings settings(NymeaSettings::settingsPath() + "/energy.conf", QSettings::IniFormat);
        settings.setValue("rootMeterThingId", rootMeter->id());

        emit rootMeterChanged();
    }
    return EnergyErrorNoError;
}

double EnergyManagerImpl::currentPowerConsumption() const
{
    return m_currentPowerConsumption;
}

double EnergyManagerImpl::currentPowerProduction() const
{
    return m_currentPowerProduction;
}

double EnergyManagerImpl::currentPowerAcquisition() const
{
    return m_currentPowerAcquisition;
}

void EnergyManagerImpl::watchThing(Thing *thing)
{
    // If we don't have a root meter yet, we'll be auto-setting the first energymeter that appears.
    // It may be changed by the user through an API call later.
    if (!m_rootMeter && thing->thingClass().interfaces().contains("energymeter")) {
        setRootMeter(thing->id());
    }

    qCDebug(dcEnergyExperience()) << "Wathing thing:" << thing->name();
    if (thing->thingClass().interfaces().contains("smartmeterproducer")
            || thing->thingClass().interfaces().contains("energymeter")
            || thing->thingClass().interfaces().contains("energystorage")) {
        connect(thing, &Thing::stateValueChanged, this, [=](const StateTypeId &stateTypeId){
            if (thing->thingClass().getStateType(stateTypeId).name() == "currentPower") {
                m_balanceUpdateTimer.start();
            }
        });
    }
}

void EnergyManagerImpl::unwatchThing(const ThingId &thingId)
{
    if (m_rootMeter && m_rootMeter->id() == thingId) {
        m_rootMeter = nullptr;
        emit rootMeterChanged();
    }
}

void EnergyManagerImpl::updatePowerBalance()
{
    double currentPowerAcquisition = 0;
    if (m_rootMeter) {
        currentPowerAcquisition = m_rootMeter->stateValue("currentPower").toDouble();
    }

    double currentPowerProduction = 0;
    foreach (Thing* thing, m_thingManager->configuredThings().filterByInterface("smartmeterproducer")) {
        currentPowerProduction += thing->stateValue("currentPower").toDouble();
    }

    double currentBatteryBalance = 0;
    foreach (Thing *thing, m_thingManager->configuredThings().filterByInterface("energystorage")) {
        currentBatteryBalance += thing->stateValue("currentPower").toDouble();
    }

    double currentPowerConsumption = -currentPowerProduction + currentPowerAcquisition - currentBatteryBalance;

    qCDebug(dcEnergyExperience()) << "Consumption:" << currentPowerConsumption << "Production:" << currentPowerProduction << "Acquisition:" << currentPowerAcquisition << "Battery:" << currentBatteryBalance;
    if (currentPowerAcquisition != m_currentPowerAcquisition
            || currentPowerConsumption != m_currentPowerConsumption
            || currentPowerProduction != m_currentPowerProduction) {
        m_currentPowerAcquisition = currentPowerAcquisition;
        m_currentPowerProduction = currentPowerProduction;
        m_currentPowerConsumption = currentPowerConsumption;
        emit powerBalanceChanged();
    }
}

void EnergyManagerImpl::logDumpConsumers()
{
    foreach (Thing *consumer, m_thingManager->configuredThings().filterByInterface("smartmeterconsumer")) {
        qCDebug(dcEnergyExperience()).nospace().noquote() << consumer->name() << ": " << (consumer->stateValue("currentPower").toDouble() / 230) << "A (" << consumer->stateValue("currentPower").toDouble() << "W)";
    }
}


