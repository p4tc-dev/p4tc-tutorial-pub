#ifndef P4C_PNA_H
#define P4C_PNA_H

#include <stdbool.h>
#include "crc32.h"

typedef __u32 PortId_t;
typedef __u64 Timestamp_t;
typedef __u8  ClassOfService_t;
typedef __u16 CloneSessionId_t;
typedef __u32 MulticastGroup_t;
typedef __u16 EgressInstance_t;
typedef __u8  MirrorSlotId_t;
typedef __u16 MirrorSessionId_t;

// Instead of using enum we define ParserError_t as __u8 to save memory.
typedef __u8 ParserError_t;
static const ParserError_t NoError = 0;  /// No error.
static const ParserError_t PacketTooShort = 1;  /// Not enough bits in packet for 'extract'.
static const ParserError_t NoMatch = 2;  /// 'select' expression has no matches.
static const ParserError_t StackOutOfBounds = 3;  /// Reference to invalid element of a header stack.
static const ParserError_t HeaderTooShort = 4;  /// Extracting too many bits into a varbit field.
static const ParserError_t ParserTimeout = 5;  /// Parser execution time limit exceeded.
static const ParserError_t ParserInvalidArgument = 6;  /// Parser operation was called with a value
/// not supported by the implementation

enum PNA_Source_t { FROM_HOST, FROM_NET };

enum MirrorType { NO_MIRROR, PRE_MODIFY, POST_MODIFY };

/*
 * INGRESS data types
 */
struct pna_main_parser_input_metadata_t {
    bool                     recirculated;
    PortId_t                 input_port;    // taken from xdp_md or __sk_buff
} __attribute__((aligned(4)));

struct pna_main_input_metadata_t {
    // All of these values are initialized by the architecture before
    // the Ingress control block begins executing.
    bool                     recirculated;
    Timestamp_t              timestamp;         // taken from bpf helper
    ParserError_t            parser_error;      // local to parser
    ClassOfService_t         class_of_service;  // 0, set in control as global metadata
    PortId_t                 input_port;
} __attribute__((aligned(4)));;

struct pna_main_output_metadata_t {
    // The comment after each field specifies its initial value when the
    // Ingress control block begins executing.
    ClassOfService_t         class_of_service;
} __attribute__((aligned(4)));

/*
 * Opaque struct to be used to share global PNA metadata fields between eBPF program attached to Ingress and Egress.
 * The size of this struct must be less than 32 bytes.
 */
struct pna_global_metadata {
    bool             recirculated;
    bool             drop; // NOTE : no drop field in PNA metadata, so we keep drop state as internal metadata.
    PortId_t         egress_port;
    enum MirrorType  mirror_type;
    MirrorSlotId_t   mirror_slot_id;
    ParserError_t    parser_error;
    MirrorSessionId_t mirror_session_id;
    __u8             mark;
    bool             pass_to_kernel; // internal metadata, forces sending packet up to kernel stack
} __attribute__((aligned(4)));

// NOTE (tomasz): This struct should be aligned with PNA specs. TBD
struct clone_session_entry {
    u32 egress_port;
    u16 instance;
    u8 class_of_service;
    u8 truncate;
    u16 packet_length_bytes;
} __attribute__((aligned(4)));

struct p4tc_table_entry_act_bpf_params__local {
        u32 pipeid;
        u32 tblid;
};

struct __attribute__((__packed__)) p4tc_table_entry_act_bpf {
        u32 act_id;
	u32 hit:1,
           is_default_miss_act:1,
           is_default_hit_act:1;
        u8 params[124];
};

struct p4tc_table_entry_create_bpf_params__local {
	struct p4tc_table_entry_act_bpf act_bpf;
        u32 profile_id;
        u32 pipeid;
        u32 tblid;
        u32 handle;
        u32 classid;
        u32 chain;
        u16 proto;
        u16 prio;
};

/* Regular table lookup */
extern struct p4tc_table_entry_act_bpf *
bpf_p4tc_tbl_read(struct __sk_buff *skb_ctx,
		  struct p4tc_table_entry_act_bpf_params__local *params,
		  const u32 params__sz,
		  void *key, const __u32 key__sz) __ksym;

