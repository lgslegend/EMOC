#include "../headers/global.h"
#include "../headers/metaheuristics.h"
#include "../headers/crossover.h"
#include "../headers/mutation.h"
#include "../headers/problem.h"
#include "../headers/print.h"
#include "../headers/analysis.h"
#include "../headers/sort.h"
#include "../headers/utility.h"
#include "../headers/memory.h"


//association_matrix_without_fl[refpoint][population]
static int **association_matrix_without_fl = NULL, **association_matrix_in_fl = NULL;
static int *association_num_without_fl = NULL, *association_num_in_fl = NULL;


static void NSGA3_clear_mem(int ref_point_num, double **distance)
{
    int i = 0, j = 0;

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size*2; i++)
    {
        memset(distance[i], 0, ref_point_num * sizeof(double));
    }


    for (i = 0; i < ref_point_num; ++i)
    {
        memset(association_matrix_without_fl[i], 0, sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
    }

    for (i = 0; i < ref_point_num; ++i)
    {
        memset(association_matrix_in_fl[i], 0, sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
    }

    memset(association_num_without_fl, 0, sizeof(int) * ref_point_num);
    memset(association_num_in_fl, 0, sizeof(int) * ref_point_num);
    return;
}


static void NSGA3_fill_nd_pop(SMRT_individual *old_pop, int old_pop_num, SMRT_individual *new_pop, SMRT_individual *candidate_pop, int *candidate_num, int *selected_num, int *last_rank)
{
    int i = 0, j = 0;
    int rank_index = 0, temp_number = 0, current_pop_num = 0;

    *candidate_num = 0;
    *selected_num = 0;


    while (1)
    {
        temp_number = 0;
        for (i = 0; i < old_pop_num; i++)
        {
            if (old_pop[i].rank == rank_index)
            {
                temp_number++;
                copy_individual(old_pop + i, candidate_pop + (*candidate_num));
                (*candidate_num)++;
            }
        }
        if (current_pop_num + temp_number <= g_algorithm_entity.algorithm_para.pop_size)
        {
            for (i = 0; i < old_pop_num; i++)
            {
                if (old_pop[i].rank == rank_index)
                {
                    copy_individual(old_pop + i, new_pop + current_pop_num);
                    current_pop_num++;
                }
            }
            rank_index++;
        }
        else
            break;
    }


    *last_rank = rank_index;
    *selected_num = current_pop_num;

    return;
}

static void NSGA3_getExtremePoints (SMRT_individual *candidate_pop, SMRT_individual *extreme_pop, int num_candidates)
{
    int i = 0, j =0, k = 0;
    int min_idx = 0;
    double *max_value = NULL, **weight_vec = NULL;
    double temp_ASF = 0, temp_max_value = 0, min_value = 0;


    max_value = (double *)malloc(sizeof(double) * num_candidates);
    if (NULL == max_value)
    {
        printf("in the NSGA3_getExtremePoints, malloc max_value Failed\n");
        return;
    }

    weight_vec = (double **)malloc(sizeof(double *) * g_algorithm_entity.algorithm_para.objective_number);
    if (NULL == weight_vec)
    {
        printf("in the NSGA3_getExtremePoints, malloc weight_vec Failed\n");
        return;
    }

    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        weight_vec[i] = (double *)malloc(sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);
        if (NULL == weight_vec[i])
        {
            printf("in the NSGA3_getExtremePoints, malloc weight_vec[i] Failed\n");
            return;
        }
    }

    /*initialize weight vector*/
    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        for (j = 0; j < g_algorithm_entity.algorithm_para.objective_number; j++)
        {
            if (i == j)
            {
                weight_vec[i][j] = 1;
            }
            else
            {
                weight_vec[i][j] = EPS;

            }
        }
    }

    /*minimum ASF*/

    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        for (j = 0; j < num_candidates; j++)
        {
            temp_max_value = 0;
            for (k = 0; k < g_algorithm_entity.algorithm_para.objective_number; k++)
            {
                temp_ASF = (candidate_pop[j].obj[k] - g_algorithm_entity.ideal_point.obj[k]) / weight_vec[i][k];

                if (temp_ASF > temp_max_value)
                {
                    temp_max_value = temp_ASF;
                }
            }
            max_value[j] = temp_max_value;
        }

        min_idx = 0;
        min_value = max_value[0];

        for (j = 1; j < num_candidates; j++)
        {
            if (max_value[j] < min_value)
            {
                min_idx = j;
            }
        }

        copy_individual(candidate_pop + min_idx, extreme_pop + i);
    }

    free(max_value);
    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        free(weight_vec[i]);
    }
    free(weight_vec);
    return;
}


