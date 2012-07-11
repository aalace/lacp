#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_pdu.h"

#define STATES {        \
  CHOOSE(DETACHED),    \
  CHOOSE(DISABLE),    \
  CHOOSE(WAITING),    \
  CHOOSE(ATTACHED),         \
  CHOOSE(RX_TX),  \
}

#define GET_STATE_NAME lac_mux_get_state_name
#include "choose.h"

int sys_is_ready()
{

}
int disable_collecting_distributing()
{
    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}
int enable_collecting_distributing()
{
    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}

int detach_mux_from_aggregator()
{
    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}
int attach_mux_to_aggregator()
{
    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}

void lac_mux_enter_state (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;

    switch (this->State) {
    case BEGIN:
    case DISABLE:
        break;

    case DETACHED:
        detach_mux_from_aggregator(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, False);
        disable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, False);
        port->ntt = True;
        break;

    case WAITING:
        port->wait_while = port->system->aggregate_wait_time;
        break;

    case ATTACHED:
        attach_mux_to_aggregator(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, True);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, False);
        disable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, False);
        port->ntt = True;
        break;

    case RX_TX:
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, True);
        enable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, True);
        port->ntt = True;
        break;

    };
}

Bool lac_mux_check_conditions (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;
    if (!port->port_enabled || !port->lacp_enabled)
    {
        if (this->State == DISABLE)
            return False;
        else
            return lac_hop_2_state (this, DISABLE);
    }

    switch (this->State) {
    case BEGIN:
        return lac_hop_2_state (this, DETACHED);
    case DISABLE:
        if (port->port_enabled && port->lacp_enabled)
            return lac_hop_2_state (this, DETACHED);

    case DETACHED:
        if (port->selected == True)
            return lac_hop_2_state (this, WAITING);
        break;
    case WAITING:
        if (port->selected == True && sys_is_ready())
            return lac_hop_2_state (this, ATTACHED);

        if (!port->selected)
            return lac_hop_2_state (this, DETACHED);
        break;

    case ATTACHED:
        if (port->selected && LAC_STATE_GET_BIT(port->partner.state, LAC_STATE_SYN))
            return lac_hop_2_state (this, RX_TX);
        if (!port->selected)
            return lac_hop_2_state (this, DETACHED);
        break;

    case RX_TX:
        if (!port->selected || !LAC_STATE_GET_BIT(port->partner.state, LAC_STATE_SYN))
            return lac_hop_2_state (this, ATTACHED);
        break;

    default:
        break;
    };
    return False;
}
