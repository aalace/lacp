#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "../lac_out.h"
#define STATES {        \
  CHOOSE(INIT),    \
  CHOOSE(SELECTION),    \
}

#define GET_STATE_NAME lac_sel_get_state_name
#include "choose.h"

static Bool partners_aport(LAC_PORT_T *port)
{
    LAC_PORT_T *p = NULL;

    for (p = port->system->ports; p; p = p->next)
    {
        if (port->agg_id != p->agg_id )
            continue;

        if(  (p->actor.system_priority   == port->partner.system_priority     )
                && (!memcmp(p->actor.system_mac, port->partner.system_mac, 6)           )
                && (p->actor.key               == port->partner.key                 )
                && (p->actor.port_index           == port->partner.port_index             )
                &&((p->actor.port_priority     <  port->actor.port_priority )
                   ||( (p->actor.port_priority  == port->actor.port_priority )
                       &&(p->actor.port_index        <  port->actor.port_index       )
                     )   ) )
            return(True);
    }

    return(False);
}
#if 0
/*
find a port 's aggregator port
aggregator port has the same:{ actor system id, [agg id], actor key, partner system id, partner key}
and has minor{port priority, portno} */
LAC_PORT_T get_dyn_agg_aggregator_port(LAC_PORT_T *port)
{
    LAC_PORT_T *ap0	= &port->system->ports;
    LAC_PORT_T *ap 	= &port->system->ports;
    LAC_PORT_T *best	=  port;

    while ((ap = ap->next) != ap0)
    {
        if(  (ap->actor.system_priority	  == port->actor.system_priority	)
                && (!memcmp(p->actor.system_mac, port->actor.system_mac))
                && (ap->actor.key				  == port->actor.key				)
                && (ap->partner.system_priority   == port->partner.system_priority	)
                && (!memcmp(p->partner.system_mac, port->partner.system_mac))
                && (ap->partner.key 			  == port->partner.key				)
                && (ap->actor.state.aggregation   &&   ap->partner.state.aggregation)
                && (port->actor.state.aggregation && port->partner.state.aggregation)
                &&((ap->actor.port_priority 	  <  best->actor.port_priority )
                   ||( (ap->actor.port_priority	  == best->actor.port_priority )
                       &&(ap->actor.port_no		  <  best->actor.port_no	   )
                     ) )
                //&& (!partners_aport(ap, port))
          )
            best = ap;
    }
    return(best);
}

}
#endif
LAC_PORT_T *select_master_port(LAC_SYS_T *this, int agg_id)
{

    LAC_PORT_T *best = NULL;
    LAC_PORT_T *p = NULL;

    //update port info
    for (p = this->ports; p; p = p->next)
    {
        if (agg_id != p->agg_id )
            continue;

        p->actor.key = lac_get_port_oper_key(p->port_index);
    }

    /* select best port(master port) */
    for (p = this->ports; p; p = p->next)
    {
        /* reduce the search range */
        if (agg_id != p->agg_id || !p->port_enabled || !p->lacp_enabled || !p->duplex)
        {
            lac_trace("<%s.%d>", __FUNCTION__, __LINE__);
            continue;
        }

        if (!best)
        {
            best = p;
            continue;
        }

        if( p->speed > best->speed)
        {
            best = p;
            continue;

        }
        else if ( p->speed < best->speed)
        {
            continue;
        }
        else if (p->actor.port_priority < best->actor.port_priority)
        {
            best = p;
            continue;

        }
        else if     (p->actor.port_priority > best->actor.port_priority)
        {
            continue;

        }
        else if ( (!LAC_STATE_GET_BIT(p->actor.state, LAC_STATE_DEF))
                  &&  LAC_STATE_GET_BIT(best->actor.state, LAC_STATE_DEF))
        {
            best = p;
            continue;

        }
        else if ( (LAC_STATE_GET_BIT(p->actor.state, LAC_STATE_DEF))
                  &&  !LAC_STATE_GET_BIT(best->actor.state, LAC_STATE_DEF))
        {
            continue;

        }
        else if (p->actor.port_index < best->actor.port_index)
        {
            best = p;
            continue;

        }
    }
    if (best)
        lac_trace("\r\n agg %d get best port: %d", agg_id, best->port_index);
    else
        lac_trace("\r\n agg %d NO best port.", agg_id);

    return best;

}
int update_agg_ports_select(LAC_SYS_T *this, int agg_id)
{
    LAC_PORT_T *best = NULL;
    LAC_PORT_T *p = NULL;
    best = select_master_port(this, agg_id);
    for (p = this->ports; p; p = p->next)
    {
        /* reduce the search range */
        if (p->agg_id != agg_id)
        {
            continue;
        }

        if (!best)
        {
            p->selected = True;
            p->standby = True;
            p->aport = p;
            continue;
        }

        p->aport = best;

        if(p->port_enabled
                && p->lacp_enabled
                && p->actor.key == best->actor.key
                && p->partner.system_priority == best->partner.system_priority
                && (!memcmp(p->partner.system_mac, best->partner.system_mac, 6))
                && (p->partner.key == best->partner.key)
                && LAC_STATE_GET_BIT(p->actor.state, LAC_STATE_AGG)
                && LAC_STATE_GET_BIT(p->partner.state, LAC_STATE_AGG)
                && LAC_STATE_GET_BIT(best->actor.state, LAC_STATE_AGG)
                && LAC_STATE_GET_BIT(best->partner.state, LAC_STATE_AGG)
                && !partners_aport(p))
        {
            p->selected = True;
            p->standby = False;
            lac_trace("\r\n <%s.%d> port %d ---> Selected",  __FUNCTION__, __LINE__, p->port_index);
        } else {
            p->selected = True;
            p->standby = True;
            lac_trace("\r\n <%s.%d> port %d ---> Standby ",  __FUNCTION__, __LINE__, p->port_index);
        }
    }
    return 0;

}


int selection_logic(LAC_PORT_T *port)
{
    LAC_SYS_T *this = port->system;

    /* maybe port delete from agg */
    if (!port->lacp_enabled)
    {
        port->selected = True;
        port->standby = True;
        return 0;
    }

    if (!port->agg_id)
    {
        return 1;
    }

    update_agg_ports_select(this, port->agg_id);

    return 0;
}

int lac_select(LAC_PORT_T *port)
{
    return selection_logic(port);
}

void lac_sel_enter_state (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;

    switch (this->State) {
    case BEGIN:
    case INIT:
        port->selected = False;
        break;

    case SELECTION:
        lac_select(port);
        break;
    };

    return ;

}

Bool lac_sel_check_conditions (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;

    switch (this->State) {
    case BEGIN:
    case INIT:
        return lac_hop_2_state (this, SELECTION);

    case SELECTION:
        if (!port->selected)
        {
            return lac_hop_2_state (this, SELECTION);
        }
        break;

    default:
        break;
    };
    return False;
}

