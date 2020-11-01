#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <iostream>

#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <list>
#include <vector>
#include <set>
#include <unordered_set>

#include "fsm.h"

namespace gnossen {
namespace fsm {

namespace {

static void WriteDotFile(const Fsm& fsm, std::string filename) {
  std::string extra_artifacts_dir(getenv("TEST_UNDECLARED_OUTPUTS_DIR"));
  std::string filepath = extra_artifacts_dir + "/" + filename;
  std::ofstream outfile(filepath, std::ofstream::out);
  outfile << fsm.ToDotGraph() << std::endl;
}

TEST(FsmTest, CanBuild) {
  std::vector<char> alphabet {'a', 'b', 'c'};
  std::vector<char> extended_alphabet = {'a', 'b', 'c', '\0'};
  Fsm fsm(extended_alphabet);
  auto initial_state = fsm.GetStartState();
  auto state2 = fsm.AddState();
  fsm.AddTransition(initial_state, state2, 'c');
  fsm.AddTransition(state2, state2, 'a');
  fsm.AddTransition(state2, state2, 'b');
  auto state3 = fsm.AddState();
  fsm.AddTransition(state2, state3, 'c');
  auto state4 = fsm.AddState();
  for (char letter : alphabet) {
    fsm.AddTransition(state3, state4, letter);
  }
  fsm.AddTransition(state4, fsm.GetSuccessState(), '\0');

  std::set<unsigned int> observed_states;
  auto states = fsm.GetStates();
  for (auto state = states.begin(); state != states.end(); ++state) {
    observed_states.insert(fsm.StateIdentifier(state));
  }
  std::set<unsigned int> expected_states = {
    fsm.StateIdentifier(initial_state),
    fsm.StateIdentifier(state2),
    fsm.StateIdentifier(state3),
    fsm.StateIdentifier(state4),
    fsm.StateIdentifier(fsm.GetSuccessState()),
    fsm.StateIdentifier(fsm.GetFailureState())
  };

  EXPECT_THAT(observed_states, ::testing::ContainerEq(expected_states));

  using Edge = std::pair<unsigned int, char>;
  using Edges = std::list<Edge>;
  std::map<unsigned int, Edges> observed_transitions;
  std::map<unsigned int, Edges> expected_transitions = {
    {fsm.StateIdentifier(initial_state), Edges{Edge{fsm.StateIdentifier(state2), 'c'}}},
    {fsm.StateIdentifier(state2), Edges{
                Edge{fsm.StateIdentifier(state2), 'a'},
                Edge{fsm.StateIdentifier(state2), 'b'},
                Edge{fsm.StateIdentifier(state3), 'c'}
             }
    },
    {fsm.StateIdentifier(state3), Edges{
                Edge{fsm.StateIdentifier(state4), 'a'},
                Edge{fsm.StateIdentifier(state4), 'b'},
                Edge{fsm.StateIdentifier(state4), 'c'}
             }
    },
    {fsm.StateIdentifier(state4), Edges{Edge{fsm.StateIdentifier(fsm.GetSuccessState()), '\0'}}},
  };

  for (auto state = states.begin(); state != states.end(); ++state) {
    for (auto transition : fsm.GetTransitions(state)) {
      EXPECT_FALSE(transition.second.empty_edge);
      EXPECT_FALSE(transition.second.remainder);
      observed_transitions[fsm.StateIdentifier(state)].emplace_back(
              fsm.StateIdentifier(transition.first),
              transition.second.edge_label);
    }
  }

  EXPECT_THAT(observed_transitions, ::testing::ContainerEq(expected_transitions));

  WriteDotFile(fsm, "fsm1.dot");
}

TEST(FsmTest, ToBinarizedFsm) {
  std::vector<char> alphabet {'a', 'b', 'c'};
  std::vector<char> extended_alphabet {'a', 'b', 'c', '\0'};
  Fsm fsm(extended_alphabet);
  auto initial_state = fsm.GetStartState();
  auto state2 = fsm.AddState();
  fsm.AddTransition(initial_state, state2, 'c');
  fsm.AddTransition(state2, state2, 'a');
  fsm.AddTransition(state2, state2, 'b');
  auto state3 = fsm.AddState();
  fsm.AddTransition(state2, state3, 'c');
  auto state4 = fsm.AddState();
  for (char letter : alphabet) {
    fsm.AddTransition(state3, state4, letter);
  }
  fsm.AddTransition(state4, fsm.GetSuccessState(), '\0');

  Fsm binarized_fsm = ToBinarizedNfsm(fsm);

  WriteDotFile(fsm, "pre_binarized_fsm1.dot");
  WriteDotFile(binarized_fsm, "binarized_fsm1.dot");

  std::unordered_set<unsigned int> visited;
  std::list<Fsm::States::iterator> to_visit;
  to_visit.push_back(binarized_fsm.GetStartState());
  Fsm::States::iterator state;
  do {
    state = to_visit.back();
    to_visit.pop_back();

    if (state == binarized_fsm.GetSuccessState() ||
        state == binarized_fsm.GetFailureState() ||
        visited.find(binarized_fsm.StateIdentifier(state)) != visited.end())
      continue;

    visited.insert(binarized_fsm.StateIdentifier(state));

    auto transitions = binarized_fsm.GetTransitions(state);
    size_t transition_count = std::distance(transitions.begin(), transitions.end());
    size_t deterministic_transitions = 0;
    std::vector<Fsm::States::iterator> next_states;
    for (auto edge : transitions) {
      if (!edge.second.empty_edge && !edge.second.remainder)
        ++deterministic_transitions;
      next_states.push_back(edge.first);
    }

    ASSERT_LE(transition_count, 2);
    ASSERT_LE(deterministic_transitions, 1);

    for (auto state : next_states) {
      to_visit.push_back(state);
    }
  } while (!to_visit.empty());
}

} // end namespace

} // end namespace fsm
} // end namespace gnossen
