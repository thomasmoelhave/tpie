// Copyright (c) 1994 Darren Erik Vengroff
//
// File: lr.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: Tue Mar 15 1994
//
// A sample piece of code that does list ranking in the TPIE system.
//

static char lr_id[] = "$Id: lr.cpp,v 1.12 1999-02-03 21:49:50 tavi Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___lr_id_compiler_fooler {
    char *pc;
    ___lr_id_compiler_fooler *next;
} the___lr_id_compiler_fooler = {
    lr_id,
    &the___lr_id_compiler_fooler
};

#ifndef BTE_STATS
#define BTE_STATS 0
#endif

// Get the application defaults.
#include "app_config.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <iostream.h>
#include <fstream.h>
#include <ami_scan_utils.h>

// Timers.
#include <wall_timer.h>
#include <cpu_timer.h>

#include "parse_args.h"

#include "list_edge.h"
#include "scan_list.h"
#include "merge_random.h"

////////////////////////////////////////////////////////////////////////
// main_mem_list_rank()
//
// This function ranks a list that can fit in main memory.  It is used
// when the recursion bottoms out.
//
////////////////////////////////////////////////////////////////////////

int main_mem_list_rank(edge *edges, size_t count)
{
    edge *edges_copy;
    unsigned int ii,jj,kk;
    unsigned int head_index, tail_index;
    unsigned long int head_node, tail_node;
    long int total_weight;
    
    // Copy the array.
    edges_copy = new edge[count];
    for (ii = count; ii--; ) {
        edges_copy[ii] = edges[ii];
    }
    
    // Sort the original set by the from fields.
    quicker_sort_cmp(edges, count, edgefromcmp); 
    
    // Sort the copy by to.
    quicker_sort_cmp(edges_copy, count, edgetocmp); 

    // Find the head of this list, which is the unique node number
    // that appears in the list sorted by from but not by to.  At the
    // same time, we can find the tail.
    {
        bool head_found = false, tail_found = false;
        
        for (ii = kk = 0; (ii < count) || (kk < count); ) {
            tp_assert((ii - kk <= 1) || (kk - ii <= 1),
                      "Indeces more than 1 out of sync.");            
            if (edges[ii].from == edges_copy[kk].to) {
                ii++; kk++;
            } else if (ii == count) {
                tp_assert(head_found, "We should have found the head by now.");
                tp_assert(!tail_found, "We already found the tail.");
                tp_assert(kk == count-1, "kk is too far behind.");
                tail_index = kk;
                tail_node = edges_copy[kk++].to;
                tail_found = true;
                break;
            } else if (kk == count) {
                tp_assert(tail_found, "We should have found the head by now.");
                tp_assert(!head_found, "We already found the tail.");
                tp_assert(ii == count-1, "ii is too far behind.");
                head_index = ii;
                head_node = edges[ii++].to;
                head_found = true;
                break;
            } else if (edges[ii].from < edges_copy[kk].to) {
                // ii is the index of the head of the list.
                tp_assert(!head_found, "We already found the head.");
                head_index = ii;
                head_node = edges[ii++].from;
                if ((head_found = true) && tail_found)
                    break;
            } else if (edges[ii].from > edges_copy[kk].to) {
                // kk is the index of the tail of the list.
                tp_assert(!tail_found, "We already found the tail.");
                tail_index = kk;
                tail_node = edges_copy[kk++].to;
                if ((tail_found = true) && head_found)
                    break;
            }
        }

        tp_assert(head_found, "Head of list not found.");
        tp_assert(tail_found, "Tail of list not found.");
    }

    // Reduce the to fields to integers in the range [0...count-1].
    // After this is done, we will resort by source, and then we can
    // walk through the list starting at head_index.

    // There are two problems, however, one related to each end of the
    // list.  These problems arise from the fact that what we are
    // actually indexing into is an array of edges, not of nodes.  One
    // node, the head, appears only once, as a source.  Another node,
    // the tail, appears only once, as a destination.  All other nodes
    // appear exactly twice, once as a source and once as a
    // destination.

    // The first edge (the one whose source is the head) of the list
    // will be embedded somewhere in the middle of the array.  If k is
    // smaller than the index of the head edge in the array sorted by
    // source then the k'th largest destination is index k - 1.  If,
    // however, k is greater than or equal to the index of the head,
    // then the k'th largest destination is index k.  This is because
    // there is no edge whose destination is the head of the list.

    // The tail of the list it the i'th largest destination for some
    // value of i.  This means that for j >= 0, the (i + j)'th
    // destination is actually the source of the edge in position (i +
    // j - 1) when the edges are sorted by source.

    // The situation is even a slight bit more complicated than it is
    // described in the preceding paragraphs, since the two end
    // effects can interact.  Luckily, however, their effects are
    // simply additive.
    
    for (ii = count; ii--; ) { 
        edges_copy[ii].to = ii;
        if (ii >= head_index) {
            edges_copy[ii].to++;
        }
        if (ii >= tail_index) {
            edges_copy[ii].to--;
        }            
    }

    // Sort the copy back by source edge.
    
    quicker_sort_cmp(edges_copy, count, edgefromcmp); 

    // Traverse the reduced copy by taking count - 1 steps, starting
    // from the index of the head.  We use jj to keep track of the
    // number of iterations.
    for (ii = head_index, jj = count, total_weight = 0;
         jj--; ii = edges_copy[ii].to) {

        tp_assert(ii < count, "ii (= " << ii <<
                  ") out of range (jj = " << jj << ").");

        tp_assert((ii != head_index) || !total_weight || (ii == tail_node),
                  "Cycled back to the head somehow (jj = " << jj << ").");
        
        // The original array edges is sorted by source node, thus the
        // edges are in the same positions within the array as the
        // reduced copies in edge_copy.  All we have to to is add the
        // weight of all edges before this one to this one.

        total_weight = (edges[ii].weight += total_weight);
        
    }

    // We should have ended up at the tail of the list.
    tp_assert(ii = tail_node,
              "Did not end up at the tail of the list.");
    
    // Free the copy.
    delete edges_copy;

    // Done 
    return 0;  
}



