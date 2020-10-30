#include "fsm.h"

#include <cstdlib>
#include <sstream>

namespace gnossen {
namespace fsm {

// TODO: Think about omitting the failure transitions.
std::string
FSMInterface::ToDotGraph() const {
    std::stringstream ss;
    ss << "digraph FSM {" << std::endl;
    for (FsmState* state : GetStates()) {
        std::string state_id(std::to_string(StateIdentifier(state)));
        for (auto transition : GetTransitions(state)) {
            FsmState* out_state = transition.first;
            char letter = transition.second;
            std::string transition_str;
            if (letter == '\0') {
                transition_str = "\\0";
            } else {
                transition_str = letter;
            }
            ss << "  " << state_id << " -> " << StateIdentifier(out_state) <<
               " [ label=\"" << transition_str << "\" ]" << std::endl;
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
    from_state->edges_out.emplace_back(to, letter);
}

std::list<FsmState*>
Fsm::GetStates() const {
    return state_pointers_;
}

// TODO: Expensive copy happening here. Iterators, son.
std::list<std::pair<FsmState*, char>>
Fsm::GetTransitions(FsmState* state) const {
    return reinterpret_cast<Fsm::State*>(state)->edges_out;
}

unsigned int Fsm::StateIdentifier(FsmState* state) const {
    return reinterpret_cast<Fsm::State*>(state)->id;
}


} // end namespace fsm
} // end namespace gnossen
