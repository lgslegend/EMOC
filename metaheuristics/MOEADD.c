/*
 * MOEADD.c:
 *  This file implements the main procedures of MOEADD. It is based on the following reference:
 *
 *  M. Wagner and F. Neumann, "A fast approximation-guided evolutionary multi-objective algorithm".
 *  Annual Conference on Genetic and Evolutionary Computation. 687-694, 2013.
 *
 * Authors:
 *  Peili Mao
 *  Lei Sun
 *  Longfei Zhang
 *  Ke Li <k.li@exeter.ac.uk>
 *  Xinyu Shan
 *  Renzhi Chen
 *
 * Institution:
 *  Computational Optimization for Learning and Adaptive System (COLA) Laboratory @ University of Exeter
 *
 * Copyright (c) 2019 Peili Mao, Lei Sun, Longfei Zhang ,Ke Li
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>;.
 */
#include "../headers/global.h"
#include "../headers/metaheuristics.h"
#include "../headers/crossover.h"
#include "../headers/mutation.h"
#include "../headers/problem.h"
#include "../headers/print.h"
#include "../headers/utility.h"
#include "../headers/analysis.h"
#include "../headers/sort.h"
#include "../headers/indicator.h"

static int **association_matrix = NULL, *association_num = NULL;

static void MOEADD_calculateLayerByObj(int *layer, int *layer_inner, int *layer_out)
{

    switch (g_algorithm_entity.algorithm_para.objective_number)
    {
        case 2:
            *layer = 13;
            break;
        case 3:
            *layer = 12;
            break;
        case 5:
            *layer = 6;
            break;
        case 8:
            *layer_inner = 2;
            *layer_out = 3;
            break;
        case 10:
            *layer_inner = 2;
            *layer_out = 3;
            break;
        case 15:
            *layer_inner = 1;
            *layer_out = 2;
            break;
        default:
            break;
    }

    return;
}

//association solution with subregion
static void MOEADD_association(SMRT_individual *pop_table, int pop_num, double **weight_vector, int weight_num)
{
    int i = 0, j = 0, k = 0, min_idx;
    double d1 = 0, lam = 0, beita = 0, theta = 0, min_distance = 0;
    double **angle = NULL;

    angle = (double **)malloc(sizeof(double *) * pop_num);
    if (NULL == angle)
    {
        printf("in the MOEADD, malloc association_matrix Failed\n");
        return;
    }

    for (i = 0; i < pop_num; i++)
    {
        angle[i] = (double *)malloc(sizeof(double) * (weight_num));
        if (NULL == angle[i])
        {
            printf("in the MOEADD, malloc angle[i] Failed\n");
            return;
        }
    }

    memset(association_num, 0, sizeof(int) * weight_num);

    // calculate perpendicular distances towards each weight vector
    for (i = 0; i < weight_num; i++)
    {
        for (j = 0; j < pop_num; j++)
        {
            d1  = 0.0;
            lam = 0.0;
            beita = 0.0;

            for (k = 0; k < g_algorithm_entity.algorithm_para.objective_number; k++)
            {
                d1 += (pop_table[j].obj[k]) * weight_vector[i][k];
                lam += weight_vector[i][k] * weight_vector[i][k];
                beita +=  pop_table[j].obj[k] * pop_table[j].obj[k];
            }

            lam = sqrt(lam);
            beita = sqrt(beita);
            theta  = d1 / (lam * beita);

            //tansform to sin
            theta = sqrt(1 - theta * theta);

            // Store the angle in the matrix and in the individual object
            angle[j][i] = theta;
        }
    }

    for (i = 0; i < pop_num; i++)
    {
        min_distance = angle[i][0];
        min_idx = 0;

        for (j = 1; j < weight_num; j++)
        {
            if (min_distance > angle[i][j])
            {
                min_distance = angle[i][j];
                min_idx = j;
            }
        }

        association_matrix[min_idx][association_num[min_idx]++] = i;
    }

    for (i = 0; i < pop_num; i++)
    {
        free(angle[i]);
    }
    free(angle);
    return;
}

