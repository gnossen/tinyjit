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

Fsm::State::State(unsigned int id) :
    id(id), edges_out() {}

Fsm::Fsm(const std::vector<char>& alphabet) :
    alphabet_(alphabet),
    states_(),
    next_id_(0)
{
    // Create start, success, and failure states.
    for (size_t i = 0; i < 3; ++i) {
        AddState();
    }
}

Fsm::States::iterator
Fsm::AddState() {
    states_.emplace_back(next_id_++);
    return --states_.end();
}

const void Fsm::AddTransition(Fsm::States::iterator from,
                              Fsm::States::iterator to,
                              char letter)
{
    (*from).edges_out.emplace_back(to, EdgeLabel(letter));
}

const void Fsm::AddNonDeterministicTransition(Fsm::States::iterator from,
                                              Fsm::States::iterator to)
{
    (*from).edges_out.emplace_back(to, EdgeLabel());
}

const void Fsm::AddTransitionForRemaining(Fsm::States::iterator from,
                                          Fsm::States::iterator to)
{
    (*from).edges_out.emplace_back(to, EdgeLabel::Remainder());
}

const std::vector<char>&
Fsm::GetAlphabet() const {
    return alphabet_;
}

Fsm::StateContainer
Fsm::GetStates() {
    return Fsm::StateContainer(this);
}

Fsm::ConstStateContainer
Fsm::GetStates() const {
    return Fsm::ConstStateContainer(this);
}

Fsm::TransitionContainer
Fsm::GetTransitions(Fsm::States::iterator state) {
    return Fsm::TransitionContainer(state);
}

Fsm::ConstTransitionContainer
Fsm::GetTransitions(Fsm::States::const_iterator state) const {
    return Fsm::ConstTransitionContainer(state);
}

unsigned int Fsm::StateIdentifier(Fsm::States::const_iterator state) const {
    return (*state).id;
}

Fsm ToBinarizedNfsm(Fsm& fsm, const std::vector<char>& alphabet) {
   return Fsm(alphabet);
}

// TODO: Think about omitting the failure transitions.
std::string
Fsm::ToDotGraph() const {
    std::stringstream ss;
    ss << "digraph FSM {" << std::endl;
    const auto states = GetStates();
    for (auto state = states.begin(); state != states.end(); ++state) {
        std::string state_id(std::to_string(StateIdentifier(state)));
        std::map<unsigned int, std::list<std::string>> edges;
        std::unordered_set<char> remaining_letters;
        std::copy(GetAlphabet().begin(), GetAlphabet().end(),
                std::inserter(remaining_letters, remaining_letters.begin()));
        auto transitions = GetTransitions(state);
        for (auto transition = transitions.begin(); transition != transitions.end(); ++transition) {
            unsigned int out_id = StateIdentifier(transition->first);
            EdgeLabel label = transition->second;
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


} // end namespace fsm
} // end namespace gnossen
