#include <string>
#include <list>
#include <utility>
#include <unordered_set>
#include <vector>

namespace gnossen {
namespace fsm {

struct EdgeLabel {
    // If set, this is a nondeterministic transition, consuming no input
    // characters. Mutually exlusive with the other two fields of this struct.
    bool empty_edge;

    // If set, this represents all remaining characters not already represented
    // in the list of out edges for this state. Mutually exclusive with the
    // other two fields in this struct.
    bool remainder;

    // The label for this edge -- an actual character.
    char edge_label;

    EdgeLabel() : empty_edge(true), remainder(false), edge_label() {}
    EdgeLabel(char edge_label) : empty_edge(false), remainder(false), edge_label(edge_label) {}

    static EdgeLabel Remainder() {
        return EdgeLabel {false, true, '\0'};
    }

private:
    EdgeLabel(bool empty_edge, bool remainder, char edge_label) :
        empty_edge(empty_edge),
        remainder(remainder),
        edge_label(edge_label) {}

};



class Fsm {
public:
    struct State;
    using States = std::list<State>;

    using Transition = std::pair<States::iterator, EdgeLabel>;
    using Transitions = std::list<std::pair<States::iterator, EdgeLabel>>;

    struct State {
        unsigned int id;
        Transitions edges_out;

        State(unsigned int id);
    };

    explicit Fsm(const std::vector<char>& alphabet);
    ~Fsm() {}

    Fsm(const Fsm&) = default;
    Fsm(Fsm&&) = default;

    Fsm& operator=(const Fsm&) = default;
    Fsm& operator=(Fsm&&) = default;

    // Mutating methods.

    // TODO: Add const_iterator version?
    States::iterator AddState() ;

    // Adds a deterministic transition from one state to another.
    const void AddTransition(States::iterator from,
                             States::iterator to,
                             EdgeLabel label);

    // Adds a deterministic transition from one state to another.
    const void AddTransition(States::iterator from,
                             States::iterator to,
                             char letter);


    // Adds a nondeterministic transition from one state to another.
    // Following this transition does not consume a character.
    const void AddNonDeterministicTransition(States::iterator from,
                                             States::iterator to);

    // Adds a determinisic transition for all letters not already
    // represented by a transition to the given state. No more transitions
    // may be added after this method has been called.
    const void AddTransitionForRemaining(States::iterator from,
                                         States::iterator to);


    // TODO: Supply an interface to add multiple letters to the
    // transition?

    // Read methods.

    const std::vector<char>& GetAlphabet() const;

    friend class StateContainer;

    struct StateContainer {
        States::iterator begin() { return fsm_->states_.begin(); }
        States::iterator end() { return fsm_->states_.end(); }

        StateContainer(Fsm* fsm) : fsm_(fsm) {}

    private:
        Fsm* fsm_;
    };

    struct ConstStateContainer {
        States::const_iterator begin() const { return fsm_->states_.begin(); }
        States::const_iterator end() const { return fsm_->states_.end(); }

        ConstStateContainer(const Fsm* fsm) : fsm_(fsm) {}

    private:
        const Fsm* fsm_;
    };

    StateContainer GetStates();
    ConstStateContainer GetStates() const;

    struct TransitionContainer {
        Transitions::iterator begin() { return (*state_).edges_out.begin(); }
        Transitions::iterator end() { return (*state_).edges_out.end(); }

        TransitionContainer(States::iterator state) : state_(state) {}
    private:
        States::iterator state_;
    };

    struct ConstTransitionContainer {
        Transitions::const_iterator begin() const { return (*state_).edges_out.begin(); }
        Transitions::const_iterator end() const { return (*state_).edges_out.end(); }

        ConstTransitionContainer(States::const_iterator state) : state_(state) {}
    private:
        const States::const_iterator state_;
    };

    TransitionContainer GetTransitions(States::iterator state);
    ConstTransitionContainer GetTransitions(States::const_iterator state) const;

    unsigned int StateIdentifier(States::const_iterator state) const;

    // These three states are automatically created without intervention
    // from the caller.
    States::iterator GetStartState() { return states_.begin(); }
    States::const_iterator GetStartState() const { return GetStartState(); }

    States::iterator GetSuccessState() { return ++states_.begin(); }
    States::const_iterator GetSuccessState() const { return GetSuccessState(); }

    States::iterator GetFailureState() { return ++GetSuccessState(); }
    States::const_iterator GetFailureState() const { return GetFailureState(); }

    // Generates a dot graph corresponding to this FSM.
    std::string ToDotGraph() const;

private:
    const std::vector<char> alphabet_;

    States states_;
    unsigned int next_id_;
};

Fsm ToBinarizedNfsm(Fsm& fsm);

} // end namespace fsm
} // end namespace gnosse
