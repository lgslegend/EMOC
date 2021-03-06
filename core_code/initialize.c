#include "../headers/global.h"
#include "../headers/print.h"
#include "../headers/memory.h"
#include "../headers/random.h"

static int parameter_check()
{
    if (g_algorithm_entity.algorithm_para.pop_size <= 50 || g_algorithm_entity.algorithm_para.pop_size % 4 != 0)
    {
        printf("Population size is too small or Not a multiple of 4 \n");
        return FAIL;
    }

    return SUCCESS;


}

static void formalize_str(char *buff)
{
    int i = 0, j = 0, len = 0;

    if (NULL == buff)
    {
        return;
    }

    len = strlen(buff);

    for (i = 0; i < len; i++)
    {
        switch (buff[i])
        {
            case ' ':
            case '\n':
                for (j = i; j < len; j++)
                {
                    buff[j] = buff[j+1];
                }
                break;
            case '\r':
                buff[i] = 0;
                break;
            default:
                break;
        }
    }
    return;
}

static void set_algorithm_name(const char *algorithm_name)
{
    int i = 0;
    if (algorithm_name == NULL)
    {
        return;
    }

    for(i = 0; i < ALGORITHM_NAME_NUM; i++)
    {
        if (g_algorithm_name_str[i] != NULL && strcmp(algorithm_name, g_algorithm_name_str[i]) == 0)
        {
            g_algorithm_entity.algorithm_Name = i;
        }
    }
}

static void set_problem_name(const char *problem_name)
{
    int i = 0;
    if (problem_name == NULL)
    {
        return;
    }

    for(i = 0; i < PROBLEM_NAME_NUM; i++)
    {
        if (g_problem_name_str[i] != NULL && strcmp(problem_name, g_problem_name_str[i]) == 0)
        {
            g_algorithm_entity.testProblem = i;
        }
    }
}

static void set_problem_para(const char *para_k)
{

    if (NULL != para_k)
    {
        if (g_algorithm_entity.testProblem >= WFG1 && g_algorithm_entity.testProblem <= WFG48)
        {
            g_algorithm_entity.algorithm_para.problem_parameter = (int*)malloc(sizeof(int));
            *((int *)g_algorithm_entity.algorithm_para.problem_parameter) = atoi(para_k);
            printf("%d\n ", *((int *)g_algorithm_entity.algorithm_para.problem_parameter));
        }
    }

    return;
}


static void set_analyse(const char *analyse_str)
{

    int i = 0;
    if (analyse_str == NULL)
    {
        return;
    }

    for(i = 0; i < ANALYSE_NAME_NUM; i++)
    {
        if (g_analyse_name_str[i] != NULL && strcmp(analyse_str, g_analyse_name_str[i]) == 0)
        {
            g_algorithm_entity.analyse_Type = i;
        }
    }
    return;
}