////////////////////////////////////////////////////////////////////////
// random_flag_scan
//
// This is a class of scan management objects that operates by flipping
// a fair coin for each edge and setting the edge's flag accordingly.
//
// There is a single instance of this class, called my_random_flag.
////////////////////////////////////////////////////////////////////////

class random_flag_scan : AMI_scan_object {
public:
    AMI_err initialize(void);  
    AMI_err operate(const edge &in, AMI_SCAN_FLAG *sfin,
                    edge *out, AMI_SCAN_FLAG *sfout);
};

AMI_err random_flag_scan::initialize(void) {
    return AMI_ERROR_NO_ERROR;
}

AMI_err random_flag_scan::operate(const edge &in, AMI_SCAN_FLAG *sfin,
                                  edge *out, AMI_SCAN_FLAG *sfout)
{ 
    if (!(sfout[0] = *sfin)) {
        return AMI_SCAN_DONE;
    }
    *out = in;
    out->flag = (random() & 1);
    
    return AMI_SCAN_CONTINUE;
}

////////////////////////////////////////////////////////////////////////
// separate_active_from_cancel
//
// A class of scan object that takes two edges, one to a node and one 
// from it, and writes an active edge and possibly a canceled edge.
//
// Let e1 = (x,y,w1,f1) be the first edge and e2 = (y,z,w2,f2) the second.
// If e1's flag (f1) is set and e2's (f2) is not, then we write 
// (x,z,w1+w2,?) to the active list and e2 to the cancel list.  The
// effect of this is to bridge over the node y with the new active edge.
// f2, which was the second half of the bridge, is saved in the cancellation
// list so that it can be ranked later after the active list is recursively 
// ranked.
//
// Since all the flags should have been set randomly before this function
// is called, the expected size of the active list is 3/4 the size of the
// original list.
////////////////////////////////////////////////////////////////////////
class separate_active_from_cancel : AMI_scan_object {
public:
    AMI_err initialize(void);
    AMI_err operate(CONST edge &e1, CONST edge &e2, AMI_SCAN_FLAG *sfin,
                    edge *active, edge *cancel, AMI_SCAN_FLAG *sfout);
};