/* Solve the linear system Ax = b */
double* gaussianElimination (double **A, double *b, double *x)
{
    int i, j, p;
    int N, max;
    double alpha, sum, t;
    double *temp;

    N = g_algorithm_entity.algorithm_para.objective_number;
    for (p = 0; p < N; p++)
    {
        // find pivot row and swap
        max = p;
        for (i = p + 1; i < N; i++)
            if (fabs(A[i][p]) > fabs(A[max][p]))
                max = i;
        temp   = A[p];
        A[p]   = A[max];
        A[max] = temp;
        t      = b[p];
        b[p]   = b[max];
        b[max] = t;

        // singular or nearly singular
        if (fabs(A[p][p]) <= EPS)
            return NULL;

        // pivot within A and b
        for (i = p + 1; i < N; i++)
        {
            alpha = A[i][p] / A[p][p];
            b[i] -= alpha * b[p];
            for ( j = p; j < N; j++)
                A[i][j] -= alpha * A[p][j];
        }
    }

    // back substitution
    for (i = N - 1; i >= 0; i--)
    {
        sum = 0.0;
        for (j = i + 1; j < N; j++)
            sum += A[i][j] * x[j];
        x[i] = (b[i] - sum) / A[i][i];
    }

    return x;
}



static void NSGA3_getIntercepts (SMRT_individual *extreme_pop, SMRT_individual *candidate_pop, int num_candidates, double *intercept)
{
    int i = 0, j = 0;
    int flag = 0;
    double **arg = NULL, *u = NULL, *max_obj_value = NULL;


    arg = (double **)malloc(sizeof(double *) * g_algorithm_entity.algorithm_para.objective_number);
    if (NULL == arg)
    {
        printf("in the NSGA3_getExtremePoints, malloc arg Failed\n");
        return;
    }

    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        arg[i] = (double *)malloc(sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);
        if (NULL == arg[i])
        {
            printf("in the NSGA3_getExtremePoints, malloc arg[i] Failed\n");
            return;
        }
    }

    max_obj_value = (double *)malloc(sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);
    if (NULL == max_obj_value)
    {
        printf("in the NSGA3_getExtremePoints, malloc max_obj_value Failed\n");
        return;
    }

    u = (double *)malloc(sizeof(double) * g_algorithm_entity.algorithm_para.objective_number);
    if (NULL == u)
    {
        printf("in the NSGA3_getExtremePoints, malloc u Failed\n");
        return;
    }

    /* initialize */
    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        max_obj_value[i] = -EPS ;
        g_algorithm_entity.nadir_point.obj[i] = -EPS ;  //??
        //nadirPoint[i] =  1e4;
    }

    /* traverse all the individuals of the population and get their maximum value of objective (The simplest way of
     * calculating the nadir point is to get these maximum values among the first front individuals) */
    for (i = 0; i < num_candidates; i++)
    {
        for (j = 0; j < g_algorithm_entity.algorithm_para.objective_number; j++)
        {
            if (max_obj_value[j] < candidate_pop[i].obj[j] - g_algorithm_entity.ideal_point.obj[j])
                max_obj_value[j] = candidate_pop[i].obj[j] - g_algorithm_entity.ideal_point.obj[j];
            if (candidate_pop[i].rank == 0)
            {
                if (g_algorithm_entity.nadir_point.obj[j] < candidate_pop[i].obj[j] - g_algorithm_entity.ideal_point.obj[j])
                    g_algorithm_entity.nadir_point.obj[j] = candidate_pop[i].obj[j] - g_algorithm_entity.ideal_point.obj[j];
            }
        }
    }

    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        u[i] = 1;
    }

    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
        for ( j = 0; j < g_algorithm_entity.algorithm_para.objective_number; j++)
            arg[i][j] = extreme_pop[i].obj[j] - g_algorithm_entity.ideal_point.obj[j];


    if (gaussianElimination(arg, u, intercept) == NULL)
    {
        flag = 1;
    }

    if (!flag)
    {
        for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
            intercept[i] = 1 / intercept[i];
    }
    else // If the follwing condition is true this means that you have to resort to the nadir point
    {
        for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
            intercept[i] = g_algorithm_entity.nadir_point.obj[i];
    }

    /* If any of the intercepts is still Zero (which means that one of the nadir values is Zero), then use the maximum
     * value of each objective instead (remember that these values were calculated among all the individuals, not just
     * the first-front individuals) */
    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        if (intercept[i] < EPS)
        {
            for (j = 0; j < g_algorithm_entity.algorithm_para.objective_number; j++)
                intercept[j] = max_obj_value[j];
            break;
        }
    }


    free(u);
    free(max_obj_value);
    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        free(arg[i]);
    }
    free(arg);
    return;
}




