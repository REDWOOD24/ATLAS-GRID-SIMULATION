#ifndef JOB_STATS_H
#define JOB_STATS_H

struct JobSiteStats {
    int pending = 0;
    int assigned = 0;
    int failed = 0;
    int finished = 0;
    int running = 0;
};

#endif 