AMI_err separate_active_from_cancel::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
}

// e1 is from the list of edges sorted by where they are from.
// e2 is from the list of edges sorted by where they are to.
AMI_err separate_active_from_cancel::operate(CONST edge &e1,
                                             CONST edge &e2, 
                                             AMI_SCAN_FLAG *sfin,
                                             edge *active, edge *cancel, 
                                             AMI_SCAN_FLAG *sfout)
{
    // If we have both inputs.
    if (sfin[0] && sfin[1]) {
        // If they have a node in common we may be in a bridging situation.
        if (e2.to == e1.from) {
            // We will write to the active list no matter what.
            sfout[0] = 1;
            *active = e2;
            if ((sfout[1] = (e2.flag && !e1.flag))) {
                // Bridge.  Put e1 on the cancel list and add its
                // weight to the active output.
                active->to = e1.to;
                active->weight += e1.weight;
                *cancel = e1;
                sfout[1] = 1;
            } else {
                // No bridge.
                sfout[1] = 0;
            }
        } else {
            // They don't have a node in common, so one of them needs
            // to catch up with the other.  What happened is that
            // either e2 is the very last edge in the list or e1 is
            // the very first or we just missed a bridge because of
            // flags.
            sfout[1] = 0;                
            if (e2.to > e1.from) {
                // e1 is behind, so just skip it.
                sfin[1] = 0;
                sfout[0] = 0;
            } else {
                // e2 is behind, so put it on the active list.
                sfin[0] = 0;
                sfout[0] = 1;
                *active = e2;
            }
        }
        return AMI_SCAN_CONTINUE;
    } else {
        // If we only have one input, either just leave it active.
        if (sfin[0]) {
            *active = e1;
            sfout[0] = 1;
            sfout[1] = 0;
            return AMI_SCAN_CONTINUE;
        } else if (sfin[1]) {
            *active = e2;
            sfout[0] = 1;
            sfout[1] = 0;
            return AMI_SCAN_CONTINUE;
        } else {
            // We have no inputs, so we're done.
            sfout[0] = sfout[1] = 0;            
            return AMI_SCAN_DONE;
        }
    }
}

// A scan management object to take an active list and remove the
// smaller weighted edge of each pair of consecutive edges with the
// same destination.  The purpose of this is to strip edges out of the
// active list that were sent to the cancel list.
class strip_cancel_from_active : AMI_scan_object {
private:
    bool holding;
    edge hold;
public:
    AMI_err initialize(void);  
    AMI_err operate(const edge &active, AMI_SCAN_FLAG *sfin,
                    edge *out, AMI_SCAN_FLAG *sfout);
};

AMI_err strip_cancel_from_active::initialize(void) {
    holding = false;
    return AMI_ERROR_NO_ERROR;
}

// Edges should be sorted by destination before being processed by
// this object.
AMI_err strip_cancel_from_active::operate(const edge &active,
                                  AMI_SCAN_FLAG *sfin,
                                  edge *out, AMI_SCAN_FLAG *sfout)
{
    // If no input then we're done, except that we might still be
    // holding one.
    if (!*sfin) {
        if (holding) {
            *sfout = 1;
            *out = hold;
            holding = false;
            return AMI_SCAN_CONTINUE;
        } else {
            *sfout = 0;
            return AMI_SCAN_DONE;
        }
    }

    if (!holding) {
        // If we are not holding anything, then just hold the current
        // input.
        hold = active;
        holding = true;
        *sfout = 0;
    } else {
        tp_assert(active.to >= hold.to, "Out of order inputs.");

        *sfout = 1;
        
        if (active.to == hold.to) {
            tp_assert(active.from != hold.from, "Same edge.");
            tp_assert(active.weight != hold.weight, "Same weights.");

            if (active.weight > hold.weight) {
                *out = active;
            } else {
                *out = hold;
            }

            holding = false;
        } else {
            *out = hold;
            hold = active;
        }
    }

    return AMI_SCAN_CONTINUE;
}


