#include "fsm.h"

#include <cstdlib>
#include <sstream>
#include <map>

namespace gnossen {
namespace fsm {

static constexpr size_t kMaxElseSize = 5;

static std::string TranslateLetter(char letter) {
    if (letter == '\0') {
        return "\\\\0";
    } else {
        return std::string(1, letter);
    }
}

// TODO: Think about omitting the failure transitions.
std::string
FSMInterface::ToDotGraph() const {
    std::stringstream ss;
    ss << "digraph FSM {" << std::endl;
    for (FsmState* state : GetStates()) {
        std::string state_id(std::to_string(StateIdentifier(state)));
        std::map<unsigned int, std::list<std::string>> edges;
        std::unordered_set<char> remaining_letters;
        std::copy(GetAlphabet().begin(), GetAlphabet().end(),
                std::inserter(remaining_letters, remaining_letters.begin()));
        for (auto transition : GetTransitions(state)) {
            unsigned int out_id = StateIdentifier(transition.first);
            EdgeLabel label = transition.second;
            std::string transition_str;
            if (label.remainder) {
                // Only enumerate all others if it's reasonably small.
                if (GetAlphabet().size() <= kMaxElseSize) {
                    for (char remaining_letter : remaining_letters) {
                        edges[out_id].push_back(TranslateLetter(remaining_letter));
                    }
                } else {
                    edges[out_id].push_back("else");
                }
                break;
            } else if (label.empty_edge) {
                transition_str = "eps.";
            } else {
                remaining_letters.erase(label.edge_label);
                transition_str = TranslateLetter(label.edge_label);
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

Fsm::Fsm(const std::vector<char>& alphabet) :
    alphabet_(alphabet),
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

Fsm::Fsm(const Fsm& other) :
    states_(other.states_),
    state_pointers_(),
    start_state_(nullptr),
    success_state_(nullptr),
    failure_state_(nullptr),
    next_id_(other.next_id_)
{
    auto it = states_.begin();
    start_state_ = reinterpret_cast<FsmState*>(&(*it));
    ++it;
    success_state_ = reinterpret_cast<FsmState*>(&(*it));
    ++it;
    failure_state_ = reinterpret_cast<FsmState*>(&(*it));
    for (auto& state : states_) {
        state_pointers_.push_back(reinterpret_cast<FsmState*>(&state));
    }
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

const void Fsm::AddTransitionForRemaining(FsmState* from,
                                          FsmState* to)
{
    Fsm::State* from_state = reinterpret_cast<Fsm::State*>(from);
    from_state->edges_out.emplace_back(to, EdgeLabel::Remainder());
}

const std::vector<char>&
Fsm::GetAlphabet() const {
    return alphabet_;
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

Fsm ToBinarizedNfsm(Fsm& fsm, const std::vector<char>& alphabet) {
   return Fsm(alphabet);
}

} // end namespace fsm
} // end namespace gnossen
