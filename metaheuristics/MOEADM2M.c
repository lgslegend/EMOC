#include "../headers/global.h"
#include "../headers/population.h"
#include "../headers/analysis.h"
#include "../headers/crossover.h"
#include "../headers/mutation.h"
#include "../headers/problem.h"
#include "../headers/utility.h"
#include "../headers/memory.h"
#include "../headers/sort.h"
#include "../headers/random.h"
#include "../headers/print.h"
#include "../headers/initialize.h"

#define MAXSUBPOPNUMBER 300




static int partition_by_obj(SMRT_individual * pop_table, int sort_pop[], int left, int right, int obj_index)
{
    double temp_obj = pop_table[sort_pop[left]].obj[obj_index];
    int temp_index = sort_pop[left];
    while(left < right)
    {
        while ((left < right) && (pop_table[sort_pop[right]].obj[obj_index] >= temp_obj))right--;
        if (left < right)
        {
            sort_pop[left] = sort_pop[right];
            left++;
        }
        while ((left < right) && (pop_table[sort_pop[left]].obj[obj_index] < temp_obj))left++;
        if (left < right)
        {
            sort_pop[right] = sort_pop[left];
            right--;
        }
    }
    sort_pop[left] = temp_index;
    return left;
}


static void quicksort_by_obj(SMRT_individual* pop_table, int sort_arr[], int left, int right, int obj_index)
{
    int pos = 0;

    if (left < right)
    {
        pos = partition_by_obj(pop_table, sort_arr, left, right, obj_index);
        quicksort_by_obj(pop_table, sort_arr, pos + 1, right, obj_index);
        quicksort_by_obj(pop_table, sort_arr, left, pos - 1, obj_index);
    }
    return;
}

/*对某一个rank的solution按照某一objective进行排序，返回当前rank的solution的个数*/
static int sort_by_obj_rank(SMRT_individual *pop_table, int *pop_index,int sort_arr[], int obj_index, int rank_index, int pop_num)
{
    int i = 0, j = 0;
    int array_num = 0;

    for (i = 0; i < pop_num; i++)
    {
        if (pop_table[pop_index[i]].rank == rank_index)
        {
            sort_arr[array_num++] = pop_index[i];
        }
    }

    quicksort_by_obj(pop_table, sort_arr, 0, array_num - 1, obj_index);

    return array_num;
}



static void setDistance_by_index(Distance_info_t *distance_arr, int index, int pop_num, double distance)
{
    int k = 0;
    for (k = 0; k < pop_num; k++)
    {
        if (distance_arr[k].idx == index)
        {
            distance_arr[k].E_distance += distance;
        }
    }
    return;
}




/*在指定的rank里面计算crowding distance 排序结果丢在pop_sort里面*/
static int crowding_distance_assign(SMRT_individual *pop_table, int *pop_index,int pop_sort[], int pop_num,  int rank_index)
{
    int i = 0, j = 0, k = 0;
    int pop_num_in_rank = 0;
    int *sort_arr = NULL;
    Distance_info_t *distance_arr;

    sort_arr = (int*)malloc(sizeof(int) * pop_num);
    if (NULL == sort_arr)
    {
        goto CROWDING_DISTANCE_FAIL_HANDLE;
    }

    distance_arr  = (Distance_info_t*) malloc(sizeof(Distance_info_t) * pop_num);
    if (NULL == distance_arr)
    {
        goto CROWDING_DISTANCE_FAIL_HANDLE;
    }


    /*找出所有对应rank的值*/
    for (i = 0; i < pop_num; i++)
    {
        if (pop_table[pop_index[i]].rank == rank_index)
        {
            distance_arr[pop_num_in_rank++].idx = pop_index[i];
            distance_arr[pop_num_in_rank-1].E_distance = 0;
        }
    }



    for (i = 0; i < g_algorithm_entity.algorithm_para.objective_number; i++)
    {
        memset(sort_arr, 0, sizeof(int) * pop_num);
        sort_by_obj_rank(pop_table, pop_index,sort_arr, i, rank_index, pop_num);

        /*第一个和最后一个赋值为无穷大，为了使其能够保存下来*/
        pop_table[sort_arr[0]].fitness = 1000;
        setDistance_by_index(distance_arr, sort_arr[0], pop_num_in_rank, 1000);
        pop_table[sort_arr[pop_num_in_rank - 1]].fitness = 1000;
        setDistance_by_index(distance_arr, sort_arr[pop_num_in_rank - 1], pop_num_in_rank, 1000);
        for (j = 1; j < pop_num_in_rank - 1; j++)
        {
            double temp = pop_table[sort_arr[pop_num_in_rank - 1]].obj[i] - pop_table[sort_arr[0]].obj[i];
            pop_table[sort_arr[j]].fitness += (pop_table[sort_arr[j+1]].obj[i] - pop_table[sort_arr[j - 1]].obj[i]) /(temp);
            setDistance_by_index(distance_arr, sort_arr[j], pop_num_in_rank, pop_table[sort_arr[j]].fitness);
        }
    }



    distance_quick_sort(distance_arr, 0, pop_num_in_rank - 1);
    for (i = 0; i < pop_num_in_rank; i++)
    {
        pop_sort[i] = distance_arr[i].idx;
    }




    CROWDING_DISTANCE_FAIL_HANDLE:
    free(distance_arr);
    free(sort_arr);
    return pop_num_in_rank;
}