////////////////////////////////////////////////////////////////////////
// interleave_active_cancel
//
// This is a class of merge object that merges two lists of edges
// based on their to fields.  The first list of edges should be active
// edges, while the second should be cancelled edges.  When we see two
// edges with the same to field, we know that the second was cancelled
// when the first was made active.  We then fix up the weights and
// output the two of them, one in the current call and one in the next
// call.
//
// The streams this operates on should be sorted by their terminal
// (to) nodes before AMI_scan() is called.

////////////////////////////////////////////////////////////////////////

class patch_active_cancel : AMI_scan_object {
private:
    bool holding;
    edge hold;
public:
    AMI_err initialize(void);
    AMI_err operate(CONST edge &active, CONST edge &cancel,
                    AMI_SCAN_FLAG *sfin,
                    edge *patch, AMI_SCAN_FLAG *sfout);
};

AMI_err patch_active_cancel::initialize(void)
{
    holding = false;
    return AMI_ERROR_NO_ERROR;
}

AMI_err patch_active_cancel::operate(CONST edge &active, CONST edge &cancel,
                                     AMI_SCAN_FLAG *sfin,
                                     edge *patch, AMI_SCAN_FLAG *sfout)
{
    // Handle the special cases that occur when holding an edge and/or
    // completely out of input.
    if (holding) {
        sfin[0] = sfin[1] = 0;
        *patch = hold;
        holding = false;
        *sfout = 1;
        return AMI_SCAN_CONTINUE;
    } else if (!sfin[0]) {
        tp_assert(!sfin[1], "We have cancel but no active");
        *sfout = 0;
        return AMI_SCAN_DONE;
    }

    tp_assert(sfin[0], "No active input.");
    
    if (!sfin[1]) {
        // If there is no cancel edge (i.e. all have been processed)
        // then just pass the active edge through.
        *patch = active;
    } else {
        tp_assert(active.to <= cancel.to, "Out of sync, or not sorted.");
    
        if (holding = (active.to == cancel.to)) {
            patch->from = active.from;
            patch->to = cancel.from;
            patch->weight = active.weight - cancel.weight;
            hold.from = cancel.from;
            hold.to = active.to;
            hold.weight = active.weight;
        } else {
            *patch = active;
            sfin[1] = 0;
        }
    }

    *sfout = 1;
    return AMI_SCAN_CONTINUE;

}



////////////////////////////////////////////////////////////////////////
// list_rank()
//
// This is the actual recursive function that gets the job done.
// We assume that all weigths are 1 when the initial call is made to
// this function.
//
// Returns 0 on success, nonzero otherwise.
////////////////////////////////////////////////////////////////////////

