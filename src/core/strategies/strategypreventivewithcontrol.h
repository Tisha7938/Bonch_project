#pragma once

#include "imaintenancestrategy.h"

//! @brief Стратегия 2.1: Профилактика + встроенный контроль
//!
//! Комбинирует периодическое профилактическое обслуживание с
//! встроенным контролем отказов. Профилактика выполняется по
//! фиксированному интервалу.
class StrategyPreventiveWithControl : public IMaintenanceStrategy {
public:
    StrategyPreventiveWithControl(double interval = 10.0);

    double detectFailure();
    double recover();
    bool checkMaintenance(double timeElapsed);
    const char *name() const { return "2.1 Профилактика + встроенный контроль"; }

    void setMaintenanceInterval(double interval);
    double getMaintenanceInterval() const;

private:
    double m_lastMaintenance;
    double m_maintenanceInterval;

    static double uniformRandom(double min, double max);
};
