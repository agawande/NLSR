#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <assert.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>


#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>


#include "nlsr.h"
#include "nlsr_route.h"
#include "nlsr_lsdb.h"
#include "nlsr_npt.h"

int
route_calculate(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	printf("route_calculate called\n");

	if( ! nlsr->is_build_adj_lsa_sheduled )
	{
		/* Calculate Route here */
		print_routing_table();
		print_npt();		

		struct hashtb_param param_me = {0};
		nlsr->map = hashtb_create(sizeof(struct map_entry), &param_me);
		make_map();
		assign_mapping_number();		
		print_map();

		 
		do_old_routing_table_updates();
	

		int i;
		int **adj_matrix;
		int map_element=hashtb_n(nlsr->map);
		adj_matrix=malloc(map_element * sizeof(int *));
		for(i = 0; i < map_element; i++)
		{
			adj_matrix[i] = malloc(map_element * sizeof(int));
		}
		make_adj_matrix(adj_matrix,map_element);
		print_adj_matrix(adj_matrix,map_element);

		long int source=get_mapping_no(nlsr->router_name);
		long int *parent=(long int *)malloc(map_element * sizeof(long int));

		calculate_path(adj_matrix,parent, map_element, source);
		
		print_all_path_from_source(parent,source);

		
		
		for(i = 0; i < map_element; i++)
		{
			free(adj_matrix[i]);
		}
		free(parent);
		free(adj_matrix);
		hashtb_destroy(&nlsr->map);
		
	}
	nlsr->is_route_calculation_scheduled=0;

	return 0;
}

void 
calculate_path(int **adj_matrix, long int *parent, long int V, long int S)
{
	int i;
	long int v,u;
	long int *dist=(long int *)malloc(V * sizeof(long int));
	long int *Q=(long int *)malloc(V * sizeof(long int));
	long int head=0;
	/* Initial the Parent */
	for (i = 0 ; i < V; i++)
	{
		parent[i]=EMPTY_PARENT;
		dist[i]=INF_DISTANCE;
		Q[i]=i;
	} 

	dist[S]=0;
	sort_queue_by_distance(Q,dist,head,V);

	while (head < V )
	{
		u=Q[head];
		if(dist[u] == INF_DISTANCE)
		{
			break;
		}

		for(v=0 ; v <V; v++)
		{
			if( adj_matrix[u][v] > 0 ) //
			{
				if ( is_not_explored(Q,v,head+1,V) )
				{
					
					if( dist[u] + adj_matrix[u][v] <  dist[v])
					{
						dist[v]=dist[u] + adj_matrix[u][v] ;
						parent[v]=u;
					}	

				}

			}

		}

		head++;
		sort_queue_by_distance(Q,dist,head,V);
	}

	free(Q);
	free(dist);	
}

void
print_all_path_from_source(long int *parent,long int source)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		if(me->mapping != source)
		{
			print_path(parent,(long int)me->mapping);
			printf("\n");
		}
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void
print_path(long int *parent, long int dest)
{
	if (parent[dest] != EMPTY_PARENT )
		print_path(parent,parent[dest]);

	printf(" %ld",dest);
}

int 
is_not_explored(long int *Q, long int u,long int start, long int element)
{
	int ret=0;
	long int i;
	for(i=start; i< element; i++)
	{
		if ( Q[i] == u )
		{
			ret=1;
			break;
		}
	}
	return ret;
}

void 
sort_queue_by_distance(long int *Q,long int *dist,long int start,long int element)
{
	long int i,j;
	long int temp_u;

	for ( i=start ; i < element ; i ++) 
	{
		for( j=i+1; j<element; j ++)
		{
			if (dist[Q[j]] < dist[Q[i]])
			{
				temp_u=Q[j];
				Q[j]=Q[i];
				Q[i]=temp_u;
			}
		}
	}
	
}

void
print_map(void)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		printf("Router: %s Mapping Number: %d \n",me->router,me->mapping);
		hashtb_next(e);		
	}

	hashtb_end(e);
}


void
assign_mapping_number(void)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		me->mapping=i;
		hashtb_next(e);		
	}

	hashtb_end(e);
}

void 
make_map(void)
{
	

	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;
		add_adj_data_to_map(adj_lsa->header->orig_router->name,adj_lsa->body,adj_lsa->no_link);
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void 
add_map_entry(char *router)
{

	struct map_entry *me=(struct map_entry*)malloc(sizeof(struct map_entry ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->map, e);
    	res = hashtb_seek(e, router, strlen(router), 0);

	if(res == HT_NEW_ENTRY)
	{
		me=e->data;
		me->router=(char *)malloc(strlen(router)+1);
		memset(me->router,0,strlen(router)+1);
		memcpy(me->router,router,strlen(router));
		me->mapping=0;
	}

	hashtb_end(e);
	
}


void 
add_adj_data_to_map(char *orig_router, char *body, int no_link)
{
	add_map_entry(orig_router);
	
	int i=0;
	char *lsa_data=(char *)malloc(strlen(body)+1);
	memset(	lsa_data,0,strlen(body)+1);
	memcpy(lsa_data,body,strlen(body)+1);
	char *sep="|";
	char *rem;
	char *rtr_id;
	char *length;
	char *face;
	char *metric;
	
	if(no_link >0 )
	{
		rtr_id=strtok_r(lsa_data,sep,&rem);
		length=strtok_r(NULL,sep,&rem);
		face=strtok_r(NULL,sep,&rem);
		metric=strtok_r(NULL,sep,&rem);

		add_map_entry(rtr_id);

		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);

			add_map_entry(rtr_id);

		}
	}
	
	free(lsa_data);
}