int list_rank(AMI_STREAM<edge> *istream, AMI_STREAM<edge> *ostream,
              unsigned int rec_level = 0)
{
    AMI_err ae;
    
    off_t stream_len = istream->stream_len();

    AMI_STREAM<edge> *edges_rand;
    AMI_STREAM<edge> *active;
    AMI_STREAM<edge> *active_2;
    AMI_STREAM<edge> *cancel;
    AMI_STREAM<edge> *ranked_active;

    AMI_STREAM<edge> *edges_from_s;
    AMI_STREAM<edge> *cancel_s;
    AMI_STREAM<edge> *active_s;
    AMI_STREAM<edge> *ranked_active_s;

    // Scan/merge management objects.
    random_flag_scan my_random_flag_scan;
    separate_active_from_cancel my_separate_active_from_cancel;
    strip_cancel_from_active my_strip_cancel_from_active;
    patch_active_cancel my_patch_active_cancel;
    
    // Check if the recursion has bottomed out.  If so, then read in the
    // array and rank it.

    {
        size_t mm_avail;
        
        MM_manager.available(&mm_avail);

        if (stream_len * sizeof(edge) < mm_avail / 2) {
            edge *mm_buf = new edge[stream_len];
            istream->seek(0);
            istream->read_array(mm_buf,&stream_len);
            main_mem_list_rank(mm_buf,stream_len);
            ostream->write_array(mm_buf,stream_len);
            delete [] mm_buf;
            // Get rid of the input stream.
            if (rec_level) {
                delete istream;
            }
            return 0;
        }
    }
    
    // Flip coins for each node, setting the flag to 0 or 1 with equal
    // probability.

    edges_rand = new AMI_STREAM<edge>;
    
    AMI_scan(istream, &my_random_flag_scan, edges_rand);

    if (verbose) {
        cout << "Random flag list is of length " <<
            edges_rand->stream_len() << ".\n";
    }

    if (rec_level) {
        delete istream;
    }
    
    // Sort one stream by source.  The original input was sorted by
    // destination, so we don't need to sort it again.

    edges_from_s = new AMI_STREAM<edge>;

    ae = AMI_sort(edges_rand, edges_from_s, edgefromcmp);

    if (verbose) {
        cout << "Sorted from list is of length " <<
            edges_from_s->stream_len() << ".\n";
    }
    
    // Scan to produce and active list and a cancel list.

    active = new AMI_STREAM<edge>;
    cancel = new AMI_STREAM<edge>;

    ae = AMI_scan(edges_from_s,
                  edges_rand,
                  &my_separate_active_from_cancel,
                  active,
                  cancel);

    delete edges_from_s;
    delete edges_rand;
    
    // Strip the edges that went to the cancel list out of the active list.

    if (verbose) {
        cout << "Cancel list is of length " <<
            cancel->stream_len() << ".\n";
        cout << "Active list is of length " <<
            active->stream_len() << ".\n";
    }

    active_s = new AMI_STREAM<edge>;

    ae = AMI_sort(active, active_s, edgetocmp);

    delete active;

    if (verbose) {
        cout << "Sorted active list is of length " <<
            active_s->stream_len() << ".\n";
    }

    active_2 = new AMI_STREAM<edge>;

    ae = AMI_scan(active_s,
                  &my_strip_cancel_from_active,
                  active_2);

    delete active_s;

    if (verbose) {
        cout << "After stripping, active list is of length " <<
            active_2->stream_len() << ".\n";
    }
    
    // Recurse on the active list.  The list we pass in is sorted by
    // destination.  The recursion will return a list sorted by
    // source.

    ranked_active = new AMI_STREAM<edge>;
    
    list_rank(active_2, ranked_active, rec_level + 1);

    // This is now done inside the recursion to avoid telescoping space
    // usage by intermediate streams that are no longer needed.
    
    // delete active_2;

    if (verbose) {
        cout << "After recursion, ranked active list is of length " <<
            ranked_active->stream_len() << ".\n";
    }

    cancel_s = new AMI_STREAM<edge>;

    AMI_sort(cancel, cancel_s, edgetocmp);

    delete cancel;

    if (verbose) {
        cout << "Sorted cancel list is of length " <<
            cancel_s->stream_len() << ".\n";
    }

    // The output of the recursive call is not necessarily sorted by
    // destination.  We'll make it so before we try to merge in the
    // cancel list.

    ranked_active_s = new AMI_STREAM<edge>;

    AMI_sort(ranked_active, ranked_active_s, edgetocmp);

    delete ranked_active;
    
    // Now merge the recursively ranked active list and the sorted cancel list.

    ae = AMI_scan(ranked_active_s,
                  cancel_s,
                  &my_patch_active_cancel,
                  ostream);

    if (verbose) {
        cout << "After patching in canceled edges, list is of length " <<
            ostream->stream_len() << ".\n";
    }
    
    delete ranked_active_s;
    delete cancel_s;
    
    return 0;
}


