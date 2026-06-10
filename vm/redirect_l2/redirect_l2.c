#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <p4tc_runtime_api.h>
#include <linux/p4tc.h>
#include <libgen.h>

/**
 * Structure to hold action path and its associated parameters.
 */
struct action_params {
	const char *action_path;
	const char *dev;
	const char *dmac;
	const char *smac;
};

/**
 * Generic function to create a table entry.
 */
static int p4tc_create_entry(struct p4tc_runt_ctx *runt_ctx,
			     const char *pipe_name, const char *table_path,
			     const char *key_str,
			     const struct action_params *act)
{
	struct p4tc_runt_tbl_attrs *entry_attrs = NULL;
	struct p4tc_runt_act_attrs *act_attrs = NULL;
	struct p4tc_obj *runtime_obj = NULL;
	struct p4tc_key *tbl_key = NULL;
	int ret = -1;

	runtime_obj = p4tc_obj_create(pipe_name, P4TC_OBJ_RUNTIME_TABLE);
	if (!runtime_obj)
		return -1;

	p4tc_obj_objname_set(runtime_obj, table_path);
	tbl_key = p4tc_make_key(runtime_obj, 1, (const char *[]){ key_str });
	if (!tbl_key) {
		fprintf(stderr, "Error: Could not create key for %s in %s\n",
			key_str, table_path);
		goto obj_cleanup;
	}
	entry_attrs = p4tc_alloc_tbl_entry(runtime_obj,
					   tbl_key, 0, P4TC_ENTITY_TC);
	if (!entry_attrs) {
		p4tc_key_destroy(tbl_key);
		goto obj_cleanup;
	}

	act_attrs = p4tc_create_runt_act(entry_attrs, act->action_path, 3,
					 (const char *[]){ act->dev, act->dmac, act->smac });
	if (!act_attrs) {
		fprintf(stderr, "Error: Could not create action %s\n",
			act->action_path);
		goto obj_cleanup;
	}

	printf("Action: Creating entry in %s/%s [Key: %s]\n", pipe_name,
	       table_path, key_str);
	ret = p4tc_create(runt_ctx, runtime_obj, 0, NULL, NULL);

obj_cleanup:
	p4tc_obj_destroy(runtime_obj);
	return ret;
}

/**
 * Generic function to update an existing table entry.
 */
static int p4tc_update_entry(struct p4tc_runt_ctx *runt_ctx, const char *pipe_name,
		      const char *table_path, const char *key_str,
		      const struct action_params *act)
{
	struct p4tc_obj *runtime_obj = NULL;
	struct p4tc_key *tbl_key = NULL;
	struct p4tc_runt_tbl_attrs *entry_attrs = NULL;
	struct p4tc_runt_act_attrs *act_attrs = NULL;
	int ret = -1;

	runtime_obj = p4tc_obj_create(pipe_name, P4TC_OBJ_RUNTIME_TABLE);
	if (!runtime_obj)
		return -1;

	p4tc_obj_objname_set(runtime_obj, table_path);
	tbl_key = p4tc_make_key(runtime_obj, 1, (const char *[]){ key_str });
	if (!tbl_key) {
		fprintf(stderr, "Error: Could not create key for %s in %s\n",
			key_str, table_path);
		goto obj_cleanup;
	}
entry_attrs = p4tc_alloc_tbl_entry(runtime_obj,
						  tbl_key, 0, P4TC_ENTITY_TC);
	if (!entry_attrs) {
		p4tc_key_destroy(tbl_key);
		goto obj_cleanup;
	}

	act_attrs = p4tc_create_runt_act(entry_attrs, act->action_path, 3,
					 (const char *[]){ act->dev, act->dmac, act->smac });
	if (!act_attrs) {
		fprintf(stderr, "Error: Could not create action %s\n",
			act->action_path);
		goto obj_cleanup;
	}

	printf("Action: Updating entry in %s/%s [Key: %s]\n", pipe_name,
	       table_path, key_str);
	ret = p4tc_update(runt_ctx, runtime_obj, 0, NULL, NULL);

obj_cleanup:
	p4tc_obj_destroy(runtime_obj);
	return ret;
}

/**
 * Generic function to read table data.
 */
static int p4tc_read_table(struct p4tc_runt_ctx *runt_ctx,
			   const char *pipe_name, const char *table_path,
			   const char *key_str)
{
	struct p4tc_runt_tbl_attrs *entry_attrs = NULL;
	struct p4tc_obj *runtime_obj = NULL;
	struct p4tc_key *tbl_key = NULL;
	int ret = -1;

	runtime_obj = p4tc_obj_create(pipe_name, P4TC_OBJ_RUNTIME_TABLE);
	if (!runtime_obj)
		return -1;

	if (key_str) {
		p4tc_obj_objname_set(runtime_obj, table_path);
		tbl_key = p4tc_make_key(runtime_obj, 1, (const char *[]){ key_str });
		if (!tbl_key)
			goto obj_cleanup;

		entry_attrs = p4tc_alloc_tbl_entry(runtime_obj,
								  tbl_key, 0, P4TC_ENTITY_TC);
		if (!entry_attrs) {
			p4tc_key_destroy(tbl_key);
			goto obj_cleanup;
		}
		printf("\n--- Reading Entry: %s in %s ---\n", key_str,
		       table_path);
	} else {
		p4tc_obj_objname_set(runtime_obj, table_path);
		printf("\n--- Dumping Table: %s ---\n", table_path);
	}

	ret = p4tc_get(runt_ctx, runtime_obj, 0, NULL, NULL);

obj_cleanup:
	p4tc_obj_destroy(runtime_obj);
	return ret;
}

