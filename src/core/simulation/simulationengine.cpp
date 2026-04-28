#include "simulationengine.h"
#include <algorithm>
#include <random>

SimulationEngine::SimulationEngine(double dt, const std::string &distMode) :
    m_dt(dt), m_currentTime(0.0), m_running(false), m_totalRuntime(0.0), m_totalDowntime(0.0), m_emulator(distMode) {}

void SimulationEngine::setNodes(const std::vector<std::shared_ptr<NodeModel>> &nodes) {
    m_nodes = nodes;
    for (const auto &node: m_nodes) {
        m_nodeStats[node->id()] = NodeStats{};
    }
}

void SimulationEngine::start() {
    if (!m_running && !m_nodes.empty()) {
        m_running = true;
    }
}

void SimulationEngine::stop() { m_running = false; }

bool SimulationEngine::isRunning() const { return m_running; }

void SimulationEngine::reset() {
    m_currentTime = 0.0;
    m_totalRuntime = 0.0;
    m_totalDowntime = 0.0;
    m_nodeStats.clear();
    for (const auto &node: m_nodes) {
        node->setState(NodeModel::State::Operational);
        node->setReliability(1.0);
        m_nodeStats[node->id()] = NodeStats{};
    }
}

double SimulationEngine::currentTime() const { return m_currentTime; }

double SimulationEngine::getGlobalAvailability() const {
    double total = m_totalRuntime + m_totalDowntime;
    return (total > 0.0) ? m_totalRuntime / total : 1.0;
}

double SimulationEngine::getNodeAvailability(unsigned int nodeId) const {
    auto it = m_nodeStats.find(nodeId);
    if (it == m_nodeStats.end())
        return 1.0;

    const auto &stats = it->second;
    double total = stats.runtime + stats.downtime;
    return (total > 0.0) ? stats.runtime / total : 1.0;
}

void SimulationEngine::setStepCallback(StepCallback cb) { m_stepCallback = std::move(cb); }

void SimulationEngine::setEventCallback(EventCallback cb) { m_eventCallback = std::move(cb); }

MessageBus &SimulationEngine::messageBus() { return m_bus; }

void SimulationEngine::step() {
    if (!m_running || m_nodes.empty()) {
        return;
    }

    for (auto &node: m_nodes) {
        node->clearInbox();
        auto msgs = m_bus.fetchFor(node->id());
        for (const auto &msg: msgs) {
            node->addMessage(msg);
        }
    }

    for (auto &node: m_nodes) {
        processNode(*node);
    }

    m_bus.flush();

    m_currentTime += m_dt;
    if (m_stepCallback) {
        m_stepCallback();
    }
}

void SimulationEngine::processNode(NodeModel &node) {
    auto *strategy = node.strategy();
    if (!strategy) {

        m_totalRuntime += m_dt;
        auto &stats = m_nodeStats[node.id()];
        stats.runtime += m_dt;
        return;
    }

    const auto state = node.state();
    if (state != NodeModel::State::Operational) {
        m_totalDowntime += m_dt;
        auto &stats = m_nodeStats[node.id()];
        stats.downtime += m_dt;

        // TODO: восстановление за один такт. В реальной системе здесь был бы таймер восстановления - потом добавить (мейби)
        if (state == NodeModel::State::Failed) {
            handleRecovery(node);
        }
        return;
    }

    if (strategy->checkMaintenance(m_currentTime)) {
        node.setState(NodeModel::State::Maintenance);
        auto &stats = m_nodeStats[node.id()];
        stats.maintenanceCount++;

        const double mainTime = m_emulator.sampleMaintenanceTime();
        m_totalDowntime += std::min(mainTime, m_dt);

        m_bus.sendTo(node.id(), "MAINT_START");
        if (m_eventCallback) {
            m_eventCallback(node.id(), "MAINTENANCE");
        }
        return;
    }

    double timeToFailure = m_emulator.sampleFailureTime();
    if (timeToFailure <= m_dt) {
        triggerFailure(node);
        return;
    }

    m_totalRuntime += m_dt;
    auto &stats = m_nodeStats[node.id()];
    stats.runtime += m_dt;

    double newReliability = node.reliability() * std::exp(-m_dt / 100.0);
    node.setReliability(std::max(0.0, newReliability));
}

void SimulationEngine::triggerFailure(NodeModel &node) {
    auto *strategy = node.strategy();
    if (!strategy)
        return;

    node.setState(NodeModel::State::Failed);
    auto &stats = m_nodeStats[node.id()];
    stats.failureCount++;

    double detectTime = strategy->detectFailure();
    double recoverTime = strategy->recover();
    double totalDowntime = detectTime + recoverTime;

    m_totalDowntime += std::min(totalDowntime, m_dt);
    stats.downtime += std::min(totalDowntime, m_dt);

    m_bus.sendTo(node.id(), "FAILURE_DETECTED");
    if (m_eventCallback) {
        m_eventCallback(node.id(), "FAILURE");
    }
}

void SimulationEngine::handleRecovery(NodeModel &node) {
    node.setState(NodeModel::State::Operational);
    node.setReliability(0.95);

    m_bus.sendTo(node.id(), "RECOVERED");
    if (m_eventCallback) {
        m_eventCallback(node.id(), "RECOVERY");
    }
}
