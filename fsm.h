#include <string>
#include <list>
#include <utility>

namespace gnossen {
namespace fsm {

typedef struct FsmState FsmState;

struct EdgeLabel {
    bool empty_edge;
    char edge_label;

    EdgeLabel() : empty_edge(true), edge_label() {}
    EdgeLabel(char edge_label) : empty_edge(false), edge_label(edge_label) {}
};

// A Finite State Machine.
class FSMInterface {
public:
    FSMInterface() = default;
    virtual ~FSMInterface() = default;

    // Mutating methods.
    virtual FsmState* AddState() = 0;
    virtual const void AddTransition(FsmState* from,
                                          FsmState* to,
                                          char letter) = 0;

    virtual const void AddNonDeterministicTransition(FsmState* from,
                                                     FsmState* to) = 0;

    // TODO: Supply an interface to add multiple letters to the
    // transition?

    // Read methods.

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
    Fsm();
    ~Fsm() {}

    FsmState* AddState() ;
    const void AddTransition(FsmState* from,
                                      FsmState* to,
                                      char letter);

    const void AddNonDeterministicTransition(FsmState* from,
                                             FsmState* to);

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

    std::list<State> states_;

    // TODO: Eliminate this when we add an iterator interface.
    std::list<FsmState*> state_pointers_;

    FsmState* start_state_;
    FsmState* success_state_;
    FsmState* failure_state_;
    unsigned int next_id_;
};

} // end namespace fsm
} // end namespace gnossen
