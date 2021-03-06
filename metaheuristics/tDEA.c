/*
 * tDEA.c:
 *  This file implements the main procedures of tDEA. It is based on the following reference:
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
#include "../headers/analysis.h"
#include "../headers/sort.h"
#include "../headers/memory.h"
#include "../headers/utility.h"
#include "../headers/random.h"


static void GetNondominatedPop(SMRT_individual *merge_pop, int merge_pop_number, SMRT_individual **ndPop, int *lastNDPOPNum)
{
    int i = 0;
    int index = 0;
    int newNDPopNum = 0;
    int  current_pop_num = 0, temp_number = 0, rank_index = 0;



    if(*lastNDPOPNum != 0 )
        destroy_memory_for_pop(ndPop,*lastNDPOPNum);

    non_dominated_sort(merge_pop, merge_pop_number);

    while (1)
    {
        temp_number = 0;
        for (i = 0; i < merge_pop_number; i++)
        {
            if (merge_pop[i].rank == rank_index)
            {
                temp_number++;
            }
        }
        if (current_pop_num + temp_number < weight_num)
        {
            current_pop_num += temp_number;
            rank_index++;
        }
        else
            break;
    }

    newNDPopNum = current_pop_num + temp_number;
    allocate_memory_for_pop(ndPop,newNDPopNum);

    *lastNDPOPNum = newNDPopNum;

    for(i = 0;i < merge_pop_number;i++)
    {
        if(merge_pop[i].rank <= rank_index)
        {
            copy_individual(merge_pop + i,(*ndPop) + index++);
        }
    }

    return;
}




//对每个点进行Cluster
static void tDEA_cluster(double **popObj, int popNum, int **C, int *count)
{
    int n = 0;
    int i = 0,j = 0;
    double min = 0, d2 = 0;

    for(i = 0;i < weight_num;i++)
        count[i] = 0;

    for(i = 0;i < popNum;i++)
    {
        n = 0;
        min = Cal_perpendicular_distance(popObj[i], lambda[0]);

        for(j = 1;j < weight_num;j++)
        {
            d2 = Cal_perpendicular_distance(popObj[i], lambda[j]);
            if( d2 < min)
            {
                min = d2;
                n = j;
            }
        }

        C[n][count[n]] = i;
        count[n] = count[n] + 1;
    }

    return;
}



static void tDEA_thetaNDSort(SMRT_individual *ndPop, double **popObj, int popNum, int **C, int *count)
{
    int i = 0, j = 0,sum = 0,index = 0;
    double *theta, tempDistance = 0, d1 = 0, d2 = 0;
    Distance_info_t distanceInfo[MAX_SIZE];

    theta = (double *)malloc(sizeof(double) * weight_num);

    //assignment to theta
    for(i = 0;i < weight_num;i++)
    {
        sum = 0;
        for(j = 0;j < g_algorithm_entity.algorithm_para.objective_number;j++)
        {
            if(lambda[i][j] > 0.0001)
            {
                sum++;
            }
        }
        if(sum == 1)
        {
            theta[i] = 100000.0;
        }
        else
        {
            theta[i] = 5.0;
        }
    }

    for(i = 0;i < weight_num;i++)
    {
        if(count[i] == 0)
            continue;
        else
        {
            for(j = 0;j < count[i];j++)
            {
                index = C[i][j];
                d1 = CalDotProduct(popObj[index],lambda[i],g_algorithm_entity.algorithm_para.objective_number)/CalNorm(lambda[i],g_algorithm_entity.algorithm_para.objective_number);
                d2 = Cal_perpendicular_distance(popObj[index],lambda[i]);
                tempDistance = d1 + theta[i] * d2;

                distanceInfo[j].value = tempDistance;
                distanceInfo[j].idx = index;
            }
            distance_quick_sort(distanceInfo,0,count[i]-1);

            for(j = 0;j < count[i];j++)
            {
                ndPop[distanceInfo[j].idx].rank = j;
            }

        }
    }

    free(theta);

    return;
}

static void tDEA_enviromentSelection(SMRT_individual *ndPop, int popNum, SMRT_individual *parent_pop)
{
    int i = 0,count = 0;
    int *index,*perm;
    int  current_pop_num = 0, temp_number = 0, rank_index = 0;

    while (1)
    {
        temp_number = 0;
        for (i = 0; i < popNum; i++)
        {
            if (ndPop[i].rank == rank_index)
            {
                temp_number++;
            }
        }
        if (current_pop_num + temp_number < weight_num)
        {
            for (i = 0; i < popNum; i++)
            {
                if (ndPop[i].rank == rank_index)
                {
                    copy_individual(ndPop + i, parent_pop + current_pop_num);
                    current_pop_num++;
                }
            }
            rank_index++;
        }
        else
            break;
    }


    index = (int *)malloc(sizeof(int) * temp_number);
    perm = (int *)malloc(sizeof(int) * temp_number);
    random_permutation(perm,temp_number);

    for(i = 0;i < popNum;i++)
    {
        if(ndPop[i].rank == rank_index)
            index[count++] = i;
    }

    while(current_pop_num < weight_num)
    {
        copy_individual(ndPop + index[rnd(0,temp_number-1)],parent_pop + current_pop_num);
        current_pop_num++;
    }

    free(perm);
    free(index);
    return;
}

extern void _tDEA_ (SMRT_individual *parent_pop, SMRT_individual* offspring_pop, SMRT_individual* mixed_pop)
{
    g_algorithm_entity.iteration_number       = 0;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;

    double **popObj;
    int *count, **C;
    int mergePopNum;
    int i = 0,j = 0;
    int lastNDPopNum = 0;
    SMRT_individual *ndPop, *extremePop;


    lambda = initialize_uniform_point (g_algorithm_entity.algorithm_para.pop_size, &weight_num);

    popObj = (double **)malloc(sizeof(double *) * g_algorithm_entity.algorithm_para.pop_size * 2);
    for(i = 0;i < g_algorithm_entity.algorithm_para.pop_size * 2;i++)
        popObj[i] = (double *)malloc(sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);

    C = (int **)malloc(sizeof(int *) * weight_num);
    for(i = 0;i < weight_num;i++)
        C[i] = (int *)malloc(sizeof(int) * weight_num * 2);

    count = (int *)malloc(sizeof(int) * weight_num);
    for(i = 0;i < weight_num;i++)
        count[i] = 0;

    allocate_memory_for_pop(&extremePop,g_algorithm_entity.algorithm_para.objective_number);


    // initialize population
    initialize_population_real (parent_pop,weight_num);
    evaluate_population (parent_pop, weight_num);

    initialize_idealpoint(parent_pop,weight_num,&g_algorithm_entity.ideal_point);
    initialize_nadirpoint(parent_pop,weight_num,&g_algorithm_entity.nadir_point);



    // track the current evolutionary progress, including population and metrics
    track_evolution (parent_pop, g_algorithm_entity.iteration_number, 0);

    while (g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {
        printf("here\n");
        g_algorithm_entity.iteration_number++;
        print_progress ();

        // reproduction (crossover and mutation)
        crossover_tDEA(parent_pop,offspring_pop,weight_num);
        mutation_pop(offspring_pop);
        evaluate_population(offspring_pop,weight_num);

        // merge population
        merge_population (mixed_pop, parent_pop,weight_num, offspring_pop, (weight_num/2) * 2);
        mergePopNum = weight_num + (weight_num/2) * 2;

        //get nondominated population
        GetNondominatedPop(mixed_pop,mergePopNum,&ndPop,&lastNDPopNum);

        Normalization(ndPop,lastNDPopNum,extremePop,popObj);

        tDEA_cluster(popObj, lastNDPopNum, C, count);

        tDEA_thetaNDSort(ndPop, popObj, lastNDPopNum, C, count);

        tDEA_enviromentSelection(ndPop, lastNDPopNum, parent_pop);

        // track the current evolutionary progress, including population and metrics
        track_evolution (parent_pop, g_algorithm_entity.iteration_number, g_algorithm_entity.algorithm_para.current_evaluation >= g_algorithm_entity.algorithm_para.max_evaluation);
    }


    for(i = 0;i < g_algorithm_entity.algorithm_para.pop_size * 2;i++)
        free(popObj[i]);
    free(popObj);

    for(i = 0;i < weight_num;i++)
        free(lambda[i]);
    free(lambda);

    destroy_memory_for_pop(&ndPop,lastNDPopNum);
    destroy_memory_for_pop(&extremePop,g_algorithm_entity.algorithm_para.objective_number);

    return;
}