int initialization_binary_para (int argc, char** argv)
{
    return 0;
}
int initialization_real_para (int argc, char** argv)
{
    int i, j;
    char buff[BUFSIZE_M] = {0};
    char PF_name[BUFSIZE_S] = {0};
    char line[BUFSIZE_L] = {0};


    FILE *PF     = NULL;
    FILE *config = NULL;

    int flag_default = 1;



    config = fopen ("../config.txt", "r");
    print_error (config == NULL, 1, "Fail to read configure file: config.txt");
    while (!feof(config))
    {

        fgets(buff, BUFSIZE_M, config);
        formalize_str(buff);
        for (i = 0; i < strlen(buff); i++)
        {
            if (buff[i] == ':')
            {
                buff[i] = 0;
                break;
            }
        }

        i++;
        if (!strcmp(buff, "algorithm_name"))
        {
            set_algorithm_name(buff + i);
        }
        else if (!strcmp(buff, "test_problem"))
        {
            set_problem_name(buff + i);
        }
        else if (!strcmp(buff, "problem_param"))
        {
            set_problem_para(buff + i);
        }
        else if (!strcmp(buff, "number_variable"))
        {
            g_algorithm_entity.algorithm_para.variable_number = atoi(buff + i);
        }
        else if (!strcmp(buff, "number_objective"))
        {
            g_algorithm_entity.algorithm_para.objective_number = atoi(buff + i);
        }
        else if (!strcmp(buff, "popSize"))
        {
            g_algorithm_entity.algorithm_para.pop_size = atoi(buff + i);
        }
        else if (!strcmp(buff, "eliteSize"))
        {
            g_algorithm_entity.algorithm_para.elite_pop_size = atoi(buff + i);
        }
        else if (!strcmp(buff, "max_evaluation"))
        {
            g_algorithm_entity.algorithm_para.max_evaluation = atoi(buff + i);
        }
        else if (!strcmp(buff, "runtime_output"))
        {
            g_algorithm_entity.algorithm_para.runtime_output = atoi(buff + i);
        }
        else if (!strcmp(buff, "output_interval"))
        {
            g_algorithm_entity.algorithm_para.output_interval = atoi(buff + i);
        }
        else if (!strcmp(buff, "run_index_begin"))
        {
            g_algorithm_entity.run_index_begin = atoi(buff + i);
        }
        else if (!strcmp(buff, "run_index_end"))
        {
            g_algorithm_entity.run_index_end = atoi(buff + i);
        }
        else if (!strcmp(buff, "analyse"))
        {
            set_analyse(buff + i);
        }
        else
        {
            print_error(1,2, "Input a wrong parameter, unknown type");
        }

    }

    fclose(config);

    if (FAIL == parameter_check())
    {
        printf("Initialize parameter failed, because input wrong parameter in the config file\n");
    }
    allocate_memory_for_pop(&g_algorithm_entity.parent_population, g_algorithm_entity.algorithm_para.pop_size);
    allocate_memory_for_pop(&g_algorithm_entity.offspring_population, g_algorithm_entity.algorithm_para.pop_size);
    allocate_memory_for_pop(&g_algorithm_entity.elit_population, g_algorithm_entity.algorithm_para.elite_pop_size);
    allocate_memory_for_pop(&g_algorithm_entity.mix_population, g_algorithm_entity.algorithm_para.pop_size * 2);
    allocate_memory_for_reference_point(&g_algorithm_entity.nadir_point);
    allocate_memory_for_reference_point(&g_algorithm_entity.ideal_point);


    g_algorithm_entity.algorithm_para.current_evaluation = 0;
    g_algorithm_entity.run_index_current = 0;

    if( flag_default )
        printf("All/Other parameters configured based on the defaut file: \'config.txt\'\n");
    // SBX parameter settings
    g_algorithm_entity.sbxPara.pcross_real = 1.0;
    g_algorithm_entity.sbxPara.eta_c       = 15.0;

    // polynomial mutation parameter settings
    g_algorithm_entity.polynomialPara.pmut_real = 1.0 / g_algorithm_entity.algorithm_para.variable_number;
    g_algorithm_entity.polynomialPara.eta_m     = 20.0;

    // differential evolution parameter settings
    g_algorithm_entity.dePara.CR = 0.5;
    g_algorithm_entity.dePara.F  = 0.5;
    g_algorithm_entity.dePara.K  = 0.5;

    //PBI parameter setting
    g_algorithm_entity.pbi_para.theta = 5.0;

    // intrisic parameters used in MOEA/D variants
    g_algorithm_entity.MOEAD_para.neighbor_size = 20;
    g_algorithm_entity.MOEAD_para.function_type = ITCH;
    g_algorithm_entity.MOEAD_para.neighborhood_selection_probability = 0.9;
    g_algorithm_entity.MOEAD_para.maximumNumberOfReplacedSolutions = 2;

    // set the reference point for Hypervolume calculation
    g_algorithm_entity.reference_point.obj = (double *) malloc (g_algorithm_entity.algorithm_para.objective_number * sizeof(double));
    g_algorithm_entity.reference_point.variable = (double *) malloc (g_algorithm_entity.algorithm_para.variable_number * sizeof(double));
    for(i = 0; i< g_algorithm_entity.algorithm_para.objective_number; i++)
        g_algorithm_entity.reference_point.obj[i] = 4.0;

    // calculate the number of points in the PF data
    sprintf (PF_name, "../PF/%s.%dD.pf", g_problem_name_str[g_algorithm_entity.testProblem], g_algorithm_entity.algorithm_para.objective_number);
    PF = fopen (PF_name, "r");
    if (NULL == PF)
    {
        //cal_pf(g_algorithm_entity.testProblem);
    }
    else
    {
        g_algorithm_entity.PF_size = 0;
        while (fgets (line, BUFSIZE_L, PF) != NULL)
            g_algorithm_entity.PF_size++;

        // read the PF data
        rewind (PF);
        g_algorithm_entity.PF_Data = (SMRT_PF_DATA *) malloc (g_algorithm_entity.PF_size * sizeof(SMRT_PF_DATA));
        for (i = 0; i < g_algorithm_entity.PF_size; i++)
            g_algorithm_entity.PF_Data[i].obj = (double *) malloc (g_algorithm_entity.algorithm_para.objective_number * sizeof(double));
        for (i = 0; i < g_algorithm_entity.PF_size; i++)
            for (j = 0; j < g_algorithm_entity.algorithm_para.objective_number; j++)
                fscanf (PF, "%lf", g_algorithm_entity.PF_Data[i].obj + j);
    }

    // boundary settings
    g_algorithm_entity.variable_lower_bound = (double *) malloc (g_algorithm_entity.algorithm_para.variable_number * sizeof(double));
    g_algorithm_entity.variable_higher_bound = (double *) malloc (g_algorithm_entity.algorithm_para.variable_number * sizeof(double));
    if (g_algorithm_entity.testProblem == ZDT4)
    {
        g_algorithm_entity.variable_lower_bound[0] = 0.0;
        g_algorithm_entity.variable_higher_bound[0] = 1.0;
        for (i = 1; i < g_algorithm_entity.algorithm_para.variable_number; i++)
        {
            g_algorithm_entity.variable_lower_bound[i] = -5.0;
            g_algorithm_entity.variable_higher_bound[i] = 5.0;
        }
    }
    else
    {
        for (i = 0; i < g_algorithm_entity.algorithm_para.variable_number; i++)
        {
            g_algorithm_entity.variable_lower_bound[i] = 0.0;
            g_algorithm_entity.variable_higher_bound[i] = 1.0;
        }
    }

    randomize();

    return SUCCESS;
}