static char def_irf[] = "/var/tmp/osi.txt";
static char def_rrf[] = "/var/tmp/osr.txt";
static char def_frf[] = "/var/tmp/osf.txt";

static char *initial_results_filename = def_irf;
static char *rand_results_filename = def_rrf;
static char *final_results_filename = def_frf;

static bool report_results_initial = false;
static bool report_results_random = false;
static bool report_results_final = false;

static const char as_opts[] = "I:iR:rF:f";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'I':
            initial_results_filename = optarg;
        case 'i':
            report_results_initial = true;
            break;
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'F':
            final_results_filename = optarg;
        case 'f':
            report_results_final = true;
            break;
    }
}

extern int register_new;

int main(int argc, char **argv)
{
    AMI_err ae;

    wall_timer wt0;
    cpu_timer ct0;
    
    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
        cout << "sizeof(edge) = " << sizeof(edge) << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }

    srandom(random_seed);
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;
        
    AMI_STREAM<edge> *pamis1;
    AMI_STREAM<edge> *pamis2;
    AMI_STREAM<edge> amis3;
    AMI_STREAM<edge> amis4;

    // Streams for reporting values to ascii streams.
    
    ofstream *osi;
    cxx_ostream_scan<edge> *rpti = NULL;

    ofstream *osr;
    cxx_ostream_scan<edge> *rptr = NULL;

    ofstream *osf;
    cxx_ostream_scan<edge> *rptf = NULL;

    if (report_results_initial) {
        osi = new ofstream(initial_results_filename);
        rpti = new cxx_ostream_scan<edge>(osi);
    }

    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<edge>(osr);
    }

    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<edge>(osf);
    }

    // Write the initial set of edges.

    {
        AMI_STREAM<edge> amis0;

        scan_list sl(test_size);

        ae = AMI_scan(&sl, &amis0);

        if (verbose) {
            cout << "Wrote the initial sequence of edges.\n";
            cout << "Stopped (didn't write) with last_to = "
                 << sl.last_to << ". operate() called " << sl.called
                 << " times.\n";
            cout << "Stream length = " << amis0.stream_len() << '\n';
        }

        if (report_results_initial) {
            ae = AMI_scan(&amis0, rpti);
        }
        
        // Randomly order them.

        merge_random<edge> mr;

        pamis1 = new AMI_STREAM<edge>;
        
        ae = AMI_partition_and_merge(&amis0, pamis1,
                                 (AMI_merge_base<edge> *)&mr);
        
        if (verbose) {
            cout << "Randomly ordered the initial sequence of edges.\n";
            cout << "Stream length = " << pamis1->stream_len() << '\n';
        }

        if (report_results_random) {
            ae = AMI_scan(pamis1, rptr);
        }
    }
    
    // Rank them.  Note that we should sort by destination before
    // calling the recursive list ranking function.

    LOG_INFO("About to start timers.\n");
    
#if BTE_STATS
    BTE_stream_mmb_base::reset_stats();
    cout << BTE_stream_mmb_base::statistics() << '\n';
    BTE_stream_mmb_base::stats_on();
#endif
        
    wt0.reset();
    ct0.reset();

    wt0.start();    
    ct0.start();

    pamis2 = new AMI_STREAM<edge>;
    
    ae = AMI_sort(pamis1, pamis2, edgetocmp);

    delete pamis1;
    
    list_rank(pamis2,
              &amis3, 1);

    delete pamis2;

    wt0.stop();
    ct0.stop();

    cout << "Wall time: " <<  wt0 << '\n';
    cout << "CPU time: " <<  ct0 << '\n';

#if BTE_STATS
    BTE_stream_mmb_base::stats_off();
    cout << BTE_stream_mmb_base::statistics() << '\n';
#endif

    LOG_INFO("Stopped timers.\n");

    if (report_results_final) {
        // Sort by rank before output, to make it easier for humans to
        // read.
        ae = AMI_sort(&amis3, &amis4, edgeweightcmp);
    
        ae = AMI_scan(&amis4, rptf);
    }
        
    return 0;
}
