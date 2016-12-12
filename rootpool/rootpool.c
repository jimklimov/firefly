/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 *
 * Copyright 2015 Nexenta Systems, Inc.  All rights reserved.
 *
 * This hook tries disks for the pool with bootfs property and print
 * its name, guid, state and bootfs:
 * rpool;9213225939923529766;ONLINE;rpool/ROOT/illumos
 */

#include <libzfs.h>

int
main(int argc, char **argv)
{
	char **searchdirs = NULL;
	nvlist_t *pools = NULL;
	nvpair_t *elem;
	nvlist_t *config;
	uint64_t searchguid = 0;
	char *searchname = NULL;
	char *cachefile = NULL;
	importargs_t pdata = { 0 };
	libzfs_handle_t *g_zfs;
	vdev_stat_t *vs;
	char *name;
	uint64_t guid;
	nvlist_t *nvroot;
	const char *health;
	uint_t vsc;
	char *bootfs;
	size_t count_hit_candidates = 0;
	size_t count_hit_outputs = 0;

	if ((g_zfs = libzfs_init()) == NULL) {
		(void) fprintf(stderr, "FATAL: rootpool helper: Could not libzfs_init()\n");
		return (1);
	}

	if ((searchdirs = calloc(2, sizeof (char *))) == NULL) {
		(void) fprintf(stderr, "FATAL: rootpool helper: Could not allocate searchdirs\n");
		return (1);
	}

	searchdirs[0] = "/dev/dsk";
	searchdirs[1] = NULL; // sentry
	pdata.path = searchdirs;
	pdata.paths = 1;
	pdata.poolname = searchname;
	pdata.guid = searchguid;
	pdata.cachefile = cachefile;

	pools = zpool_search_import(g_zfs, &pdata);
	if (pools == NULL) {
		(void) fprintf(stderr, "FATAL: rootpool helper: Could not zpool_search_import(): returned NULL\n");
		free(searchdirs);
		return (1);
	}

	elem = NULL;
	while ((elem = nvlist_next_nvpair(pools, elem)) != NULL) {
		count_hit_candidates++;
		verify(nvpair_value_nvlist(elem, &config) == 0);
		verify(nvlist_lookup_string(config, ZPOOL_CONFIG_POOL_NAME,
		    &name) == 0);
		verify(nvlist_lookup_uint64(config, ZPOOL_CONFIG_POOL_GUID,
		    &guid) == 0);
		verify(nvlist_lookup_nvlist(config, ZPOOL_CONFIG_VDEV_TREE,
		    &nvroot) == 0);
		verify(nvlist_lookup_uint64_array(nvroot,
		    ZPOOL_CONFIG_VDEV_STATS,
		    (uint64_t **)&vs, &vsc) == 0);
		if (nvlist_lookup_string(config, ZPOOL_CONFIG_BOOTFS,
		    &bootfs) != 0)
			continue;

		health = zpool_state_to_name(vs->vs_state, vs->vs_aux);
		(void) printf("%s;%" PRIu64 ";%s;%s\n", name,
		    (u_longlong_t)guid, health, bootfs);
		count_hit_outputs++;
	}

	if (count_hit_candidates == 0) {
		(void) fprintf(stderr, "WARNING: rootpool helper: Did not locate any disks with a pool via zpool_search_import()\n");
	}

	if (count_hit_outputs == 0) {
		(void) fprintf(stderr, "WARNING: rootpool helper: Did not locate any pools via zpool_search_import() with a bootfs property\n");
	}

	nvlist_free(pools);
	free(searchdirs);
	return (0);
}
