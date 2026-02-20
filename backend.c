#include "backend.h"
#include <stdio.h>
#include <float.h>
#include <string.h>

static const char* upbeatPlaylist[] = {
    "Upbeat Track 1", "Upbeat Track 2", "Upbeat Track 3"
};
static const char* chillPlaylist[] = {
    "Lo-fi Chill 1", "Lo-fi Chill 2"
};
static const char* ambientPlaylist[] = {
    "Ambient Mix 1", "Ambient Mix 2"
};


Stop stops[MAX_STOPS];
int stopcount = 0;

void add_stop(const char *name) {
    stops[stopcount].name = name;
    stops[stopcount].edgecount = 0;
    stopcount++;
}

void add_edge(int from, int to, double distance, double time, double cost) {
    int i = stops[from].edgecount;
    stops[from].edge[i].to = to;
    stops[from].edge[i].distance = distance;
    stops[from].edge[i].time = time;
    stops[from].edge[i].cost = cost;
    stops[from].edgecount++;
}

void initbusnet() {
    stopcount = 0;

    add_stop("A"); add_stop("B"); add_stop("C"); add_stop("D"); add_stop("E");
    add_stop("F"); add_stop("G"); add_stop("H"); add_stop("I"); add_stop("J");

    add_edge(0,1,2.5,8,5);  add_edge(1,0,2.5,8,5);
    add_edge(0,3,4.0,15,8); add_edge(3,0,4.0,15,8);
    add_edge(1,2,3.0,10,7); add_edge(2,1,3.0,10,7);
    add_edge(1,4,5.0,18,12); add_edge(4,1,5.0,18,12);
    add_edge(2,5,5.0,12,10); add_edge(5,2,5.0,12,10);
    add_edge(3,4,2.0,6,4);   add_edge(4,3,2.0,6,4);
    add_edge(4,5,3.5,9,6);   add_edge(5,4,3.5,9,6);
    add_edge(5,6,4.0,11,8);  add_edge(6,5,4.0,11,8);
    add_edge(6,7,2.5,7,5);   add_edge(7,6,2.5,7,5);
    add_edge(7,8,3.0,9,6);   add_edge(8,7,3.0,9,6);
    add_edge(8,9,4.5,12,9);  add_edge(9,8,4.5,12,9);
    add_edge(5,8,6.0,15,11); add_edge(8,5,6.0,15,11);
    add_edge(3,6,5.5,14,10); add_edge(6,3,5.5,14,10);
    add_edge(2,7,7.0,18,13); add_edge(7,2,7.0,18,13);
}

Routeresults find_route(const char *source, const char *destination, int optimizeByTime) {
    Routeresults result;
    result.pathlength = 0;
    result.distance = 0;
    result.time = 0;
    result.cost = 0;
    result.transfers = 0;

    int start=-1, end=-1;
    for (int i=0; i<stopcount; i++) {
        if (strcmp(stops[i].name, source) == 0) start = i;
        if (strcmp(stops[i].name, destination) == 0) end = i;
    }
    if (start == -1 || end == -1) return result;

    double dist[MAX_STOPS];
    int visited[MAX_STOPS] = {0};
    int prev[MAX_STOPS];
    for (int i=0; i<MAX_STOPS; i++) {
        dist[i] = DBL_MAX;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int count=0; count<stopcount; count++) {
        double minDist = DBL_MAX;
        int u = -1;
        for (int i=0; i<stopcount; i++) {
            if (!visited[i] && dist[i]<minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visited[u] = 1;

        for (int i=0; i<stops[u].edgecount; i++) {
            int v = stops[u].edge[i].to;
            double weight = optimizeByTime ? stops[u].edge[i].time : stops[u].edge[i].distance;
            if (!visited[v] && dist[u]+weight < dist[v]) {
                dist[v] = dist[u]+weight;
                prev[v] = u;
            }
        }
    }

    int path[MAX_STOPS];
    int len = 0;
    int u = end;
    while (u != -1) {
        path[len++] = u;
        u = prev[u];
    }

    for (int i=0; i<len; i++) {
        result.path[i] = path[len-1-i];
    }
    result.pathlength = len;

    for (int i=0; i<result.pathlength-1; i++) {
        int from = result.path[i];
        int to = result.path[i+1];
        for (int j=0; j<stops[from].edgecount; j++) {
            if (stops[from].edge[j].to == to) {
                result.distance += stops[from].edge[j].distance;
                result.time += stops[from].edge[j].time;
                result.cost += stops[from].edge[j].cost;
                break;
            }
        }
    }
    result.transfers = result.pathlength > 1 ? result.pathlength-1 : 0;

    return result;


}
PlaylistResult recommend_playlist(int time, int distance) {
    PlaylistResult result;

    if(time < 15 && distance < 5) {
        result.tracks = upbeatPlaylist;
        result.trackCount = sizeof(upbeatPlaylist)/sizeof(upbeatPlaylist[0]);
        strcpy(result.name, "Upbeat Hits");
    } else if(time< 30) {
        result.tracks = chillPlaylist;
        result.trackCount = sizeof(chillPlaylist)/sizeof(chillPlaylist[0]);
        strcpy(result.name, "Lo-fi Chill");
    } else {
        result.tracks = ambientPlaylist;
        result.trackCount = sizeof(ambientPlaylist)/sizeof(ambientPlaylist[0]);
        strcpy(result.name, "Ambient Journey");
    }

    return result;
}

