/*
  Copyright (c) 2010 Gluster, Inc. <http://www.gluster.com>
  This file is part of GlusterFS.

  GlusterFS is GF_FREE software; you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation; either version 3 of the License,
  or (at your option) any later version.

  GlusterFS is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/


#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <fnmatch.h>

#include "xlator.h"
#include "glusterd.h"
#include "defaults.h"
#include "logging.h"
#include "dict.h"
#include "graph-utils.h"
#include "glusterd-mem-types.h"
#include "cli1.h"
#include "glusterd-volgen.h"


/* dispatch table for VOLUME SET
 * -----------------------------
 *
 * Format of entries:
 *
 * First field is the <key>, for the purpose of looking it up
 * in volume dictionary. Each <key> is of the format "<domain>.<specifier>".
 *
 * Second field is <voltype>.
 *
 * Third field is <option>, if its unset, it's assumed to be
 * the same as <specifier>.
 *
 * Fourth field is <value>. In this context they are used to specify
 * a default. That is, even the volume dict doesn't have a value,
 * we procced as if the default value were set for it.
 *
 * There are two type of entries: basic and special.
 *
 * - Basic entries are the ones where the <option> does _not_ start with
 *   the bang! character ('!').
 *
 *   In their case, <option> is understood as an option for an xlator of
 *   type <voltype>. Their effect is to copy over the volinfo->dict[<key>]
 *   value to all graph nodes of type <voltype> (if such a value is set).
 *
 *   You are free to add entries of this type, they will become functional
 *   just by being present in the table.
 *
 * - Special entries where the <option> starts with the bang!.
 *
 *   They are not applied to all graphs during generation, and you cannot
 *   extend them in a trivial way which could be just picked up. Better
 *   not touch them unless you know what you do.
 *
 * "NODOC" entries are not part of the public interface and are subject
 * to change at any time.
 */

struct volopt_map_entry {
        char *key;
        char *voltype;
        char *option;
        char *value;
};

static struct volopt_map_entry glusterd_volopt_map[] = {
        {"cluster.lookup-unhashed",              "cluster/distribute",        }, /* NODOC */
        {"cluster.min-free-disk",                "cluster/distribute",        }, /* NODOC */

        {"cluster.entry-change-log",             "cluster/replicate",         }, /* NODOC */
        {"cluster.read-subvolume",               "cluster/replicate",         }, /* NODOC */
        {"cluster.background-self-heal-count",   "cluster/replicate",         }, /* NODOC */
        {"cluster.metadata-self-heal",           "cluster/replicate",         }, /* NODOC */
        {"cluster.data-self-heal",               "cluster/replicate",         }, /* NODOC */
        {"cluster.entry-self-heal",              "cluster/replicate",         }, /* NODOC */
        {"cluster.strict-readdir",               "cluster/replicate",         }, /* NODOC */
        {"cluster.self-heal-window-size",        "cluster/replicate",         "data-self-heal-window-size",},
        {"cluster.data-change-log",              "cluster/replicate",         }, /* NODOC */
        {"cluster.metadata-change-log",          "cluster/replicate",         }, /* NODOC */

        {"cluster.stripe-block-size",            "cluster/stripe",            "block-size",},

        {"diagnostics.latency-measurement",      "debug/io-stats",            },
        {"diagnostics.dump-fd-stats",            "debug/io-stats",            },
        {"diagnostics.brick-log-level",          "debug/io-stats",            "!log-level",},
        {"diagnostics.client-log-level",         "debug/io-stats",            "!log-level",},

        {"performance.cache-max-file-size",      "performance/io-cache",      "max-file-size",},
        {"performance.cache-min-file-size",      "performance/io-cache",      "min-file-size",},
        {"performance.cache-refresh-timeout",    "performance/io-cache",      "cache-timeout",},
        {"performance.cache-priority",           "performance/io-cache",      "priority",}, /* NODOC */
        {"performance.cache-size",               "performance/io-cache",      },
        {"performance.cache-size",               "performance/quick-read",    },
        {"performance.flush-behind",             "performance/write-behind",      "flush-behind",},

        {"performance.io-thread-count",          "performance/io-threads",    "thread-count",},

        {"performance.disk-usage-limit",         "performance/quota",         }, /* NODOC */
        {"performance.min-free-disk-limit",      "performance/quota",         }, /* NODOC */

        {"performance.write-behind-window-size", "performance/write-behind",  "cache-size",},

        {"network.frame-timeout",                "protocol/client",           },
        {"network.ping-timeout",                 "protocol/client",           },
        {"network.inode-lru-limit",              "protocol/server",           }, /* NODOC */

        {"auth.allow",                           "protocol/server",           "!server-auth", "*"},
        {"auth.reject",                          "protocol/server",           "!server-auth",},

        {"transport.keepalive",                   "protocol/server",           "transport.socket.keepalive",},

