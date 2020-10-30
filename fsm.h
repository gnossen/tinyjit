#include <string>
#include <list>
#include <utility>
#include <unordered_set>
#include <vector>

namespace gnossen {
namespace fsm {

typedef struct FsmState FsmState;

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

// A Finite State Machine.
class FSMInterface {
public:
    FSMInterface() = default;
    virtual ~FSMInterface() = default;

    // Mutating methods.
    virtual FsmState* AddState() = 0;

    // Adds a deterministic transition from one state to another.
    virtual const void AddTransition(FsmState* from,
                                          FsmState* to,
                                          char letter) = 0;

    // Adds a nondeterministic transition from one state to another.
    // Following this transition does not consume a character.
    virtual const void AddNonDeterministicTransition(FsmState* from,
                                                     FsmState* to) = 0;

    // Adds a determinisic transition for all letters not already
    // represented by a transition to the given state. No more transitions
    // may be added after this method has been called.
    virtual const void AddTransitionForRemaining(FsmState* from,
                                                 FsmState* to) = 0;


    // TODO: Supply an interface to add multiple letters to the
    // transition?

    // Read methods.

    virtual const std::vector<char>& GetAlphabet() const = 0;

    // TODO: Consider an iterator-based interface.
    virtual std::list<FsmState*> GetStates() const = 0;
    virtual std::list<std::pair<FsmState*, EdgeLabel>>
        GetTransitions(FsmState*) const = 0;

    virtual unsigned int StateIdentifier(FsmState*) const = 0;

    // These three states are automatically created without intervention
    // from the caller.
    virtual FsmState* GetStartState() const = 0;
    virtual FsmState* GetSuccessState() const = 0;
    virtual FsmState* GetFailureState() const = 0;

    // Generates a dot graph corresponding to this FSM.
    std::string ToDotGraph() const;
};

class Fsm : public FSMInterface {
public:
    explicit Fsm(const std::vector<char>& alphabet);
    ~Fsm() {}

    Fsm(const Fsm&);

    FsmState* AddState() ;
    const void AddTransition(FsmState* from,
                                      FsmState* to,
                                      char letter);

    const void AddNonDeterministicTransition(FsmState* from,
                                             FsmState* to);

    const void AddTransitionForRemaining(FsmState* from,
                                         FsmState* to);

    const std::vector<char>& GetAlphabet() const;

    std::list<FsmState*> GetStates() const;
    std::list<std::pair<FsmState*, EdgeLabel>>
        GetTransitions(FsmState*) const;

    unsigned int StateIdentifier(FsmState*) const;

    FsmState* GetStartState() const { return start_state_; }
    FsmState* GetSuccessState() const { return success_state_; }
    FsmState* GetFailureState() const { return failure_state_; }

private:
    struct State {
        unsigned int id;
        std::list<std::pair<FsmState*, EdgeLabel>> edges_out;

        State(unsigned int id);
    };

    const std::vector<char> alphabet_;

    std::list<State> states_;

    // TODO: Eliminate this when we add an iterator interface.
    std::list<FsmState*> state_pointers_;

    FsmState* start_state_;
    FsmState* success_state_;
    FsmState* failure_state_;
    unsigned int next_id_;
};

Fsm ToBinarizedNfsm(Fsm& fsm, const std::vector<char>& alphabet);

} // end namespace fsm
} // end namespace gnosse