/* Regular table lookup */
extern struct p4tc_table_entry_act_bpf *
xdp_p4tc_tbl_read(struct xdp_md *skb_ctx,
		  struct p4tc_table_entry_act_bpf_params__local *params,
		  const u32 params__sz,
		  void *key, const __u32 key__sz) __ksym;


/* No mapping to PNA, but are useful utilities */
extern int
bpf_p4tc_entry_create(struct __sk_buff *skb_ctx,
                      struct p4tc_table_entry_create_bpf_params__local *params,
		      const u32 params__sz,
                      void *key, const u32 key__sz) __ksym;

extern int
xdp_p4tc_entry_create(struct xdp_md *xdp_ctx,
                      struct p4tc_table_entry_create_bpf_params__local *params,
		      const u32 params__sz,
                      void *bpf_key_mask, u32 bpf_key_mask__sz) __ksym;

/* Equivalent to PNA add-on-miss */
extern int
bpf_p4tc_entry_create_on_miss(struct __sk_buff *skb_ctx,
			      struct p4tc_table_entry_create_bpf_params__local *params,
			      const u32 params__sz,
			      void *key, const u32 key__sz) __ksym;

extern int
xdp_p4tc_entry_create_on_miss(struct xdp_md *xdp_ctx,
			      struct p4tc_table_entry_create_bpf_params__local *params,
			      const u32 params__sz,
			      void *key, const u32 key__sz) __ksym;

/* No mapping to PNA, but are useful utilities */
extern int
bpf_p4tc_entry_update(struct __sk_buff *skb_ctx,
		      struct p4tc_table_entry_create_bpf_params__local *params,
		      const u32 params__sz,
		      void *key, const u32 key__sz) __ksym;

extern int
xdp_p4tc_entry_update(struct xdp_md *xdp_ctx,
		      struct p4tc_table_entry_create_bpf_params__local *params,
		      const u32 params__sz,
		      void *key, const u32 key__sz) __ksym;

/* No mapping to PNA, but are useful utilities */
extern int
bpf_p4tc_entry_delete(struct __sk_buff *skb_ctx,
		      struct p4tc_table_entry_create_bpf_params__local *params,
		      const u32 params__sz,
		      void *key, const u32 key__sz) __ksym;

extern int
xdp_p4tc_entry_delete(struct xdp_md *xdp_ctx,
		      struct p4tc_table_entry_create_bpf_params__local *params,
		      const u32 params__sz,
		      void *key, const u32 key__sz) __ksym;

struct p4tc_ext_bpf_params {
        u32 pipe_id;
        u32 ext_id;
        u32 inst_id;
        u32 index;
        u32 flags;
        u8  in_params[128]; /* extern specific params if any */
};

struct p4tc_ext_bpf_val {
        u32 ext_id;
        u32 index;
        u32 verdict;
        u8 out_params[128]; /* specific values if any */
};

/* Equivalent to PNA indirect counters */
extern int
bpf_p4tc_extern_indirect_count_pktsnbytes(struct __sk_buff *skb_ctx,
					  struct p4tc_ext_bpf_params *params,
					  const u32 params__sz) __ksym;

extern int
bpf_p4tc_extern_indirect_count_pktsonly(struct __sk_buff *skb_ctx,
					struct p4tc_ext_bpf_params *params,
					const u32 params__sz) __ksym;

extern int
bpf_p4tc_extern_indirect_count_bytesonly(struct __sk_buff *skb_ctx,
					 struct p4tc_ext_bpf_params *params,
					 const u32 params__sz) __ksym;

extern int
xdp_p4tc_extern_indirect_count_pktsnbytes(struct xdp_md *xdp_ctx,
					  struct p4tc_ext_bpf_params *params,
					  const u32 params__sz) __ksym;

extern int
xdp_p4tc_extern_indirect_count_pktsonly(struct xdp_md *xdp_ctx,
					struct p4tc_ext_bpf_params *params,
					const u32 params__sz) __ksym;

