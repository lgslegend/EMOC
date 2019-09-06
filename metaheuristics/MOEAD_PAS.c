#include "../headers/global.h"
#include "../headers/metaheuristics.h"
#include "../headers/crossover.h"
#include "../headers/mutation.h"
#include "../headers/problem.h"
#include "../headers/print.h"
#include "../headers/initialize.h"
#include "../headers/utility.h"
#include "../headers/selection.h"
#include "../headers/sort.h"
#include "../headers/analysis.h"
#include "../headers/random.h"

static int *Pi = NULL;


static int MOEAD_PAS_update_pi(SMRT_individual *pop, double *weight, int *candidate_p, int candidate_p_num)
{
    int i = 0, j = 0, k = 0;
    double *min_value_table = NULL, temp_value = 0, min_d2 = INF;
    int current_pi_index = 0, index = 0, *index_table = NULL;
    double d1 = 0, d2 = 0, nl = 0;


    min_value_table = (double *)malloc(sizeof(double) * candidate_p_num);
    if (NULL == min_value_table)
    {
        printf("in the NSGA3_getExtremePoints, malloc min_value_table Failed\n");
        return INF_NORM;
    }

    index_table = (int *)malloc(sizeof(int) *  candidate_p_num);
    if (NULL == index_table)
    {
        printf("in the NSGA3_getExtremePoints, malloc index_table Failed\n");
        return INF_NORM;
    }

    for (i = 0; i < candidate_p_num; i++)
    {
        min_value_table[i] = cal_nnormal_NORM(pop, weight, candidate_p[i]);
        index_table[i] = 0;
        for (j = 1; j < weight_num; j++)
        {
            temp_value = cal_nnormal_NORM(pop + j, weight, candidate_p[i]);

            if (min_value_table[i] > temp_value)
            {
                min_value_table[i] = temp_value;
                index_table[i] = j;
            }
        }
    }

    for (i = 0; i < candidate_p_num; i++)
    {
        index = index_table[i];
        d2 = Cal_perpendicular_distance(pop[index].obj, weight);

        if (min_d2 > d2)
        {
            min_d2 = d2;
            current_pi_index = i;
        }
    }

    free(min_value_table);
    free(index_table);

    return candidate_p[current_pi_index];
}


static int MOEAD_PAS_update_subproblem_pas(SMRT_individual *offspring, int pop_index, NeighborType type)
{
    int i = 0;
    int index = 0, replace_num = 0;
    double temp = 0;

    if (NEIGHBOR == type)
    {
        for (i = 0; i < g_algorithm_entity.MOEAD_para.neighbor_size; i++)
        {
            if (replace_num >= g_algorithm_entity.MOEAD_para.maximumNumberOfReplacedSolutions)
            {
                break;
            }
            index = g_algorithm_entity.MOEAD_para.neighbor_table[pop_index].neighbor[i];
            temp = cal_nnormal_NORM(offspring, lambda[index], Pi[index]);
            cal_nnormal_NORM(g_algorithm_entity.parent_population + index, lambda[index], Pi[index]);
            if (temp < g_algorithm_entity.parent_population[index].fitness)
            {
                memcpy(g_algorithm_entity.parent_population[index].variable,offspring->variable,
                       sizeof(double) * g_algorithm_entity.algorithm_para.variable_number);
                memcpy(g_algorithm_entity.parent_population[index].obj, offspring->obj,
                       sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);
                g_algorithm_entity.parent_population[index].fitness = temp;
                replace_num++;
            }
        }
    }
    else
    {
        for (i = 0; i < weight_num; i++)
        {
            if (replace_num >= g_algorithm_entity.MOEAD_para.maximumNumberOfReplacedSolutions)
            {
                break;
            }
            temp = cal_nnormal_NORM(offspring, lambda[i], Pi[i]);
            cal_nnormal_NORM(g_algorithm_entity.parent_population + i, lambda[i], Pi[i]);

            if (temp < g_algorithm_entity.parent_population[i].fitness)
            {
                memcpy(g_algorithm_entity.parent_population[i].variable, offspring->variable,
                       sizeof(double) * g_algorithm_entity.algorithm_para.variable_number);
                memcpy(g_algorithm_entity.parent_population[i].obj, offspring->obj,
                       sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);

                g_algorithm_entity.parent_population[i].fitness = temp;


                replace_num++;
            }
        }
    }

    return SUCCESS;
}