        {"performance.write-behind",             "performance/write-behind",  "!perf", "on"}, /* NODOC */
        {"performance.read-ahead",               "performance/read-ahead",    "!perf", "on"}, /* NODOC */
        {"performance.io-cache",                 "performance/io-cache",      "!perf", "on"}, /* NODOC */
        {"performance.quick-read",               "performance/quick-read",    "!perf", "on"}, /* NODOC */
        {"performance.stat-prefetch",            "performance/stat-prefetch", "!perf",},      /* NODOC */

        {NULL,                                                                }
};


#define VOLGEN_GET_NFS_DIR(path)                                        \
        do {                                                            \
                glusterd_conf_t *priv = THIS->private;                  \
                snprintf (path, PATH_MAX, "%s/nfs", priv->workdir);     \
        } while (0);                                                    \

#define VOLGEN_GET_VOLUME_DIR(path, volinfo)                            \
        do {                                                            \
                glusterd_conf_t *priv = THIS->private;                  \
                snprintf (path, PATH_MAX, "%s/vols/%s", priv->workdir,  \
                          volinfo->volname);                            \
        } while (0);                                                    \




/*********************************************
 *
 * xlator generation / graph manipulation API
 *
 *********************************************/



static xlator_t *
xlator_instantiate_va (const char *type, const char *format, va_list arg)
{
        xlator_t *xl = NULL;
        char *volname = NULL;
        int ret = 0;

        ret = gf_vasprintf (&volname, format, arg);
        if (ret < 0) {
                volname = NULL;

                goto error;
        }

        xl = GF_CALLOC (1, sizeof (*xl), gf_common_mt_xlator_t);
        if (!xl)
                goto error;
        ret = xlator_set_type_virtual (xl, type);
        if (ret)
                goto error;
        xl->options = get_new_dict();
        if (!xl->options)
                goto error;
        xl->name = volname;
        INIT_LIST_HEAD (&xl->volume_options);

        return xl;

 error:
        gf_log ("", GF_LOG_ERROR, "creating xlator of type %s failed",
                type);
        if (volname)
                GF_FREE (volname);
        if (xl)
                xlator_destroy (xl);

        return NULL;
}

#ifdef __not_used_as_of_now_
static xlator_t *
xlator_instantiate (const char *type, const char *format, ...)
{
        va_list arg;
        xlator_t *xl;

        va_start (arg, format);
        xl = xlator_instantiate_va (type, format, arg);
        va_end (arg);

        return xl;
}
#endif

static int
volgen_xlator_link (xlator_t *pxl, xlator_t *cxl)
{
        int ret = 0;

        ret = glusterfs_xlator_link (pxl, cxl);
        if (ret == -1) {
                gf_log ("", GF_LOG_ERROR,
                        "Out of memory, cannot link xlators %s <- %s",
                        pxl->name, cxl->name);
        }

        return ret;
}

static int
volgen_graph_link (glusterfs_graph_t *graph, xlator_t *xl)
{
        int ret = 0;

        /* no need to care about graph->top here */
        if (graph->first)
                ret = volgen_xlator_link (xl, graph->first);
        if (ret == -1) {
                gf_log ("", GF_LOG_ERROR, "failed to add graph entry %s",
                        xl->name);

                return -1;
        }

        return 0;
}

static xlator_t *
volgen_graph_add_as (glusterfs_graph_t *graph, const char *type,
                     const char *format, ...)
{
        va_list arg;
        xlator_t *xl = NULL;

        va_start (arg, format);
        xl = xlator_instantiate_va (type, format, arg);
        va_end (arg);

        if (!xl)
                return NULL;

        if (volgen_graph_link (graph, xl)) {
                xlator_destroy (xl);

                return NULL;
        } else
                glusterfs_graph_set_first (graph, xl);

        return xl;
}

static xlator_t *
volgen_graph_add_nolink (glusterfs_graph_t *graph, const char *type,
                         const char *format, ...)
{
        va_list arg;
        xlator_t *xl = NULL;

        va_start (arg, format);
        xl = xlator_instantiate_va (type, format, arg);
        va_end (arg);

        if (!xl)
                return NULL;

        glusterfs_graph_set_first (graph, xl);

        return xl;
}

static xlator_t *
volgen_graph_add (glusterfs_graph_t *graph, char *type, char *volname)
{
        char *shorttype = NULL;

        shorttype = strrchr (type, '/');
        GF_ASSERT (shorttype);
        shorttype++;
        GF_ASSERT (*shorttype);

        return volgen_graph_add_as (graph, type, "%s-%s", volname, shorttype);
}

/* XXX Seems there is no such generic routine?
 * Maybe should put to xlator.c ??
 */
static int
xlator_set_option (xlator_t *xl, char *key, char *value)
{
        char *dval     = NULL;

        dval = gf_strdup (value);
        if (!dval) {
                gf_log ("", GF_LOG_ERROR,
                        "failed to set xlator opt: %s[%s] = %s",
                        xl->name, key, value);

                return -1;
        }

        return dict_set_dynstr (xl->options, key, dval);
}

