// Copyright (c) 1995 Darren Vengroff
//
// File: lru_sim.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/20/95
//
// $Id: lru_sim.h,v 1.1 1995-03-07 15:11:18 darrenv Exp $
//
// A simulation of LRU paging behaviour.
//
#ifndef _LRU_SIM_H
#define _LRU_SIM_H

// Simulation parameters.

#define MAIN_MEM_MBYTES 16

#define MAIN_MEM_BYTES (MAIN_MEM_MBYTES * 1024 * 1024)

#define OBJECT_SIZE 16

#define MAIN_MEM_OBJECTS (MAIN_MEM_BYTES / OBJECT_SIZE)

#define BYTES_PER_PAGE (64 * 1024)

#define OBJECTS_PER_PAGE (BYTES_PER_PAGE / OBJECT_SIZE)

#define TEST_SIZE_BYTES (32 * 1024 * 1024)

#define TEST_REFERENCES (TEST_SIZE_BYTES / OBJECT_SIZE)

#define REPORT_INTERVAL (32 * 1024)

typedef long int page_id;

struct lru_list_node {
    page_id page;
    lru_list_node *more_recent;
    lru_list_node *less_recent;
};



#endif // _LRU_SIM_H 
