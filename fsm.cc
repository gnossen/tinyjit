#include "fsm.h"

#include <cstdlib>
#include <iterator>
#include <sstream>
#include <map>
#include <unordered_map>

namespace gnossen {
namespace fsm {

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
                              EdgeLabel label)
{
    (*from).edges_out.emplace_back(to, label);
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
                edges[out_id].push_back("else");
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

Fsm ToBinarizedNfsm(Fsm& original) {
  // Each state in the input graph will have a corresponding state in the
  // output graph, so we start by copying those over.

  // Maps from index of state in the original to the iterator for a state
  // in the derived graph.
  Fsm derived(original.GetAlphabet());
  std::unordered_map<unsigned int, Fsm::States::iterator> correspondences;

  correspondences[original.StateIdentifier(original.GetStartState())] = derived.GetStartState();
  correspondences[original.StateIdentifier(original.GetSuccessState())] = derived.GetSuccessState();
  correspondences[original.StateIdentifier(original.GetFailureState())] = derived.GetFailureState();

  // Start by making a copy of every state in the original.
  auto states = original.GetStates();
  auto state = states.begin();
  for (size_t i = 0; i < 3; ++i) {
    // Skip special states.
    ++state;
  }
  for (; state != states.end(); ++state) {
    auto mirror_state = derived.AddState();
    correspondences[original.StateIdentifier(state)] = mirror_state;
  }

  std::unordered_set<unsigned int> visited;
  std::vector<Fsm::States::iterator> to_visit;
  to_visit.push_back(original.GetStartState());

  while (!to_visit.empty()) {
    auto original_state = to_visit.back();
    to_visit.pop_back();

    if (visited.find(original.StateIdentifier(original_state)) != visited.end()) {
      continue;
    }

    visited.insert(original.StateIdentifier(original_state));

    // This is just a single transformation into a form taht I know I can map
    // well to machine code. There are certainly others that could result in
    // more performant code. This would be the proper place to look at the state
    // and its children to determine if there is a more appropriate
    // transformation to apply. It's possible that at some point we need to
    // record metadata that only applies to machine code -- such as some sort
    // of ordering of the nodes. We'll cross that bridge when we get to it.
    auto mirror_state = correspondences[original.StateIdentifier(original_state)];

    auto transitions = original.GetTransitions(original_state);
    size_t transition_count = std::distance(transitions.begin(), transitions.end());
    if (transition_count == 0) {
      continue;
    } else if (transition_count == 1) {
      auto transition = transitions.begin();
      derived.AddTransition(
          mirror_state,
          correspondences[original.StateIdentifier(transition->first)],
          transition->second);
      to_visit.push_back(transition->first);
    } else {
      // TODO: If there's an immediate loop, prioritize that and don't make an
      // extra state for it. We can use `repe scasb` for it.
      auto previous_state = mirror_state;
      for (auto transition = transitions.begin(); transition != transitions.end(); ++transition) {
        if (transition->second.remainder)  {
          derived.AddTransitionForRemaining(previous_state, transition->first);
        } else {
          auto current_state = derived.AddState();
          derived.AddNonDeterministicTransition(previous_state, current_state);
          derived.AddTransition(
              current_state,
              correspondences[original.StateIdentifier(transition->first)],
              transition->second);
          previous_state = current_state;

          to_visit.push_back(transition->first);
        }
      }
    }
  }

  // TODO: Make sure we get copy elision here.
  return derived;
}


} // end namespace fsm
} // end namespace gnossen