extern int destroy_real_para (int argc, char** argv)
{
    int i = 0;
    destroy_memory_for_pop(&g_algorithm_entity.parent_population, g_algorithm_entity.algorithm_para.pop_size);
    destroy_memory_for_pop(&g_algorithm_entity.offspring_population, g_algorithm_entity.algorithm_para.pop_size);
    destroy_memory_for_pop(&g_algorithm_entity.elit_population, g_algorithm_entity.algorithm_para.elite_pop_size);
    destroy_memory_for_pop(&g_algorithm_entity.mix_population, g_algorithm_entity.algorithm_para.pop_size * 2);
    destroy_memory_for_reference_point(&g_algorithm_entity.nadir_point);
    destroy_memory_for_reference_point(&g_algorithm_entity.ideal_point);
    destroy_memory_for_reference_point(&g_algorithm_entity.reference_point);

    free(g_algorithm_entity.variable_lower_bound);
    free(g_algorithm_entity.variable_higher_bound);

    for (i = 0; i < g_algorithm_entity.PF_size; i++)
    {
        free(g_algorithm_entity.PF_Data[i].obj);
    }
    free(g_algorithm_entity.PF_Data);

    if (NULL != g_algorithm_entity.algorithm_para.problem_parameter && g_algorithm_entity.testProblem >= WFG1 && g_algorithm_entity.testProblem <= WFG48)
    {
        free(g_algorithm_entity.algorithm_para.problem_parameter);

    }

    return SUCCESS;
}