static void MOEADD_associationAddByInd(SMRT_individual *ind, int ind_id, double **weight_vector, int weight_num)
{
    int i = 0, j = 0, min_idx;
    double d1 = 0, lam = 0, beita = 0, theta = 0, min_distance = 0;
    double *angle = NULL;

    angle = (double *)malloc(sizeof(double ) * weight_num);
    if (NULL == angle)
    {
        printf("in the MOEADD, malloc association_matrix Failed\n");
        return;
    }

    // calculate perpendicular distances towards each weight vector
    for (i = 0; i < weight_num; i++)
    {
        d1  = 0.0;
        lam = 0.0;
        beita = 0.0;

        for (j = 0; j < g_algorithm_entity.algorithm_para.objective_number; j++)
        {
            d1 += (ind->obj[j]) * weight_vector[i][j];
            lam += weight_vector[i][j] * weight_vector[i][j];
            beita += ind->obj[j] * ind->obj[j];
        }

        lam = sqrt(lam);
        beita = sqrt(beita);
        theta  = d1 / (lam * beita);

        //tansform to sin
        theta = sqrt(1 - theta * theta);

        // Store the angle in the matrix and in the individual object
        angle[i] = theta;
    }

    min_distance = angle[0];
    min_idx = 0;
    for (i = 1; i < weight_num; i++)
    {
        if (min_distance > angle[i])
        {
            min_distance = angle[i];
            min_idx = i;
        }
    }

    association_matrix[min_idx][association_num[min_idx]++] = ind_id;

    free(angle);
    return;
}

static void MOEADD_associationDelByInd(int ind_id, int weight_num)
{
    int i = 0, j = 0, k = 0;
    int flag = 0;
    for (i = 0; i < weight_num; i++)
    {
        for (j = 0; j < association_num[i]; j++)
        {
            if (ind_id == association_matrix[i][j])
            {
                flag = 1;
                break;
            }
        }
        if(flag == 1)
            break;
    }

    for (k = j; k < association_num[i] - 1; k++)
    {
        association_matrix[i][k] = association_matrix[i][k+1];
    }
    association_num[i]--;

    return;
}


static void MOEADD_Ini()
{
    int i = 0, j = 0, k = 0;
    int layer = 0, weight_num_inner = 0, weight_num_out = 0, layer_inner = 0, layer_out = 0;
    double **lambda_inner = NULL, **lambda_out = NULL;
    double difference = 0, distance_temp = 0, Euc_distance = 0;
    Distance_info_t sort_list[MAX_SIZE];

    g_algorithm_entity.MOEADD_para.theta = 5.0;
    g_algorithm_entity.MOEADD_para.neighbor_size = 20;
    g_algorithm_entity.MOEADD_para.neighborhood_selection_probability = 0.9;
    g_algorithm_entity.MOEADD_para.tau = 0.5;

    MOEADD_calculateLayerByObj(&layer, &layer_inner, &layer_out);
    if (g_algorithm_entity.algorithm_para.objective_number <= 7)
    {
        lambda = initialize_uniform_weight_by_layer (layer, &weight_num);
    }
    else
    {
        lambda_inner = initialize_uniform_weight_by_layer(layer_inner, &weight_num_inner);
        lambda_out = initialize_uniform_weight_by_layer(layer_out, &weight_num_out);

        lambda = (double **) malloc ((weight_num_inner + weight_num_out) * sizeof(double *));
        for (i = 0; i < weight_num_inner + weight_num_out; i++)
        {
            lambda[i] = (double *) malloc(g_algorithm_entity.algorithm_para.objective_number  * sizeof(double));

            if (i < weight_num_inner)
            {
                for (j = 0; j < g_algorithm_entity.algorithm_para.objective_number; ++j)
                {
                    lambda_inner[i][j] = (1 - g_algorithm_entity.MOEADD_para.tau) / g_algorithm_entity.algorithm_para.objective_number + g_algorithm_entity.MOEADD_para.tau * lambda_inner[i][j];
                }
                memcpy(lambda[i], lambda_inner[i], g_algorithm_entity.algorithm_para.objective_number * sizeof(double));
            }
            else
            {
                memcpy(lambda[i], lambda_out [i - weight_num_inner], g_algorithm_entity.algorithm_para.objective_number * sizeof(double));
            }
        }
        weight_num = weight_num_inner + weight_num_out;
    }

    g_algorithm_entity.MOEADD_para.neighbor_table = (MOEAD_NEIGHBOR*)malloc(sizeof(MOEAD_NEIGHBOR) * weight_num);
    if(NULL == g_algorithm_entity.MOEADD_para.neighbor_table)
    {
        printf("In the state of initiate parameter malloc MOEADD_neighbor_table Fail\n");
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
            sort_list[j].value = Euc_distance;
            sort_list[j].idx = j;
        }

        distance_quick_sort(sort_list, 0, weight_num - 1);

        g_algorithm_entity.MOEADD_para.neighbor_table[i].neighbor = (int *)malloc(sizeof(int) * g_algorithm_entity.MOEADD_para.neighbor_size);
        if(NULL == g_algorithm_entity.MOEADD_para.neighbor_table[i].neighbor)
        {
            printf("In the state of initiate parameter malloc weight neighbor Fail\n");
            return ;
        }

        for (j = 0; j < g_algorithm_entity.MOEADD_para.neighbor_size; j++)
        {
            g_algorithm_entity.MOEADD_para.neighbor_table[i].neighbor[j] = sort_list[j].idx;
        }
    }

    for (i = 0; i < weight_num_inner; i++)
        free (lambda_inner[i]);
    free (lambda_inner);
    for (i = 0; i < weight_num_out; i++)
        free (lambda_out[i]);
    free (lambda_out);

    return ;
}