int 
get_mapping_no(char *router)
{
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res,ret;

   	hashtb_start(nlsr->map, e);
    	res = hashtb_seek(e, router, strlen(router), 0);

	if( res == HT_OLD_ENTRY )
	{
		me=e->data;
		ret=me->mapping;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);	

	return ret;

}

void
assign_adj_matrix_for_lsa(struct alsa *adj_lsa, int **adj_matrix)
{
	int mapping_orig_router=get_mapping_no(adj_lsa->header->orig_router->name);
	int mapping_nbr_router;

	int i;
	char *lsa_data=(char *)malloc(strlen(adj_lsa->body)+1);
	memset(	lsa_data,0,strlen(adj_lsa->body)+1);
	memcpy(lsa_data,adj_lsa->body,strlen(adj_lsa->body)+1);
	char *sep="|";
	char *rem;
	char *rtr_id;
	char *length;
	char *face;
	char *metric;
	
	if(adj_lsa->no_link >0 )
	{
		rtr_id=strtok_r(lsa_data,sep,&rem);
		length=strtok_r(NULL,sep,&rem);
		face=strtok_r(NULL,sep,&rem);
		metric=strtok_r(NULL,sep,&rem);

		mapping_nbr_router=get_mapping_no(rtr_id);
		adj_matrix[mapping_orig_router][mapping_nbr_router]=atoi(metric);

		for(i=1;i<adj_lsa->no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);

			mapping_nbr_router=get_mapping_no(rtr_id);
			adj_matrix[mapping_orig_router][mapping_nbr_router]=atoi(metric);

		}
	}
	
	free(lsa_data);
}

void 
make_adj_matrix(int **adj_matrix,int map_element)
{

	init_adj_matrix(adj_matrix,map_element);

	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;
		assign_adj_matrix_for_lsa(adj_lsa,adj_matrix);
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void 
init_adj_matrix(int **adj_matrix,int map_element)
{
	int i, j;
	for(i=0;i<map_element;i++)
		for(j=0;j<map_element;j++)
			adj_matrix[i][j]=0;
}

void print_adj_matrix(int **adj_matrix, int map_element)
{
	int i, j;
	for(i=0;i<map_element;i++)
	{
		for(j=0;j<map_element;j++)
			printf("%d ",adj_matrix[i][j]);
		printf("\n");
	}
}

int 
get_next_hop(char *dest_router)
{
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res,ret;

   	hashtb_start(nlsr->routing_table, e);
    	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_OLD_ENTRY )
	{
		rte=e->data;
		ret=rte->next_hop_face;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
		ret=NO_NEXT_HOP;
	}

	hashtb_end(e);	

	return ret;
}

void
add_next_hop_router(char *dest_router)
{
	if ( strcmp(dest_router,nlsr->router_name)== 0)
	{
		return ;
	}

	struct routing_table_entry *rte=(struct routing_table_entry *)malloc(sizeof(struct routing_table_entry));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->routing_table, e);
    	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_NEW_ENTRY )
	{
		rte=e->data;
		rte->dest_router=(char *)malloc(strlen(dest_router)+1);
		memset(rte->dest_router,0,strlen(dest_router)+1);
		memcpy(rte->dest_router,dest_router,strlen(dest_router));
		rte->next_hop_face=NO_NEXT_HOP;
	}
	hashtb_end(e);

}

void 
add_next_hop_from_lsa_adj_body(char *body, int no_link)
{

	int i=0;
	char *lsa_data=(char *)malloc(strlen(body)+1);
	memset(	lsa_data,0,strlen(body)+1);
	memcpy(lsa_data,body,strlen(body)+1);
	char *sep="|";
	char *rem;
	char *rtr_id;
	char *length;
	char *face;
	char *metric;

	if(no_link >0 )
	{
		rtr_id=strtok_r(lsa_data,sep,&rem);
		length=strtok_r(NULL,sep,&rem);
		face=strtok_r(NULL,sep,&rem);
		metric=strtok_r(NULL,sep,&rem);

		//printf("		Link %d	 	\n",i+1);
		//printf("		Neighbor		 : %s	\n",rtr_id);
		//printf("		Neighbor Length		 : %s	\n",length);
		//printf("		Connecting Face		 : %s	\n",face);
		//printf("		Metric			 : %s	\n",metric);

		add_next_hop_router(rtr_id);

		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);
			//printf("		Link %d	 	\n",i+1);
			//printf("		Neighbor		 : %s	\n",rtr_id);
			//printf("		Neighbor Length		 : %s	\n",length);
			//printf("		Connecting Face		 : %s	\n",face);
			//printf("		Metric			 : %s	\n",metric);

			add_next_hop_router(rtr_id);
	
		}
	}

	free(lsa_data);


}

void 
print_routing_table(void)
{
	printf("\n");
	printf("print_routing_table called\n");
	int i, rt_element;
	
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	rt_element=hashtb_n(nlsr->routing_table);

	for(i=0;i<rt_element;i++)
	{
		printf("----------Routing Table Entry %d------------------\n",i+1);
		rte=e->data;
		printf(" Destination Router: %s \n",rte->dest_router);
		rte->next_hop_face == NO_NEXT_HOP ? printf(" Next Hop Face: NO_NEXT_HOP \n") : printf(" Next Hop Face: %d \n", rte->next_hop_face);
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}

void 
do_old_routing_table_updates()
{
	
}
