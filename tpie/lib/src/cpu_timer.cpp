// Copyright (c) 1995 Darren Vengroff
//
// File: cpu_timer.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//

static char cpu_timer_id[] = "$Id: cpu_timer.cpp,v 1.1 1995-03-07 15:20:14 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___cpu_timer_id_compiler_fooler {
    char *pc;
    ___cpu_timer_id_compiler_fooler *next;
} the___cpu_timer_id_compiler_fooler = {
    cpu_timer_id,
    &the___cpu_timer_id_compiler_fooler
};



#include <cpu_timer.h>


cpu_timer::cpu_timer() :
        running(false)
{
    elapsed.tms_utime = 0;
    elapsed.tms_stime = 0;
    elapsed.tms_cutime = 0;
    elapsed.tms_cstime = 0;

    elapsed_real = 0;
}

cpu_timer::~cpu_timer()
{
}

void cpu_timer::sync()
{
    tms current;
    clock_t current_real;
    
    current_real = times(&current);

    elapsed.tms_utime += current.tms_utime - last_sync.tms_utime;
    elapsed.tms_stime += current.tms_stime - last_sync.tms_stime;
    elapsed.tms_cutime += current.tms_cutime - last_sync.tms_cutime;
    elapsed.tms_cstime += current.tms_cstime - last_sync.tms_cstime;

    elapsed_real += current_real - last_sync_real;

    last_sync = current;
    last_sync_real = current_real;
}


void cpu_timer::start()
{
    if (!running) {
        last_sync_real = times(&last_sync);
        running = true;
    }
}

void cpu_timer::stop()
{
    if (running) {
        sync();
        running = false;
    }
}

void cpu_timer::reset()
{
    if (running) {
        last_sync_real = times(&last_sync);
    }
    
    elapsed.tms_utime = 0;
    elapsed.tms_stime = 0;
    elapsed.tms_cutime = 0;
    elapsed.tms_cstime = 0;

    elapsed_real = 0;
}

ostream &operator<<(ostream &s, cpu_timer &wt)
{
    if (wt.running) {
        wt.sync();
    }

    return s << wt.elapsed.tms_utime / 100 << '.' <<
        wt.elapsed.tms_utime % 100 << "u " <<
        wt.elapsed.tms_stime / 100 << '.' <<
        wt.elapsed.tms_stime % 100 << "s " <<
        wt.elapsed_real / 100 << '.' <<
        wt.elapsed_real % 100;
}