static inline xlator_t *
first_of (glusterfs_graph_t *graph)
{
        return (xlator_t *)graph->first;
}




/**************************
 *
 * Volume generation engine
 *
 **************************/


typedef int (*volgen_opthandler_t) (glusterfs_graph_t *graph,
                                    struct volopt_map_entry *vme,
                                    void *param);

struct opthandler_data {
        glusterfs_graph_t *graph;
        volgen_opthandler_t handler;
        struct volopt_map_entry *vme;
        gf_boolean_t found;
        gf_boolean_t data_t_fake;
        int rv;
        void *param;
};

#define pattern_match_options 0


static void
process_option (dict_t *dict, char *key, data_t *value, void *param)
{
        struct opthandler_data *odt = param;
        struct volopt_map_entry vme = {0,};

        if (odt->rv)
                return;
#if pattern_match_options
        if (fnmatch (odt->vme->key, key, 0) != 0)
                return;
#endif
        odt->found = _gf_true;

        vme.key = key;
        vme.voltype = odt->vme->voltype;
        vme.option = odt->vme->option;
        if (!vme.option) {
                vme.option = strrchr (key, '.');
                if (vme.option)
                        vme.option++;
                else
                        vme.option = key;
        }
        if (odt->data_t_fake)
                vme.value = (char *)value;
        else
                vme.value = value->data;

        odt->rv = odt->handler (odt->graph, &vme, odt->param);
}

static int
volgen_graph_set_options_generic (glusterfs_graph_t *graph, dict_t *dict,
                                  void *param, volgen_opthandler_t handler)
{
        struct volopt_map_entry *vme = NULL;
        struct opthandler_data odt = {0,};
        data_t *data = NULL;

        odt.graph = graph;
        odt.handler = handler;
        odt.param = param;
        (void)data;

        for (vme = glusterd_volopt_map; vme->key; vme++) {
                odt.vme = vme;
                odt.found = _gf_false;
                odt.data_t_fake = _gf_false;

#if pattern_match_options
                dict_foreach (dict, process_option, &odt);
#else
                data = dict_get (dict, vme->key);

                if (data)
                        process_option (dict, vme->key, data, &odt);
#endif
                if (odt.rv)
                        return odt.rv;

                if (odt.found)
                        continue;

                /* check for default value */

                if (vme->value) {
                        /* stupid hack to be able to reuse dict iterator
                         * in this context
                         */
                        odt.data_t_fake = _gf_true;
                        process_option (NULL, vme->key, (data_t *)vme->value,
                                        &odt);
                        if (odt.rv)
                                return odt.rv;
                }
        }

        return 0;
}

static int
basic_option_handler (glusterfs_graph_t *graph, struct volopt_map_entry *vme,
                      void *param)
{
        xlator_t *trav;
        int ret = 0;

        if (vme->option[0] == '!')
                return 0;

        for (trav = first_of (graph); trav; trav = trav->next) {
                if (strcmp (trav->type, vme->voltype) != 0)
                        continue;

                ret = xlator_set_option (trav, vme->option, vme->value);
                if (ret)
                        return -1;
        }

        return 0;
}

static int
volgen_graph_set_options (glusterfs_graph_t *graph, dict_t *dict)
{
        return volgen_graph_set_options_generic (graph, dict, NULL,
                                                 &basic_option_handler);
}

static int
optget_option_handler (glusterfs_graph_t *graph, struct volopt_map_entry *vme,
                       void *param)
{
        struct volopt_map_entry *vme2 = param;

        if (strcmp (vme->key, vme2->key) == 0)
                vme2->value = vme->value;

        return 0;
}

/* This getter considers defaults also. */
int
glusterd_volinfo_get (glusterd_volinfo_t *volinfo, char *key, char **value)
{
        struct volopt_map_entry vme = {0,};
        int ret = 0;

        vme.key = key;

        ret = volgen_graph_set_options_generic (NULL, volinfo->dict, &vme,
                                                &optget_option_handler);
        if (ret) {
                gf_log ("", GF_LOG_ERROR, "Out of memory");

                return -1;
        }

        *value = vme.value;

        return 0;
}

static char *
option_complete (char *key)
{
        struct volopt_map_entry *vme = NULL;
        char *completion = NULL;

        for (vme = glusterd_volopt_map; vme->key; vme++) {
                if (strcmp (strchr (vme->key, '.') + 1, key) != 0)
                        continue;

                if (completion)
                        return NULL;
                else
                        completion = vme->key;
        }

        return completion;
}

