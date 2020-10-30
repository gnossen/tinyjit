#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <algorithm>
#include <list>
#include <vector>
#include <set>

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

} // end namespace

} // end namespace fsm
} // end namespace gnossen
