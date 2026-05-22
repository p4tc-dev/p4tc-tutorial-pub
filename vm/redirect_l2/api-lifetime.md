# P4TC Runtime API: Lifecycle of Operations

This document describes the end-to-end workflow of P4TC operations, using the `redirect_l2` example for illustration.

---

## 1. General Preparation Stage

Every operation begins with initializing the environment and staging binary data.
The assumption is the pipeline has already been manifested in the kernel.
The description here is entirely for runtime.

### A. Context and Provisioning
```c

// Create the runtime context (Netlink transport)
struct p4tc_runt_ctx *ctx = p4tc_runt_ctx_create(P4TC_TML_OPS_NL);
```

### B. The Object Container
```c
// Create a container for Table operations
struct p4tc_obj *obj = p4tc_obj_create("redirect_l2", P4TC_OBJ_RUNTIME_TABLE);

// (Optional) Set a filter for the entire object
// Filters can be used as an alternative to keys with get, update, delete, and subscribe
p4tc_obj_filter_set(obj, "param.act.ingress.send_nh.port_id = \"eth0\"");
```

---

## 2. Implementation Patterns by Operation

### A. Create Table Entries (including Batching)
Stages one or more keys and actions, then sends them to the datapath in a single transaction.
```c
p4tc_obj_objname_set(obj, "ingress/nh_table");

// Entry 1
struct p4tc_key *key1 = p4tc_make_key(obj, "192.168.1.1");
struct p4tc_runt_tbl_attrs *entry1 = p4tc_alloc_tbl_entry(obj, key1, 0, P4TC_ENTITY_TC);
p4tc_create_runt_act(entry1, "ingress/send_nh", "eth0", "00:11:22:33:44:55", "66:77:88:99:AA:BB");

// Entry 2 (Batching)
struct p4tc_key *key2 = p4tc_make_key(obj, "192.168.1.2");
struct p4tc_runt_tbl_attrs *entry2 = p4tc_alloc_tbl_entry(obj, key2, 0, P4TC_ENTITY_TC);
p4tc_create_runt_act(entry2, "ingress/drop");

// Invoke and Confirm for the entire batch
if (p4tc_create(ctx, obj, P4TC_MSG_ACK, NULL, NULL) == 0) {
    p4tc_resp_handle(ctx); // Blocks for ACK
}
```

### B. Update Entries (Key or Filter)
Updates can be targeted using either a specific entry key or a filter. These methods are mutually exclusive for a single operation.

```c
// Pattern 1: Update by Key
p4tc_obj_objname_set(obj, "ingress/nh_table");
struct p4tc_key *key = p4tc_make_key(obj, "192.168.1.1");
struct p4tc_runt_tbl_attrs *entry = p4tc_alloc_tbl_entry(obj, key, 0, P4TC_ENTITY_TC);
p4tc_create_runt_act(entry, "ingress/drop");

p4tc_update(ctx, obj, P4TC_MSG_ACK, NULL, NULL);
p4tc_resp_handle(ctx);

// Pattern 2: Update by Filter (Mass Update)
struct p4tc_obj *filter_obj = p4tc_obj_create("redirect_l2", P4TC_OBJ_RUNTIME_TABLE);
p4tc_obj_objname_set(filter_obj, "ingress/nh_table");
p4tc_obj_filter_set(filter_obj, "param.act.ingress.send_nh.port_id = \"eth0\"");

// No key required when a filter is present
struct p4tc_runt_tbl_attrs *f_entry = p4tc_alloc_tbl_entry(filter_obj, NULL, 0, P4TC_ENTITY_TC);
p4tc_create_runt_act(f_entry, "ingress/drop");

p4tc_update(ctx, filter_obj, P4TC_MSG_ACK, NULL, NULL);
p4tc_resp_handle(ctx);
```

### C. Get/Read Operations
*   **Single Entry:** Use a key to fetch one specific entry.
*   **Filtered Read:** Use a filter string to fetch matching entries.
*   **Dump:** No key and no filter fetches all entries.
```c
// Pattern 1: Single Entry
p4tc_obj_objname_set(obj, "ingress/nh_table");
struct p4tc_key *key = p4tc_make_key(obj, "192.168.1.1");
p4tc_alloc_tbl_entry(obj, key, 0, P4TC_ENTITY_TC);
p4tc_get(ctx, obj, 0, my_callback, NULL);
p4tc_resp_handle(ctx);

// Pattern 2: Filtered Read
struct p4tc_obj *f_obj = p4tc_obj_create("redirect_l2", P4TC_OBJ_RUNTIME_TABLE);
p4tc_obj_objname_set(f_obj, "ingress/nh_table");
p4tc_obj_filter_set(f_obj, "prio = 0"); 
p4tc_get(ctx, f_obj, 0, my_callback, NULL);
p4tc_resp_handle(ctx);

// Pattern 3: Table Dump (No key or filter)
p4tc_obj_objname_set(obj, "ingress/nh_table");
p4tc_get(ctx, obj, 0, my_callback, NULL); // ROOT flag is automatically applied
p4tc_resp_handle(ctx);
```