int
glusterd_check_option_exists (char *key, char **completion)
{
        dict_t *dict = NULL;
        struct volopt_map_entry vme = {0,};
        struct volopt_map_entry *vmep = NULL;
        int ret = 0;

        (void)vme;
        (void)vmep;
        (void)dict;

        if (!strchr (key, '.')) {
                if (completion) {
                        *completion = option_complete (key);

                        return !!*completion;
                } else
                        return 0;
        }

#if !pattern_match_options
        for (vmep = glusterd_volopt_map; vmep->key; vmep++) {
                if (strcmp (vmep->key, key) == 0) {
                        ret = 1;
                        break;
                }
        }

        return ret;
#else
        vme.key = key;

        /* We are getting a bit anal here to avoid typing
         * fnmatch one more time. Orthogonality foremost!
         * The internal logic of looking up in the volopt_map table
         * should be coded exactly once.
         *
         * [[Ha-ha-ha, so now if I ever change the internals then I'll
         * have to update the fnmatch in this comment also :P ]]
         */
        dict = get_new_dict ();
        if (!dict || dict_set_str (dict, key, ""))
                goto oom;

        ret = volgen_graph_set_options_generic (NULL, dict, &vme,
                                                &optget_option_handler);
        dict_destroy (dict);
        if (ret)
                goto oom;

        return !!vme.value;

 oom:
        gf_log ("", GF_LOG_ERROR, "Out of memory");

        return -1;
#endif
}

static int
volgen_graph_merge_sub (glusterfs_graph_t *dgraph, glusterfs_graph_t *sgraph)
{
        xlator_t *trav = NULL;

        GF_ASSERT (dgraph->first);

        if (volgen_xlator_link (first_of (dgraph), first_of (sgraph)) == -1)
                return -1;

        for (trav = first_of (dgraph); trav->next; trav = trav->next);

        trav->next = sgraph->first;
        trav->next->prev = trav;
        dgraph->xl_count += sgraph->xl_count;

        return 0;
}

static int
volgen_write_volfile (glusterfs_graph_t *graph, char *filename)
{
        char *ftmp = NULL;
        FILE *f = NULL;

        if (gf_asprintf (&ftmp, "%s.tmp", filename) == -1) {
                ftmp = NULL;

                goto error;
        }

        f = fopen (ftmp, "w");
        if (!f)
                goto error;

        if (glusterfs_graph_print_file (f, graph) == -1)
                goto error;

        if (fclose (f) == -1)
                goto error;

        if (rename (ftmp, filename) == -1)
                goto error;

        GF_FREE (ftmp);

        return 0;

 error:

        if (ftmp)
                GF_FREE (ftmp);
        if (f)
                fclose (f);

        gf_log ("", GF_LOG_ERROR, "failed to create volfile %s", filename);

        return -1;
}

static void
volgen_graph_free (glusterfs_graph_t *graph)
{
        xlator_t *trav = NULL;
        xlator_t *trav_old = NULL;

        for (trav = first_of (graph) ;; trav = trav->next) {
                if (trav_old)
                        xlator_destroy (trav_old);

                trav_old = trav;

                if (!trav)
                        break;
        }
}

static int
build_graph_generic (glusterfs_graph_t *graph, glusterd_volinfo_t *volinfo,
                     dict_t *mod_dict, void *param,
                     int (*builder) (glusterfs_graph_t *graph,
                                     glusterd_volinfo_t *volinfo,
                                     dict_t *set_dict, void *param))
{
        dict_t *set_dict = NULL;
        int ret = 0;

        if (mod_dict) {
                set_dict = dict_copy (volinfo->dict, NULL);
                if (!set_dict)
                        return -1;
                dict_copy (mod_dict, set_dict);
                /* XXX dict_copy swallows errors */
        } else
                set_dict = volinfo->dict;

        ret = builder (graph, volinfo, set_dict, param);
        if (!ret)
                ret = volgen_graph_set_options (graph, set_dict);

        if (mod_dict)
                dict_destroy (set_dict);

        return ret;
}

static void
get_vol_transport_type (glusterd_volinfo_t *volinfo, char *tt)
{
        volinfo->transport_type == GF_TRANSPORT_RDMA ?
        strcpy (tt, "rdma"):
        strcpy (tt, "tcp");
}

static int
server_auth_option_handler (glusterfs_graph_t *graph,
                            struct volopt_map_entry *vme, void *param)
{
        xlator_t *xl = NULL;
        xlator_list_t *trav = NULL;
        char *aa = NULL;
        int   ret   = 0;
        char *key = NULL;

        if (strcmp (vme->option, "!server-auth") != 0)
                return 0;

        xl = first_of (graph);

        /* from 'auth.allow' -> 'allow', and 'auth.reject' -> 'reject' */
        key = strchr (vme->key, '.') + 1;

        for (trav = xl->children; trav; trav = trav->next) {
                ret = gf_asprintf (&aa, "auth.addr.%s.%s", trav->xlator->name,
                                   key);
                if (ret != -1) {
                        ret = xlator_set_option (xl, aa, vme->value);
                        GF_FREE (aa);
                }
                if (ret)
                        return -1;
        }

        return 0;
}

static int
loglevel_option_handler (glusterfs_graph_t *graph,
                         struct volopt_map_entry *vme, void *param)
{
        char *role = param;
        struct volopt_map_entry vme2 = {0,};