static void NSGA3_association (SMRT_individual *candidate_pop, int num_candidates, int selected_num, double **distance, double **ref_point,
        int point_num,  double *intercepts)
{
    int i = 0, j = 0, k = 0;
    double d1 = 0, d2 = 0, lam = 0, min_distance = 0;
    int min_idx;

    // calculate perpendicular distances towards each reference point
    for (i = 0; i < point_num; i++)
    {
        for (j = 0; j < num_candidates; j++)
        {
            d1  = 0.0;
            lam = 0.0;
            for (k = 0; k < g_algorithm_entity.algorithm_para.objective_number; k++)
            {
                d1 += (candidate_pop[j].obj[k] - g_algorithm_entity.ideal_point.obj[k]) * ref_point[i][k] / intercepts[k];
                lam += ref_point[i][k] * ref_point[i][k];
            }
            lam = sqrt(lam);
            d1  = d1 / lam;
            d2  = 0.0;
            for (k = 0; k < g_algorithm_entity.algorithm_para.objective_number; k++)
                d2 += pow(((candidate_pop[j].obj[k] - g_algorithm_entity.ideal_point.obj[k]) / intercepts[k] - d1 * ref_point[i][k] / lam), 2.0);

            // Store the distance in the matrix and in the individual object
            distance[j][i] = sqrt(d2);
        }
    }

    for (i = 0; i < num_candidates; i++)
    {
        min_distance = distance[i][0];
        min_idx = 0;
        for (j = 1; j < point_num; j++)
        {
            if (min_distance > distance[i][j])
            {
                min_distance = distance[i][j];
                min_idx = j;
            }
        }


        if(i >= selected_num)
        {
            association_matrix_in_fl[min_idx][association_num_in_fl[min_idx]++] = i;
        }
        else
        {
            association_matrix_without_fl[min_idx][association_num_without_fl[min_idx]++] = i;
        }
    }

    return;
}