static void MOEADD_free()
{
    int i = 0;

    for (i = 0; i < weight_num; i++)
        free (lambda[i]);
    free (lambda);

    for (i = 0; i < weight_num; i++)
        free (association_matrix[i]);
    free (association_matrix);
    free(association_num);

    for (i = 0; i < weight_num; ++i)
    {
        free(g_algorithm_entity.MOEADD_para.neighbor_table[i].neighbor);
    }

    free(g_algorithm_entity.MOEADD_para.neighbor_table);

    return;
}

static int MOEADD_locateWorstSolution(SMRT_individual *pop_table, int pop_num)
{
    int i = 0, j = 0;
    int max_num = 0, temp_num = 0, delete_id = 0, max_subreg_id = 0, current_subregion_id = 0;
    double temp_value = 0, max_value = 0;
    int *max_crowded_subregion = NULL;

    max_crowded_subregion = (int *)malloc(sizeof(int) * pop_num);
    if (NULL == max_crowded_subregion)
    {
        printf("in the MOEADD, malloc max_crowded_subregion Failed\n");
        return 0;
    }

    //find the most density subregion by pbi, and the largest solution
    temp_num = association_num[0];
    for (i = 0; i < weight_num; i++)
    {
        if (temp_num < association_num[i])
        {
            temp_num = association_num[i];
            max_num = 0;
        }
        if (temp_num == association_num[i])
        {
            max_crowded_subregion[max_num++] = i;
        }
    }

    if (max_num == 1)
    {
        max_subreg_id = max_crowded_subregion[0];
        max_value = 0;

        for (i = 0; i < association_num[max_subreg_id]; i++)
        {
            temp_value = cal_PBI(pop_table + association_matrix[max_subreg_id][i], lambda[max_subreg_id], g_algorithm_entity.pbi_para.theta);
            if (max_value < temp_value)
            {
                max_value = temp_value;
                delete_id = association_matrix[max_subreg_id][i];
            }
        }
    }
    else
    {
        max_value = 0;
        for (i = 0; i < max_num; i++)
        {
            temp_value = 0;
            current_subregion_id = max_crowded_subregion[i];
            for (j = 0; j < association_num[current_subregion_id]; j++)
            {
                temp_value += cal_PBI(pop_table + association_matrix[current_subregion_id][j], lambda[current_subregion_id], g_algorithm_entity.pbi_para.theta);
            }
            if (max_value < temp_value)
            {
                max_value = temp_value;
                max_subreg_id = current_subregion_id;
            }
        }

        max_value = 0;

        for (i = 0; i < association_num[max_subreg_id]; i++)
        {
            temp_value = cal_PBI(pop_table + association_matrix[max_subreg_id][i], lambda[max_subreg_id], g_algorithm_entity.pbi_para.theta);
            if(max_value < temp_value)
            {
                max_value = temp_value;
                delete_id = association_matrix[max_subreg_id][i];
            }
        }
    }

    free(max_crowded_subregion);
    return delete_id;
}


