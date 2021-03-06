/*
 * SPEA2_R.c:
 *  This file implements the main procedures of SPEA2_R. It is based on the following reference:
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
#include "../headers/crossover.h"
#include "../headers/mutation.h"
#include "../headers/problem.h"
#include "../headers/print.h"
#include "../headers/memory.h"
#include "../headers/utility.h"
#include "../headers/analysis.h"
#include "../headers/sort.h"
#include "../headers/dominance_relation.h"
#include "../headers/population.h"

static int  **association_matrix_in_fl = NULL;
static int *association_num_in_fl = NULL;

static void SPEA2_R_clearMem(int ref_point_num, double **store_angle)
{
    int i = 0;

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size * 2; i++)
    {
        memset(store_angle[i], 0, ref_point_num * sizeof(double));
    }

    for (i = 0; i < ref_point_num; ++i)
    {
        memset(association_matrix_in_fl[i], 0, sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
    }
    memset(association_num_in_fl, 0, sizeof(int) * ref_point_num);

    return;
}

static void SPEA2_R_setFitness(SMRT_individual *pop_table, double **store_angle, int point_num, double *max_angle,
                               int pop_num)
{
    int i = 0, j = 0,temp_index = 0, index = 0;
    DOMINATE_RELATION relation;
    SMRT_individual *temp_i = NULL, *temp_j = NULL;
    int **si = NULL, *si_size = NULL, *Q = NULL;

    si = (int **)malloc(sizeof(int *) * pop_num);
    if (NULL == si)
    {
        printf("in the non_dominated_sort, malloc si Failed\n");
        goto SPEA2_SET_FIT_TERMINATE;
    }
    for (i = 0; i < pop_num; i++)
    {
        si[i] = (int *)malloc(sizeof(int) * pop_num);
        if (NULL == si[i])
        {
            printf("in the non_dominated_sort, malloc si Failed\n");
            goto SPEA2_SET_FIT_TERMINATE;
        }
        memset(si[i], 0, sizeof(int) * pop_num);
    }

    si_size = (int *)malloc(sizeof(int) * pop_num);
    if (NULL == si_size)
    {
        printf("in the non_dominated_sort, malloc si_size Failed\n");
        goto SPEA2_SET_FIT_TERMINATE;
    }
    memset(si_size, 0, sizeof(int) * pop_num);

    Q = (int *)malloc(sizeof(int) * pop_num);
    if (NULL == Q)
    {
        printf("in the non_dominated_sort, malloc Q Failed\n");
        goto SPEA2_SET_FIT_TERMINATE;
    }
    memset(Q, 0, sizeof(int) * pop_num);

    for (i = 0; i < pop_num; i++)
    {
        temp_i = pop_table + i;
        for (j = 0; j < pop_num; j++)
        {
            if (i == j)
                continue;
            temp_j = pop_table + j;
            relation = check_dominance(temp_i, temp_j);
            if (relation == DOMINATE)
            {
                Q[i]++;
            }
            else if (relation == DOMINATED)
            {
                si[i][si_size[i]++] = j;
            }
            else
            {
                ;
            }

        }
    }

    for (i = 0; i < pop_num; i++)
    {
        if (0 == si_size[i])
        {
            pop_table[i].fitness = 0;
        }
        else
        {
            for (j = 0; j < si_size[i]; j++)
            {
                temp_index = si[i][j];
                pop_table[i].fitness += Q[temp_index];
            }
        }
    }

    for(i = 0; i < point_num; i++)
    {
        for(j = 0; j < association_num_in_fl[i]; j++)
        {
            index = association_matrix_in_fl[i][j];
            temp_i = pop_table + index;
            temp_i->fitness += (store_angle[index][j]) /  (store_angle[index][j] + *max_angle);
        }
    }

    SPEA2_SET_FIT_TERMINATE:
    for (i = 0; i < pop_num; i++)
    {
        free(si[i]);
    }
    free(si);
    free(Q);
    free(si_size);
    return;
}

static void SPEA2_R_environmentalSelect(SMRT_individual *parent_pop, SMRT_individual * offspring_pop, int point_num,
                                        int pop_num) {
    int  ** index_matrix = NULL;
    int * store_index = malloc(sizeof(int) * point_num);
    SMRT_individual * H_pop = NULL;
    Fitness_info_t *fitnessInfo = (Fitness_info_t *)malloc(sizeof(Fitness_info_t) * pop_num);
    int i = 0, j = 0, m = 0, remove_index1 = 0, offspring_index = 0, min_index = 0, H_pop_number = 0, All_pop_number = 0;

    allocate_memory_for_pop(&H_pop, pop_num);

    index_matrix = (int **)malloc(sizeof(int *) * point_num);
    for (i = 0; i < point_num; i++)
    {
        index_matrix[i] = (int *)malloc(sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
        memset(index_matrix[i], 0, sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
    }

    while (offspring_index != pop_num )
    {
        H_pop_number = 0;
        for(i = 0; i < point_num; i++)
        {
            remove_index1= 0;

            if(association_num_in_fl[i] > 0)
            {
                min_index = association_matrix_in_fl[i][0];

                for(j = 1; j < association_num_in_fl[i]; j++)
                {
                    if(parent_pop[min_index].fitness - parent_pop [association_matrix_in_fl[i][j]].fitness > EPS )
                    {
                        min_index = association_matrix_in_fl[i][j];
                        remove_index1 = j;
                    }
                }
                store_index[H_pop_number++] = min_index;
                association_matrix_in_fl[i][remove_index1] = association_matrix_in_fl[i][association_num_in_fl[i] - 1];
                association_num_in_fl[i]--;
                All_pop_number++;
            }
        }

        if(All_pop_number <= pop_num)
        {
            for( m = 0; m < H_pop_number; m++)
            {
                copy_individual(parent_pop + store_index[m], offspring_pop + offspring_index);
                offspring_index++;

                if( offspring_index == pop_num )
                {
                    break;
                }
            }
        }
        else
        {
            for (i = 0; i < H_pop_number; i++)
            {

                fitnessInfo[i].value = (parent_pop + store_index[i])->fitness;
                fitnessInfo[i].idx = store_index[i];
            }

            fitness_quicksort(fitnessInfo, 0, H_pop_number - 1);

            for(i = 0; i <  pop_num - offspring_index; i++)
            {
                copy_individual(parent_pop + fitnessInfo[i].idx, offspring_pop + offspring_index);
                offspring_index++;
                if( offspring_index == pop_num - 1)
                {
                    break;
                }
            }
        }
    }

    for (i = 0; i < point_num; i++)
    {
        free(index_matrix[i]) ;
    }
    free(index_matrix);
    free(store_index);
    destroy_memory_for_pop(&H_pop, pop_num);
    free(fitnessInfo);
    return;
}

static void SPEA2_R_objectiveNormalizationAssociate(SMRT_individual * parent_pop, double **store_angle,
                                                    double **ref_point, int point_num, double *max_angle, int pop_num)
{
    int min_index = 0,i = 0, j = 0;
    double min_angle = 0,  value_cos = 0, angle = 0;
    double ** store_max_angle = NULL;
    SMRT_individual * temp_pop = NULL;
    allocate_memory_for_pop(&temp_pop, pop_num);

    store_max_angle = (double **)malloc(sizeof(double *) * point_num);
    for (i = 0; i < point_num; i++)
    {
        store_max_angle[i] = (double *)malloc(sizeof(double) * point_num);
        if (NULL == store_max_angle[i])
        {
            printf("in the non_dominated_sort, malloc distance_arr[i] Failed\n");
            return;
        }
        memset(store_max_angle[i], 0, sizeof(double) * point_num);
    }

    for(i = 0; i < pop_num; i++)
    {
        copy_individual(parent_pop + i, temp_pop + i);
    }

    for(i = 0; i < pop_num; i++)
    {
        for(j = 0; j < g_algorithm_entity.algorithm_para.objective_number ; j++)
        {
            temp_pop[i].obj[j] = (temp_pop[i].obj[j] - g_algorithm_entity.ideal_point.obj[j]) / (g_algorithm_entity.nadir_point.obj[j] - g_algorithm_entity.ideal_point.obj[j]);
        }
    }
    //the proess of the Associate and we need to calculate the angle
    for(i = 0; i < pop_num; i++)
    {
        for(j = 0 ; j < point_num ; j++)
        {
            value_cos = CalDotProduct(temp_pop[i].obj, ref_point[j],  g_algorithm_entity.algorithm_para.objective_number) / (CalNorm(temp_pop[i].obj, g_algorithm_entity.algorithm_para.objective_number) *CalNorm(ref_point[j], g_algorithm_entity.algorithm_para.objective_number));
            angle = acos(value_cos);
            store_angle[i][j] = angle ;
        }
    }
    //calculate the max_angle
    for( i = 0 ; i < point_num; i++)
    {
        for(j = 0; j < point_num; j++)
        {
            if(i == j)
                store_max_angle[i][j] = 0;
            else
            {
                value_cos = CalDotProduct(temp_pop[i].obj, ref_point[j],  g_algorithm_entity.algorithm_para.objective_number) / (CalNorm(temp_pop[i].obj, g_algorithm_entity.algorithm_para.objective_number) *CalNorm(ref_point[j], g_algorithm_entity.algorithm_para.objective_number));
                angle = acos(value_cos);
                store_max_angle[i][j] = angle;
            }
        }
    }
    //find the max——angle and store it
    *max_angle = store_max_angle[0][0];
    for(i = 0; i < point_num; i++)
    {
        for(j = 0; j < point_num; j++)
        {
            if( store_max_angle[i][j] > *max_angle )
            {
                *max_angle = store_max_angle[i][j];
            }
        }
    }

    for(i = 0; i < pop_num; i++)
    {
        min_index = 0;
        min_angle = store_angle[i][0];

        for (j = 0; j < point_num; j++)
        {
            if (min_angle > store_angle[i][j])
            {
                 min_angle = store_angle[i][j];
                 min_index = j;
            }
        }
        association_matrix_in_fl[min_index][association_num_in_fl[min_index]++] = i;
    }

    for (i = 0; i < point_num; i++)
    {
        free(store_max_angle[i]);
    }

    free(store_max_angle);
    destroy_memory_for_pop(&temp_pop, pop_num);
    return;
}

extern void SPEA2_R_framework (SMRT_individual *parent_pop, SMRT_individual *offspring_pop, SMRT_individual *mixed_pop)
{
    int i, ref_point_num = 0;
    double max_angle = 0;
    double ** uniform_ref_point = NULL, ** store_angle = NULL;
    SMRT_individual * temp_parent_pop = NULL;

    g_algorithm_entity.iteration_number                  = 0;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;
    printf ("Progress: 1%%");

    allocate_memory_for_pop(&temp_parent_pop, g_algorithm_entity.algorithm_para.pop_size);

    uniform_ref_point = initialize_uniform_point(g_algorithm_entity.algorithm_para.pop_size, &ref_point_num);

    store_angle = (double **)malloc(sizeof(double *) * g_algorithm_entity.algorithm_para.pop_size * 2);
    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size * 2; i++)
    {
        store_angle[i] = (double *)malloc(sizeof(double) * ref_point_num);
    }

    association_matrix_in_fl = (int **)malloc(sizeof(int *) * ref_point_num);
    for (i = 0; i < ref_point_num; i++)
    {
        association_matrix_in_fl[i] = (int *)malloc(sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
    }
    association_num_in_fl = (int *)malloc(sizeof(int) * ref_point_num);
    memset(association_num_in_fl, 0, sizeof(int) * ref_point_num);

    initialize_population_real (parent_pop, g_algorithm_entity.algorithm_para.pop_size);
    evaluate_population (parent_pop, g_algorithm_entity.algorithm_para.pop_size);

    initialize_nadirpoint (parent_pop, g_algorithm_entity.algorithm_para.pop_size, &g_algorithm_entity.nadir_point);
    initialize_idealpoint(parent_pop, g_algorithm_entity.algorithm_para.pop_size, &g_algorithm_entity.ideal_point);

    track_evolution (parent_pop, g_algorithm_entity.iteration_number, 0);

    while (g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {
        g_algorithm_entity.iteration_number++;
        print_progress ();

        crossover_nsga2 (parent_pop, offspring_pop);
        mutation_pop(offspring_pop);
        evaluate_population (offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        merge_population(mixed_pop, parent_pop, g_algorithm_entity.algorithm_para.pop_size, offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        non_dominated_sort(mixed_pop, 2 * g_algorithm_entity.algorithm_para.pop_size);
        update_ideal_point(mixed_pop, 2 * g_algorithm_entity.algorithm_para.pop_size);
        update_nadir_point(mixed_pop, 2 * g_algorithm_entity.algorithm_para.pop_size);

        SPEA2_R_objectiveNormalizationAssociate(mixed_pop, store_angle, uniform_ref_point, ref_point_num, &max_angle, 2 * g_algorithm_entity.algorithm_para.pop_size);

        SPEA2_R_setFitness(mixed_pop, store_angle, ref_point_num, &max_angle, 2 * g_algorithm_entity.algorithm_para.pop_size);

        SPEA2_R_environmentalSelect(mixed_pop, parent_pop, ref_point_num, g_algorithm_entity.algorithm_para.pop_size);

        SPEA2_R_clearMem(ref_point_num, store_angle);
    }

    destroy_memory_for_pop(&temp_parent_pop, g_algorithm_entity.algorithm_para.pop_size);
    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size*2; i++)
       free(store_angle[i]);

    for (i = 0; i < ref_point_num; i++)
        free (uniform_ref_point[i]);
    free (uniform_ref_point);

    return;
}