extern int
xdp_p4tc_extern_indirect_count_bytesonly(struct xdp_md *xdp_ctx,
					 struct p4tc_ext_bpf_params *params,
					 const u32 params__sz) __ksym;

extern int bpf_p4tc_extern_meter_bytes_color(struct __sk_buff *skb_ctx,
                                             struct p4tc_ext_bpf_params *params,
                                             const u32 params__sz) __ksym;

extern int bpf_p4tc_extern_meter_bytes(struct __sk_buff *skb_ctx,
                                       struct p4tc_ext_bpf_params *params,
                                       const u32 params__sz) __ksym;

extern int bpf_p4tc_extern_meter_pkts_color(struct __sk_buff *skb_ctx,
                                            struct p4tc_ext_bpf_params *params,
                                            const u32 params__sz) __ksym;

extern int xdp_p4tc_extern_meter_pkts(struct xdp_md *xdp_ctx,
                                      struct p4tc_ext_bpf_params *params,
                                      const u32 params__sz) __ksym;

extern int xdp_p4tc_extern_meter_bytes_color(struct xdp_md *xdp_ctx,
                                             const u32 params__sz) __ksym;

extern int xdp_p4tc_extern_meter_bytes(struct xdp_md *xdp_ctx,
                                       struct p4tc_ext_bpf_params *params,
                                       const u32 params__sz) __ksym;

extern int xdp_p4tc_extern_meter_pkts_color(struct xdp_md *xdp_ctx,
                                            struct p4tc_ext_bpf_params *params,
                                            const u32 params__sz) __ksym;

extern int xdp_p4tc_extern_meter_pkts(struct xdp_md *xdp_ctx,
                                      struct p4tc_ext_bpf_params *params,
                                      const u32 params__sz) __ksym;

/* Start checksum related kfuncs */
struct p4tc_ext_csum_params {
        __wsum csum;
};

/* Equivalent to PNA CRC16 checksum */
/* Basic checksums are not implemented in DPDK */
extern u16
bpf_p4tc_ext_csum_crc16_add(struct p4tc_ext_csum_params *params,
                            const u32 params__sz,
                            const void *data, const u32 data__sz) __ksym;

extern u16
bpf_p4tc_ext_csum_crc16_get(struct p4tc_ext_csum_params *params,
                            const u32 params__sz) __ksym;

extern void
bpf_p4tc_ext_csum_crc16_clear(struct p4tc_ext_csum_params *params,
                              const u32 params__sz) __ksym;

/* Equivalent to PNA CRC32 checksum */
/* Basic checksums are not implemented in DPDK */
extern u32
bpf_p4tc_ext_csum_crc32_add(struct p4tc_ext_csum_params *params,
                            const u32 params__sz,
                            const void *data, const u32 data__sz) __ksym;

extern u32
bpf_p4tc_ext_csum_crc32_get(struct p4tc_ext_csum_params *params,
                            const u32 params__sz) __ksym;

extern void
bpf_p4tc_ext_csum_crc32_clear(struct p4tc_ext_csum_params *params,
                              const u32 params__sz) __ksym;

extern u16
bpf_p4tc_ext_csum_16bit_complement_get(struct p4tc_ext_csum_params *params,
                                       const u32 params__sz) __ksym;

/* Equivalent to PNA 16bit complement checksum (incremental checksum) */
extern __wsum
bpf_p4tc_ext_csum_16bit_complement_add(struct p4tc_ext_csum_params *params,
                                       const u32 params__sz,
                                       const void *data, int len) __ksym;

extern int
bpf_p4tc_ext_csum_16bit_complement_sub(struct p4tc_ext_csum_params *params,
                                       const u32 params__sz,
                                       const void *data, const u32 data__sz) __ksym;

extern void
bpf_p4tc_ext_csum_16bit_complement_clear(struct p4tc_ext_csum_params *params,
                                         const u32 params__sz) __ksym;

extern void
bpf_p4tc_ext_csum_16bit_complement_set_state(struct p4tc_ext_csum_params *params,
                                             const u32 params__sz,
                                             u16 csum) __ksym;

/* Equivalent to PNA crc16 hash */
extern u16
bpf_p4tc_ext_hash_crc16(const void *data, int len, u16 seed) __ksym;

