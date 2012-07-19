#ifndef __LAC_TYPE_H__
#define __LAC_TYPE_H__


typedef unsigned short	LAC_PORT_ID;
typedef unsigned short	LAC_KEY;
typedef int      SYSTEM_PRIORITY;
typedef unsigned short	LAC_PORT_PRIORITY;
typedef unsigned short LAC_TIME;

#define LAC_MAC_LEN 6
typedef unsigned char	SYSTEM_MAC[LAC_MAC_LEN];

typedef enum {LACP_ACTIVE = True, LAC_PASSIVE = False} LACP_ACTIVE_E;
typedef enum {SHORT_TIMEOUT = True, LONG_TIMEOUT = False} LACP_TIMEOUT_E;
typedef enum {LACP_ENABLED  = True, LACP_DISABLED= False} LACP_ENABLE_E;

#if 0
typedef struct /* LAC_STATE */
{
    unsigned lacp_activity   : 1;
    unsigned lacp_timeout    : 1;
    unsigned aggregation     : 1;
    unsigned synchronization : 1;
    unsigned collecting      : 1;
    unsigned distributing    : 1;
    unsigned defaulted       : 1;
    unsigned expired         : 1;
} LAC_STATE;
#endif
typedef unsigned char LAC_STATE;
#define LAC_STATE_ACT 0x1
#define LAC_STATE_TMT 0X2
#define LAC_STATE_AGG 0X4
#define LAC_STATE_SYN 0X8
#define LAC_STATE_COL 0X10
#define LAC_STATE_DIS 0X20
#define LAC_STATE_DEF 0X40
#define LAC_STATE_EXP 0X80


#define LAC_STATE_SET_BIT(flag,mask,value) ((value) ? (flag |= mask) : (flag &= ~mask))
#define LAC_STATE_GET_BIT(flag,mask) ((flag & mask) ? 1 : 0)
#define LAC_STATE_COPY_BIT(flag_to, flag_from, mask) ((flag_from & mask) ?  (flag_to |= mask) : (flag_to &= ~mask))
/* return zero if equal */
#define LAC_STATE_CMP_BIT(flag1, flag2, mask) ((flag1 & mask) != (flag2 & mask))

typedef struct /* LAC_PORT_INFO */
{
    SYSTEM_PRIORITY       system_priority;
    SYSTEM_MAC            system_mac;
    LAC_KEY               key;
    LAC_PORT_PRIORITY     port_priority;
    LAC_PORT_ID           port_index;
    LAC_STATE             state;
} LAC_PORT_INFO;

#define TIMERS_NUMBER   3
#include "statmch.h"
typedef struct lac_port_t /* Lac_port */
{
    struct lac_port_t  *next;
    struct lac_sys_t   *system;
    LAC_PORT_ID     port_index;
    char*			port_name;

    LAC_STATE_MACH_T*	rx;
    LAC_STATE_MACH_T*	sel;
    LAC_STATE_MACH_T*	mux;
    LAC_STATE_MACH_T*	tx;

    LAC_STATE_MACH_T*	machines; /* list of machines */

    Bool         	port_enabled;
    Bool         	port_moved;

    LAC_PORT_INFO        actor;
    LAC_PORT_INFO        partner;
    LAC_PORT_INFO        actor_admin;
    LAC_PORT_INFO        partner_admin;

    LAC_PORT_INFO        msg_actor;
    LAC_PORT_INFO        msg_partner;

    Bool         	lacp_enabled;
    Bool         	selected;

    LAC_TIME           current_while;
    LAC_TIME           periodic_timer;
    LAC_TIME           wait_while;

    LAC_TIME*	timers[TIMERS_NUMBER]; /*list of timers */

    struct lac_port_t      *aport;

    int speed;
    int duplex;
    Bool            static_agg;
    int             agg_id;

    Bool         	standby;
    Bool         	ntt;
    int             hold_count;
    Bool ready_n;

    Bool 			rcvdLacpdu;

    unsigned int tx_lacpdu_cnt;
    unsigned int rx_lacpdu_cnt;
} LAC_PORT_T;

typedef struct lacpdu_t
{
    unsigned char  slow_protocols_address[6];
    unsigned char  src_address[6];
    unsigned short	ethertype;
    unsigned char protocol_subtype;
    unsigned char protocol_version;


    unsigned char type_actor;
    unsigned char len_actor;
    LAC_PORT_INFO  actor;
    unsigned char reverved1[3];

    unsigned char type_partner;
    unsigned char len_partner;
    LAC_PORT_INFO  partner;
    unsigned char reverved2[3];

    unsigned char type_collector;
    unsigned char len_collector;
    unsigned short collector_max_delay;
    unsigned char reverved3[12];

    unsigned char type_terminator;
    unsigned char len_terminator;
    unsigned char reverved4[50];
} LACPDU_T;


enum    {Slow_protocols_ethertype = (USHORT)0x4242};
enum    {Lacp_subtype             = (Octet)1};

enum    {Null_system = 0};

typedef struct lac_sys_t /* Lac_system */
{
    LAC_PORT_T * ports;  /* all lacp port link list */
    BITMAP_T * portmap;
    int number_of_ports;
    int admin_state;

    SYSTEM_PRIORITY priority;
    SYSTEM_MAC       id;

    LAC_TIME fast_periodic_time;
    LAC_TIME slow_periodic_time;
    LAC_TIME short_timeout_time;
    LAC_TIME long_timeout_time;
    LAC_TIME aggregate_wait_time;

    Bool lacp_timeout;
    int tx_hold_count;
} LAC_SYS_T;

typedef struct {
    int speed;
    int duplex;
    int cd;
    int tid;
    int link_status;
} port_attr;


#endif
