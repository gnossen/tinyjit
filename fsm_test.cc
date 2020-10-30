#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <algorithm>
#include <list>
#include <vector>
#include <set>
#include <unordered_set>

#include "fsm.h"

namespace gnossen {
namespace fsm {

namespace {

TEST(FsmTest, CanBuild) {
  Fsm fsm;
  std::vector<char> alphabet {'a', 'b', 'c'};
  auto* initial_state = fsm.GetStartState();
  auto* state2 = fsm.AddState();
  fsm.AddTransition(initial_state, state2, 'c');
  fsm.AddTransition(state2, state2, 'a');
  fsm.AddTransition(state2, state2, 'b');
  auto* state3 = fsm.AddState();
  fsm.AddTransition(state2, state3, 'c');
  auto* state4 = fsm.AddState();
  for (char letter : alphabet) {
    fsm.AddTransition(state3, state4, letter);
  }
  fsm.AddTransition(state4, fsm.GetSuccessState(), '\0');

  std::set<FsmState*> observed_states;
  const auto states = fsm.GetStates();
  std::copy(states.begin(), states.end(),
          std::inserter(observed_states, observed_states.begin()));
  std::set<FsmState*> expected_states = {
    initial_state,
    state2,
    state3,
    state4,
    fsm.GetSuccessState(),
    fsm.GetFailureState(),
  };

  EXPECT_THAT(observed_states, ::testing::ContainerEq(expected_states));

  using Edge = std::pair<FsmState*, char>;
  using Edges = std::list<Edge>;
  std::map<FsmState*, Edges> observed_transitions;
  std::map<FsmState*, Edges> expected_transitions = {
    {initial_state, Edges{Edge{state2, 'c'}}},
    {state2, Edges{
                Edge{state2, 'a'},
                Edge{state2, 'b'},
                Edge{state3, 'c'}
             }
    },
    {state3, Edges{
                Edge{state4, 'a'},
                Edge{state4, 'b'},
                Edge{state4, 'c'}
             }
    },
    {state4, Edges{Edge{fsm.GetSuccessState(), '\0'}}},
  };

  for (FsmState* state : fsm.GetStates()) {
    for (auto transition : fsm.GetTransitions(state)) {
      EXPECT_FALSE(transition.second.empty_edge);
      observed_transitions[state].emplace_back(transition.first,
                                               transition.second.edge_label);
    }
  }

  EXPECT_THAT(observed_transitions, ::testing::ContainerEq(expected_transitions));
  std::cerr << fsm.ToDotGraph() << std::endl;
}

TEST(FsmTest, ToBinarizedFsm) {
  Fsm fsm;
  std::vector<char> alphabet {'a', 'b', 'c'};
  auto* initial_state = fsm.GetStartState();
  auto* state2 = fsm.AddState();
  fsm.AddTransition(initial_state, state2, 'c');
  fsm.AddTransition(state2, state2, 'a');
  fsm.AddTransition(state2, state2, 'b');
  auto* state3 = fsm.AddState();
  fsm.AddTransition(state2, state3, 'c');
  auto* state4 = fsm.AddState();
  for (char letter : alphabet) {
    fsm.AddTransition(state3, state4, letter);
  }
  fsm.AddTransition(state4, fsm.GetSuccessState(), '\0');

  Fsm binarized_fsm = ToBinarizedNfsm(fsm, alphabet);
  std::unordered_set<FsmState*> visited;
  std::list<FsmState*> to_visit;
  to_visit.push_back(binarized_fsm.GetStartState());
  FsmState* state;
  do {
    state = to_visit.back();
    to_visit.pop_back();

    visited.insert(state);
    if (state == binarized_fsm.GetSuccessState() ||
        state == binarized_fsm.GetFailureState())
      continue;

    std::unordered_set<FsmState*> next_states;
    bool found_empty_edge = false;
    for (auto edge : binarized_fsm.GetTransitions(state)) {
      next_states.insert(edge.first);
      if (edge.second.empty_edge) {
        found_empty_edge = true;
      }
    }
    EXPECT_EQ(next_states.size(), 2) << "Binarized state must have exactly two successors";
    if (next_states.find(binarized_fsm.GetFailureState()) == next_states.end() &&
        !(binarized_fsm.GetTransitions(state).size() == 2 && found_empty_edge)) {
      EXPECT_TRUE(false) << "State neither had exactly two successors with one being epsilon nor" <<
        " linked to failure state.";
    }
    for (FsmState* state : next_states) {
      to_visit.push_back(state);
    }
  } while (!to_visit.empty());
}

} // end namespace

} // end namespace fsm
} // end namespace gnossen
