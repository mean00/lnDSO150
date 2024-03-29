/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8-dev */

#ifndef PB_MESSAGING_PB_H_INCLUDED
#define PB_MESSAGING_PB_H_INCLUDED
#include "defs/defines.pb.h"
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _msg_query
{
    int32_t dummy;
} msg_query;

typedef struct _msg_reply
{
    STATUS s;
} msg_reply;

typedef struct _msg_set_voltage
{
    VOLTAGE voltage;
} msg_set_voltage;

typedef struct _msg_set_timebase
{
    TIMEBASE timebase;
} msg_set_timebase;

typedef struct _msg_set_trigger
{
    TRIGGER trigger;
} msg_set_trigger;

typedef struct _msg_set_trigger_value
{
    float trigger_value;
} msg_set_trigger_value;

typedef struct _UnionMessage
{
    pb_size_t which_msg;
    union {
        msg_reply msg_r;
        msg_set_voltage msg_sv;
        msg_set_timebase msg_stb;
        msg_set_trigger msg_str;
        msg_query msg_gv;
        msg_query msg_gtb;
        msg_query msg_gtr;
        msg_set_trigger_value msg_stv;
        msg_query msg_gtv;
        msg_query msg_data;
        msg_reply msg_rdata; /* if the status is ok, it is followed immedately by a raw screen dump */
    } msg;
} UnionMessage;

