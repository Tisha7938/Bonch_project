#include "nodemodel.h"

NodeModel::NodeModel(unsigned int id) : m_id(id), m_state(State::Operational), m_reliability(1.0) {}

unsigned int NodeModel::id() const { return m_id; }
NodeModel::State NodeModel::state() const { return m_state; }
void NodeModel::setState(State s) { m_state = s; }
double NodeModel::reliability() const { return m_reliability; }
void NodeModel::setReliability(double r) { m_reliability = r; }

void NodeModel::setStrategy(std::unique_ptr<IMaintenanceStrategy> strategy) { m_strategy = std::move(strategy); }
IMaintenanceStrategy *NodeModel::strategy() const { return m_strategy.get(); }

void NodeModel::addMessage(const std::string &msg) { m_inbox.push_back(msg); }
const std::vector<std::string> &NodeModel::inbox() const { return m_inbox; }
void NodeModel::clearInbox() { m_inbox.clear(); }
void NodeModel::changeStrategy(std::unique_ptr<IMaintenanceStrategy> newStrategy) {
    m_strategy = std::move(newStrategy);
}