        if (strcmp (vme->option, "!log-level") != 0 ||
            !strstr (vme->key, role))
                return 0;

        if (glusterd_check_log_level(vme->value) == -1)
                return -1;
        memcpy (&vme2, vme, sizeof (vme2));
        vme2.option = "log-level";

        return basic_option_handler (graph, &vme2, NULL);
}

static int
server_spec_option_handler (glusterfs_graph_t *graph,
                            struct volopt_map_entry *vme, void *param)
{
        int ret = 0;

        ret = server_auth_option_handler (graph, vme, NULL);
        if (!ret)
                ret = loglevel_option_handler (graph, vme, "brick");

        return ret;
}

static int
server_graph_builder (glusterfs_graph_t *graph, glusterd_volinfo_t *volinfo,
                      dict_t *set_dict, void *param)
{
        char     *volname = NULL;
        char     *path = NULL;
        int       pump = 0;
        xlator_t *xl = NULL;
        xlator_t *txl = NULL;
        xlator_t *rbxl = NULL;
        int       ret = 0;
        char      transt[16] = {0,};

        path = param;
        volname = volinfo->volname;
        get_vol_transport_type (volinfo, transt);

        xl = volgen_graph_add (graph, "storage/posix", volname);
        if (!xl)
                return -1;

        ret = xlator_set_option (xl, "directory", path);
        if (ret)
                return -1;

        xl = volgen_graph_add (graph, "features/access-control", volname);
        if (!xl)
                return -1;

        xl = volgen_graph_add (graph, "features/locks", volname);
        if (!xl)
                return -1;

        ret = dict_get_int32 (volinfo->dict, "enable-pump", &pump);
        if (ret == -ENOENT)
                ret = pump = 0;
        if (ret)
                return -1;
        if (pump) {
                txl = first_of (graph);

                rbxl = volgen_graph_add_nolink (graph, "protocol/client",
                                                "%s-replace-brick", volname);
                if (!rbxl)
                        return -1;
                ret = xlator_set_option (rbxl, "transport-type", transt);
                if (ret)
                        return -1;
                xl = volgen_graph_add_nolink (graph, "cluster/pump", "%s-pump",
                                              volname);
                if (!xl)
                        return -1;
                ret = volgen_xlator_link (xl, txl);
                if (ret)
                        return -1;
                ret = volgen_xlator_link (xl, rbxl);
                if (ret)
                        return -1;
        }

        xl = volgen_graph_add (graph, "performance/io-threads", volname);
        if (!xl)
                return -1;
        ret = xlator_set_option (xl, "thread-count", "16");
        if (ret)
                return -1;

        xl = volgen_graph_add_as (graph, "debug/io-stats", path);
        if (!xl)
                return -1;

        xl = volgen_graph_add (graph, "protocol/server", volname);
        if (!xl)
                return -1;
        ret = xlator_set_option (xl, "transport-type", transt);
        if (ret)
                return -1;

        ret = volgen_graph_set_options_generic (graph, set_dict, NULL,
                                                &server_spec_option_handler);

        return ret;
}


/* builds a graph for server role , with option overrides in mod_dict */
static int
build_server_graph (glusterfs_graph_t *graph, glusterd_volinfo_t *volinfo,
                    dict_t *mod_dict, char *path)
{
        return build_graph_generic (graph, volinfo, mod_dict, path,
                                    &server_graph_builder);
}

static int
perfxl_option_handler (glusterfs_graph_t *graph, struct volopt_map_entry *vme,
                       void *param)
{
        char *volname = NULL;
        gf_boolean_t enabled = _gf_false;

        volname = param;

        if (strcmp (vme->option, "!perf") != 0)
                return 0;

        if (gf_string2boolean (vme->value, &enabled) == -1)
                return -1;
        if (!enabled)
                return 0;

        if (volgen_graph_add (graph, vme->voltype, volname))
                return 0;
        else
                return -1;
}

