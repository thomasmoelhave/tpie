// Copyright (c) 1995 Darren Vengroff
//
// File: lru_sim.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/20/95
//

#include <versions.h>
VERSION(lru_sim_cpp,"$Id: lru_sim.cpp,v 1.4 2003-04-20 23:51:40 tavi Exp $");

#include <stdlib.h>
#include <strstream>
#include <iostream>

using std::istrstream;
using std::cout;

#include "lru_sim.h"

///extern "C" int srandom(int);
///extern "C" int random(void);

static int random_seed = 17;
static int mem_size = MAIN_MEM_OBJECTS;
static int test_size = MAIN_MEM_OBJECTS;
static int page_size = OBJECTS_PER_PAGE;
static bool verbose = false;

int main(int argc, char **argv)
{
    long int ii;

    long int page_faults = 0;

    static char opts[] = "z:t:vp:m:";
    
    ///GetOpt go(argc, argv, opts);
    char c;

    ///while ((c = go()) != -1) {
    optarg = NULL;
    while((c = getopt(argc, argv, opts)) != -1) {
        switch (c) {
            case 'v':
	      verbose = !verbose;
	      break;
            case 't':  // Test size in objects.
	      ///istrstream(go.optarg,strlen(go.optarg)) >> test_size;
	      istrstream(optarg, strlen(optarg)) >> test_size;
	      break;
            case 'm':  // Mem size in objects.
	      ///istrstream(go.optarg,strlen(go.optarg)) >> mem_size;
	      istrstream(optarg, strlen(optarg)) >> mem_size;
	      break;
            case 'p':  // Page size in objects.
	      ///istrstream(go.optarg,strlen(go.optarg)) >> page_size;
              istrstream(optarg, strlen(optarg)) >> page_size;
	      break;                
            case 'z':
	      ///istrstream(go.optarg,strlen(go.optarg)) >> random_seed;
	      istrstream(optarg, strlen(optarg)) >> random_seed;
	      break;
        }
    }

    lru_list_node *page_table = new lru_list_node[mem_size / page_size];
    lru_list_node *lru_page;
    lru_list_node *mru_page;

    page_id *page_references = new page_id[test_size];

    
    srandom(random_seed);
    
    // Fill the array of address references.
    for (ii = test_size; ii--; ) {
        page_references[ii] = ii / page_size;
    }
    
    // Randomly order the list of address references.
    {
        page_id tmp;
        long int swap_index;
        
        for (ii = test_size; ii--; ) {
            swap_index = random() % (ii + 1);
            tmp = page_references[ii];
            page_references[ii] = page_references[swap_index];
            page_references[swap_index] = tmp;
        }
    }

    if (verbose) {
        cout << "Page references:\n";
        for (ii = test_size; ii--; ) {
            cout << page_references[ii] << '\n';
        }
    }

    // Set up an empty page table.
    {
        lru_list_node *p;

        for (p = page_table + (mem_size / page_size) - 1;
             p >= page_table; p--) {
            p->page= -1;
            p->more_recent = p+1;
            p->less_recent = p-1;
        }
        lru_page = page_table;
        lru_page->less_recent = NULL;
        mru_page = page_table  + (mem_size / page_size) - 1;
        mru_page->more_recent = NULL;
    }

    if (verbose) {
        cout << "Set up empty page table.\n";
    }
    
    // Walk through the list of page references.
    {
        page_id current;
        lru_list_node *pte;

        page_faults = 0;
        
        for (ii = test_size; ii--; ) {
            current = page_references[ii];

            if (verbose && !(ii % REPORT_INTERVAL)) {
                cout << ii << '\n';
            }
            
            if (verbose) {
                cout << "Current page table (LRU first):\n";
                for (pte = lru_page; pte; pte = pte->more_recent) {
                    cout << pte->page << '\n';
                }
                cout << "Current page table (MRU first):\n";
                for (pte = mru_page; pte; pte = pte->less_recent) {
                    cout << pte->page << '\n';
                }
                
                cout << "\npage " << current;
            }
            
            // Search the page table for the current page.

            for (pte = lru_page; (pte != NULL) && (pte->page != current);
                                 pte = pte->more_recent) {
                if (verbose) {
                    //cout << "checking " << pte->page << '\n';
                    ;
                }
            }
            // Did we find it?
            if (pte) {

                if (verbose) {
                    cout << " found.\n";
                }
                
                // We found it, so pull it out of the list.
                
                if (pte->less_recent != NULL) {
                    pte->less_recent->more_recent = pte->more_recent;
                } else {
                    // pte is lru
                    lru_page = lru_page->more_recent;
                }
                if (pte->more_recent != NULL) {
                    pte->more_recent->less_recent = pte->less_recent;
                } else {
                    // pte is mru
                    mru_page = pte->less_recent;
                }
            } else {
                
                if (verbose) {
                    cout << " not found.\n";
                }
                
                // We did not find it, so take the lru instead.
                pte = lru_page;
                lru_page = lru_page->more_recent;
                lru_page->less_recent = NULL;
                pte->page = current;

                page_faults++;
            }

            // Now patch it in at the mru end of the list.
            pte->more_recent = NULL;
            pte->less_recent = mru_page;
            mru_page->more_recent = pte;
            mru_page = pte;
        }
    }
    
    // Report the number of page faults.

    if (verbose) {
        cout << "Total page faults: ";
    }
    
    cout << random_seed << '\t' << mem_size << '\t' << test_size << '\t' << page_faults << '\n';

    // Done
    
    return 0;
}


