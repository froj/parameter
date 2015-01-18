#include <string.h>
#include "parameter.h"



/* find the length of the next element in hierarchical id
 * returns number of characters until the first '/' or the entire
 * length if no '/' is found)
 */
static int id_split(const char *id, int id_len)
{
    int i = 0;
    for (i = 0; i < id_len; i++) {
        if (id[i] == '/') {
            return i;
        }
    }
    return id_len;
}


static void _ns_link_to_parent(parameter_namespace_t *ns)
{
    // todo make atomic
    ns->next = ns->parent->subspaces;
    ns->parent->subspaces = ns;
}

static void _param_link_to_parent(parameter_t *p)
{
    // todo make atomic
    p->next = p->ns->parameter_list;
    p->ns->parameter_list = p;
}

static parameter_namespace_t *get_subnamespace(parameter_namespace_t *ns,
                                               const char *ns_id,
                                               size_t ns_id_len)
{
    if (ns_id_len == 0) {
        return ns; // this allows to start with a '/' or have '//' instead of '/'
    }
    parameter_namespace_t *i = ns->subspaces;
    while (i != NULL) {
        if (strncmp(ns_id, i->id, ns_id_len) == 0 && i->id[ns_id_len] == '\0') {
            // if the first ns_id_len bytes of ns_id match with i->id and
            // i->id is only ns_id_len bytes long, we've found the namespace
            break;
        }
        i = i->next;
    }
    return i;
}

static parameter_t *get_param(parameter_namespace_t *ns, const char *id,
                              size_t param_id_len)
{
    if (param_id_len == 0) {
        return NULL;
    }
    parameter_t *i = ns->parameter_list;
    while (i != NULL) {
        if (strncmp(id, i->id, param_id_len) == 0 && i->id[param_id_len] == '\0') {
            // if the first param_id_len bytes of id match with i->id and
            // i->id is only param_id_len bytes long, we've found the parameter
            break;
        }
        i = i->next;
    }
    return i;
}

void parameter_namespace_declare(parameter_namespace_t *ns,
                                 parameter_namespace_t *parent,
                                 const char *id)
{
    ns->id = id;
    ns->changed_cnt = 0;
    ns->parent = parent;
    ns->subspaces = NULL;
    ns->parameter_list = NULL;
    if (parent != NULL) {
        _ns_link_to_parent(ns);
    }
}


parameter_namespace_t *_parameter_namespace_find_w_id_len(parameter_namespace_t *ns,
                                                const char *id, size_t id_len)
{
    parameter_namespace_t *nret = ns;
    int i = 0;
    while(nret != NULL && i < id_len) {
        int id_elem_len = id_split(&id[i], id_len - i);
        nret = get_subnamespace(nret, &id[i], id_elem_len);
        i += id_elem_len + 1;
    }
    return nret;
}

parameter_namespace_t *parameter_namespace_find(parameter_namespace_t *ns,
                                                const char *id)
{
    return _parameter_namespace_find_w_id_len(ns, id, strlen(id));
}

parameter_t *_parameter_find_w_id_len(parameter_namespace_t *ns,
                                      const char *id, size_t id_len)
{
    parameter_namespace_t *pns = ns;
    int i = 0;
    while(pns != NULL) {
        int id_elem_len = id_split(&id[i], id_len - i);
        if (id_elem_len + i < id_len) {
            pns = get_subnamespace(pns, &id[i], id_elem_len);
        } else {
            return get_param(pns, &id[i], id_elem_len);
        }
        i += id_elem_len + 1;
    }
    return NULL;
}

parameter_t *parameter_find(parameter_namespace_t *ns, const char *id)
{
    return _parameter_find_w_id_len(ns, id, strlen(id));
}

void _parameter_declare(parameter_t *p, parameter_namespace_t *ns,
                        const char *id)
{
    p->id = id;
    p->ns = ns;
    p->changed = false;
    p->set = false;
    _param_link_to_parent(p);
}