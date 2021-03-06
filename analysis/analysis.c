#include "../headers/global.h"
#include "../headers/indicator.h"
#include "../headers/print.h"
#include "errno.h"
/* Build up multi-level directories */
void _mkdir (const char *dir)
{
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf (tmp, sizeof(tmp), "%s", dir);
    len = strlen (tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir (tmp, S_IRWXU);
            *p = '/';
        }
    }
   mkdir (tmp, S_IRWXU);
}


extern void track_evolution (SMRT_individual *pop, int generation, int end)
{
    char id_char[10];
    char output_dir_level1[BUFSIZE_L];    // upper level directory
    char output_dir_level2[BUFSIZE_L];    // lower level directory
    char output_file[BUFSIZE_L];
    int n = 0;
    sprintf (id_char, "%d", generation);
    // set the output directory
    sprintf (output_dir_level1, "../out/%s_M%d_D%d/%s/",
             g_problem_name_str[g_algorithm_entity.testProblem],
             g_algorithm_entity.algorithm_para.objective_number,
             g_algorithm_entity.algorithm_para.variable_number,
             g_algorithm_name_str[g_algorithm_entity.algorithm_Name]
    );
    sprintf (output_dir_level2, "../out/%s_M%d_D%d/%s/%d/",
             g_problem_name_str[g_algorithm_entity.testProblem],
             g_algorithm_entity.algorithm_para.objective_number,
             g_algorithm_entity.algorithm_para.variable_number,
             g_algorithm_name_str[g_algorithm_entity.algorithm_Name],
             g_algorithm_entity.run_index_current
    );

    _mkdir (output_dir_level2);


    /*
    // first time output, init the parameter and output var and fun
    if (generation == 1)
    {
        // set analyse list
        for (i = 0; i < BUFSIZE_S; i++)
            analyse_list[i] = 0;

        read_ptr = 0;
        while (1)
        {
            int name_c = 0;
            while (analyse_stream[read_ptr] != ' '
                   && analyse_stream[read_ptr] != '\t'
                   && analyse_stream[read_ptr] != '\n'
                   && analyse_stream[read_ptr] != 0)
            {
                name[name_c] = analyse_stream[read_ptr];
                name_c++;
                read_ptr++;
            }
            if (analyse_stream[read_ptr] == 0)
                name[name_c] = 0;
            name[name_c] = 0;

            if (!strcmp(name, "VAR"))
                analyse_list[VAR] = 1;
            else if (!strcmp(name, "FUN"))
                analyse_list[FUN] = 1;
            else if (!strcmp(name, "GD"))
                analyse_list[GD] = 1;
            else if (!strcmp(name, "IGD"))
                analyse_list[IGD] = 1;
            else if (!strcmp(name, "HV"))
                analyse_list[HV] = 1;
            else if (!strcmp(name, "PLOT"))
                analyse_list[PLOT] = 1;
            else if (!strcmp(name, "analyse:"))
                ;
            else
                print_error(1,2,"unknown setting for analyse ",name);

            if (analyse_stream[read_ptr] == 0)
                break;
            read_ptr++;
        }
    }
     */

    if (g_algorithm_entity.algorithm_para.runtime_output == 1 && (generation % g_algorithm_entity.algorithm_para.output_interval == 0 || generation == 1 || end == 1))
    {
        switch (g_algorithm_entity.analyse_Type)
        {
            case VAR:
                sprintf (output_file, "%smedium_VAR_%s.out", output_dir_level2, id_char);
                print_variable (output_file, pop);
                break;
            case FUN:
                sprintf (output_file, "%smedium_FUN_%s.out", output_dir_level2, id_char);
                print_objective (output_file, pop);
                break;
            case GD:
                record_GD (pop, generation);
                break;
            case IGD:
                record_IGD (pop, generation);
                break;
            case HV:
                record_HV (pop,generation);
                break;
            case PLOT:
                //py_plot(ptr,id);
                break;
            default:
                printf("unknown setting for analyse \n");
                break;
        }
    }

    if (end == 1)
    {
        switch (g_algorithm_entity.analyse_Type)
        {
            case VAR:
                sprintf (output_file, "%sVAR%d.out", output_dir_level1, g_algorithm_entity.run_index_current);
                print_variable (output_file, pop);
                break;
            case FUN:
                sprintf (output_file, "%sFUN%d.out", output_dir_level1, g_algorithm_entity.run_index_current);
                print_objective (output_file, pop);
                break;
            case GD:
                sprintf (output_file, "%sGD_%d.txt", output_dir_level2, g_algorithm_entity.run_index_current);
                print_GD(output_file);
                break;
            case IGD:
                sprintf (output_file, "%sIGD_%d.txt", output_dir_level2, g_algorithm_entity.run_index_current);
                print_IGD(output_file);
                break;
            case HV:
                sprintf (output_file, "%sHV_%d.txt", output_dir_level2, g_algorithm_entity.run_index_current);
                print_hv(output_file);
                break;
            case PLOT:
               // py_plot(NULL,0);
                sprintf (output_file, "%sFUN%d.out", output_dir_level1, g_algorithm_entity.run_index_current);
                //gnu_plot(output_file, "FUN");
                break;
            default:
                break;
        }
    }

    return;
}