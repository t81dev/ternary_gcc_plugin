#include <map>
#include <string>
#include <utility>
#include <cstdio>

#include <gcc-plugin.h>
#include <plugin-version.h>
#include <tree.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <gimple-pretty-print.h>
#include <tree-pass.h>
#include <basic-block.h>
#include <context.h>
#include <c-family/c-common.h>
#include <diagnostic-core.h>
#include <dumpfile.h>
#include <tree-core.h>
#include <langhooks.h>

// Undefine conflicting macros from safe-ctype.h
#undef toupper
#undef tolower
#undef isspace
#undef isprint
#undef iscntrl
#undef isupper
#undef islower
#undef isalpha
#undef isalnum
#undef isdigit
#undef isgraph
#undef ispunct
#undef isxdigit
#include <cctype>
#include <cstring>
#include <cstdint>

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
static bool opt_mem = false;
static bool opt_vector = false;
static bool opt_version = false;
static bool opt_selftest = false;
static bool opt_trace = false;
static bool opt_dump_gimple = false;
static std::string opt_prefix = "__ternary";

static tree get_cmp_decl(tree result_type);
static tree get_shift_decl(const char *name, tree result_type);
static tree get_conv_to_ternary_decl(const char *name, tree result_type, tree arg_type);
static tree get_conv_from_ternary_decl(const char *name, tree result_type, tree arg_type);
static tree get_select_decl(tree result_type, tree cond_type);
static tree get_ternary_cmp_decl(const char *name, tree result_type);
static tree create_selftest_type(unsigned trit_count);

static unsigned long ternary_count = 0;
static unsigned long lowered_count = 0;
static unsigned long surviving_count = 0;

static std::map<unsigned, tree> select_decl_cache;
static std::map<unsigned, unsigned> ternary_type_uids;
static std::map<unsigned, unsigned> ternary_vector_type_uids;

static tree lower_cond_expr_tree(tree expr, gimple_stmt_iterator *gsi);
static tree lower_cond_expr_in_tree(tree expr, gimple_stmt_iterator *gsi);
static bool tree_has_cond_expr(tree expr);
static tree build_cmp_call(tree arg1, tree arg2, gimple_stmt_iterator *gsi);
static std::string build_helper_name(const char *base);

static std::string read_version_file()
{
    FILE *file = fopen("VERSION", "r");
    if (!file)
        return "unknown";

    char buf[64] = {0};
    if (!fgets(buf, sizeof(buf), file))
    {
        fclose(file);
        return "unknown";
    }
    fclose(file);

    size_t len = strlen(buf);
    while (len > 0 && isspace(static_cast<unsigned char>(buf[len - 1])))
        buf[--len] = '\0';
    return std::string(buf);
}

static bool check_gcc_version_compatibility(struct plugin_gcc_version *version)
{
    // Supported GCC versions: 9.0 to 15.x
    int major = 0, minor = 0;
    if (sscanf(version->basever, "%d.%d", &major, &minor) != 2)
        return false;

    if (major < 9 || major > 15)
        return false;

    // Special handling for GCC 15 (newer features)
    if (major == 15)
    {
        // GCC 15 has some API changes, but we handle them with compatibility code
        inform(UNKNOWN_LOCATION, "ternary plugin: GCC 15 detected, using compatibility mode");
    }

    return true;
}

static void run_selftest()
{
    tree cond_type = create_selftest_type(32);
    tree conv_decl = get_conv_from_ternary_decl("tt2b",
                                                long_long_integer_type_node,
                                                cond_type);
    tree select_decl = get_select_decl(integer_type_node, cond_type);

    if (conv_decl && select_decl)
        inform(UNKNOWN_LOCATION, "ternary plugin selftest: ternary cond lowering ok");
    else
        warning(0, "ternary plugin selftest: ternary cond lowering missing helpers");
}

static void maybe_dump_stmt(gimple *stmt)
{
    if (!opt_dump_gimple)
        return;

    print_gimple_stmt(stderr, stmt, 0, TDF_SLIM);
    fputc('\n', stderr);
}

static bool get_ternary_type_trits(tree type, unsigned *trit_count)
{
    if (!type)
        return false;

    const unsigned uid = TYPE_UID(type);
    const auto it = ternary_type_uids.find(uid);
    if (it == ternary_type_uids.end())
    {
        tree name = TYPE_NAME(type);
        if (name && TREE_CODE(name) == TYPE_DECL)
            name = DECL_NAME(name);
        if (!name || TREE_CODE(name) != IDENTIFIER_NODE)
            return false;

        const char *type_name = IDENTIFIER_POINTER(name);
        unsigned matched_trits = 0;
        if (!strcmp(type_name, "t32_t"))
            matched_trits = 32;
        else if (!strcmp(type_name, "t64_t"))
            matched_trits = 64;
        else if (!strcmp(type_name, "t128_t"))
            matched_trits = 128;
        else
            return false;

        if (TYPE_PRECISION(type) != matched_trits * 2)
            return false;

        ternary_type_uids.emplace(uid, matched_trits);
        if (trit_count)
            *trit_count = matched_trits;
        return true;
    }

    if (trit_count)
        *trit_count = it->second;
    return true;
}

static bool get_ternary_vector_type_trits(tree type, unsigned *element_trit_count)
{
    if (!type || TREE_CODE(type) != VECTOR_TYPE)
        return false;

    const unsigned uid = TYPE_UID(type);
    const auto it = ternary_vector_type_uids.find(uid);
    if (it == ternary_vector_type_uids.end())
    {
        tree name = TYPE_NAME(type);
        if (name && TREE_CODE(name) == TYPE_DECL)
            name = DECL_NAME(name);
        if (!name || TREE_CODE(name) != IDENTIFIER_NODE)
            return false;

        const char *type_name = IDENTIFIER_POINTER(name);
        unsigned matched_trits = 0;
        if (!strcmp(type_name, "tv32_t"))
            matched_trits = 32;
        else if (!strcmp(type_name, "tv64_t"))
            matched_trits = 64;
        else if (!strcmp(type_name, "tv128_t"))
            matched_trits = 128;
        else
            return false;

        // Check that it's a vector of 2 elements
        if (TYPE_VECTOR_SUBPARTS(type) != 2)
            return false;

        ternary_vector_type_uids.emplace(uid, matched_trits);
        if (element_trit_count)
            *element_trit_count = matched_trits;
        return true;
    }

    if (element_trit_count)
        *element_trit_count = it->second;
    return true;
}

