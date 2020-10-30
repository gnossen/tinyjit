#include "fsm.h"

#include <cstdlib>
#include <sstream>
#include <map>

namespace gnossen {
namespace fsm {

// TODO: Think about omitting the failure transitions.
std::string
FSMInterface::ToDotGraph() const {
    std::stringstream ss;
    ss << "digraph FSM {" << std::endl;
    for (FsmState* state : GetStates()) {
        std::string state_id(std::to_string(StateIdentifier(state)));
        std::map<unsigned int, std::list<std::string>> edges;
        for (auto transition : GetTransitions(state)) {
            unsigned int out_id = StateIdentifier(transition.first);
            EdgeLabel label = transition.second;
            std::string transition_str;
            if (label.empty_edge) {
                transition_str = "eps.";
            } else {
                if (label.edge_label == '\0') {
                    transition_str = "\\\\0";
                } else {
                    transition_str = label.edge_label;
                }
            }
            edges[out_id].push_back(transition_str);
        }
        for(const auto& edge_data : edges) {
            unsigned int out_id = edge_data.first;
            const auto& letters = edge_data.second;
            ss << "  " << state_id << " -> " << out_id << " [ label=\" ";
            auto it = letters.begin();
            ss << *it;
            ++it;
            while (it != letters.end()) {
                ss << ",";
                ss << *it;
                ++it;
            }
            ss << "\" ]" << std::endl;
        }
    }
    ss << "}" << std::endl;
    return ss.str();
}

Fsm::State::State(unsigned int id) :
    id(id), edges_out() {}

Fsm::Fsm() :
    states_(),
    start_state_(nullptr),
    success_state_(nullptr),
    failure_state_(nullptr),
    next_id_(0)
{
    start_state_ = AddState();
    success_state_ = AddState();
    failure_state_ = AddState();
}

FsmState*
Fsm::AddState() {
    states_.emplace_back(next_id_++);
    FsmState* state = reinterpret_cast<FsmState*>(&states_.back());
    state_pointers_.push_back(state);
    return state;
}

const void Fsm::AddTransition(FsmState* from,
                                  FsmState* to,
                                  char letter)
{
    Fsm::State* from_state = reinterpret_cast<Fsm::State*>(from);
    from_state->edges_out.emplace_back(to, EdgeLabel(letter));
}

const void Fsm::AddNonDeterministicTransition(FsmState* from,
                                              FsmState* to)
{
    Fsm::State* from_state = reinterpret_cast<Fsm::State*>(from);
    from_state->edges_out.emplace_back(to, EdgeLabel());
}

std::list<FsmState*>
Fsm::GetStates() const {
    return state_pointers_;
}

// TODO: Expensive copy happening here. Iterators, son.
std::list<std::pair<FsmState*, EdgeLabel>>
Fsm::GetTransitions(FsmState* state) const {
    return reinterpret_cast<Fsm::State*>(state)->edges_out;
}

unsigned int Fsm::StateIdentifier(FsmState* state) const {
    return reinterpret_cast<Fsm::State*>(state)->id;
}


} // end namespace fsm
} // end namespace gnossen