### D. Delete Operations
*   **Single Entry:** Use a key to remove one specific entry.
*   **Filtered Delete:** Use a filter to remove matching entries.
*   **Flush:** No key and no filter clears the entire table.
```c
// Pattern 1: Single Delete
p4tc_obj_objname_set(obj, "ingress/nh_table");
struct p4tc_key *key = p4tc_make_key(obj, "192.168.1.1");
p4tc_alloc_tbl_entry(obj, key, 0, P4TC_ENTITY_TC);
p4tc_del(ctx, obj, P4TC_MSG_ACK, NULL, NULL);
p4tc_resp_handle(ctx);

// Pattern 2: Filtered Delete (Mass Delete)
p4tc_obj_filter_set(obj, "param.act.ingress.send_nh.port_id = \"eth1\"");
p4tc_del(ctx, obj, P4TC_MSG_ACK, NULL, NULL);
p4tc_resp_handle(ctx);

// Pattern 3: Table Flush (No key or filter)
p4tc_obj_objname_set(obj, "ingress/nh_table");
p4tc_del(ctx, obj, P4TC_MSG_ACK, NULL, NULL); // ROOT flag is automatically applied
p4tc_resp_handle(ctx);
```

### E. Subscription Management
Subscriptions can be narrowed using filters to only receive events for specific entries.
```c
// 1. Subscribe to events on nh_table matching a filter
p4tc_obj_objname_set(obj, "ingress/nh_table");
p4tc_obj_filter_set(obj, "key.srcAddr = \"192.168.1.2\"");
int sub_id = p4tc_subscribe(ctx, obj, 0, my_event_callback, NULL);

if (sub_id > 0) {
    // 2. Start background event listener
    p4tc_subscribe_resp_handle(ctx, sub_id);
    
    // ... wait or perform other work ...

    // 3. Unsubscribe when finished
    p4tc_unsubscribe(ctx, sub_id);
}
```

---

## 3. Parameter and Callback Reference

The parameters used in `p4tc_get()` and the logic in the resulting callback differ based on whether you are fetching a single entry or performing a dump.

### Common Parameters
*   **`flags`**:
    *   `0`: Used for single entry retrieval or table dumps.
*   **`cookie`**: An optional `__u64` value (or pointer cast to `__u64*`) passed to the API. It is returned unmodified to your callback, allowing you to track which request is being processed.

### The Callback Lifecycle (`p4tc_callback`)
The callback is triggered by `p4tc_resp_handle()` and receives the data retrieved from the datapath. The `trans_phase` and `p4tc_obj` parameters tell you the status of the retrieval.

#### Single Entry Pattern
When fetching one entry, the callback is usually triggered once or twice:
1.  **`P4TC_PHASE_SOT`**: The data for the requested entry is present in `p4tc_obj`.
2.  **`P4TC_PHASE_EOT`**: The transaction is complete. `p4tc_obj` is typically `NULL`.

#### Table Dump Pattern
When performing a dump, the callback is triggered for **every entry** in the table:
1.  **`P4TC_PHASE_SOT`**: Triggered for the **first entry** found. `p4tc_obj` contains this entry.
2.  **`P4TC_PHASE_MOT`**: Triggered for all **subsequent entries**. Each call contains exactly one entry in `p4tc_obj`.
3.  **`P4TC_PHASE_EOT`**: Triggered after the final entry has been delivered. `p4tc_obj` is `NULL`.
4.  **`P4TC_PHASE_ABT`**: Triggered if the dump was interrupted or failed.

### Sample Implementation
This example handles both single-entry and multi-entry results for `redirect_l2`:

```c
int my_get_callback(const struct p4tc_obj *p4tc_obj, struct p4tc_runt_ctx *ctx, 
                    __u64 *cookie, enum p4tc_trans_phase trans_phase)
{
    const char *label = (const char *)cookie;

    switch (trans_phase) {
    case P4TC_PHASE_SOT:
        printf("[%s] Beginning data reception...\n", label);
        /* Fall through: SOT also carries the first entry */
    case P4TC_PHASE_MOT:
        if (p4tc_obj) {
            // Process the entry (e.g., print it)
            printf("[%s] Entry found:\n", label);
            p4tc_obj_dump(p4tc_obj); 
        }
        break;
    case P4TC_PHASE_EOT:
        printf("[%s] All data received successfully.\n", label);
        break;
    case P4TC_PHASE_ABT:
        fprintf(stderr, "[%s] Operation failed or was aborted.\n", label);
        return -1;
    }
    return 0;
}
```
