#include "../headers/global.h"
#include "../headers/metaheuristics.h"
#include "../headers/crossover.h"
#include "../headers/mutation.h"
#include "../headers/problem.h"
#include "../headers/print.h"
#include "../headers/dominance_relation.h"
#include "../headers/initialize.h"
#include "../headers/utility.h"
#include "../headers/selection.h"
#include "../headers/sort.h"





static void ini_MOEAD(SMRT_individual *pop_table, int weight_num)
{
    int i = 0, j = 0, k = 0;
    double difference = 0, distance_temp = 0, Euc_distance = 0;
    Weight_distance_info_t sort_list[MAX_SIZE];

    g_algorithm_entity.MOEAD_para.neighbor_table = (MOEAD_NEIGHBOR*)malloc(sizeof(MOEAD_NEIGHBOR) * weight_num);
    if(NULL == g_algorithm_entity.MOEAD_para.neighbor_table)
    {
        printf("In the state of initiate parameter malloc G_MOEAD_weighted Fail\n");
        return;
    }
    initialize_uniform_weight();

    for (i = 0; i < weight_num; i++)
    {
        for (j = 0; j < weight_num; j++)
        {
            distance_temp = 0;
            for (k = 0; k < g_algorithm_entity.algorithm_para.objective_number; k++)
            {
                difference = fabs(pop_table[i].weight[k] -  pop_table[j].weight[k]);
                distance_temp += (double)difference * difference;
            }

            Euc_distance = sqrt((double)distance_temp);
            sort_list[j].E_distance = Euc_distance;
            sort_list[j].idx = j;
        }
        bublesort_weight(sort_list, weight_num);

        for (j = 0; j < g_algorithm_entity.MOEAD_para.neighbor_size; j++)
        {
            g_algorithm_entity.MOEAD_para.neighbor_table[i].neighbor[j] = sort_list[j].idx;
        }
    }
    return ;
}

static int update_subproblem(SMRT_individual *pop_table, SMRT_individual *offspring)
{
    int i = 0, j = 0;
    int index = 0, replace_num = 0;
    double temp = 0;


    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
    {
        cal_moead_fitness(pop_table + i, pop_table[i].weight, g_algorithm_entity.MOEAD_para.function_type);
    }

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
    {
        for (j = 0; j < g_algorithm_entity.MOEAD_para.neighbor_size; j++)
        {
            if (replace_num >= g_algorithm_entity.MOEAD_para.maximumNumberOfReplacedSolutions)
            {
                replace_num = 0;
                break;
            }
            index = g_algorithm_entity.MOEAD_para.neighbor_table[i].neighbor[j];
            temp = cal_moead_fitness(offspring + i, pop_table[index].weight, g_algorithm_entity.MOEAD_para.function_type);
            if (temp < g_algorithm_entity.parent_population[index].fitness)
            {
                memcpy(g_algorithm_entity.parent_population[index].variable, g_algorithm_entity.offspring_population[i].variable,
                       sizeof(double) * g_algorithm_entity.algorithm_para.variable_number);
                memcpy(g_algorithm_entity.parent_population[index].obj, g_algorithm_entity.offspring_population[i].obj,
                       sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);
                g_algorithm_entity.parent_population[index].fitness = temp;
                replace_num++;
            }
        }
    }

    return SUCCESS;
}

extern void MOEAD_framework (SMRT_individual *pop, SMRT_individual *offspring_pop, SMRT_individual *mixed_pop)
{
    g_algorithm_entity.iteration_number          = 1;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;
    printf ("|\tThe %d run\t|\t1%%\t|", g_algorithm_entity.run_index_current);

    // initialization process
    ini_MOEAD(pop, g_algorithm_entity.algorithm_para.pop_size);


    //print_error (number_weight != popsize, 1, "Number of weight vectors must be equal to the population size!");
    initialize_population_real (pop, g_algorithm_entity.algorithm_para.pop_size);


    evaluate_population (pop, g_algorithm_entity.algorithm_para.pop_size);

    initialize_idealpoint (pop, g_algorithm_entity.algorithm_para.pop_size, &g_algorithm_entity.ideal_point);

    //track_evolution (pop, generation, 0);

    while (g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {
        print_progress ();
        // crossover and mutation
        crossover_MOEAD (pop, offspring_pop);
        mutation_real (offspring_pop);
        evaluate_population (offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        // update ideal point
        update_ideal_point (offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        // update subproblem
        update_subproblem (pop, offspring_pop);

        g_algorithm_entity.iteration_number++;

        //track_evolution (pop, generation, evaluation_count >= max_evaluation);
    }

    return;
}