static tree lower_cond_expr_tree(tree expr, gimple_stmt_iterator *gsi)
{
    if (!expr || TREE_CODE(expr) != COND_EXPR)
        return expr;

    ternary_count++;
    if (opt_warn)
        warning_at(gimple_location(gsi_stmt(*gsi)), 0, "ternary operator detected");
    if (opt_trace)
        inform(gimple_location(gsi_stmt(*gsi)), "ternary: found conditional operator");

    tree cond = COND_EXPR_COND(expr);
    tree true_val = COND_EXPR_THEN(expr);
    tree false_val = COND_EXPR_ELSE(expr);

    cond = lower_cond_expr_in_tree(cond, gsi);
    true_val = lower_cond_expr_in_tree(true_val, gsi);
    false_val = lower_cond_expr_in_tree(false_val, gsi);

    tree packed = NULL_TREE;
    if (TREE_CODE(cond) == INTEGER_CST && ternary_pack_constant(cond, TREE_TYPE(cond), &packed))
        cond = packed;
    if (TREE_CODE(true_val) == INTEGER_CST && ternary_pack_constant(true_val, TREE_TYPE(true_val), &packed))
        true_val = packed;
    if (TREE_CODE(false_val) == INTEGER_CST && ternary_pack_constant(false_val, TREE_TYPE(false_val), &packed))
        false_val = packed;

    if (TREE_CODE(cond) == INTEGER_CST) {
        bool cond_known = false;
        bool cond_zero = false;
        if (get_ternary_type_trits(TREE_TYPE(cond), nullptr)) {
            int64_t logical = 0;
            if (ternary_unpack_constant(cond, TREE_TYPE(cond), &logical)) {
                cond_known = true;
                cond_zero = (logical == 0);
            }
        } else {
            cond_known = true;
            cond_zero = integer_zerop(cond);
        }
        if (cond_known)
            return cond_zero ? false_val : true_val;
    }

    if (operand_equal_p(true_val, false_val, 0))
        return true_val;

    tree result_type = TREE_TYPE(expr);
    tree cond_arg = cond;
    tree cond_type = TREE_TYPE(cond);
    unsigned cond_trits = 0;
    if (get_ternary_type_trits(cond_type, &cond_trits)) {
        tree conv_decl = get_conv_from_ternary_decl("tt2b",
                                                    long_long_integer_type_node,
                                                    cond_type);
        if (!conv_decl)
            return expr;

        tree tmp_cond = create_tmp_var(long_long_integer_type_node, "ternary_cond");
        gcall *conv_call = gimple_build_call(conv_decl, 1, cond);
        gimple_call_set_lhs(conv_call, tmp_cond);
        gsi_insert_before(gsi, conv_call, GSI_SAME_STMT);
        cond_arg = tmp_cond;
    } else if (!types_compatible_p(cond_type, long_long_integer_type_node)) {
        cond_arg = fold_convert(long_long_integer_type_node, cond);
    }

    tree decl = get_select_decl(result_type, cond_type);
    if (!decl)
        return expr;

    tree tmp = create_tmp_var(result_type, "ternary_sel");
    gcall *call = gimple_build_call(decl, 3, cond_arg, true_val, false_val);
    gimple_call_set_lhs(call, tmp);
    gsi_insert_before(gsi, call, GSI_SAME_STMT);
    lowered_count++;
    if (opt_trace)
        inform(gimple_location(gsi_stmt(*gsi)), "ternary: lowered conditional operator in expression");
    return tmp;
}

static tree lower_cond_expr_in_tree(tree expr, gimple_stmt_iterator *gsi)
{
    if (!expr)
        return expr;
    if (TREE_CODE(expr) == COND_EXPR)
        return lower_cond_expr_tree(expr, gsi);

    if (TREE_SIDE_EFFECTS(expr) && tree_has_cond_expr(expr)) {
        if (opt_warn)
            warning_at(gimple_location(gsi_stmt(*gsi)), 0,
                       "ternary: skipping conditional lowering under side-effecting expression");
        return expr;
    }

    const enum tree_code_class cls = TREE_CODE_CLASS(TREE_CODE(expr));
    if (cls == tcc_type || cls == tcc_declaration || cls == tcc_constant)
        return expr;

    const int len = TREE_OPERAND_LENGTH(expr);
    for (int i = 0; i < len; ++i) {
        tree op = TREE_OPERAND(expr, i);
        if (!op)
            continue;
        const enum tree_code_class op_cls = TREE_CODE_CLASS(TREE_CODE(op));
        if (op_cls == tcc_type || op_cls == tcc_declaration)
            continue;
        tree lowered = lower_cond_expr_in_tree(op, gsi);
        if (lowered != op)
            TREE_OPERAND(expr, i) = lowered;
    }
    return expr;
}

static bool tree_has_cond_expr(tree expr)
{
    if (!expr)
        return false;
    if (TREE_CODE(expr) == COND_EXPR)
        return true;

    const enum tree_code_class cls = TREE_CODE_CLASS(TREE_CODE(expr));
    if (cls == tcc_type || cls == tcc_declaration || cls == tcc_constant)
        return false;

    const int len = TREE_OPERAND_LENGTH(expr);
    for (int i = 0; i < len; ++i) {
        tree op = TREE_OPERAND(expr, i);
        if (!op)
            continue;
        const enum tree_code_class op_cls = TREE_CODE_CLASS(TREE_CODE(op));
        if (op_cls == tcc_type || op_cls == tcc_declaration)
            continue;
        if (tree_has_cond_expr(op))
            return true;
    }
    return false;
}

static std::string build_helper_name(const char *base)
{
    return opt_prefix + "_" + base;
}

static tree build_cmp_call(tree arg1, tree arg2, gimple_stmt_iterator *gsi)
{
    if (!arg1 || !arg2)
        return NULL_TREE;

    tree arg1_type = TREE_TYPE(arg1);
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(arg1_type, &trit_count))
        return NULL_TREE;
    if (!types_compatible_p(arg1_type, TREE_TYPE(arg2)))
        return NULL_TREE;

    tree decl = get_cmp_decl(arg1_type);
    if (!decl)
    {
        if (opt_warn) {
            std::string helper = build_helper_name("cmp");
            warning_at(gimple_location(gsi_stmt(*gsi)), 0,
                       "ternary: cannot lower comparison (missing %s_t%u helper)",
                       helper.c_str(), trit_count);
        }
        return NULL_TREE;
    }

    tree tmp = create_tmp_var(integer_type_node, "ternary_cmp");
    gcall *call = gimple_build_call(decl, 2, arg1, arg2);
    gimple_call_set_lhs(call, tmp);
    gsi_insert_before(gsi, call, GSI_SAME_STMT);
    return tmp;
}

static bool ternary_value_fits_trits(int64_t value, unsigned trit_count)
{
    if (trit_count >= 40)
        return true;

    int64_t pow3 = 1;
    for (unsigned i = 0; i < trit_count; ++i) {
        if (pow3 > (INT64_MAX / 3))
            return true;
        pow3 *= 3;
    }
    const int64_t max_abs = (pow3 - 1) / 2;
    return value >= -max_abs && value <= max_abs;
}