static void MOEADD_update(SMRT_individual *merge_pop, int merge_num)
{
    int i = 0, j = 0;
    int flag = 0, delete_id = 0, terminate_flag = 0, max_num = 0, max_subregion_num = 0, delete_subregion_id = 0;
    int *last_front = NULL, last_front_num = 0, last_rank = 0;
    int **fl_association_matrix = NULL, *fl_association_num = NULL, *max_subregion_id = NULL;
    double max_value = 0, temp_value = 0;

    last_front = (int *)malloc(sizeof(int) * merge_num);
    if (NULL == last_front)
    {
        printf("in the MOEADD, malloc last_front Failed\n");
        return;
    }

    fl_association_num = (int *)malloc(sizeof(int) * weight_num);
    if (NULL == fl_association_num)
    {
        printf("in the MOEADD, malloc fl_association_num Failed\n");
        return;
    }
    memset(fl_association_num, 0, sizeof(int) * weight_num);

    max_subregion_id = (int *)malloc(sizeof(int) * weight_num);
    if (NULL == max_subregion_id)
    {
        printf("in the MOEADD, malloc max_subregion_id Failed\n");
        return;
    }

    fl_association_matrix = (int **)malloc(sizeof(int *) * weight_num);
    if (NULL == fl_association_matrix)
    {
        printf("in the MOEADD, malloc association_matrix Failed\n");
        return;
    }

    for (i = 0; i < weight_num; i++)
    {
        fl_association_matrix[i] = (int *)malloc(sizeof(int) * (merge_num));
        if (NULL == fl_association_matrix[i])
        {
            printf("in the MOEADD, malloc fl_association_matrix[i] Failed\n");
            return;
        }
    }

    non_dominated_sort(merge_pop, merge_num);

    for (i = 0; i < merge_num; i++)
    {
        if (merge_pop[i].rank != 0)
        {
            flag = 1;
            break;
        }
    }

    if (flag)
    {
        for (i = 0; i < merge_num; i++)
        {
            if (last_rank < merge_pop[i].rank)
            {
                last_front_num = 0;
                last_rank = merge_pop[i].rank;
            }

            if(last_rank == merge_pop[i].rank)
            {
                last_front[last_front_num++] = i;
            }
        }

        if (last_front_num == 1)
        {
            for (i = 0; i < weight_num; i++)
            {
                if (association_num[i] <= 1)
                    continue;
                for (j = 0; j < association_num[i]; j++)
                {
                    if (last_front[0] == association_matrix[i][j])
                    {
                        delete_id = last_front[0];
                        terminate_flag = 1;
                        break;
                    }
                }
            }

            //if terminate flag = 0,means the subregion which associated with the last front point has only one point
            if (!terminate_flag)
            {
                delete_id = MOEADD_locateWorstSolution(merge_pop, merge_num);
                goto MOEADD_ELIMINATE;
            }
        }
        else
        {

            for (i = 0; i < weight_num; i++)
            {
                for (j = 0; j < association_num[i]; j++)
                {
                    if (merge_pop[association_matrix[i][j]].rank == last_rank)
                    {
                        fl_association_matrix[i][fl_association_num[i]++] = association_matrix[i][j];
                    }
                }
            }

            max_num = 0;

            for (i = 0; i < weight_num; i++)
            {
                if (max_num < fl_association_num[i])
                {
                    max_num = fl_association_num[i];
                    max_subregion_num = 0;
                }
                if (max_num == fl_association_num[i])
                {
                    max_subregion_id[max_subregion_num++] = i;
                }
            }

            if (max_num == 1)
            {
                delete_id = MOEADD_locateWorstSolution(merge_pop, merge_num);
            }
            else
            {
                if (max_subregion_num == 1)
                {
                    for (i = 0; i < fl_association_num[max_subregion_id[0]]; i++)
                    {
                        temp_value = cal_PBI(merge_pop + fl_association_matrix[max_subregion_id[0]][i], lambda[max_subregion_id[0]], g_algorithm_entity.pbi_para.theta);
                        if (max_value < temp_value)
                        {
                            max_value = temp_value;
                            delete_id = fl_association_matrix[max_subregion_id[0]][i];
                        }
                    }
                }
                else
                {
                    for (i = 0; i < max_subregion_num; i++)
                    {
                        temp_value = 0;
                        for (j = 0; j < fl_association_num[max_subregion_id[i]]; j++)
                        {
                            temp_value += cal_PBI(merge_pop + fl_association_matrix[max_subregion_id[i]][j], lambda[max_subregion_id[i]], g_algorithm_entity.pbi_para.theta);

                        }
                        if (max_value < temp_value)
                        {
                            max_value = temp_value;
                            delete_subregion_id = max_subregion_id[i];
                        }
                    }

                    max_value = 0;

                    for (i = 0; i < fl_association_num[delete_subregion_id]; i++)
                    {
                        temp_value = cal_PBI(merge_pop + fl_association_matrix[delete_subregion_id][i], lambda[delete_subregion_id], g_algorithm_entity.pbi_para.theta);
                        if (max_value < temp_value)
                        {
                            max_value = temp_value;
                            delete_id = fl_association_matrix[delete_subregion_id][i];
                        }
                    }
                }
            }
        }
    }
    else
    {
        delete_id = MOEADD_locateWorstSolution(merge_pop, merge_num);
    }

MOEADD_ELIMINATE:
    if (delete_id == (merge_num - 1))
    {
        MOEADD_associationDelByInd(merge_num - 1, weight_num);
    }
    else
    {
        MOEADD_associationDelByInd(merge_num - 1, weight_num);
        MOEADD_associationDelByInd(delete_id, weight_num);
        copy_individual(merge_pop + merge_num - 1, g_algorithm_entity.parent_population + delete_id);
        MOEADD_associationAddByInd(g_algorithm_entity.parent_population + delete_id, delete_id, lambda, weight_num);
    }

    for (i = 0; i < weight_num; i++)
        free (fl_association_matrix[i]);
    free (fl_association_matrix);
    free(fl_association_num);
    free(last_front);
    free(max_subregion_id);

    return;
}