static void NSGA3_niching (SMRT_individual *candidate_pop, int candidate_num, int selected_num, SMRT_individual *new_pop, int ref_pop_num,  double **distance)
{
    int i = 0, j = 0;
    int min_num = 0, min_ref_id = 0, min_distance_id = 0, break_flag = 0;
    int selected_num_origin = 0;
    double min_distance = 0;
    int *select_flag = NULL;
    int *ref_exausted_flag = NULL;


    select_flag = (int *)malloc(sizeof(int) * candidate_num);
    if (NULL == select_flag)
    {
        printf("in the NSGA3_niching, malloc select_flag Failed\n");
        return;
    }
    memset(select_flag, 0, sizeof(int) * candidate_num);


    ref_exausted_flag = (int *)malloc(sizeof(int) * ref_pop_num);
    if (NULL == ref_exausted_flag)
    {
        printf("in the NSGA3_niching, malloc association_count1 Failed\n");
        return;
    }
    memset(ref_exausted_flag, 0, sizeof(int) * ref_pop_num);

    selected_num_origin = selected_num;

    while (selected_num != g_algorithm_entity.algorithm_para.pop_size)
    {
        min_num = INF;
        min_ref_id = 0;
        for (i = 0; i < ref_pop_num; i++)
        {
            if(min_num > association_num_without_fl[i] && (ref_exausted_flag[i] == 0))
            {
                min_num = association_num_without_fl[i];
                min_ref_id = i;
            }
        }

        if (association_num_in_fl[min_ref_id])
        {

            min_distance = INF;
            for (i = selected_num_origin; i < candidate_num; i++)
            {
                if ((min_distance > distance[i][min_ref_id]) && (select_flag[i] == 0))
                {
                    min_distance = distance[i][min_ref_id];
                    min_distance_id = i;
                }
            }
        }
        else
        {
            ref_exausted_flag[min_ref_id] = 1;
            continue;
        }

        copy_individual(candidate_pop + min_distance_id, new_pop + selected_num);
        select_flag[min_distance_id] = 1;
        association_num_without_fl[min_ref_id]++;
        association_num_in_fl[min_ref_id]--;
        selected_num++;
    }

    free(select_flag);
    free(ref_exausted_flag);
    return;
}