/* Equivalent to PNA crc16 hash base */
static inline u16
bpf_p4tc_ext_hash_base_crc16(const void *data, const u32 data__sz,
                             u32 base, u32 max, u16 seed) {
	u16 hash = bpf_p4tc_ext_hash_crc16(data, data__sz, seed);

	return (base + (hash % max));
}

/* Equivalent to PNA crc32 hash */
extern u32
bpf_p4tc_ext_hash_crc32(const void *data, const u32 data__sz, u32 seed) __ksym;

/* Equivalent to PNA crc32 hash base */
static inline u32
bpf_p4tc_ext_hash_base_crc32(const void *data, const u32 data__sz,
			     u32 base, u32 max, u32 seed) {
	u32 hash = bpf_p4tc_ext_hash_crc32(data, data__sz, seed);

	return (base + (hash % max));
}

/* Equivalent to PNA 16-bit ones complement hash */
extern u16
bpf_p4tc_ext_hash_16bit_complement(const void *data, const u32 data__sz,
				   u16 seed) __ksym;

/* Equivalent to PNA 16-bit ones complement hash base */
static inline u16
bpf_p4tc_ext_hash_base_16bit_complement(const void *data, const u32 data__sz,
					u32 base, u32 max, u16 seed) {
	u16 hash = bpf_p4tc_ext_hash_16bit_complement(data, data__sz, seed);

	return (base + (hash % max));
}

/* DPDK doesn't implement this yet */
/* Equivalent to PNA is_net_port */
extern bool
bpf_p4tc_is_net_port(struct __sk_buff *skb_ctx, const u32 ifindex) __ksym;

/* DPDK doesn't implement this yet */
/* Equivalent to PNA is_host_port */
extern bool
bpf_p4tc_is_host_port(struct __sk_buff *skb_ctx, const u32 ifindex) __ksym;

/* DPDK doesn't implement this yet */
/* Equivalent to PNA is_net_port */
extern bool
xdp_p4tc_is_net_port(struct xdp_md *xdp_ctx, const u32 ifindex) __ksym;

/* DPDK doesn't implement this yet */
/* Equivalent to PNA is_host_port */
extern bool
xdp_p4tc_is_host_port(struct xdp_md *xdp_ctx, const u32 ifindex) __ksym;


/* Extern control path read (for example, used for register read) */
extern struct p4tc_ext_bpf_val *
bpf_p4tc_extern_md_read(struct __sk_buff *skb_ctx,
			struct p4tc_ext_bpf_params *params,
			const u32 params__sz) __ksym;

/* Extern control path write (for example, used for register write) */
extern int bpf_p4tc_extern_md_write(struct __sk_buff *skb_ctx,
				    struct p4tc_ext_bpf_params *params,
				    const u32 params__sz,
				    struct p4tc_ext_bpf_val *val,
				    const u32 val__sz) __ksym;

/* Extern control path read (for example, used for register read) for XDP */
extern struct p4tc_ext_bpf_val *
xdp_p4tc_extern_md_read(struct xdp_md *xdp_ctx,
			struct p4tc_ext_bpf_params *params,
			const u32 params__sz) __ksym;

/* Extern control path read (for example, used for register write for XDP */
extern int xdp_p4tc_extern_md_write(struct xdp_md *xdp_ctx,
				    struct p4tc_ext_bpf_params *params,
				    const u32 params__sz,
				    struct p4tc_ext_bpf_val *val,
				    const u32 val__sz) __ksym;

/* Timestamp  PNA extern */
static inline u64 bpf_p4tc_extern_timestamp() {
	return bpf_ktime_get_ns();
}

#define U32_MAX            ((u32)~0U)

/* Random PNA extern */
static inline u32 bpf_p4tc_extern_random(u32 min, u32 max) {
	if (max == U32_MAX)
		return (min + bpf_get_prandom_u32());

	return (min + bpf_get_prandom_u32()) % (max + 1);
}

#define send_to_port(x) (compiler_meta__->egress_port = x)
#define drop_packet() (compiler_meta__->drop = true)

#endif //P4C_PNA_H