//pop_index中存储当前需要进行率选的个体在种群中的index
static void MOEADM2M_select(SMRT_individual *parent_pop, int pop_num,int subpop_num,int *pop_index,int *sub_partition)
{
    int i = 0, sort_num = 0;
    int *pop_sort = NULL;
    int  current_pop_num = 0, temp_number = 0, rank_index = 0;

    pop_sort = (int*)malloc(sizeof(int) * pop_num);
    if (NULL == pop_sort)
    {
        printf("malloc failed in the pop_sort\n");
        goto MOEADM2M_SELECT_TERMINATE_HANDLE;
    }

    non_dominated_sort_MOEADM2M(parent_pop,pop_num,pop_index);


    while (1)
    {
        temp_number = 0;
        for (i = 0; i < pop_num; i++)
        {
            //
            if (parent_pop[pop_index[i]].rank == rank_index)
            {
                temp_number++;
            }
        }

        //printf("\n");
        if (current_pop_num + temp_number <= subpop_num)
        {
            for (i = 0; i < pop_num; i++)
            {
                if (parent_pop[pop_index[i]].rank == rank_index)
                {

                    sub_partition[current_pop_num] = pop_index[i];
                    current_pop_num++;
                }
            }
            rank_index++;
        }
        else
            break;
    }

    if (current_pop_num == subpop_num)
    {
        goto MOEADM2M_SELECT_TERMINATE_HANDLE;
    }
    else
    {
        sort_num = crowding_distance_assign(parent_pop, pop_index,pop_sort, pop_num, rank_index);
        /*这一行有点问题，出现了SIGSEG*/
        while(current_pop_num < subpop_num)
        {
            /*对最后一层rank的solution，计算distance后在依据distance值继续加入subpopulation*/
            sub_partition[current_pop_num] = pop_sort[--sort_num];
            current_pop_num++;
        }
    }


    MOEADM2M_SELECT_TERMINATE_HANDLE:
    free(pop_sort);
    return ;
}


/* 选择K的参数 */
static void SelectParam(int obj_numm,int *K)
{

    switch (obj_numm)
    {
        case 2:
            *K = 15;
            break;
        case 3:
            *K = 45;
            break;
        default:
            break;

    }

    return;
}

/* 计算向量之间的点乘 */
static double CalDotProduct(double *vector1,double *vector2,int dimension)
{
    double dotProduct = 0;

    for (int i = 0;i < dimension;i++)
    {
        dotProduct += (vector1[i])*vector2[i];
    }

    return dotProduct;
}

/* 计算一个向量的模 */
static double CalNorm(double *vector, int dimension)
{

    double norm = 0;

    for (int i = 0;i < dimension;i++)
    {
        norm += (vector[i]*vector[i]);
    }

    return sqrt(norm);

}



