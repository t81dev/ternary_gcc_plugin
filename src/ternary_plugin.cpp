#include <gcc-plugin.h>
#include <plugin-version.h>
#include <tree.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <tree-pass.h>
#include <basic-block.h>
#include <context.h>
#include <c-family/c-common.h>
#include <diagnostic-core.h>
#include <tree-core.h>
#include <langhooks.h>

#include <map>
#include <string>
#include <utility>

#include <cstdio>
#include <cstring>

extern "C" {
int plugin_is_GPL_compatible = 1;
}

opt_pass *opt_pass::clone()
{
    return this;
}

void opt_pass::set_pass_param(unsigned int, bool)
{
    // GCC 15 macOS plugin loader expects this symbol; provide a no-op definition.
}

bool opt_pass::gate(function *)
{
    return true;
}

unsigned int opt_pass::execute(function *)
{
    return 0;
}

static bool opt_warn = false;
static bool opt_stats = false;
static bool opt_lower = false;
static bool opt_arith = false;
static bool opt_logic = false;
static bool opt_cmp = false;
static bool opt_shift = false;
static bool opt_conv = false;
static bool opt_types = false;
static bool opt_vector = false;
static std::string opt_prefix = "__ternary_select";

static tree get_cmp_decl(tree result_type);
static tree get_shift_decl(const char *name, tree result_type);
static tree get_conv_to_ternary_decl(const char *name, tree result_type, tree arg_type);
static tree get_conv_from_ternary_decl(const char *name, tree result_type, tree arg_type);

static unsigned long ternary_count = 0;
static unsigned long lowered_count = 0;

static std::map<std::pair<unsigned, unsigned>, tree> select_decl_cache;
static std::map<unsigned, unsigned> ternary_type_uids;

static bool get_ternary_type_trits(tree type, unsigned *trit_count)
{
    if (!type)
        return false;

    const unsigned uid = TYPE_UID(type);
    const auto it = ternary_type_uids.find(uid);
    if (it == ternary_type_uids.end())
        return false;

    if (trit_count)
        *trit_count = it->second;
    return true;
}

static tree create_ternary_type(unsigned trit_count)
{
    const unsigned precision_bits = trit_count * 2;
    tree type = build_nonstandard_integer_type(precision_bits, 1);

    char name_buf[32];
    snprintf(name_buf, sizeof(name_buf), "t%u_t", trit_count);
    add_builtin_type(name_buf, type);

    ternary_type_uids.emplace(TYPE_UID(type), trit_count);
    return type;
}

static tree get_select_decl(tree result_type, tree cond_type);

static tree get_arith_decl(const char *name, tree result_type)
{
    if (!INTEGRAL_TYPE_P(result_type))
        return NULL_TREE;

    unsigned trit_count = 0;
    if (get_ternary_type_trits(result_type, &trit_count)) {
        // Ternary arithmetic function
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "%s_t%u", name, trit_count);

        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    } else {
        // Standard integer arithmetic
        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    }
}