static bool ternary_pack_constant(tree value, tree type, tree *out)
{
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(type, &trit_count))
        return false;
    if (trit_count > 32)
        return false;
    if (!tree_fits_shwi_p(value))
        return false;

    int64_t logical = tree_to_shwi(value);
    if (!ternary_value_fits_trits(logical, trit_count))
        return false;

    uint64_t packed = 0;
    int64_t v = logical;
    for (unsigned i = 0; i < trit_count; ++i) {
        int64_t rem = v % 3;
        v /= 3;
        if (rem == 2) {
            rem = -1;
            v += 1;
        } else if (rem == -2) {
            rem = 1;
            v -= 1;
        }
        uint64_t bits = (rem < 0) ? 0U : (rem == 0 ? 1U : 2U);
        packed |= bits << (2U * i);
    }

    *out = build_int_cst_type(type, (HOST_WIDE_INT)packed);
    return true;
}

static bool ternary_unpack_constant(tree value, tree type, int64_t *out)
{
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(type, &trit_count))
        return false;
    if (trit_count > 32)
        return false;
    if (!tree_fits_uhwi_p(value))
        return false;

    uint64_t packed = (uint64_t)tree_to_uhwi(value);
    int64_t logical = 0;
    int64_t pow3 = 1;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits = (unsigned)((packed >> (2U * i)) & 0x3U);
        int trit = (bits == 0U) ? -1 : (bits == 1U ? 0 : 1);
        logical += (int64_t)trit * pow3;
        pow3 *= 3;
    }
    *out = logical;
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

static tree create_ternary_vector_type(unsigned element_trit_count)
{
    // Create vector type for packed ternary vectors
    // tv32_t: vector of 2 x t32_t (128 bits total)
    // tv64_t: vector of 2 x t64_t (256 bits total - represented as struct)
    
    if (element_trit_count == 32) {
        // tv32_t: 128-bit vector
        tree element_type = create_ternary_type(element_trit_count);
        tree vector_type = build_vector_type(element_type, 2);
        
        char name_buf[32];
        snprintf(name_buf, sizeof(name_buf), "tv%u_t", element_trit_count);
        add_builtin_type(name_buf, vector_type);
        
        ternary_vector_type_uids.emplace(TYPE_UID(vector_type), element_trit_count);
        return vector_type;
    } else if (element_trit_count == 64) {
        // tv64_t: For now, skip complex struct handling
        // TODO: Implement proper struct type creation
        return NULL_TREE;
    }
    
    return NULL_TREE;
}

static tree create_selftest_type(unsigned trit_count)
{
    const unsigned precision_bits = trit_count * 2;
    tree type = build_nonstandard_integer_type(precision_bits, 1);
    ternary_type_uids.emplace(TYPE_UID(type), trit_count);
    return type;
}

static tree get_select_decl(tree result_type, tree cond_type);

static tree get_arith_decl(const char *name, tree result_type)
{
    if (!INTEGRAL_TYPE_P(result_type) && TREE_CODE(result_type) != VECTOR_TYPE)
        return NULL_TREE;

    std::string base_name = build_helper_name(name);
    unsigned trit_count = 0;
    if (get_ternary_type_trits(result_type, &trit_count)) {
        // Ternary arithmetic function
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    } else if (get_ternary_vector_type_trits(result_type, &trit_count)) {
        // Ternary vector arithmetic function
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "%s_tv%u", base_name.c_str(), trit_count);

        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    } else {
        // Standard integer arithmetic
        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(base_name.c_str(), fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    }
}

static tree get_cmp_decl(tree result_type)
{
    if (!INTEGRAL_TYPE_P(result_type) && TREE_CODE(result_type) != VECTOR_TYPE)
        return NULL_TREE;

    std::string base_name = build_helper_name("cmp");
    unsigned trit_count = 0;
    if (get_ternary_type_trits(result_type, &trit_count)) {
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    } else if (get_ternary_vector_type_trits(result_type, &trit_count)) {
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "%s_tv%u", base_name.c_str(), trit_count);

        tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    } else {
        tree fn_type = build_function_type_list(integer_type_node, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(base_name.c_str(), fn_type);
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

    std::string base_name = build_helper_name(name);
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(result_type, &trit_count))
    {
        tree fn_type = build_function_type_list(result_type, result_type, integer_type_node, NULL_TREE);
        tree decl = build_fn_decl(base_name.c_str(), fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;
        return decl;
    }

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

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

    std::string base_name = build_helper_name(name);
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(result_type, &trit_count))
        return NULL_TREE;

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

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

    std::string base_name = build_helper_name(name);
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(arg_type, &trit_count))
        return NULL_TREE;

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

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
    const tree abi_cond_type = long_long_integer_type_node;

    unsigned trit_count = 0;
    if (get_ternary_type_trits(result_type, &trit_count)) {
        const unsigned key = 0x40000000U | trit_count;
        const auto it = select_decl_cache.find(key);
        if (it != select_decl_cache.end())
            return it->second;

        char name_buf[32];
        std::string base_name = build_helper_name("select");
        snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

        tree fn_type = build_function_type_list(result_type, abi_cond_type, result_type, result_type, NULL_TREE);
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

        const auto it = select_decl_cache.find(result_key);
        if (it != select_decl_cache.end())
            return it->second;

        char name_buf[64];
        std::string base_name = build_helper_name("select");
        const char sign_char = unsigned_p ? 'u' : 'i';
        snprintf(name_buf, sizeof(name_buf), "%s_%c%u", base_name.c_str(), sign_char, precision);

        tree fn_type = build_function_type_list(result_type, abi_cond_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;

        select_decl_cache.emplace(result_key, decl);
        return decl;
    }
    else if (SCALAR_FLOAT_TYPE_P(result_type))
    {
        const unsigned precision = TYPE_PRECISION(result_type);
        unsigned result_key = 0x80000000U | precision;  // Use high bit for float

        const auto it = select_decl_cache.find(result_key);
        if (it != select_decl_cache.end())
            return it->second;

        char name_buf[64];
        std::string base_name = build_helper_name("select");
        if (precision == 32)
            snprintf(name_buf, sizeof(name_buf), "%s_f32", base_name.c_str());
        else if (precision == 64)
            snprintf(name_buf, sizeof(name_buf), "%s_f64", base_name.c_str());
        else
            return NULL_TREE;  // Unsupported float precision

        tree fn_type = build_function_type_list(result_type, abi_cond_type, result_type, result_type, NULL_TREE);
        tree decl = build_fn_decl(name_buf, fn_type);
        TREE_PUBLIC(decl) = 1;
        DECL_EXTERNAL(decl) = 1;
        DECL_ARTIFICIAL(decl) = 1;

        select_decl_cache.emplace(result_key, decl);
        return decl;
    }

        return NULL_TREE;
    }