static int
client_graph_builder (glusterfs_graph_t *graph, glusterd_volinfo_t *volinfo,
                      dict_t *set_dict, void *param)
{
        int                      replicate_count    = 0;
        int                      stripe_count       = 0;
        int                      dist_count         = 0;
        int                      num_bricks         = 0;
        char                     transt[16]         = {0,};
        int                      cluster_count      = 0;
        char                    *volname            = NULL;
        dict_t                  *dict               = NULL;
        glusterd_brickinfo_t    *brick = NULL;
        char                    *replicate_args[]   = {"cluster/replicate",
                                                       "%s-replicate-%d"};
        char                    *stripe_args[]      = {"cluster/stripe",
                                                       "%s-stripe-%d"};
        char                   **cluster_args       = NULL;
        int                      i                  = 0;
        int                      j                  = 0;
        int                      ret                = 0;
        xlator_t                *xl                 = NULL;
        xlator_t                *txl                = NULL;
        xlator_t                *trav               = NULL;

        volname = volinfo->volname;
        dict    = volinfo->dict;
        GF_ASSERT (dict);
        get_vol_transport_type (volinfo, transt);

        list_for_each_entry (brick, &volinfo->bricks, brick_list)
                num_bricks++;

        if (GF_CLUSTER_TYPE_REPLICATE == volinfo->type) {
                if (volinfo->brick_count <= volinfo->sub_count) {
                        gf_log ("", GF_LOG_DEBUG,
                                "Volfile is plain replicated");
                        replicate_count = volinfo->sub_count;
                        dist_count = num_bricks / replicate_count;
                        if (!dist_count) {
                                replicate_count = num_bricks;
                                dist_count = num_bricks / replicate_count;
                        }
                } else {
                        gf_log ("", GF_LOG_DEBUG,
                                "Volfile is distributed-replicated");
                        replicate_count = volinfo->sub_count;
                        dist_count = num_bricks / replicate_count;
                }

        } else if (GF_CLUSTER_TYPE_STRIPE == volinfo->type) {
                if (volinfo->brick_count == volinfo->sub_count) {
                        gf_log ("", GF_LOG_DEBUG,
                                "Volfile is plain striped");
                        stripe_count = volinfo->sub_count;
                        dist_count = num_bricks / stripe_count;
                } else {
                        gf_log ("", GF_LOG_DEBUG,
                                "Volfile is distributed-striped");
                        stripe_count = volinfo->sub_count;
                        dist_count = num_bricks / stripe_count;
                }
        } else {
                gf_log ("", GF_LOG_DEBUG,
                        "Volfile is plain distributed");
                dist_count = num_bricks;
        }

        if (stripe_count && replicate_count) {
                gf_log ("", GF_LOG_DEBUG,
                        "Striped Replicate config not allowed");
                return -1;
        }
        if (replicate_count > 1) {
                cluster_count = replicate_count;
                cluster_args = replicate_args;
        }
        if (stripe_count > 1) {
                cluster_count = stripe_count;
                cluster_args = stripe_args;
        }

        i = 0;
        list_for_each_entry (brick, &volinfo->bricks, brick_list) {
                xl = volgen_graph_add_nolink (graph, "protocol/client",
                                              "%s-client-%d", volname, i);
                if (!xl)
                        return -1;
                ret = xlator_set_option (xl, "remote-host", brick->hostname);
                if (ret)
                        return -1;
                ret = xlator_set_option (xl, "remote-subvolume", brick->path);
                if (ret)
                        return -1;
                ret = xlator_set_option (xl, "transport-type", transt);
                if (ret)
                        return -1;

                i++;
        }

        if (cluster_count > 1) {
                j = 0;
                i = 0;
                txl = first_of (graph);
                for (trav = txl; trav->next; trav = trav->next);
                for (;; trav = trav->prev) {
                        if (i % cluster_count == 0) {
                                xl = volgen_graph_add_nolink (graph,
                                                              cluster_args[0],
                                                              cluster_args[1],
                                                              volname, j);
                                if (!xl)
                                        return -1;
                                j++;
                        }

                        ret = volgen_xlator_link (xl, trav);
                        if (ret)
                                return -1;

                        if (trav == txl)
                                break;
                        i++;
                }
        }

        if (dist_count > 1) {
                xl = volgen_graph_add_nolink (graph, "cluster/distribute",
                                              "%s-dht", volname);
                if (!xl)
                        return -1;

                trav = xl;
                for (i = 0; i < dist_count; i++)
                        trav = trav->next;
                for (; trav != xl; trav = trav->prev) {
                        ret = volgen_xlator_link (xl, trav);
                        if (ret)
                                return -1;
                }
        }

        ret = volgen_graph_set_options_generic (graph, set_dict, volname,
                                                &perfxl_option_handler);
        if (ret)
                return -1;

        xl = volgen_graph_add_as (graph, "debug/io-stats", volname);
        if (!xl)
                return -1;

        ret = volgen_graph_set_options_generic (graph, set_dict, "client",
                                                &loglevel_option_handler);

        return ret;
}


/* builds a graph for client role , with option overrides in mod_dict */
static int
build_client_graph (glusterfs_graph_t *graph, glusterd_volinfo_t *volinfo,
                    dict_t *mod_dict)
{
        return build_graph_generic (graph, volinfo, mod_dict, NULL,
                                    &client_graph_builder);
}