static void ini_MOEAD_ENS()
{
    int i = 0, j = 0, k = 0;
    double difference = 0, distance_temp = 0, Euc_distance = 0;
    Distance_info_t sort_list[MAX_SIZE];


    lambda = initialize_uniform_point (&weight_num);

    g_algorithm_entity.MOEAD_para.neighbor_table = (MOEAD_NEIGHBOR*)malloc(sizeof(MOEAD_NEIGHBOR) * weight_num);
    if(NULL == g_algorithm_entity.MOEAD_para.neighbor_table)
    {
        printf("In the state of initiate parameter malloc G_MOEAD_weighted Fail\n");
        return;
    }

    for (i = 0; i < weight_num; i++)
    {
        for (j = 0; j < weight_num; j++)
        {
            distance_temp = 0;
            for (k = 0; k < g_algorithm_entity.algorithm_para.objective_number; k++)
            {
                difference = fabs(lambda[i][k] -  lambda[j][k]);
                distance_temp += (double)difference * difference;
            }

            Euc_distance = sqrt((double)distance_temp);
            sort_list[j].E_distance = Euc_distance;
            sort_list[j].idx = j;
        }
        distance_quick_sort(sort_list, 0, weight_num - 1);

        g_algorithm_entity.MOEAD_para.neighbor_table[i].neighbor = (int *)malloc(sizeof(int) * g_algorithm_entity.MOEAD_para.neighbor_size);
        if(NULL == g_algorithm_entity.MOEAD_para.neighbor_table[i].neighbor)
        {
            printf("In the state of initiate parameter malloc weight neighbor Fail\n");
            return ;
        }


        for (j = 0; j < g_algorithm_entity.MOEAD_para.neighbor_size; j++)
        {
            g_algorithm_entity.MOEAD_para.neighbor_table[i].neighbor[j] = sort_list[j].idx;
        }
    }
    return ;
}




extern void MOEAD_PAS_framework (SMRT_individual *pop, SMRT_individual *offspring_pop, SMRT_individual *mixed_pop)
{
    int  i = 0, j = 0;
    NeighborType type;
    double rand = 0;
    int candidate_p_num = 11;
    int candidate_p[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, INF_NORM};
    SMRT_individual *offspring = g_algorithm_entity.offspring_population;

    int bb = 0;

    g_algorithm_entity.iteration_number          = 1;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;


    printf ("|\tThe %d run\t|\t1%%\t|", g_algorithm_entity.run_index_current);

    // initialization process
    ini_MOEAD_ENS();

    initialize_population_real (pop, weight_num);

    evaluate_population (pop, weight_num);

    initialize_idealpoint (pop, weight_num, &g_algorithm_entity.ideal_point);

    non_dominated_sort(pop, weight_num);
    update_nadirpoint_nds(pop, weight_num, &g_algorithm_entity.nadir_point);

    Pi = (int *)malloc(sizeof(int) * weight_num);
    if (NULL == Pi)
    {

        printf("in the NSGA3_getExtremePoints, malloc Pi Failed\n");
        return;
    }


    for (i = 0; i < weight_num; ++i)
    {
        Pi[i] = INF_NORM;
        cal_nnormal_NORM(pop + i, lambda[i], Pi[i]);
    }

    track_evolution (pop, g_algorithm_entity.iteration_number, 0);

    while (g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {
        print_progress ();

        // crossover and mutation
        for (i = 0; i < weight_num; i++)
        {
            rand = randomperc();
            if (rand < g_algorithm_entity.MOEAD_para.neighborhood_selection_probability)
            {
                type = NEIGHBOR;
            }
            else
            {
                type = GLOBAL_PARENT;
            }
            //crossover_SMSEMOA(pop, offspring);
            crossover_MOEAD (pop, pop + i, i, offspring, type);

            mutation_ind(offspring);

            evaluate_individual (offspring);

            update_ideal_point_by_ind(offspring);

            // update subproblem
            MOEAD_PAS_update_subproblem_pas(offspring, i, type);
        }
        non_dominated_sort(pop, weight_num);
        update_nadirpoint_nds(pop, weight_num, &g_algorithm_entity.nadir_point);

        g_algorithm_entity.iteration_number++;


        for (i = 0; i < weight_num; i++)
        {
            if (randomperc() >= (double)g_algorithm_entity.algorithm_para.current_evaluation / (double)g_algorithm_entity.algorithm_para.max_evaluation)
            {
                Pi[i] = MOEAD_PAS_update_pi(pop, i,lambda[i], candidate_p, candidate_p_num);

                cal_nnormal_NORM(pop + i, lambda[i], Pi[i]);
            }
        }

        track_evolution (pop, g_algorithm_entity.iteration_number, g_algorithm_entity.algorithm_para.current_evaluation >= g_algorithm_entity.algorithm_para.max_evaluation);
    }

    free(Pi);

    return;
}