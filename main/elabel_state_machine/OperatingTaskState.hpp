#ifndef OPERATINGTASKSTATE_HPP
#define OPERATINGTASKSTATE_HPP

#include "StateMachine.hpp"
#include "ElabelController.hpp"

class OperatingTaskState : public State<ElabelController>
{
private:

public:
    virtual void Init(ElabelController* pOwner);
    virtual void Enter(ElabelController* pOwner);
    virtual void Execute(ElabelController* pOwner);
    virtual void Exit(ElabelController* pOwner);

    static OperatingTaskState* Instance()
    {
        static OperatingTaskState instance;
        return &instance;
    }
};
#endif