/* builds a graph for nfs server role */
static int
build_nfs_graph (glusterfs_graph_t *graph)
{
        glusterfs_graph_t   cgraph        = {{0,},};
        glusterd_volinfo_t *voliter       = NULL;
        xlator_t           *this          = NULL;
        glusterd_conf_t    *priv          = NULL;
        xlator_t           *nfsxl         = NULL;
        char               *skey          = NULL;
        char                volume_id[64] = {0,};
        int                 ret           = 0;

        this = THIS;
        GF_ASSERT (this);
        priv = this->private;
        GF_ASSERT (priv);

        nfsxl = volgen_graph_add_as (graph, "nfs/server", "nfs-server");
        if (!nfsxl)
                return -1;
        ret = xlator_set_option (nfsxl, "nfs.dynamic-volumes", "on");
        if (ret)
                return -1;

        list_for_each_entry (voliter, &priv->volumes, vol_list) {
                if (voliter->status != GLUSTERD_STATUS_STARTED)
                        continue;

                ret = gf_asprintf (&skey, "rpc-auth.addr.%s.allow",
                                   voliter->volname);
                if (ret == -1)
                        goto oom;
                ret = xlator_set_option (nfsxl, skey, "*");
                GF_FREE (skey);
                if (ret)
                        return -1;

                ret = gf_asprintf (&skey, "nfs3.%s.volume-id",
                                   voliter->volname);
                if (ret == -1)
                        goto oom;
                uuid_unparse (voliter->volume_id, volume_id);
                ret = xlator_set_option (nfsxl, skey, volume_id);
                GF_FREE (skey);
                if (ret)
                        return -1;

                memset (&cgraph, 0, sizeof (cgraph));
                ret = build_client_graph (&cgraph, voliter, NULL);
                if (ret)
                        return -1;
                ret = volgen_graph_merge_sub (graph, &cgraph);
        }

        return ret;

 oom:
        gf_log ("", GF_LOG_ERROR, "Out of memory");

        return -1;
}




/****************************
 *
 * Volume generation interface
 *
 ****************************/


static void
get_brick_filepath (char *filename, glusterd_volinfo_t *volinfo,
                    glusterd_brickinfo_t *brickinfo)
{
        char  path[PATH_MAX]   = {0,};
        char  brick[PATH_MAX]  = {0,};

        GLUSTERD_REMOVE_SLASH_FROM_PATH (brickinfo->path, brick);
        VOLGEN_GET_VOLUME_DIR (path, volinfo);

        snprintf (filename, PATH_MAX, "%s/%s.%s.%s.vol",
                  path, volinfo->volname,
                  brickinfo->hostname,
                  brick);
}

static int
glusterd_generate_brick_volfile (glusterd_volinfo_t *volinfo,
                                 glusterd_brickinfo_t *brickinfo)
{
        glusterfs_graph_t graph = {{0,},};
        char    filename[PATH_MAX] = {0,};
        int     ret = -1;

        GF_ASSERT (volinfo);
        GF_ASSERT (brickinfo);

        get_brick_filepath (filename, volinfo, brickinfo);

        ret = build_server_graph (&graph, volinfo, NULL, brickinfo->path);
        if (!ret)
                ret = volgen_write_volfile (&graph, filename);

        volgen_graph_free (&graph);

        return ret;
}

static int
generate_brick_volfiles (glusterd_volinfo_t *volinfo)
{
        glusterd_brickinfo_t    *brickinfo = NULL;
        int                     ret = -1;

        list_for_each_entry (brickinfo, &volinfo->bricks, brick_list) {
                gf_log ("", GF_LOG_DEBUG,
                        "Found a brick - %s:%s", brickinfo->hostname,
                        brickinfo->path);

                ret = glusterd_generate_brick_volfile (volinfo, brickinfo);
                if (ret)
                        goto out;

        }

        ret = 0;

out:
        gf_log ("", GF_LOG_DEBUG, "Returning %d", ret);
        return ret;
}

static void
get_client_filepath (char *filename, glusterd_volinfo_t *volinfo)
{
        char  path[PATH_MAX] = {0,};

        VOLGEN_GET_VOLUME_DIR (path, volinfo);

        snprintf (filename, PATH_MAX, "%s/%s-fuse.vol",
                  path, volinfo->volname);
}

static int
generate_client_volfile (glusterd_volinfo_t *volinfo)
{
        glusterfs_graph_t graph = {{0,},};
        char    filename[PATH_MAX] = {0,};
        int     ret = -1;

        get_client_filepath (filename, volinfo);

        ret = build_client_graph (&graph, volinfo, NULL);
        if (!ret)
                ret = volgen_write_volfile (&graph, filename);

        volgen_graph_free (&graph);

        return ret;
}

int
glusterd_create_rb_volfiles (glusterd_volinfo_t *volinfo,
                             glusterd_brickinfo_t *brickinfo)
{
        int ret = -1;

        ret = glusterd_generate_brick_volfile (volinfo, brickinfo);
        if (!ret)
                ret = generate_client_volfile (volinfo);
        if (!ret)
                ret = glusterd_fetchspec_notify (THIS);

        return ret;
}

int
glusterd_create_volfiles (glusterd_volinfo_t *volinfo)
{
        int ret = -1;

        ret = generate_brick_volfiles (volinfo);
        if (ret) {
                gf_log ("", GF_LOG_ERROR,
                        "Could not generate volfiles for bricks");
                goto out;
        }

        ret = generate_client_volfile (volinfo);
        if (ret) {
                gf_log ("", GF_LOG_ERROR,
                        "Could not generate volfile for client");
                goto out;
        }

        ret = glusterd_fetchspec_notify (THIS);

out:
        return ret;
}