/* 分配种群 */
static void AllocatePop(SMRT_individual *parent_pop, int parent_pop_num,SMRT_individual *allocated_pop, double **weight,int K,int S)
{


    int index_t = 0; //allocated_pop的临时索引
    int i = 0;
    int j = 0;
    int N = parent_pop_num;

    int M = g_algorithm_entity.algorithm_para.objective_number;
    int **partition;//wait to release K*S //存储被分配之后的index
    Angle_info_t **angle_info_array;//wait to release  N*K //存储每个个体与direction的角度信息




    partition = (int **) malloc(sizeof(int *) * K);
    for (i = 0; i < K; i++)
    {
        partition[i] = (int *) malloc(sizeof(int) * S);
        memset(partition[i], 0, sizeof(int) * S);
    }


    angle_info_array = (Angle_info_t **) malloc(sizeof(Angle_info_t *) * N);
    for (i = 0; i < N; i++)
    {
        angle_info_array[i] = (Angle_info_t *) malloc(sizeof(Angle_info_t) * K);
    }
    //angle_info_array初始化
    for (i = 0; i < N; i++)
    {

        for (j = 0; j < K; j++)
        {
            angle_info_array[i][j].idx = j;
            double temp_value = CalDotProduct(parent_pop[i].obj, weight[j],  M) / (CalNorm(parent_pop[i].obj, M) *CalNorm(weight[j], M));


            angle_info_array[i][j].cosValue = temp_value;
        }

    }

    for(i = 0;i < N;i++)
    {
        parent_pop[i].fitness = 0;
    }



    //每一行排序
    for (i = 0; i < N; i++)
    {
        angle_quick_sort(angle_info_array[i], 0, K-1 );
    }

    //统计不同subpopulation的个数
    for (int class = 0; class < K; class++)
    {
        int index = 0;

        int count = 0;
        for (i = 0; i < N; i++)
        {

            if (angle_info_array[i][K - 1].idx == class)
            {
                count++;
            }

        }

        //根据不同情况重新分配
        if (count <= S)
        {

            for (i = 0; i < N; i++)
            {
                if (angle_info_array[i][K - 1].idx == class)
                {
                    partition[class][index++] = i;
                }
            }

            //随即选加入subpopulation
            while (index < S)
            {
                int rand = rnd(0, N - 1);

                partition[class][index++] = rand;
            }

        } else
        {

            //用非支配排序筛选
            int pop_index[MAXSUBPOPNUMBER] = {0};
            for (i = 0; i < N; i++)
            {
                if (angle_info_array[i][K - 1].idx == class)
                {
                    pop_index[index++] = i;

                }
            }
            MOEADM2M_select(parent_pop, index, S, pop_index, partition[class]);


        }

    }




    for (int class = 0; class < K; class++)
    {

        for (int s = 0; s < S; s++)
        {
            copy_individual(&parent_pop[partition[class][s]], &allocated_pop[index_t++]);
        }


    }


    //释放内存
    for (i = 0; i < N; i++) {
        free(angle_info_array[i]);
    }
    free(angle_info_array);

    for (i = 0; i < K; i++) {
        free(partition[i]);
    }
    free(partition);

    return;


}






extern void MOEADM2M_framework (SMRT_individual *parent_pop, SMRT_individual *offspring_pop, SMRT_individual *mixed_pop)
{
    g_algorithm_entity.iteration_number                  = 0;
    g_algorithm_entity.algorithm_para.current_evaluation = 0;
    int N = g_algorithm_entity.algorithm_para.pop_size;
    int M = g_algorithm_entity.algorithm_para.objective_number;

    //K设置
    int K = 0;
    SelectParam(M,&K);

    //各个子种群大小设置
    int S = N/K;

    //均匀权重，即direction
    double **weight;
    int number_weight = 0;
    weight = initialize_direction_MOEADM2M(&number_weight,K);



    //额外数组存储分配之后的种群 wait to release
    SMRT_individual *allocated_pop;
    allocate_memory_for_pop(&allocated_pop,N);
    //初始化种群
    initialize_population_real(parent_pop,N);
    evaluate_population(parent_pop,N);



    //分配种群
    AllocatePop(parent_pop, N,allocated_pop, weight,K,S);


    // track the current evolutionary progress, including population and metrics
    track_evolution (allocated_pop, g_algorithm_entity.iteration_number, 0);
    while(g_algorithm_entity.algorithm_para.current_evaluation < g_algorithm_entity.algorithm_para.max_evaluation)
    {

        g_algorithm_entity.iteration_number++;
        print_progress ();

        //crossover and mutation
        crossover_MOEADM2M(allocated_pop,offspring_pop,K,S);





        //mutation_MOEADM2M(offspring_pop);

        evaluate_population(offspring_pop,N);



        //父代子代合并
        merge_population(mixed_pop,allocated_pop,N,offspring_pop,N);

        //分配种群
        AllocatePop(mixed_pop,2*N,allocated_pop,weight,K,S);


        // track the current evolutionary progress, including population and metrics
        track_evolution(allocated_pop,g_algorithm_entity.iteration_number,g_algorithm_entity.algorithm_para.current_evaluation >= g_algorithm_entity.algorithm_para.max_evaluation);

    }
    g_algorithm_entity.parent_population = allocated_pop;




    //释放内存


    for(int i = 0;i < K;i++)
    {
        free(weight[i]);
    }
    free(weight);


    return;
}