static tree get_cmp_decl(tree result_type)
{
    if (!INTEGRAL_TYPE_P(result_type))
        return NULL_TREE;

    unsigned trit_count = 0;
    if (get_ternary_type_trits(result_type, &trit_count)) {
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "__ternary_cmp_t%u", trit_count);

        tree fn_type = build_function_type_list(integer_type_node, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    } else {
        tree fn_type = build_function_type_list(integer_type_node, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl("__ternary_cmp", fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    }
}

static tree get_shift_decl(const char *name, tree result_type)
{
    if (!INTEGRAL_TYPE_P(result_type))
        return NULL_TREE;

    unsigned trit_count = 0;
    if (!get_ternary_type_trits(result_type, &trit_count))
        return NULL_TREE;

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", name, trit_count);

    tree fn_type = build_function_type_list(result_type, result_type, integer_type_node, NULL_TREE);
    tree decl = build_fn_decl(name_buf, fn_type);
    TREE_PUBLIC(decl) = 1;
    DECL_EXTERNAL(decl) = 1;
    DECL_ARTIFICIAL(decl) = 1;
    return decl;
}

static tree get_conv_to_ternary_decl(const char *name, tree result_type, tree arg_type)
{
    if (!INTEGRAL_TYPE_P(result_type))
        return NULL_TREE;

    unsigned trit_count = 0;
    if (!get_ternary_type_trits(result_type, &trit_count))
        return NULL_TREE;

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", name, trit_count);

    tree fn_type = build_function_type_list(result_type, arg_type, NULL_TREE);
    tree decl = build_fn_decl(name_buf, fn_type);
    TREE_PUBLIC(decl) = 1;
    DECL_EXTERNAL(decl) = 1;
    DECL_ARTIFICIAL(decl) = 1;
    return decl;
}

static tree get_conv_from_ternary_decl(const char *name, tree result_type, tree arg_type)
{
    if (!INTEGRAL_TYPE_P(arg_type))
        return NULL_TREE;

    unsigned trit_count = 0;
    if (!get_ternary_type_trits(arg_type, &trit_count))
        return NULL_TREE;

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", name, trit_count);

    tree fn_type = build_function_type_list(result_type, arg_type, NULL_TREE);
    tree decl = build_fn_decl(name_buf, fn_type);
    TREE_PUBLIC(decl) = 1;
    DECL_EXTERNAL(decl) = 1;
    DECL_ARTIFICIAL(decl) = 1;
    return decl;
}

static tree get_select_decl(tree result_type, tree cond_type)
{
    if (!INTEGRAL_TYPE_P(cond_type))
        return NULL_TREE;

    const unsigned cond_key = TYPE_UID(cond_type);

    unsigned trit_count = 0;
    if (get_ternary_type_trits(result_type, &trit_count)) {
        // Ternary type select
        const std::pair<unsigned, unsigned> key(trit_count, cond_key);
        const auto it = select_decl_cache.find(key);
        if (it != select_decl_cache.end())
            return it->second;

        char name_buf[32];
        snprintf(name_buf, sizeof(name_buf), "%s_t%u", opt_prefix.c_str(), trit_count);

        tree fn_type = build_function_type_list(result_type, cond_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;

        select_decl_cache.emplace(key, decl);
        return decl;
    }
    else if (INTEGRAL_TYPE_P(result_type))
    {
        const unsigned precision = TYPE_PRECISION(result_type);
        const bool unsigned_p = TYPE_UNSIGNED(result_type);
        const unsigned result_key = (precision << 1) | (unsigned_p ? 1U : 0U);
        const std::pair<unsigned, unsigned> key(result_key, cond_key);

        const auto it = select_decl_cache.find(key);
        if (it != select_decl_cache.end())
            return it->second;

        char name_buf[64];
        const char sign_char = unsigned_p ? 'u' : 'i';
        snprintf(name_buf, sizeof(name_buf), "%s_%c%u", opt_prefix.c_str(), sign_char, precision);

        tree fn_type = build_function_type_list(result_type, cond_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;

        select_decl_cache.emplace(key, decl);
        return decl;
    }
    else if (SCALAR_FLOAT_TYPE_P(result_type))
    {
        const unsigned precision = TYPE_PRECISION(result_type);
        unsigned result_key = 0x80000000U | precision;  // Use high bit for float
        const std::pair<unsigned, unsigned> key(result_key, cond_key);

        const auto it = select_decl_cache.find(key);
        if (it != select_decl_cache.end())
            return it->second;

        char name_buf[64];
        if (precision == 32)
            snprintf(name_buf, sizeof(name_buf), "%s_f32", opt_prefix.c_str());
        else if (precision == 64)
            snprintf(name_buf, sizeof(name_buf), "%s_f64", opt_prefix.c_str());
        else
            return NULL_TREE;  // Unsupported float precision

        tree fn_type = build_function_type_list(result_type, cond_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;

        select_decl_cache.emplace(key, decl);
        return decl;
    }

        return NULL_TREE;
    }

namespace
{
const pass_data ternary_pass_data = {
    GIMPLE_PASS,
    "ternary",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_gimple_any,
    0,
    0,
    0,
    0,
};

class ternary_pass : public gimple_opt_pass
{
public:
    ternary_pass() : gimple_opt_pass(ternary_pass_data, g) {}

    void set_pass_param(unsigned int n, bool value) override
    {
        // GCC 15 compatibility: handle pass parameter setting
        (void)n;
        (void)value;
    }

    unsigned int execute(function *fun) override
    {
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun)
        {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
            {
                gimple *stmt = gsi_stmt(gsi);
                if (!is_gimple_assign(stmt) && !is_gimple_call(stmt))
                    continue;

                if (is_gimple_assign(stmt) && gimple_assign_rhs_code(stmt) == COND_EXPR)
                {
                    ternary_count++;
                    if (opt_warn)
                        warning_at(gimple_location(stmt), 0, "ternary operator detected");

                    if (!opt_lower)
                        continue;

                    tree lhs = gimple_assign_lhs(stmt);
                    tree cond = gimple_assign_rhs1(stmt);
                    tree true_val = gimple_assign_rhs2(stmt);
                    tree false_val = gimple_assign_rhs3(stmt);
                    tree result_type = TREE_TYPE(lhs);

                    tree decl = get_select_decl(result_type, TREE_TYPE(cond));
                    if (!decl)
                        continue;

                    gcall *call = gimple_build_call(decl, 3, cond, true_val, false_val);
                    gimple_call_set_lhs(call, lhs);
                    gsi_replace(&gsi, call, true);
                    lowered_count++;
                    continue;
                }

                if (is_gimple_call(stmt) && (opt_arith || opt_logic || opt_cmp || opt_shift || opt_conv))
                {
                    tree fndecl = gimple_call_fndecl(stmt);
                    tree lhs = gimple_call_lhs(stmt);
                    if (!fndecl || !lhs)
                        continue;

                    const char *name = IDENTIFIER_POINTER(DECL_NAME(fndecl));
                    tree lhs_type = TREE_TYPE(lhs);

                    if (opt_arith && !strcmp(name, "__builtin_ternary_add"))
                    {
                        tree decl = get_arith_decl("__ternary_add", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_mul"))
                    {
                        tree decl = get_arith_decl("__ternary_mul", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_sub"))
                    {
                        tree decl = get_arith_decl("__ternary_sub", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_div"))
                    {
                        tree decl = get_arith_decl("__ternary_div", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_mod"))
                    {
                        tree decl = get_arith_decl("__ternary_mod", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_neg"))
                    {
                        if (INTEGRAL_TYPE_P(lhs_type) && gimple_call_num_args(stmt) == 1)
                        {
                            tree fn_type = build_function_type_list(lhs_type, lhs_type, NULL_TREE);
                            tree decl = build_fn_decl("__ternary_neg", fn_type);
                            TREE_PUBLIC(decl) = 1;
                            DECL_EXTERNAL(decl) = 1;
                            DECL_ARTIFICIAL(decl) = 1;

                            gcall *new_call = gimple_build_call(decl, 1, gimple_call_arg(stmt, 0));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_cmp && !strcmp(name, "__builtin_ternary_cmp"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree arg1 = gimple_call_arg(stmt, 1);
                        tree arg0_type = TREE_TYPE(arg0);
                        tree decl = get_cmp_decl(arg0_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2, arg0, arg1);
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_not"))
                    {
                        if (INTEGRAL_TYPE_P(lhs_type) && gimple_call_num_args(stmt) == 1)
                        {
                            tree fn_type = build_function_type_list(lhs_type, lhs_type, NULL_TREE);
                            tree decl = build_fn_decl("__ternary_not", fn_type);
                            TREE_PUBLIC(decl) = 1;
                            DECL_EXTERNAL(decl) = 1;
                            DECL_ARTIFICIAL(decl) = 1;

                            gcall *new_call = gimple_build_call(decl, 1, gimple_call_arg(stmt, 0));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_and"))
                    {
                        tree decl = get_arith_decl("__ternary_and", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_or"))
                    {
                        tree decl = get_arith_decl("__ternary_or", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_xor"))
                    {
                        tree decl = get_arith_decl("__ternary_xor", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_select"))
                    {
                        if (gimple_call_num_args(stmt) == 3)
                        {
                            tree cond = gimple_call_arg(stmt, 0);
                            tree true_val = gimple_call_arg(stmt, 1);
                            tree false_val = gimple_call_arg(stmt, 2);
                            tree decl = get_select_decl(lhs_type, TREE_TYPE(cond));
                            if (decl)
                            {
                                gcall *new_call = gimple_build_call(decl, 3, cond, true_val, false_val);
                                gimple_call_set_lhs(new_call, lhs);
                                gsi_replace(&gsi, new_call, true);
                                lowered_count++;
                            }
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_shl"))
                    {
                        tree decl = get_shift_decl("__ternary_shl", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_shr"))
                    {
                        tree decl = get_shift_decl("__ternary_shr", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_rol"))
                    {
                        tree decl = get_shift_decl("__ternary_rol", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_ror"))
                    {
                        tree decl = get_shift_decl("__ternary_ror", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_tb2t"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree decl = get_conv_to_ternary_decl("__ternary_tb2t", lhs_type, TREE_TYPE(arg0));
                        if (decl && gimple_call_num_args(stmt) == 1)
                        {
                            gcall *new_call = gimple_build_call(decl, 1, arg0);
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_tt2b"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree decl = get_conv_from_ternary_decl("__ternary_tt2b", lhs_type, TREE_TYPE(arg0));
                        if (decl && gimple_call_num_args(stmt) == 1)
                        {
                            gcall *new_call = gimple_build_call(decl, 1, arg0);
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_t2f"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        if (SCALAR_FLOAT_TYPE_P(lhs_type) && gimple_call_num_args(stmt) == 1)
                        {
                            const unsigned precision = TYPE_PRECISION(lhs_type);
                            const char *conv_name = (precision == 32) ? "__ternary_t2f32" :
                                                    (precision == 64) ? "__ternary_t2f64" : NULL;
                            if (conv_name)
                            {
                                tree decl = get_conv_from_ternary_decl(conv_name, lhs_type, TREE_TYPE(arg0));
                                if (decl)
                                {
                                    gcall *new_call = gimple_build_call(decl, 1, arg0);
                                    gimple_call_set_lhs(new_call, lhs);
                                    gsi_replace(&gsi, new_call, true);
                                }
                            }
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_f2t"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        if (SCALAR_FLOAT_TYPE_P(TREE_TYPE(arg0)) && gimple_call_num_args(stmt) == 1)
                        {
                            const unsigned precision = TYPE_PRECISION(TREE_TYPE(arg0));
                            const char *conv_name = (precision == 32) ? "__ternary_f2t32" :
                                                    (precision == 64) ? "__ternary_f2t64" : NULL;
                            if (conv_name)
                            {
                                tree decl = get_conv_to_ternary_decl(conv_name, lhs_type, TREE_TYPE(arg0));
                                if (decl)
                                {
                                    gcall *new_call = gimple_build_call(decl, 1, arg0);
                                    gimple_call_set_lhs(new_call, lhs);
                                    gsi_replace(&gsi, new_call, true);
                                }
                            }
                        }
                    }
                }
            }
        }

        return 0;
    }
};
} // namespace

static void parse_args(struct plugin_name_args *plugin_info)
{
    for (int i = 0; i < plugin_info->argc; ++i)
    {
        const char *key = plugin_info->argv[i].key;
        const char *value = plugin_info->argv[i].value;

        if (!key)
            continue;
        if (!strcmp(key, "warn"))
            opt_warn = true;
        else if (!strcmp(key, "stats"))
            opt_stats = true;
        else if (!strcmp(key, "lower"))
            opt_lower = true;
        else if (!strcmp(key, "arith"))
            opt_arith = true;
        else if (!strcmp(key, "logic"))
            opt_logic = true;
        else if (!strcmp(key, "cmp"))
            opt_cmp = true;
        else if (!strcmp(key, "shift"))
            opt_shift = true;
        else if (!strcmp(key, "conv"))
            opt_conv = true;
        else if (!strcmp(key, "types"))
            opt_types = true;
        else if (!strcmp(key, "vector"))
            opt_vector = true;
        else if (!strcmp(key, "prefix") && value)
            opt_prefix = value;
        else
            warning(0, "unknown plugin argument '%s'", key);
    }
}

static void ternary_plugin_init(void *gcc_data, void *user_data)
{
    (void)gcc_data;
    (void)user_data;

    if (!opt_types)
        return;

    create_ternary_type(6);
    create_ternary_type(12);
    create_ternary_type(24);
    create_ternary_type(48);
    create_ternary_type(96);
    create_ternary_type(192);
}

static void ternary_plugin_finish(void *, void *)
{
    if (opt_stats)
        inform(UNKNOWN_LOCATION, "ternary plugin: %lu ternary ops, %lu lowered",
               ternary_count, lowered_count);
}

extern "C" int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version)
{
    if (!plugin_default_version_check(version, &gcc_version))
        return 1;

    parse_args(plugin_info);

    register_callback(plugin_info->base_name, PLUGIN_START_UNIT, ternary_plugin_init, NULL);
    register_callback(plugin_info->base_name, PLUGIN_FINISH, ternary_plugin_finish, NULL);
    register_pass_info pass_info;
    pass_info.pass = new ternary_pass();
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

    return 0;
}