extern void _MOEADD_ (SMRT_individual *pop, SMRT_individual *offspring_pop, SMRT_individual *mixed_pop)
{
    int i = 0;
    g_algorithm_entity.iteration_number          = 1;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;
    SMRT_individual *offspring = g_algorithm_entity.offspring_population;

    printf ("|\tThe %d run\t|\t1%%\t|", g_algorithm_entity.run_index_current);

    // initialization process
    MOEADD_Ini();

    association_matrix = (int **)malloc(sizeof(int *) * weight_num);
    if (NULL == association_matrix)
    {
        printf("in the MOEADD, malloc association_matrix Failed\n");
        return;
    }

    for (i = 0; i < weight_num; i++)
    {
        association_matrix[i] = (int *)malloc(sizeof(int) * (weight_num + 1));
        if (NULL == association_matrix[i])
        {
            printf("in the MOEADD, malloc association_matrix[i] Failed\n");
            return;
        }
    }

    association_num = (int *)malloc(sizeof(int) * weight_num);
    if (NULL == association_num)
    {
        printf("in the MOEADD, malloc association_num Failed\n");
        return;
    }
    memset(association_num, 0, sizeof(int) * weight_num);

    initialize_population_real (pop, weight_num);

    evaluate_population (pop, weight_num);

    initialize_idealpoint (pop, weight_num, &g_algorithm_entity.ideal_point);

    track_evolution (pop, g_algorithm_entity.iteration_number, 0);

    non_dominated_sort(pop, weight_num);

    MOEADD_association(pop, weight_num, lambda, weight_num);

    while (g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {
        print_progress ();
        // crossover and mutation
        for (i = 0; i < weight_num; i++)
        {
            crossover_MOEADD (pop, i, offspring, association_matrix, association_num, weight_num);
            mutation_ind(offspring);
            evaluate_individual (offspring);

            update_ideal_point_by_ind (offspring);

            //merge offspring and population
            merge_population(mixed_pop, pop, weight_num, offspring, 1);

            MOEADD_associationAddByInd(offspring, weight_num, lambda, weight_num);
            
            MOEADD_update(mixed_pop, weight_num + 1);
        }

        g_algorithm_entity.iteration_number++;

        track_evolution (pop, g_algorithm_entity.iteration_number, g_algorithm_entity.algorithm_para.current_evaluation >= g_algorithm_entity.algorithm_para.max_evaluation);
    }

    MOEADD_free();

    return;
}