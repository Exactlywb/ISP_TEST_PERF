/**
 * @file c3-ipa.cpp
 */

/*
 * This plugin is a truncated version of Martin Liska's c3-pass
 * The original sources you may see on
 * https://gcc.gnu.org/legacy-ml/gcc-patches/2019-09/msg01142.html
 * */

#include <iostream>

// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "cgraph.h"
#include "symbol-summary.h"
#include "tree-vrp.h"
#include "alloc-pool.h"
#include "ipa-prop.h"
#include "ipa-fnsummary.h"
#include "fibonacci_heap.h"

int plugin_is_GPL_compatible; ///<To prove our dedication to free software

static struct plugin_info C3_plugin_info = {"1.0", "C3 gcc plugin"};

namespace {


    static int func_cmp (const void *a_p, const void *b_p)
    {
        cgraph_node *a = *(cgraph_node **)a_p;
        cgraph_node *b = *(cgraph_node **)b_p;
        return strcmp(a->name(), b->name());
    }

    /**
     * @brief c3 reorder itself
     * @return error code
     */
    static unsigned int c3_reorder ()
    {
        FILE * fp;
        fp = fopen ("/home/exactlywb/Desktop/ISP_RAS/c3_reorder_plugin/PASS_WORK.txt", "w+"); //PLEASE, CHANGE IT
        cgraph_node *node;
        auto_vec<cgraph_node *> functions;

        fprintf(fp, "Start c3_ipa plugin\n");
        /* Create a cluster for each function.  */
        FOR_EACH_DEFINED_FUNCTION (node) {
          if(node == nullptr) continue;
          if (!node->alias && !node->global.inlined_to)
          {
              //fprintf(fp, "func: %s\n", node->name());
              functions.safe_push (node);
          }
        }

        /* Sort the candidate clusters.  */
        functions.qsort (func_cmp);
        for (int i = 0; i < functions.length(); i++) {
          functions[i]->text_sorted_order = i;
          fprintf(fp, "[%d] %s\n", i, functions[i]->name());
        }

        fclose(fp);

        return 0;
    }

    ///passage info
    const pass_data pass_data_ipa_reorder =
    {
        IPA_PASS, /* type */
        "c3_reorder", /* name */
        OPTGROUP_NONE, /* optinfo_flags */
        TV_NONE, /* tv_id */
        0, /* properties_required */
        0, /* properties_provided */
        0, /* properties_destroyed */
        0, /* todo_flags_start */
        0, /* todo_flags_finish */
    };

    /**
     * @brief the executable passage
     */
    struct c3_pass: public ipa_opt_pass_d
    {
        c3_pass (gcc::context *ctxt):
            ipa_opt_pass_d (pass_data_ipa_reorder, ctxt,
            		        NULL, /* generate_summary */
 		                    NULL, /* write_summary */
                            NULL, /* read_summary */
                            NULL, /* write_optimization_summary */
                            NULL, /* read_optimization_summary */
                            NULL, /* stmt_fixup */
                            0, /* function_transform_todo_flags_start */
                            NULL, /* function_transform */
                            NULL  /* variable_transform */)
        {}

        /**
         * @brief The execute method
         * @warning do not call it by yourself
         */
        virtual unsigned int execute (function *) { return c3_reorder (); }
        /**
         * @brief This function is responsible for when the passage
         * should work and when not
         */
        virtual bool gate (function *);

    };

    bool c3_pass::gate (function *)
    {
        //return flag_profile_reorder_functions && flag_profile_use && flag_wpa;
        return flag_reorder_functions && flag_wpa;
    }

}

/**
 * @brief The main plugin function hooking the desired passage
 * @param[in] plugin_info Plugin invocation information
 * @param[in] version GCC version
 * @return error code
 */
int plugin_init (struct plugin_name_args *plugin_info,
	        	 struct plugin_gcc_version *version)
{

    if (!plugin_default_version_check (version, &gcc_version))
    {
        std::cerr << "Bad gcc version to compile the plugin" << std::endl;
        return 1;
    }

    register_callback(plugin_info->base_name, PLUGIN_INFO, NULL, 
                      &C3_plugin_info);

    struct register_pass_info pass_info;

    pass_info.pass = new c3_pass (g); // "g" is a global gcc::context pointer
    pass_info.reference_pass_name       = "pure-const";
    pass_info.ref_pass_instance_number  = 1;
    pass_info.pos_op                    = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP,
                       NULL, &pass_info);

    return 0;
}