static tree get_ternary_cmp_decl(const char *name, tree result_type)
{
    unsigned trit_count = 0;
    if (!get_ternary_type_trits(result_type, &trit_count))
        return NULL_TREE;

    const unsigned key = 0x50000000U | trit_count;
    const auto it = select_decl_cache.find(key);  // Reuse cache
    if (it != select_decl_cache.end())
        return it->second;

    char name_buf[32];
    std::string base_name = build_helper_name(name);
    snprintf(name_buf, sizeof(name_buf), "%s_t%u", base_name.c_str(), trit_count);

    tree fn_type = build_function_type_list(result_type, result_type, result_type, NULL_TREE);
    tree decl = build_fn_decl(name_buf, fn_type);
    TREE_PUBLIC(decl) = 1;
    DECL_EXTERNAL(decl) = 1;
    DECL_ARTIFICIAL(decl) = 1;

    select_decl_cache.emplace(key, decl);
    return decl;
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
                if (!is_gimple_assign(stmt) && !is_gimple_call(stmt) && !is_gimple_cond(stmt) &&
                    !is_gimple_return(stmt))
                    continue;

                if (is_gimple_assign(stmt) && gimple_assign_rhs_code(stmt) == COND_EXPR)
                {
                    ternary_count++;
                    if (opt_warn)
                        warning_at(gimple_location(stmt), 0, "ternary operator detected");
                    if (opt_trace)
                        inform(gimple_location(stmt), "ternary: found conditional operator");

                    if (!opt_lower) {
                        surviving_count++;
                        continue;
                    }

                    maybe_dump_stmt(stmt);
                    tree lhs = gimple_assign_lhs(stmt);
                    tree cond = gimple_assign_rhs1(stmt);
                    tree true_val = gimple_assign_rhs2(stmt);
                    tree false_val = gimple_assign_rhs3(stmt);
                    tree packed = NULL_TREE;

                    if (TREE_CODE(cond) == INTEGER_CST && ternary_pack_constant(cond, TREE_TYPE(cond), &packed))
                        cond = packed;
                    if (TREE_CODE(true_val) == INTEGER_CST && ternary_pack_constant(true_val, TREE_TYPE(true_val), &packed))
                        true_val = packed;
                    if (TREE_CODE(false_val) == INTEGER_CST && ternary_pack_constant(false_val, TREE_TYPE(false_val), &packed))
                        false_val = packed;

                    // Simplify if cond is constant
                    if (TREE_CODE(cond) == INTEGER_CST) {
                        bool cond_known = false;
                        bool cond_zero = false;
                        if (get_ternary_type_trits(TREE_TYPE(cond), nullptr)) {
                            int64_t logical = 0;
                            if (ternary_unpack_constant(cond, TREE_TYPE(cond), &logical)) {
                                cond_known = true;
                                cond_zero = (logical == 0);
                            }
                        } else {
                            cond_known = true;
                            cond_zero = integer_zerop(cond);
                        }
                        if (!cond_known)
                            goto skip_cond_simplify;

                        tree selected_val = cond_zero ? false_val : true_val;
                        gimple_assign_set_rhs_from_tree(&gsi, selected_val);
                        lowered_count++;
                        if (opt_trace)
                            inform(gimple_location(stmt), "ternary: simplified constant conditional to %s", cond_zero ? "false" : "true");
                        continue;
                    }
skip_cond_simplify:

                    // Simplify if true_val == false_val
                    if (operand_equal_p(true_val, false_val, 0)) {
                        gimple_assign_set_rhs_from_tree(&gsi, true_val);
                        lowered_count++;
                        if (opt_trace)
                            inform(gimple_location(stmt), "ternary: simplified conditional with equal branches");
                        continue;
                    }

                    tree result_type = TREE_TYPE(lhs);
                    tree cond_arg = cond;
                    tree cond_type = TREE_TYPE(cond);
                    unsigned cond_trits = 0;
                    if (get_ternary_type_trits(cond_type, &cond_trits)) {
                        tree conv_decl = get_conv_from_ternary_decl("tt2b",
                                                                    long_long_integer_type_node,
                                                                    cond_type);
                        if (!conv_decl) {
                            surviving_count++;
                            continue;
                        }

                        tree tmp = create_tmp_var(long_long_integer_type_node, "ternary_cond");
                        gcall *conv_call = gimple_build_call(conv_decl, 1, cond);
                        gimple_call_set_lhs(conv_call, tmp);
                        gsi_insert_before(&gsi, conv_call, GSI_SAME_STMT);
                        cond_arg = tmp;
                    } else if (!types_compatible_p(cond_type, long_long_integer_type_node)) {
                        cond_arg = fold_convert(long_long_integer_type_node, cond);
                    }

                    tree decl = get_select_decl(result_type, cond_type);
                    if (!decl) {
                        surviving_count++;
                        continue;
                    }

                    gcall *call = gimple_build_call(decl, 3, cond_arg, true_val, false_val);
                    gimple_call_set_lhs(call, lhs);
                    gsi_replace(&gsi, call, true);
                    lowered_count++;
                    if (opt_trace)
                        inform(gimple_location(stmt), "ternary: lowered conditional operator");
                    continue;
                }

                // Lower operations on ternary types
                if (is_gimple_assign(stmt) && opt_lower) {
                    const unsigned num_ops = gimple_num_ops(stmt);
                    if (num_ops > 1) {
                        tree rhs1 = gimple_assign_rhs1(stmt);
                        tree lowered_rhs1 = lower_cond_expr_in_tree(rhs1, &gsi);
                        if (lowered_rhs1 != rhs1)
                            gimple_assign_set_rhs1(stmt, lowered_rhs1);
                    }
                    if (num_ops > 2) {
                        tree rhs2 = gimple_assign_rhs2(stmt);
                        tree lowered_rhs2 = lower_cond_expr_in_tree(rhs2, &gsi);
                        if (lowered_rhs2 != rhs2)
                            gimple_assign_set_rhs2(stmt, lowered_rhs2);
                    }
                    if (num_ops > 3) {
                        tree rhs3 = gimple_assign_rhs3(stmt);
                        tree lowered_rhs3 = lower_cond_expr_in_tree(rhs3, &gsi);
                        if (lowered_rhs3 != rhs3)
                            gimple_assign_set_rhs3(stmt, lowered_rhs3);
                    }
                    tree lhs = gimple_assign_lhs(stmt);
                    tree lhs_type = TREE_TYPE(lhs);
                    unsigned trit_count;
                    if (get_ternary_type_trits(lhs_type, &trit_count)) {
                        enum tree_code code = gimple_assign_rhs_code(stmt);
                        tree arg1 = gimple_assign_rhs1(stmt);
                        tree arg2 = gimple_assign_rhs2(stmt);
                        tree packed = NULL_TREE;
                        int64_t arg1_logical = 0;
                        int64_t arg2_logical = 0;
                        bool arg1_const = false;
                        bool arg2_const = false;

                        if (arg1 && TREE_CODE(arg1) == INTEGER_CST &&
                            ternary_pack_constant(arg1, TREE_TYPE(arg1), &packed))
                            arg1 = packed;
                        if (arg2 && TREE_CODE(arg2) == INTEGER_CST &&
                            ternary_pack_constant(arg2, TREE_TYPE(arg2), &packed))
                            arg2 = packed;

                        if (arg1 && TREE_CODE(arg1) == INTEGER_CST &&
                            ternary_unpack_constant(arg1, lhs_type, &arg1_logical))
                            arg1_const = true;
                        if (arg2 && TREE_CODE(arg2) == INTEGER_CST &&
                            ternary_unpack_constant(arg2, lhs_type, &arg2_logical))
                            arg2_const = true;

                        if (code == CONVERT_EXPR && arg1 && TREE_CODE(arg1) == INTEGER_CST) {
                            tree source_type = TREE_TYPE(arg1);
                            if (INTEGRAL_TYPE_P(source_type) && tree_fits_shwi_p(arg1)) {
                                tree logical_tree = build_int_cst_type(long_long_integer_type_node, tree_to_shwi(arg1));
                                if (ternary_pack_constant(logical_tree, lhs_type, &packed)) {
                                    gimple_assign_set_rhs_from_tree(&gsi, packed);
                                    lowered_count++;
                                    if (opt_trace)
                                        inform(gimple_location(stmt), "ternary: folded constant conversion to ternary type");
                                    continue;
                                }
                            }
                        }

                        // Check for mixed-type operations
                        if (arg1 && TREE_TYPE(arg1) != lhs_type) {
                            unsigned arg1_trits = 0;
                            if (!get_ternary_type_trits(TREE_TYPE(arg1), &arg1_trits) || arg1_trits != trit_count) {
                                if (opt_warn)
                                    warning_at(gimple_location(stmt), 0, "ternary: mixed-type operation %s (lhs: ternary %u trits, arg1: %s) - conversion needed", 
                                               get_tree_code_name(code), trit_count, INTEGRAL_TYPE_P(TREE_TYPE(arg1)) ? "integer" : "other");
                                continue;
                            }
                        }
                        if (arg2 && TREE_TYPE(arg2) != lhs_type) {
                            unsigned arg2_trits = 0;
                            if (!get_ternary_type_trits(TREE_TYPE(arg2), &arg2_trits) || arg2_trits != trit_count) {
                                if (opt_warn)
                                    warning_at(gimple_location(stmt), 0, "ternary: mixed-type operation %s (lhs: ternary %u trits, arg2: %s) - conversion needed", 
                                               get_tree_code_name(code), trit_count, INTEGRAL_TYPE_P(TREE_TYPE(arg2)) ? "integer" : "other");
                                continue;
                            }
                        }

                        // Constant folding on ternary values
                        if (code == NEGATE_EXPR && arg1_const) {
                            tree logical_tree = build_int_cst_type(long_long_integer_type_node, -arg1_logical);
                            if (ternary_pack_constant(logical_tree, lhs_type, &packed)) {
                                gimple_assign_set_rhs_from_tree(&gsi, packed);
                                lowered_count++;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: folded constant negate");
                                continue;
                            }
                        } else if (arg1_const && arg2_const) {
                            int64_t folded_val = 0;
                            bool can_fold = true;
                            if (code == PLUS_EXPR)
                                folded_val = arg1_logical + arg2_logical;
                            else if (code == MINUS_EXPR)
                                folded_val = arg1_logical - arg2_logical;
                            else if (code == MULT_EXPR)
                                folded_val = arg1_logical * arg2_logical;
                            else if (code == TRUNC_DIV_EXPR)
                                folded_val = (arg2_logical == 0) ? 0 : (arg1_logical / arg2_logical);
                            else if (code == TRUNC_MOD_EXPR)
                                folded_val = (arg2_logical == 0) ? 0 : (arg1_logical % arg2_logical);
                            else
                                can_fold = false;

                            if (can_fold) {
                                tree logical_tree = build_int_cst_type(long_long_integer_type_node, folded_val);
                                if (ternary_pack_constant(logical_tree, lhs_type, &packed)) {
                                    gimple_assign_set_rhs_from_tree(&gsi, packed);
                                    lowered_count++;
                                    if (opt_trace)
                                        inform(gimple_location(stmt), "ternary: folded constant %s", get_tree_code_name(code));
                                    continue;
                                }
                            }
                        }

                        // Simplifications
                        bool simplified = false;
                        if (code == PLUS_EXPR && arg2_const && arg2_logical == 0) {
                            // a + 0 = a
                            gimple_assign_set_rhs_from_tree(&gsi, arg1);
                            simplified = true;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: simplified a + 0 to a");
                        } else if (code == MINUS_EXPR && arg2_const && arg2_logical == 0) {
                            // a - 0 = a
                            gimple_assign_set_rhs_from_tree(&gsi, arg1);
                            simplified = true;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: simplified a - 0 to a");
                        } else if (code == MULT_EXPR && arg2_const) {
                            if (arg2_logical == 0) {
                                // a * 0 = 0
                                gimple_assign_set_rhs_from_tree(&gsi, arg2);
                                simplified = true;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: simplified a * 0 to 0");
                            } else if (arg2_logical == 1) {
                                // a * 1 = a
                                gimple_assign_set_rhs_from_tree(&gsi, arg1);
                                simplified = true;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: simplified a * 1 to a");
                            }
                        }

                        if (simplified) {
                            lowered_count++;
                            continue;
                        }

                        const char *helper_name = nullptr;
                        bool is_shift = false;
                        if (code == PLUS_EXPR) {
                            helper_name = "add";
                        } else if (code == MINUS_EXPR) {
                            helper_name = "sub";
                        } else if (code == MULT_EXPR) {
                            helper_name = "mul";
                        } else if (code == TRUNC_DIV_EXPR) {
                            helper_name = "div";
                        } else if (code == TRUNC_MOD_EXPR) {
                            helper_name = "mod";
                        } else if (code == NEGATE_EXPR) {
                            helper_name = "neg";
                        } else if (code == BIT_NOT_EXPR) {
                            helper_name = "not";
                        } else if (code == BIT_AND_EXPR) {
                            helper_name = "and";
                        } else if (code == BIT_IOR_EXPR) {
                            helper_name = "or";
                        } else if (code == BIT_XOR_EXPR) {
                            helper_name = "xor";
                        } else if (code == LSHIFT_EXPR) {
                            helper_name = "shl";
                            is_shift = true;
                        } else if (code == RSHIFT_EXPR) {
                            helper_name = "shr";
                            is_shift = true;
                        } else if (code == CONVERT_EXPR) {
                            tree source = gimple_assign_rhs1(stmt);
                            tree source_type = TREE_TYPE(source);
                            if (INTEGRAL_TYPE_P(source_type)) {
                                helper_name = "tb2t";
                            } else if (SCALAR_FLOAT_TYPE_P(source_type)) {
                                if (TYPE_PRECISION(source_type) == 32) {
                                    helper_name = "f2t32";
                                } else if (TYPE_PRECISION(source_type) == 64) {
                                    helper_name = "f2t64";
                                }
                            }
                        }
                        if (helper_name) {
                            tree decl;
                            if (is_shift) {
                                decl = get_shift_decl(helper_name, lhs_type);
                            } else if (code == CONVERT_EXPR) {
                                tree source_type = TREE_TYPE(gimple_assign_rhs1(stmt));
                                decl = get_conv_to_ternary_decl(helper_name, lhs_type, source_type);
                            } else {
                                decl = get_arith_decl(helper_name, lhs_type);
                            }
                            if (decl) {
                                int num_args = (code == NEGATE_EXPR || code == BIT_NOT_EXPR ||
                                                code == CONVERT_EXPR) ? 1 : 2;
                                tree arg1 = gimple_assign_rhs1(stmt);
                                tree arg2 = (num_args == 2) ? gimple_assign_rhs2(stmt) : NULL_TREE;
                                gcall *call = gimple_build_call(decl, num_args, arg1, arg2);
                                gimple_call_set_lhs(call, lhs);
                                gsi_replace(&gsi, call, true);
                                lowered_count++;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: lowered %s on ternary type", get_tree_code_name(code));
                            } else {
                                surviving_count++;
                                if (opt_warn)
                                    warning_at(gimple_location(stmt), 0, "ternary: cannot lower %s on ternary type (missing runtime helper for %u trits)", get_tree_code_name(code), trit_count);
                            }
                        } else {
                            surviving_count++;
                            if (opt_warn)
                                warning_at(gimple_location(stmt), 0, "ternary: unsupported operation %s on ternary type (operation not implemented)", get_tree_code_name(code));
                        }
                    }
                }

                // Lower comparisons on ternary operands
                if (is_gimple_assign(stmt) && opt_lower) {
                    enum tree_code code = gimple_assign_rhs_code(stmt);
                    if (code == EQ_EXPR || code == NE_EXPR || code == LT_EXPR || code == LE_EXPR || code == GT_EXPR || code == GE_EXPR) {
                        tree arg1 = gimple_assign_rhs1(stmt);
                        tree arg2 = gimple_assign_rhs2(stmt);
                        tree cmp_tmp = build_cmp_call(arg1, arg2, &gsi);
                        if (cmp_tmp) {
                            tree zero = build_int_cst(integer_type_node, 0);
                            gimple_assign_set_rhs_with_ops(&gsi, code, cmp_tmp, zero, NULL_TREE);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered %s via ternary cmp", get_tree_code_name(code));
                        }
                    }
                }

                // Lower conversions from ternary types
                if (is_gimple_assign(stmt) && opt_lower && gimple_assign_rhs_code(stmt) == CONVERT_EXPR) {
                    tree lhs = gimple_assign_lhs(stmt);
                    tree lhs_type = TREE_TYPE(lhs);
                    if (!get_ternary_type_trits(lhs_type, nullptr)) {  // lhs not ternary
                        tree source = gimple_assign_rhs1(stmt);
                        tree source_type = TREE_TYPE(source);
                        unsigned trit_count;
                        if (get_ternary_type_trits(source_type, &trit_count)) {
                            const char *helper_name = nullptr;
                            if (INTEGRAL_TYPE_P(lhs_type)) {
                                helper_name = "tt2b";
                            } else if (SCALAR_FLOAT_TYPE_P(lhs_type)) {
                                if (TYPE_PRECISION(lhs_type) == 32) {
                                    helper_name = "t2f32";
                                } else if (TYPE_PRECISION(lhs_type) == 64) {
                                    helper_name = "t2f64";
                                }
                            }
                            if (helper_name) {
                                tree decl = get_conv_from_ternary_decl(helper_name, lhs_type, source_type);
                                if (decl) {
                                    gcall *call = gimple_build_call(decl, 1, source);
                                    gimple_call_set_lhs(call, lhs);
                                    gsi_replace(&gsi, call, true);
                                    lowered_count++;
                                    if (opt_trace)
                                        inform(gimple_location(stmt), "ternary: lowered conversion from ternary type");
                                } else {
                                    surviving_count++;
                                    if (opt_warn)
                                        warning_at(gimple_location(stmt), 0, "ternary: cannot lower conversion from ternary type to %s (missing runtime helper)", get_tree_code_name(TREE_CODE(lhs_type)));
                                }
                            }
                        }
                    }
                }

                if (is_gimple_call(stmt) && (opt_arith || opt_logic || opt_cmp || opt_shift || opt_conv || opt_mem))
                {
                    tree fndecl = gimple_call_fndecl(stmt);
                    tree lhs = gimple_call_lhs(stmt);
                    if (!fndecl || !lhs)
                        continue;

                    const char *name = IDENTIFIER_POINTER(DECL_NAME(fndecl));
                    tree lhs_type = TREE_TYPE(lhs);

                    if (opt_arith && !strcmp(name, "__builtin_ternary_add"))
                    {
                        tree decl = get_arith_decl("add", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_add");
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_mul"))
                    {
                        tree decl = get_arith_decl("mul", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_mul");
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_sub"))
                    {
                        tree decl = get_arith_decl("sub", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_sub");
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_div"))
                    {
                        tree decl = get_arith_decl("div", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_div");
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_mod"))
                    {
                        tree decl = get_arith_decl("mod", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_mod");
                        }
                    }
                    else if (opt_arith && !strcmp(name, "__builtin_ternary_neg"))
                    {
                        if (INTEGRAL_TYPE_P(lhs_type) && gimple_call_num_args(stmt) == 1)
                        {
                            maybe_dump_stmt(stmt);
                            tree fn_type = build_function_type_list(lhs_type, lhs_type, NULL_TREE);
                            std::string helper = build_helper_name("neg");
                            tree decl = build_fn_decl(helper.c_str(), fn_type);
                            TREE_PUBLIC(decl) = 1;
                            DECL_EXTERNAL(decl) = 1;
                            DECL_ARTIFICIAL(decl) = 1;

                            gcall *new_call = gimple_build_call(decl, 1, gimple_call_arg(stmt, 0));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_neg");
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
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2, arg0, arg1);
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_cmp");
                        }
                    }
                    else if (opt_cmp && (!strcmp(name, "__builtin_ternary_eq") ||
                                         !strcmp(name, "__builtin_ternary_ne") ||
                                         !strcmp(name, "__builtin_ternary_lt") ||
                                         !strcmp(name, "__builtin_ternary_le") ||
                                         !strcmp(name, "__builtin_ternary_gt") ||
                                         !strcmp(name, "__builtin_ternary_ge")))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree arg1 = gimple_call_arg(stmt, 1);
                        tree arg0_type = TREE_TYPE(arg0);
                        unsigned trit_count;
                        if (get_ternary_type_trits(arg0_type, &trit_count) && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            tree fn_type = build_function_type_list(integer_type_node, arg0_type, arg0_type, NULL_TREE);
                            const char *base_name = nullptr;
                            if (!strcmp(name, "__builtin_ternary_eq")) base_name = "eq";
                            else if (!strcmp(name, "__builtin_ternary_ne")) base_name = "ne";
                            else if (!strcmp(name, "__builtin_ternary_lt")) base_name = "lt";
                            else if (!strcmp(name, "__builtin_ternary_le")) base_name = "le";
                            else if (!strcmp(name, "__builtin_ternary_gt")) base_name = "gt";
                            else if (!strcmp(name, "__builtin_ternary_ge")) base_name = "ge";
                            if (base_name) {
                                std::string helper = build_helper_name(base_name);
                                tree decl = build_fn_decl(helper.c_str(), fn_type);
                                TREE_PUBLIC(decl) = 1;
                                DECL_EXTERNAL(decl) = 1;
                                DECL_ARTIFICIAL(decl) = 1;
                                gcall *new_call = gimple_build_call(decl, 2, arg0, arg1);
                                gimple_call_set_lhs(new_call, lhs);
                                gsi_replace(&gsi, new_call, true);
                                lowered_count++;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: lowered builtin %s", name);
                            }
                        }
                    }
                    else if (opt_cmp && (!strcmp(name, "__builtin_ternary_cmplt") ||
                                         !strcmp(name, "__builtin_ternary_cmpeq") ||
                                         !strcmp(name, "__builtin_ternary_cmpgt") ||
                                         !strcmp(name, "__builtin_ternary_cmpneq") ||
                                         !strcmp(name, "__builtin_ternary_cmplt_t64") ||
                                         !strcmp(name, "__builtin_ternary_cmpeq_t64") ||
                                         !strcmp(name, "__builtin_ternary_cmpgt_t64") ||
                                         !strcmp(name, "__builtin_ternary_cmpneq_t64")))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree arg1 = gimple_call_arg(stmt, 1);
                        tree arg0_type = TREE_TYPE(arg0);
                        const char *base_name = nullptr;
                        if (!strcmp(name, "__builtin_ternary_cmplt") || !strcmp(name, "__builtin_ternary_cmplt_t64"))
                            base_name = "cmplt";
                        else if (!strcmp(name, "__builtin_ternary_cmpeq") || !strcmp(name, "__builtin_ternary_cmpeq_t64"))
                            base_name = "cmpeq";
                        else if (!strcmp(name, "__builtin_ternary_cmpgt") || !strcmp(name, "__builtin_ternary_cmpgt_t64"))
                            base_name = "cmpgt";
                        else if (!strcmp(name, "__builtin_ternary_cmpneq") || !strcmp(name, "__builtin_ternary_cmpneq_t64"))
                            base_name = "cmpneq";
                        
                        if (base_name && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            tree decl = get_ternary_cmp_decl(base_name, lhs_type);
                            if (decl)
                            {
                                gcall *new_call = gimple_build_call(decl, 2, arg0, arg1);
                                gimple_call_set_lhs(new_call, lhs);
                                gsi_replace(&gsi, new_call, true);
                                lowered_count++;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: lowered builtin %s", name);
                            }
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_not"))
                    {
                        if (INTEGRAL_TYPE_P(lhs_type) && gimple_call_num_args(stmt) == 1)
                        {
                            maybe_dump_stmt(stmt);
                            tree fn_type = build_function_type_list(lhs_type, lhs_type, NULL_TREE);
                            std::string helper = build_helper_name("not");
                            tree decl = build_fn_decl(helper.c_str(), fn_type);
                            TREE_PUBLIC(decl) = 1;
                            DECL_EXTERNAL(decl) = 1;
                            DECL_ARTIFICIAL(decl) = 1;

                            gcall *new_call = gimple_build_call(decl, 1, gimple_call_arg(stmt, 0));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_not");
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_and"))
                    {
                        tree decl = get_arith_decl("and", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_and");
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_or"))
                    {
                        tree decl = get_arith_decl("or", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_or");
                        }
                    }
                    else if (opt_logic && !strcmp(name, "__builtin_ternary_xor"))
                    {
                        tree decl = get_arith_decl("xor", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_xor");
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
                                maybe_dump_stmt(stmt);
                                gcall *new_call = gimple_build_call(decl, 3, cond, true_val, false_val);
                                gimple_call_set_lhs(new_call, lhs);
                                gsi_replace(&gsi, new_call, true);
                                lowered_count++;
                                if (opt_trace)
                                    inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_select");
                            }
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_shl"))
                    {
                        tree decl = get_shift_decl("shl", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_shl");
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_shr"))
                    {
                        tree decl = get_shift_decl("shr", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_shr");
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_rol"))
                    {
                        tree decl = get_shift_decl("rol", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_rol");
                        }
                    }
                    else if (opt_shift && !strcmp(name, "__builtin_ternary_ror"))
                    {
                        tree decl = get_shift_decl("ror", lhs_type);
                        if (decl && gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 2,
                                                                gimple_call_arg(stmt, 0),
                                                                gimple_call_arg(stmt, 1));
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_ror");
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_tb2t"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree decl = get_conv_to_ternary_decl("tb2t", lhs_type, TREE_TYPE(arg0));
                        if (decl && gimple_call_num_args(stmt) == 1)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 1, arg0);
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_tb2t");
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_tt2b"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        tree decl = get_conv_from_ternary_decl("tt2b", lhs_type, TREE_TYPE(arg0));
                        if (decl && gimple_call_num_args(stmt) == 1)
                        {
                            maybe_dump_stmt(stmt);
                            gcall *new_call = gimple_build_call(decl, 1, arg0);
                            gimple_call_set_lhs(new_call, lhs);
                            gsi_replace(&gsi, new_call, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_tt2b");
                        }
                    }
                    else if (opt_conv && !strcmp(name, "__builtin_ternary_t2f"))
                    {
                        tree arg0 = gimple_call_arg(stmt, 0);
                        if (SCALAR_FLOAT_TYPE_P(lhs_type) && gimple_call_num_args(stmt) == 1)
                        {
                            const unsigned precision = TYPE_PRECISION(lhs_type);
                            const char *conv_name = (precision == 32) ? "t2f32" :
                                                    (precision == 64) ? "t2f64" : NULL;
                            if (conv_name)
                            {
                                tree decl = get_conv_from_ternary_decl(conv_name, lhs_type, TREE_TYPE(arg0));
                                if (decl)
                                {
                                    maybe_dump_stmt(stmt);
                                    gcall *new_call = gimple_build_call(decl, 1, arg0);
                                    gimple_call_set_lhs(new_call, lhs);
                                    gsi_replace(&gsi, new_call, true);
                                    lowered_count++;
                                    if (opt_trace)
                                        inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_t2f");
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
                            const char *conv_name = (precision == 32) ? "f2t32" :
                                                    (precision == 64) ? "f2t64" : NULL;
                            if (conv_name)
                            {
                                tree decl = get_conv_to_ternary_decl(conv_name, lhs_type, TREE_TYPE(arg0));
                                if (decl)
                                {
                                    maybe_dump_stmt(stmt);
                                    gcall *new_call = gimple_build_call(decl, 1, arg0);
                                    gimple_call_set_lhs(new_call, lhs);
                                    gsi_replace(&gsi, new_call, true);
                                    lowered_count++;
                                    if (opt_trace)
                                        inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_f2t");
                                }
                            }
                        }
                    }
                }

                // Lower memory operations
                if (is_gimple_call(stmt) && opt_mem)
                {
                    const char *name = IDENTIFIER_POINTER(DECL_NAME(gimple_call_fndecl(stmt)));
                    if (!strcmp(name, "__builtin_ternary_load_t32"))
                    {
                        if (gimple_call_num_args(stmt) == 1)
                        {
                            maybe_dump_stmt(stmt);
                            tree arg0 = gimple_call_arg(stmt, 0);
                            tree load_expr = build2(MEM_REF, lhs_type, arg0, build_int_cst(ptr_type_node, 0));
                            gimple_assign_set_rhs_from_tree(&gsi, load_expr);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_load_t32");
                        }
                    }
                    else if (!strcmp(name, "__builtin_ternary_store_t32"))
                    {
                        if (gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            tree arg0 = gimple_call_arg(stmt, 0); // addr
                            tree arg1 = gimple_call_arg(stmt, 1); // value
                            tree store_expr = build2(MEM_REF, TREE_TYPE(arg1), arg0, build_int_cst(ptr_type_node, 0));
                            gimple *store_stmt = gimple_build_assign(store_expr, arg1);
                            gsi_insert_before(&gsi, store_stmt, GSI_SAME_STMT);
                            gsi_remove(&gsi, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_store_t32");
                        }
                    }
                    else if (!strcmp(name, "__builtin_ternary_load_t64"))
                    {
                        if (gimple_call_num_args(stmt) == 1)
                        {
                            maybe_dump_stmt(stmt);
                            tree arg0 = gimple_call_arg(stmt, 0);
                            tree load_expr = build2(MEM_REF, lhs_type, arg0, build_int_cst(ptr_type_node, 0));
                            gimple_assign_set_rhs_from_tree(&gsi, load_expr);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_load_t64");
                        }
                    }
                    else if (!strcmp(name, "__builtin_ternary_store_t64"))
                    {
                        if (gimple_call_num_args(stmt) == 2)
                        {
                            maybe_dump_stmt(stmt);
                            tree arg0 = gimple_call_arg(stmt, 0); // addr
                            tree arg1 = gimple_call_arg(stmt, 1); // value
                            tree store_expr = build2(MEM_REF, TREE_TYPE(arg1), arg0, build_int_cst(ptr_type_node, 0));
                            gimple *store_stmt = gimple_build_assign(store_expr, arg1);
                            gsi_insert_before(&gsi, store_stmt, GSI_SAME_STMT);
                            gsi_remove(&gsi, true);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered builtin __builtin_ternary_store_t64");
                        }
                    }
                }

                if (is_gimple_call(stmt) && opt_lower)
                {
                    int nargs = gimple_call_num_args(stmt);
                    for (int i = 0; i < nargs; ++i) {
                        tree arg = gimple_call_arg(stmt, i);
                        if (arg) {
                            tree lowered = lower_cond_expr_in_tree(arg, &gsi);
                            if (lowered != arg)
                                gimple_call_set_arg(stmt, i, lowered);
                        }
                    }
                }

                if (is_gimple_cond(stmt) && opt_lower)
                {
                    tree lhs = gimple_cond_lhs(stmt);
                    tree rhs = gimple_cond_rhs(stmt);
                    enum tree_code code = gimple_cond_code(stmt);
                    if (code == EQ_EXPR || code == NE_EXPR || code == LT_EXPR || code == LE_EXPR ||
                        code == GT_EXPR || code == GE_EXPR) {
                        tree cmp_tmp = build_cmp_call(lhs, rhs, &gsi);
                        if (cmp_tmp) {
                            tree zero = build_int_cst(integer_type_node, 0);
                            gimple_cond_set_lhs(stmt, cmp_tmp);
                            gimple_cond_set_rhs(stmt, zero);
                            lowered_count++;
                            if (opt_trace)
                                inform(gimple_location(stmt), "ternary: lowered branch %s via ternary cmp", get_tree_code_name(code));
                            continue;
                        }
                    }
                    if (lhs) {
                        tree lowered = lower_cond_expr_in_tree(lhs, &gsi);
                        if (lowered != lhs)
                            gimple_cond_set_lhs(stmt, lowered);
                    }
                    if (rhs) {
                        tree lowered = lower_cond_expr_in_tree(rhs, &gsi);
                        if (lowered != rhs)
                            gimple_cond_set_rhs(stmt, lowered);
                    }
                }

                if (is_gimple_return(stmt) && opt_lower)
                {
                    tree retval = gimple_return_retval(stmt);
                    if (retval) {
                        tree lowered = lower_cond_expr_in_tree(retval, &gsi);
                        if (lowered != retval)
                            gimple_return_set_retval(stmt, lowered);
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
        else if (!strcmp(key, "mem"))
            opt_mem = true;
        else if (!strcmp(key, "types"))
            opt_types = true;
        else if (!strcmp(key, "vector"))
            opt_vector = true;
        else if (!strcmp(key, "version"))
            opt_version = true;
        else if (!strcmp(key, "selftest"))
            opt_selftest = true;
        else if (!strcmp(key, "trace"))
            opt_trace = true;
        else if (!strcmp(key, "dump-gimple"))
            opt_dump_gimple = true;
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

    create_ternary_type(32);
    create_ternary_type(64);
    create_ternary_type(128);
    
    // Create vector types if vector operations are enabled
    if (opt_vector) {
        create_ternary_vector_type(32);  // tv32_t
        create_ternary_vector_type(64);  // tv64_t
        create_ternary_vector_type(128); // tv128_t
    }
}

static void ternary_plugin_finish(void *, void *)
{
    if (opt_stats)
        inform(UNKNOWN_LOCATION, "ternary plugin: %lu ternary ops, %lu lowered, %lu surviving gimple ops",
               ternary_count, lowered_count, surviving_count);
}

extern "C" int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version)
{
    if (!plugin_default_version_check(version, &gcc_version))
        return 1;

    if (!check_gcc_version_compatibility(version))
    {
        error(0, "ternary plugin: unsupported GCC version %s (supported: 9.0-15.x)", version->basever);
        return 1;
    }

    parse_args(plugin_info);
    if (opt_version || opt_selftest)
    {
        std::string plugin_version = read_version_file();
        if (opt_version)
            inform(UNKNOWN_LOCATION, "ternary plugin version %s (GCC %s)",
                   plugin_version.c_str(), version->basever);
        if (opt_selftest)
        {
            inform(UNKNOWN_LOCATION,
                   "ternary plugin selftest: ok (lower=%s arith=%s logic=%s cmp=%s shift=%s conv=%s types=%s)",
                   opt_lower ? "on" : "off",
                   opt_arith ? "on" : "off",
                   opt_logic ? "on" : "off",
                   opt_cmp ? "on" : "off",
                   opt_shift ? "on" : "off",
                   opt_conv ? "on" : "off",
                   opt_types ? "on" : "off");
            run_selftest();
        }
    }

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
