#ifndef BACKEND_H
#define BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif
# define MAX_STOPS 20

typedef struct
{
    int to;
    double distance;
    double time;
    double cost;
}Edge;

typedef struct
{
    const char *name;
    Edge edge[10];
    int edgecount;

}Stop;

typedef struct {
    double distance;
    double time;
    double cost;
    int transfers;
    int path[20];
    int pathlength;
}Routeresults;
typedef struct {
    const char** tracks;
    int trackCount;
    char name[32];
} PlaylistResult;

PlaylistResult recommend_playlist(int time, int distance);

void initbusnet();
Routeresults find_route(const char*source,const char*destination ,int optimizetime);
extern Stop stops[MAX_STOPS];
extern int stopcount;
#ifdef __cplusplus
}
#endif

#endif
