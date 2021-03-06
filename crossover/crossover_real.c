#include "../headers/global.h"
#include "../headers/crossover.h"
#include "../headers/mating.h"
#include "../headers/random.h"
#include "../headers/memory.h"
#include "../headers/dominance_relation.h"
#include "../headers/population.h"


extern void crossover_nsga2(SMRT_individual *parent_pop_table, SMRT_individual *offspring_pop_table)
{
    int i, temp, rand;
    int *a1, *a2;
    SMRT_individual *parent1, *parent2;

    a1 = (int *) malloc (g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    a2 = (int *) malloc (g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
        a1[i] = a2[i] = i;

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
    {
        rand     = rnd (i, g_algorithm_entity.algorithm_para.pop_size - 1);
        temp     = a1[rand];
        a1[rand] = a1[i];
        a1[i]    = temp;
        temp     = a2[rand];
        a2[rand] = a2[i];
        a2[i]    = temp;
    }



    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size ; i += 4)
    {
        parent1 = tournament_by_rank(&parent_pop_table[a1[i]], &parent_pop_table[a1[i + 1]]);
        parent2 = tournament_by_rank(&parent_pop_table[a1[i + 2]], &parent_pop_table[a1[i + 3]]);
        sbx_crossover (parent1, parent2, offspring_pop_table + i, offspring_pop_table + i + 1);
        parent1 = tournament_by_rank(&parent_pop_table[a2[i]], &parent_pop_table[a2[i + 1]]);
        parent2 = tournament_by_rank(&parent_pop_table[a2[i + 2]], &parent_pop_table[a2[i + 3]]);
        sbx_crossover (parent1, parent2, &offspring_pop_table[i + 2], &offspring_pop_table[i + 3]);
    }

    free(a1);
    free(a2);
    return;
}


extern void crossover_spea2(SMRT_individual *elite_pop_table, SMRT_individual *offspring_pop_table)
{
    int i, temp, rand;
    int *a1, *a2;
    SMRT_individual *parent1, *parent2;

    a1 = (int *) malloc (g_algorithm_entity.algorithm_para.elite_pop_size * sizeof(int));
    a2 = (int *) malloc (g_algorithm_entity.algorithm_para.elite_pop_size * sizeof(int));
    for (i = 0; i < g_algorithm_entity.algorithm_para.elite_pop_size; i++)
        a1[i] = a2[i] = i;

    for (i = 0; i < g_algorithm_entity.algorithm_para.elite_pop_size; i++)
    {
        rand     = rnd (i, g_algorithm_entity.algorithm_para.elite_pop_size - 1);
        temp     = a1[rand];
        a1[rand] = a1[i];
        a1[i]    = temp;
        temp     = a2[rand];
        a2[rand] = a2[i];
        a2[i]    = temp;
    }



    for (i = 0; i < g_algorithm_entity.algorithm_para.elite_pop_size ; i += 4)
    {
        parent1 = tournament_by_fitness(&elite_pop_table[a1[i]], &elite_pop_table[a1[i + 1]], LESSER);
        parent2 = tournament_by_fitness(&elite_pop_table[a1[i + 2]], &elite_pop_table[a1[i + 3]], LESSER);
        sbx_crossover (parent1, parent2, offspring_pop_table + i, offspring_pop_table + i + 1);
        parent1 = tournament_by_fitness (&elite_pop_table[a2[i]], &elite_pop_table[a2[i + 1]], LESSER);
        parent2 = tournament_by_fitness (&elite_pop_table[a2[i + 2]], &elite_pop_table[a2[i + 3]], LESSER);
        sbx_crossover (parent1, parent2, &offspring_pop_table[i + 2], &offspring_pop_table[i + 3]);
    }

    free(a1);
    free(a2);
    return;
}


extern void crossover_MOEAD(SMRT_individual *parent_pop_table, SMRT_individual *parent, int parent_index, SMRT_individual *offspring, NeighborType type)
{
    int i = 0;
    int rand = 0;
    int selected_id, *selected_flag = NULL;
    int parent_id[2] = {0}, choose_num = 0;

    selected_flag = malloc(g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    memset(selected_flag, 0, sizeof(int) * g_algorithm_entity.algorithm_para.pop_size);

    while(choose_num < 2)
    {
        if (NEIGHBOR == type)
        {
            rand = rnd (0, g_algorithm_entity.MOEAD_para.neighbor_size - 1);
            selected_id = g_algorithm_entity.MOEAD_para.neighbor_table[parent_index].neighbor[rand];
        }
        else
        {
            rand = rnd(0, weight_num - 1);
            selected_id = rand;
        }

        if (selected_flag[selected_id] == 0)
        {
            selected_flag[selected_id] = 1;
            parent_id[choose_num++] = selected_id;
        }
    }

    de_crossover(parent, parent_pop_table + parent_id[0],
                 parent_pop_table + parent_id[1], offspring);

    free(selected_flag);
    return;
}


extern void crossover_SMSEMOA(SMRT_individual *parent_pop_table, SMRT_individual *offspring)
{
    int i, temp, rand;
    int *a1, *a2;
    SMRT_individual *parent1, *parent2, *offspring1, *offspring2;
    DOMINATE_RELATION dominateRelation;

    allocate_memory_for_ind (&offspring1);
    allocate_memory_for_ind (&offspring2);

    a1 = (int *) malloc (g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    a2 = (int *) malloc (g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
        a1[i] = a2[i] = i;

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
    {
        rand     = rnd (i, g_algorithm_entity.algorithm_para.pop_size - 1);
        temp     = a1[rand];
        a1[rand] = a1[i];
        a1[i]    = temp;
        temp     = a2[rand];
        a2[rand] = a2[i];
        a2[i]    = temp;
    }

    parent1 = tournament_by_rank(&parent_pop_table[a1[0]], &parent_pop_table[a1[1]]);
    parent2 = tournament_by_rank(&parent_pop_table[a1[2]], &parent_pop_table[a1[3]]);
    sbx_crossover (parent1, parent2, offspring1, offspring2);

    dominateRelation = check_dominance(offspring1, offspring2);


    if (DOMINATED == dominateRelation)
    {
        copy_individual(offspring2, offspring);
    }
    else
    {
        copy_individual(offspring1, offspring);
    }

    destroy_memory_for_ind(offspring1);
    destroy_memory_for_ind(offspring2);

    return;
}


extern void crossover_HypE(SMRT_individual *parent_pop_table, SMRT_individual *offspring_pop_table)
{
    int i, temp, rand;
    int *a1, *a2;
    SMRT_individual *parent1, *parent2;

    a1 = (int *) malloc (g_algorithm_entity.algorithm_para.elite_pop_size * sizeof(int));
    a2 = (int *) malloc (g_algorithm_entity.algorithm_para.elite_pop_size * sizeof(int));
    for (i = 0; i < g_algorithm_entity.algorithm_para.elite_pop_size; i++)
        a1[i] = a2[i] = i;

    for (i = 0; i < g_algorithm_entity.algorithm_para.elite_pop_size; i++)
    {
        rand     = rnd (i, g_algorithm_entity.algorithm_para.elite_pop_size - 1);
        temp     = a1[rand];
        a1[rand] = a1[i];
        a1[i]    = temp;
        temp     = a2[rand];
        a2[rand] = a2[i];
        a2[i]    = temp;
    }



    for (i = 0; i < g_algorithm_entity.algorithm_para.elite_pop_size ; i += 4)
    {
        parent1 = tournament_by_fitness(&parent_pop_table[a1[i]], &parent_pop_table[a1[i + 1]], GREATER);
        parent2 = tournament_by_fitness(&parent_pop_table[a1[i + 2]], &parent_pop_table[a1[i + 3]], GREATER);
        sbx_crossover (parent1, parent2, offspring_pop_table + i, offspring_pop_table + i + 1);
        parent1 = tournament_by_fitness (&parent_pop_table[a2[i]], &parent_pop_table[a2[i + 1]], GREATER);
        parent2 = tournament_by_fitness (&parent_pop_table[a2[i + 2]], &parent_pop_table[a2[i + 3]], GREATER);
        sbx_crossover (parent1, parent2, &offspring_pop_table[i + 2], &offspring_pop_table[i + 3]);
    }

    free(a1);
    free(a2);
    return;
}

extern void crossover_MOEADD(SMRT_individual *parent_pop_table, int weight_id, SMRT_individual *offspring, int **association_matrix, int *association_num, int weight_num)
{
    int i = 0, a, b = 0;
    int rand_i, neighbor_id[2] = {0};
    double rand_d;
    SMRT_individual *parent1 = NULL, *parent2 = NULL, *offspring1 = NULL, *offspring2 = NULL;
    DOMINATE_RELATION dominateRelation;

    allocate_memory_for_ind (&offspring1);
    allocate_memory_for_ind (&offspring2);

    rand_d = randomperc();

    rand_i = rnd(0, g_algorithm_entity.MOEADD_para.neighbor_size - 1);
    neighbor_id[0] = g_algorithm_entity.MOEADD_para.neighbor_table[weight_id].neighbor[rand_i];
    do
    {
        rand_i = rnd(0, g_algorithm_entity.MOEADD_para.neighbor_size - 1);
        neighbor_id[1] = g_algorithm_entity.MOEADD_para.neighbor_table[weight_id].neighbor[rand_i];
    }while (neighbor_id[0] == neighbor_id[1]);

    if (association_num[neighbor_id[0]] + association_num[neighbor_id[1]] >=2 && rand_d < g_algorithm_entity.MOEADD_para.neighborhood_selection_probability)
    {
        rand_i = rnd(0, association_num[neighbor_id[0]] + association_num[neighbor_id[1]] - 1);
        if (rand_i >= association_num[neighbor_id[0]])
        {
            parent1 = parent_pop_table + association_matrix[neighbor_id[1]][rand_i - association_num[neighbor_id[0]]];
        }
        else
        {
            parent1 = parent_pop_table + association_matrix[neighbor_id[0]][rand_i];
        }
        do
        {
            rand_i = rnd(0, association_num[neighbor_id[0]] + association_num[neighbor_id[1]] - 1);
            if (rand_i >= association_num[neighbor_id[0]])
            {
                parent2 = parent_pop_table + association_matrix[neighbor_id[1]][rand_i - association_num[neighbor_id[0]]];
            }
            else
            {
                parent2 = parent_pop_table + association_matrix[neighbor_id[0]][rand_i];
            }
        }while(parent1 == parent2);
    }
    else
    {
        parent1 = parent_pop_table + rnd(0, weight_num - 1);
        do
        {
            parent2 = parent_pop_table + rnd(0,  weight_num - 1);
        }while (parent1 == parent2);
    }

    sbx_crossover (parent1, parent2, offspring1, offspring2);

    dominateRelation = check_dominance(offspring1, offspring2);

    if (DOMINATED == dominateRelation)
    {
        copy_individual(offspring2, offspring);
    }
    else
    {
        copy_individual(offspring1, offspring);
    }

    destroy_memory_for_ind(offspring1);
    destroy_memory_for_ind(offspring2);

    return;
}


extern void crossover_MOEADM2M(SMRT_individual *parent_pop_table, SMRT_individual *offspring_pop_table,int K, int S)
{


    int i = 0;int j = 0;
    int index_parent2 = 0;
    double rand = 0;double selectPro = 0.7;
    SMRT_individual *parent1 = NULL, *parent2 = NULL;
    for(i = 0;i < K;i++)
    {
        for(j = 0;j < S;j++)
        {
            parent1 = parent_pop_table+i*S+j;
            if(randomperc() < selectPro)
            {
                index_parent2 = rnd(i*S,i*S+S-1);
                parent2 = parent_pop_table+index_parent2;
                MOEADM2M_crossover_operator(parent1,parent2,offspring_pop_table+i*S+j);
            }else
            {
                index_parent2 = rnd(0,K*S-1);
                parent2 = parent_pop_table + index_parent2;
                //sbx_crossover(parent1,parent2,offspring_pop_table+i*S+j,offspring_pop_table+i*S+j);
                MOEADM2M_crossover_operator(parent1,parent2,offspring_pop_table+i*S+j);
            }
        }
    }



    return;
}


extern void crossover_MOEADFRRMAB(int op,SMRT_individual *parent,SMRT_individual *offspring,SMRT_individual *parent1,
                                  SMRT_individual *parent2,SMRT_individual *parent3,SMRT_individual *parent4,SMRT_individual *parent5)
{
    int i = 0;
    double value = 0;

    for(i = 0;i < g_algorithm_entity.algorithm_para.variable_number;i++)
    {
        switch(op)
        {
            case 0:
                value = parent->variable[i] + g_algorithm_entity.dePara.F * (parent1->variable[i] - parent2->variable[i]);

                break;
            case 1:
                value = parent->variable[i] + g_algorithm_entity.dePara.F * (parent1->variable[i] - parent2->variable[i])
                        + g_algorithm_entity.dePara.F * (parent3->variable[i] - parent4->variable[i]);


                break;
            case 2:
                value = parent->variable[i] + g_algorithm_entity.dePara.K * (parent->variable[i] - parent1->variable[i])
                        + g_algorithm_entity.dePara.F * (parent2->variable[i] - parent3->variable[i])
                        + g_algorithm_entity.dePara.F * (parent4->variable[i] - parent5->variable[i]);

                break;
            case 3:
                value = parent->variable[i] + g_algorithm_entity.dePara.K * (parent->variable[i] - parent1->variable[i])
                        + g_algorithm_entity.dePara.F * (parent2->variable[i] - parent3->variable[i]);

                break;
            default:
                break;
        }


        if(value > g_algorithm_entity.variable_higher_bound[i])
            value = g_algorithm_entity.variable_higher_bound[i];
        if(value < g_algorithm_entity.variable_lower_bound[i])
            value = g_algorithm_entity.variable_lower_bound[i];


        if(op < 2)
        {
            if(randomperc() < g_algorithm_entity.dePara.CR)
                offspring->variable[i] = fabs(value);
            else
                offspring->variable[i] = fabs(parent->variable[i]);
        }else
        {
            offspring->variable[i] = fabs(value);
        }

    }

    return;
}


 extern void RVEA_crossover_operator (SMRT_individual *parent_table, SMRT_individual *offspring_table,int popNum)
{

    int i = 0;
    int k = 0, l = 0,index = 0;
    SMRT_individual *parent1 = NULL, *parent2 = NULL;

    for(i = 0;i < popNum/2;i++)
    {
        //随即选两个父代
        k = rnd(0,popNum-1);
        l = rnd(0,popNum-1);
        while(k == l)
            l = rnd(0,popNum-1);

        parent1 = parent_table + k;
        parent2 = parent_table + l;

        sbx_crossover(parent1,parent2,offspring_table+index,offspring_table+index+1);
        index+=2;

    }
}


extern void crossover_TWO_ARCH2(SMRT_individual *CA, int CA_num, SMRT_individual *DA, int DA_num, SMRT_individual *offspring_pop_table, int off_num)
{
    int i = 0, rand_1 = 0, rand_2 = 0;
    SMRT_individual *parent1 = NULL,*parent2 = NULL;

    for (i = 0; i < off_num; i+= 2)
    {
        rand_1 = rnd(0, CA_num - 1);
        rand_2 = rnd(0, CA_num - 1);
        if (DOMINATE == check_dominance(CA + rand_1, CA + rand_2))
        {
            parent1 = CA + rand_1;
        }
        else
        {
            parent1 = CA + rand_2;
        }
        rand_1 = rnd(0, DA_num - 1);
        parent2 = DA + rand_1;
        sbx_crossover(parent1, parent2, offspring_pop_table + i, offspring_pop_table + i + 1);
    }

    return;
}

extern void crossover_ONEBYONE (SMRT_individual *parent_pop_table, SMRT_individual *offspring_pop_table)
{
    int i, temp, rand;
    int *a1, *a2;
    SMRT_individual *parent1, *parent2;

    a1 = (int *) malloc (g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    a2 = (int *) malloc (g_algorithm_entity.algorithm_para.pop_size * sizeof(int));
    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
        a1[i] = a2[i] = i;

    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size; i++)
    {
        rand     = rnd (i, g_algorithm_entity.algorithm_para.pop_size - 1);
        temp     = a1[rand];
        a1[rand] = a1[i];
        a1[i]    = temp;
        temp     = a2[rand];
        a2[rand] = a2[i];
        a2[i]    = temp;
    }



    for (i = 0; i < g_algorithm_entity.algorithm_para.pop_size ; i += 4)
    {
        parent1 = tournament_by_rank(&parent_pop_table[a1[i]], &parent_pop_table[a1[i + 1]]);
        parent2 = tournament_by_rank(&parent_pop_table[a1[i + 2]], &parent_pop_table[a1[i + 3]]);
        sbx_crossover (parent1, parent2, offspring_pop_table + i, offspring_pop_table + i + 1);
        parent1 = tournament_by_rank(&parent_pop_table[a2[i]], &parent_pop_table[a2[i + 1]]);
        parent2 = tournament_by_rank(&parent_pop_table[a2[i + 2]], &parent_pop_table[a2[i + 3]]);
        sbx_crossover (parent1, parent2, &offspring_pop_table[i + 2], &offspring_pop_table[i + 3]);
    }

    free(a1);
    free(a2);
    return;
}


extern void crossover_KnEA (SMRT_individual *parent_table, SMRT_individual *offspring_table,int *K,int popNum,double *weightedDis)
{

    int i = 0;
    int k = 0, l = 0,index = 0;
    SMRT_individual *parent1 = NULL, *parent2 = NULL;

    for(i = 0;i < popNum/2;i++)
    {
        //随即选两个父代
        k = rnd(0,popNum-1);
        l = rnd(0,popNum-1);
        while(k == l)
            l = rnd(0,popNum-1);

        parent1 = tournament_KnEA(parent_table,k, l, K, weightedDis);

        k = rnd(0,popNum-1);
        l = rnd(0,popNum-1);
        while(k == l)
            l = rnd(0,popNum-1);

        parent2 = tournament_KnEA(parent_table,k, l, K, weightedDis);

        sbx_crossover(parent1,parent2,offspring_table+index,offspring_table+index+1);
        index+=2;

    }
}

extern void crossover_AGE2(SMRT_individual *parent_table, SMRT_individual *offspring_table)
{

    int i = 0;
    int *matingPool, index2 = 0;
    int k = 0, l = 0,index = 0;

    SMRT_individual *parent1 = NULL, *parent2 = NULL;

    matingPool = (int *)malloc(sizeof(int) * g_algorithm_entity.algorithm_para.pop_size);
    for(i = 0;i < g_algorithm_entity.algorithm_para.pop_size;i++)
        matingPool[i] = -1;

    for(i = 0;i < g_algorithm_entity.algorithm_para.pop_size;i++)
    {
        //printf("\n %d",parent_table[i].rank );
        if(randomperc() < 1/(double)(parent_table[i].rank+1))
            matingPool[index++] = i;
    }

    for(i = 0;i < g_algorithm_entity.algorithm_para.pop_size/2;i++)
    {
        //随即选两个父代
        k = rnd(0,index-1);
        l = rnd(0,index-1);
        while(k == l)
            l = rnd(0,index-1);

        parent1 = tournament_AGE2(parent_table,k, l, matingPool);

        k = rnd(0,index-1);
        l = rnd(0,index-1);
        while(k == l)
            l = rnd(0,index-1);

        parent2 = tournament_AGE2(parent_table,k, l, matingPool);

        sbx_crossover(parent1,parent2,offspring_table+index2,offspring_table+index2+1);
        index2 += 2;

    }

}


extern void real_crossover_Borg(SMRT_individual *parent_table,int pop_num, SMRT_individual *Archive,int archive_num,SMRT_individual *offspring,int currentOPNum, int tournmentSize)
{
    int i = 0;
    int *perm;
    double zeta;
    double eta;
    double expansion;
    int k = 0, l = 0,index = 0;
    DOMINATE_RELATION result;
    SMRT_individual *parent_list;
    SMRT_individual *offspring1, *offspring2 ;

    perm = (int *)malloc(sizeof(int) * pop_num);
    allocate_memory_for_pop(&parent_list,10);

    allocate_memory_for_ind (&offspring1);
    allocate_memory_for_ind (&offspring2);

    random_permutation(perm,pop_num);

    switch(currentOPNum)
    {
        case 0:
            //DE
            g_algorithm_entity.dePara.CR = 0.1;
            g_algorithm_entity.dePara.F  = 0.5;

            k = rnd(0,archive_num-1);
            copy_individual(Archive+k, parent_list);
            copy_individual(tournament_Borg(parent_table,perm,tournmentSize), parent_list + 1);
            random_permutation(perm,pop_num);
            copy_individual(tournament_Borg(parent_table,perm,tournmentSize), parent_list + 2);


            de_crossover(parent_list,parent_list+1,parent_list+2,offspring);
            break;
        case 1:
            //SBX
            k = rnd(0,archive_num-1);
            copy_individual(Archive+k, parent_list);
            copy_individual(tournament_Borg(parent_table,perm,tournmentSize), parent_list + 1);
            sbx_crossover (parent_list, parent_list+1, offspring, offspring + 1);

            break;
        case 2:
            //PCX
            zeta = 0.1;
            eta = 0.1;

            k = rnd(0,archive_num-1);
            copy_individual(Archive+k, parent_list);
            for(i = 1;i < 10;i++)
            {
                copy_individual(tournament_Borg(parent_table,perm,tournmentSize), parent_list + i);
                random_permutation(perm,pop_num);
            }

            PCX(parent_list,10,offspring,zeta,eta);
            PCX(parent_list,10,offspring+1,zeta,eta);
            break;
        case 3:
            //SPX
            expansion = 3.0;

            k = rnd(0,archive_num-1);
            copy_individual(Archive+k, parent_list);
            for(i = 1;i < 10;i++)
            {
                copy_individual(tournament_Borg(parent_table,perm,tournmentSize), parent_list + i);
                random_permutation(perm,pop_num);
            }

            SPX(parent_list,10,offspring,expansion);
            SPX(parent_list,10,offspring+1,expansion);
            break;

        case 4:
            //UNDX
            zeta = 0.5;
            eta = 0.35;

            k = rnd(0,archive_num-1);
            copy_individual(Archive+k, parent_list);
            for(i = 1;i < 10;i++)
            {
                copy_individual(tournament_Borg(parent_table,perm,tournmentSize), parent_list + i);
                random_permutation(perm,pop_num);
            }

            UNDX(parent_list,10,offspring,zeta,eta);
            UNDX(parent_list,10,offspring+1,zeta,eta);
            break;
        case 5:

            //UM
            k = rnd(0,archive_num-1);
            copy_individual(Archive+k , parent_list);
            UniformMutation(parent_list,offspring);
            break;

        default:
            break;

    }

    offspring->operatorNum = currentOPNum;

    destroy_memory_for_ind(offspring1);
    destroy_memory_for_ind(offspring2);
    destroy_memory_for_pop(&parent_list,10);

}


extern void crossover_tDEA(SMRT_individual *parent_table, SMRT_individual *offspring_table,int popNum)
{

    int i = 0;
    int k = 0, l = 0,index = 0;
    SMRT_individual *parent1 = NULL, *parent2 = NULL;

    for(i = 0;i < popNum/2;i++)
    {
        //随即选两个父代
        k = rnd(0,popNum-1);
        l = rnd(0,popNum-1);
        while(k == l)
            l = rnd(0,popNum-1);

        parent1 = parent_table + k;
        parent2 = parent_table + l;

        sbx_crossover(parent1,parent2,offspring_table+index,offspring_table+index+1);
        index+=2;

    }
}

extern void crossover_MaOEAIT(SMRT_individual *parent_table, SMRT_individual *offspring_table,int popNum)
{

    int i = 0;
    int k = 0, l = 0,index = 0;
    Compare_type type = LESSER;
    SMRT_individual *parent1 = NULL, *parent2 = NULL;


    for(i = 0;i < popNum/2;i++)
    {
        //随即选两个父代
        k = rnd(0,popNum-1);
        l = rnd(0,popNum-1);
        while(k == l)
            l = rnd(0,popNum-1);

        parent1 = tournament_by_fitness(parent_table + k,parent_table + l,type);

        k = rnd(0,popNum-1);
        l = rnd(0,popNum-1);
        while(k == l)
            l = rnd(0,popNum-1);

        parent2 = tournament_by_fitness(parent_table + k,parent_table + l,type);

        sbx_crossover(parent1,parent2,offspring_table+index,offspring_table+index+1);
        index+=2;
    }
}



extern void crossover_I_DBEA(SMRT_individual *parent_pop_table, SMRT_individual *offspring, int parent_index)
{
    int rand_index, parent2_index;
    SMRT_individual *parent1, *parent2,*offspring2;
    parent1 = &parent_pop_table[parent_index];

    rand_index = rnd(1, g_algorithm_entity.algorithm_para.pop_size - 1);
    parent2_index = (parent_index + rand_index) % g_algorithm_entity.algorithm_para.pop_size;
    parent2 = &parent_pop_table[parent2_index];
    offspring2 = parent2;

    sbx_crossover (parent1, parent2, offspring, offspring2);

    return;
}