#ifdef __cplusplus
extern "C"
{
#endif

/* Initializer values for message structs */
#define msg_query_init_default                                                                                         \
    {                                                                                                                  \
        0                                                                                                              \
    }
#define msg_reply_init_default                                                                                         \
    {                                                                                                                  \
        _STATUS_MIN                                                                                                    \
    }
#define msg_set_voltage_init_default                                                                                   \
    {                                                                                                                  \
        _VOLTAGE_MIN                                                                                                   \
    }
#define msg_set_timebase_init_default                                                                                  \
    {                                                                                                                  \
        _TIMEBASE_MIN                                                                                                  \
    }
#define msg_set_trigger_init_default                                                                                   \
    {                                                                                                                  \
        _TRIGGER_MIN                                                                                                   \
    }
#define msg_set_trigger_value_init_default                                                                             \
    {                                                                                                                  \
        0                                                                                                              \
    }
#define UnionMessage_init_default                                                                                      \
    {                                                                                                                  \
        0,                                                                                                             \
        {                                                                                                              \
            msg_reply_init_default                                                                                     \
        }                                                                                                              \
    }
#define msg_query_init_zero                                                                                            \
    {                                                                                                                  \
        0                                                                                                              \
    }
#define msg_reply_init_zero                                                                                            \
    {                                                                                                                  \
        _STATUS_MIN                                                                                                    \
    }
#define msg_set_voltage_init_zero                                                                                      \
    {                                                                                                                  \
        _VOLTAGE_MIN                                                                                                   \
    }
#define msg_set_timebase_init_zero                                                                                     \
    {                                                                                                                  \
        _TIMEBASE_MIN                                                                                                  \
    }
#define msg_set_trigger_init_zero                                                                                      \
    {                                                                                                                  \
        _TRIGGER_MIN                                                                                                   \
    }
#define msg_set_trigger_value_init_zero                                                                                \
    {                                                                                                                  \
        0                                                                                                              \
    }
#define UnionMessage_init_zero                                                                                         \
    {                                                                                                                  \
        0,                                                                                                             \
        {                                                                                                              \
            msg_reply_init_zero                                                                                        \
        }                                                                                                              \
    }

/* Field tags (for use in manual encoding/decoding) */
#define msg_query_dummy_tag 1
#define msg_reply_s_tag 1
#define msg_set_voltage_voltage_tag 1
#define msg_set_timebase_timebase_tag 1
#define msg_set_trigger_trigger_tag 1
#define msg_set_trigger_value_trigger_value_tag 1
#define UnionMessage_msg_r_tag 1
#define UnionMessage_msg_sv_tag 2
#define UnionMessage_msg_stb_tag 3
#define UnionMessage_msg_str_tag 4
#define UnionMessage_msg_gv_tag 5
#define UnionMessage_msg_gtb_tag 6
#define UnionMessage_msg_gtr_tag 7
#define UnionMessage_msg_stv_tag 8
#define UnionMessage_msg_gtv_tag 9
#define UnionMessage_msg_data_tag 12
#define UnionMessage_msg_rdata_tag 13

/* Struct field encoding specification for nanopb */
#define msg_query_FIELDLIST(X, a) X(a, STATIC, REQUIRED, INT32, dummy, 1)
#define msg_query_CALLBACK NULL
#define msg_query_DEFAULT NULL

#define msg_reply_FIELDLIST(X, a) X(a, STATIC, REQUIRED, UENUM, s, 1)
#define msg_reply_CALLBACK NULL
#define msg_reply_DEFAULT NULL

#define msg_set_voltage_FIELDLIST(X, a) X(a, STATIC, REQUIRED, UENUM, voltage, 1)
#define msg_set_voltage_CALLBACK NULL
#define msg_set_voltage_DEFAULT NULL

#define msg_set_timebase_FIELDLIST(X, a) X(a, STATIC, REQUIRED, UENUM, timebase, 1)
#define msg_set_timebase_CALLBACK NULL
#define msg_set_timebase_DEFAULT NULL

#define msg_set_trigger_FIELDLIST(X, a) X(a, STATIC, REQUIRED, UENUM, trigger, 1)
#define msg_set_trigger_CALLBACK NULL
#define msg_set_trigger_DEFAULT NULL

#define msg_set_trigger_value_FIELDLIST(X, a) X(a, STATIC, REQUIRED, FLOAT, trigger_value, 1)
#define msg_set_trigger_value_CALLBACK NULL
#define msg_set_trigger_value_DEFAULT NULL

#define UnionMessage_FIELDLIST(X, a)                                                                                   \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_r, msg.msg_r), 1)                                                           \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_sv, msg.msg_sv), 2)                                                         \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_stb, msg.msg_stb), 3)                                                       \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_str, msg.msg_str), 4)                                                       \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_gv, msg.msg_gv), 5)                                                         \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_gtb, msg.msg_gtb), 6)                                                       \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_gtr, msg.msg_gtr), 7)                                                       \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_stv, msg.msg_stv), 8)                                                       \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_gtv, msg.msg_gtv), 9)                                                       \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_data, msg.msg_data), 12)                                                    \
    X(a, STATIC, ONEOF, MESSAGE, (msg, msg_rdata, msg.msg_rdata), 13)
#define UnionMessage_CALLBACK NULL
#define UnionMessage_DEFAULT NULL
#define UnionMessage_msg_msg_r_MSGTYPE msg_reply
#define UnionMessage_msg_msg_sv_MSGTYPE msg_set_voltage
#define UnionMessage_msg_msg_stb_MSGTYPE msg_set_timebase
#define UnionMessage_msg_msg_str_MSGTYPE msg_set_trigger
#define UnionMessage_msg_msg_gv_MSGTYPE msg_query
#define UnionMessage_msg_msg_gtb_MSGTYPE msg_query
#define UnionMessage_msg_msg_gtr_MSGTYPE msg_query
#define UnionMessage_msg_msg_stv_MSGTYPE msg_set_trigger_value
#define UnionMessage_msg_msg_gtv_MSGTYPE msg_query
#define UnionMessage_msg_msg_data_MSGTYPE msg_query
#define UnionMessage_msg_msg_rdata_MSGTYPE msg_reply

    extern const pb_msgdesc_t msg_query_msg;
    extern const pb_msgdesc_t msg_reply_msg;
    extern const pb_msgdesc_t msg_set_voltage_msg;
    extern const pb_msgdesc_t msg_set_timebase_msg;
    extern const pb_msgdesc_t msg_set_trigger_msg;
    extern const pb_msgdesc_t msg_set_trigger_value_msg;
    extern const pb_msgdesc_t UnionMessage_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define msg_query_fields &msg_query_msg
#define msg_reply_fields &msg_reply_msg
#define msg_set_voltage_fields &msg_set_voltage_msg
#define msg_set_timebase_fields &msg_set_timebase_msg
#define msg_set_trigger_fields &msg_set_trigger_msg
#define msg_set_trigger_value_fields &msg_set_trigger_value_msg
#define UnionMessage_fields &UnionMessage_msg

/* Maximum encoded size of messages (where known) */
#define UnionMessage_size 13
#define msg_query_size 11
#define msg_reply_size 2
#define msg_set_timebase_size 2
#define msg_set_trigger_size 2
#define msg_set_trigger_value_size 5
#define msg_set_voltage_size 2

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