/**
 * Generic function to delete table data.
 */
static int p4tc_delete_table(struct p4tc_runt_ctx *runt_ctx,
			     const char *pipe_name, const char *table_path,
			     const char *key_str)
{
	struct p4tc_obj *runtime_obj = NULL;
	struct p4tc_key *tbl_key = NULL;
	struct p4tc_runt_tbl_attrs *entry_attrs = NULL;
	uint32_t flags = 0;
	int ret = -1;

	runtime_obj = p4tc_obj_create(pipe_name, P4TC_OBJ_RUNTIME_TABLE);
	if (!runtime_obj)
		return -1;

	if (key_str) {
		p4tc_obj_objname_set(runtime_obj, table_path);
		tbl_key = p4tc_make_key(runtime_obj, 1, (const char *[]){ key_str });
		if (!tbl_key)
			goto obj_cleanup;

		entry_attrs = p4tc_alloc_tbl_entry(runtime_obj,
								  tbl_key, 0, P4TC_ENTITY_TC);
		if (!entry_attrs) {
			p4tc_key_destroy(tbl_key);
			goto obj_cleanup;
		}
		printf("Action: Deleting entry [%s] from %s\n", key_str,
		       table_path);
	} else {
		p4tc_obj_objname_set(runtime_obj, table_path);
		flags = 0;
		printf("Action: Flushing entire table %s\n", table_path);
	}

	ret = p4tc_del(runt_ctx, runtime_obj, flags, NULL, NULL);

obj_cleanup:
	p4tc_obj_destroy(runtime_obj);
	return ret;
}

#define PNAME "redirect_l2"

int main(int argc, char **argv)
{
	const char *pname = "redirect_l2";
	const char *tname = "ingress/nh_table";
	const char *aname = "ingress/send_nh";
	struct p4tc_runt_ctx *runt_ctx = NULL;
	struct p4tc_pipe_config *config_info;

	struct {
		const char *ip; const char *dev; const char *dmac;
		const char *smac;
	} entries[] = {
		{"192.168.1.10", "port0", "00:AA:BB:CC:DD:EE",
		 "00:11:22:33:44:55"},
		{"10.0.0.5",     "port0", "00:22:33:44:55:66",
		 "00:AA:BB:CC:DD:FF"},
		{"172.16.0.100", "port0", "00:DE:AD:BE:EF:00",
		 "00:CA:FE:BA:BE:01"}
	};

	printf("P4TC Generic API - Basic CRUD Demonstration\n");
	printf("===========================================\n");

	/* manifest the program pname into the datapath. NULL for the template
	   path implies the template is in the current working dir.
	   */
	config_info = p4tc_provision(PNAME, NULL);
	if (!config_info) {
		fprintf(stderr, "failed to provision %s\n", pname);
		return -1;
	}

	runt_ctx = p4tc_runt_ctx_create(P4TC_TML_OPS_NL);
	if (!runt_ctx)
		return -1;

	/* 1. CREATE: 3 Entries */
	for (int i = 0; i < 3; i++) {
		struct action_params act = {
			.action_path = aname,
			.dev = entries[i].dev,
			.dmac = entries[i].dmac,
			.smac = entries[i].smac
		};
		p4tc_create_entry(runt_ctx, pname, tname, entries[i].ip, &act);
	}

	/* 2. READ: Initial state of 10.0.0.5 */
	printf("\n>>> Targeted Read (Before Update): 10.0.0.5 <<<\n");
	p4tc_read_table(runt_ctx, pname, tname, "10.0.0.5");

	/* 3. UPDATE: Change action parameters for 10.0.0.5 */
	printf("\n>>> Targeted Update: 10.0.0.5 <<<\n");
	struct action_params updated_act = {
		.action_path = aname,
		.dev = "port0",
		.dmac = "FF:FF:FF:FF:FF:FF",
		.smac = "00:00:00:00:00:00"
	};
	p4tc_update_entry(runt_ctx, pname, tname, "10.0.0.5", &updated_act);

	/* 4. READ: Verify Update for 10.0.0.5 */
	printf("\n>>> Targeted Read (After Update): 10.0.0.5 <<<\n");
	p4tc_read_table(runt_ctx, pname, tname, "10.0.0.5");

	/* 5. DELETE: Targeted Delete */
	printf("\n>>> Targeted Delete: 172.16.0.100 <<<\n");
	p4tc_delete_table(runt_ctx, pname, tname, "172.16.0.100");

	/* 6. READ: Full Dump */
	p4tc_read_table(runt_ctx, pname, tname, NULL);

	/* 7. CLEANUP: Flush Table */
	p4tc_delete_table(runt_ctx, pname, tname, NULL);

	/* 8. FINAL CHECK: Should be empty */
	p4tc_read_table(runt_ctx, pname, tname, NULL);

	/* Destroy the runtime context at the end */
	p4tc_runt_ctx_destroy(runt_ctx);
	p4tc_pipe_config_destroy(config_info);

	return 0;
}
