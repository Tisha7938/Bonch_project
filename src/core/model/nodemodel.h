#pragma once
#include <memory>
#include <string>
#include <vector>
#include "imaintenancestrategy.h"

//! @brief Чистая модель вершины графа
class NodeModel {
public:
    //! @brief Состояние вершины
    enum class State { Operational, Failed, Maintenance };

    explicit NodeModel(unsigned int id);

    unsigned int id() const;
    State state() const;
    void setState(State s);
    double reliability() const;
    void setReliability(double r);

    //! @brief Привязка стратегии обслуживания
    void setStrategy(std::unique_ptr<IMaintenanceStrategy> strategy);
    IMaintenanceStrategy *strategy() const;

    //! @brief Очередь входящих сообщений
    void addMessage(const std::string &msg);
    const std::vector<std::string> &inbox() const;
    void clearInbox();

    void changeStrategy(std::unique_ptr<IMaintenanceStrategy> newStrategy);

private:
    unsigned int m_id;
    State m_state;
    double m_reliability;
    std::unique_ptr<IMaintenanceStrategy> m_strategy;
    std::vector<std::string> m_inbox;
};