extern void NSGA3_framework (SMRT_individual *parent_pop, SMRT_individual *offspring_pop, SMRT_individual *mixed_pop)
{
    int i = 0;
    int ref_point_num = 0, candidate_num = 0, selected_num = 0, last_rank = 0;
    double **uniform_ref_point = NULL, **distance = NULL; //distance[pop_size][refpoint]
    double  *intercept = NULL;
    SMRT_individual *extreme_pop = NULL, *candidate_pop = NULL;

    g_algorithm_entity.iteration_number                  = 1;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;
    printf ("Progress: 1%%");

    intercept = (double *)malloc(sizeof(double ) * g_algorithm_entity.algorithm_para.objective_number);
    if (NULL == intercept)
    {
        printf("in the NSGA3_select, malloc intercept Failed\n");
        return;
    }


    allocate_memory_for_pop(&extreme_pop, g_algorithm_entity.algorithm_para.objective_number);
    allocate_memory_for_pop(&candidate_pop, g_algorithm_entity.algorithm_para.pop_size * 2);

    // initialize population
    initialize_population_real (parent_pop, g_algorithm_entity.algorithm_para.pop_size);
    evaluate_population (parent_pop, g_algorithm_entity.algorithm_para.pop_size);
    uniform_ref_point = initialize_uniform_point(&ref_point_num);

    distance = (double **)malloc(sizeof(double *) * g_algorithm_entity.algorithm_para.pop_size * 2);
    if (NULL == distance)
    {
        printf("in the NSGA3_select, malloc intercept Failed\n");
        return;
    }

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size*2; i++)
    {
        distance[i] = (double *)malloc(sizeof(double) * ref_point_num);
        if (NULL == distance[i])
        {
            printf("in the NSGA3_select, malloc distance[i] Failed\n");
            return;
        }
    }

    association_matrix_without_fl = (int **)malloc(sizeof(int *) * ref_point_num);
    if (NULL == association_matrix_without_fl)
    {
        printf("in the NSGA3_select, malloc association_matrix_without_fl Failed\n");
        return;
    }

    for (i = 0; i < ref_point_num; i++)
    {
        association_matrix_without_fl[i] = (int *)malloc(sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
        if (NULL == association_matrix_without_fl[i])
        {
            printf("in the NSGA3_select, malloc association_matrix_without_fl[i] Failed\n");
            return;
        }
    }

    association_num_without_fl = (int *)malloc(sizeof(int) * ref_point_num);
    if (NULL == association_num_without_fl)
    {
        printf("in the NSGA3_select, malloc association_num_without_fl Failed\n");
        return;
    }

    association_matrix_in_fl = (int **)malloc(sizeof(int *) * ref_point_num);
    if (NULL == association_matrix_in_fl)
    {
        printf("in the NSGA3_select, malloc association_matrix_in_fl Failed\n");
        return;
    }

    for (i = 0; i < ref_point_num; i++)
    {
        association_matrix_in_fl[i] = (int *)malloc(sizeof(int) * g_algorithm_entity.algorithm_para.pop_size * 2);
        if (NULL == association_matrix_in_fl[i])
        {
            printf("in the NSGA3_select, malloc association_matrix_in_fl[i] Failed\n");
            return;
        }
    }

    association_num_in_fl = (int *)malloc(sizeof(int) * ref_point_num);
    if (NULL == association_num_in_fl)
    {
        printf("in the NSGA3_select, malloc association_num_in_fl Failed\n");
        return;
    }

    // track the current evolutionary progress, including population and metrics
    track_evolution (parent_pop, g_algorithm_entity.iteration_number, 0);
    while (g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {
        g_algorithm_entity.iteration_number++;
        print_progress ();

        // reproduction (crossover and mutation)
        crossover_nsga2 (parent_pop, offspring_pop);
        mutation_pop(offspring_pop);
        evaluate_population (offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        update_ideal_point(offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        // environmental selection
        merge_population(mixed_pop, parent_pop, g_algorithm_entity.algorithm_para.pop_size, offspring_pop, g_algorithm_entity.algorithm_para.pop_size);

        non_dominated_sort(mixed_pop, g_algorithm_entity.algorithm_para.pop_size * 2);

        NSGA3_fill_nd_pop(mixed_pop, g_algorithm_entity.algorithm_para.pop_size * 2, g_algorithm_entity.parent_population, candidate_pop, &candidate_num, &selected_num, &last_rank);


        NSGA3_getExtremePoints (candidate_pop, extreme_pop, candidate_num);

        NSGA3_getIntercepts (extreme_pop, candidate_pop, candidate_num, intercept);

        NSGA3_association (candidate_pop, candidate_num, selected_num, distance, uniform_ref_point, ref_point_num, intercept);

        NSGA3_niching (candidate_pop, candidate_num, selected_num, parent_pop, ref_point_num , distance);

        NSGA3_clear_mem(ref_point_num, distance);


        // track the current evolutionary progress, including population and metrics
        track_evolution (parent_pop, g_algorithm_entity.iteration_number, g_algorithm_entity.algorithm_para.current_evaluation >= g_algorithm_entity.algorithm_para.max_evaluation);
    }

    for (i = 0; i < ref_point_num; i++)
        free (uniform_ref_point[i]);
    free (uniform_ref_point);
    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size*2; i++)
        free (distance[i]);
    for (i = 0; i < ref_point_num; ++i)
        free(association_matrix_without_fl[i]);
    free(association_matrix_without_fl);
    free(association_num_without_fl);
    for (i = 0; i < ref_point_num; ++i)
        free(association_matrix_in_fl[i]);
    free(association_matrix_in_fl);
    free(association_num_in_fl);
    free (distance);
    free(intercept);
    destroy_memory_for_pop(&candidate_pop, g_algorithm_entity.algorithm_para.pop_size * 2);
    destroy_memory_for_pop(&extreme_pop, g_algorithm_entity.algorithm_para.objective_number);

    return;
}