void
glusterd_get_nfs_filepath (char *filename)
{
        char  path[PATH_MAX] = {0,};

        VOLGEN_GET_NFS_DIR (path);

        snprintf (filename, PATH_MAX, "%s/nfs-server.vol", path);
}

int
glusterd_create_nfs_volfile ()
{
        glusterfs_graph_t graph = {{0,},};
        char    filename[PATH_MAX] = {0,};
        int     ret = -1;

        glusterd_get_nfs_filepath (filename);

        ret = build_nfs_graph (&graph);
        if (!ret)
                ret = volgen_write_volfile (&graph, filename);

        volgen_graph_free (&graph);

        return ret;
}

int
glusterd_delete_volfile (glusterd_volinfo_t *volinfo,
                         glusterd_brickinfo_t *brickinfo)
{
        char filename[PATH_MAX] = {0,};

        GF_ASSERT (volinfo);
        GF_ASSERT (brickinfo);

        get_brick_filepath (filename, volinfo, brickinfo);
        return unlink (filename);
}

int
validate_clientopts (glusterd_volinfo_t *volinfo, 
                    dict_t *val_dict, 
                    char **op_errstr)
{
        glusterfs_graph_t graph = {{0,},};
        int     ret = -1;

        GF_ASSERT (volinfo);


        ret = build_client_graph (&graph, volinfo, val_dict);
        if (!ret)
                ret = graph_reconf_validateopt (&graph, op_errstr);

        volgen_graph_free (&graph);

        gf_log ("", GF_LOG_DEBUG, "Returning %d", ret);
        return ret;
}

int
validate_brickopts (glusterd_volinfo_t *volinfo, 
                    char *brickinfo_path,
                    dict_t *val_dict,
                    char **op_errstr)
{
        glusterfs_graph_t graph = {{0,},};
        int     ret = -1;

        GF_ASSERT (volinfo);



        ret = build_server_graph (&graph, volinfo, val_dict, brickinfo_path);
        if (!ret)
                ret = graph_reconf_validateopt (&graph, op_errstr);

        volgen_graph_free (&graph);

        gf_log ("", GF_LOG_DEBUG, "Returning %d", ret);
        return ret;
}

int
glusterd_validate_brickreconf (glusterd_volinfo_t *volinfo,
                               dict_t *val_dict,
                               char **op_errstr)
{
        glusterd_brickinfo_t *brickinfo = NULL;
        int                   ret = -1;
        
        list_for_each_entry (brickinfo, &volinfo->bricks, brick_list) {
                gf_log ("", GF_LOG_DEBUG,
                        "Validating %s", brickinfo->hostname);

                ret = validate_brickopts (volinfo, brickinfo->path, val_dict, 
                                          op_errstr);
                if (ret)
                        goto out;
        }

        ret = 0;
out:
        
                return ret;
}

int
glusterd_validate_reconfopts (glusterd_volinfo_t *volinfo, dict_t *val_dict, 
                              char **op_errstr)
{
        int ret = -1;

        gf_log ("", GF_LOG_DEBUG, "Inside Validate reconfigure options");

        ret = glusterd_validate_brickreconf (volinfo, val_dict, op_errstr);

        if (ret) {
                gf_log ("", GF_LOG_DEBUG,
                        "Could not Validate  bricks");
                goto out;
        }
        
        ret = validate_clientopts (volinfo, val_dict, op_errstr);
        if (ret) {
                gf_log ("", GF_LOG_DEBUG,
                        "Could not Validate client");
                goto out;
        }


out:
                gf_log ("", GF_LOG_DEBUG, "Returning %d", ret);
                return ret;
}

int
glusterd_check_log_level (const char *value)
{
        int log_level = -1;

        if (!strcasecmp (value, "CRITICAL")) {
                log_level = GF_LOG_CRITICAL;
        } else if (!strcasecmp (value, "ERROR")) {
                log_level = GF_LOG_ERROR;
        } else if (!strcasecmp (value, "WARNING")) {
                log_level = GF_LOG_WARNING;
        } else if (!strcasecmp (value, "INFO")) {
                log_level = GF_LOG_INFO;
        } else if (!strcasecmp (value, "DEBUG")) {
                log_level = GF_LOG_DEBUG;
        } else if (!strcasecmp (value, "TRACE")) {
                log_level = GF_LOG_TRACE;
        } else if (!strcasecmp (value, "NONE")) {
                log_level = GF_LOG_NONE;
        }

        if (log_level == -1)
                gf_log ("", GF_LOG_ERROR, "Invalid log-level. possible values "
                        "are DEBUG|WARNING|ERROR|CRITICAL|NONE|TRACE");

        return log_level;
}
