// Copyright (c) 1994 Darren Erik Vengroff
//
// File: lr.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: Tue Mar 15 1994
//
// A sample piece of code that does list ranking in the TPIE system.
//

static char lr_id[] = "$Id: lr.cpp,v 1.1 1994-06-10 16:57:39 dev Exp $";

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Use the MMB version of BTE streams.
#define BTE_IMP_MMB

// Define it all.
#include <ami.h>

////////////////////////////////////////////////////////////////////////
//
// The edge class.  This is what our list ranking function will work on.
//
////////////////////////////////////////////////////////////////////////

class edge {
public:
  long int from;        // Node it is from
  long int to;          // Node it is to
  long int weight;      // Position when ranked.
  int flag;             // A flag used to randomly select some edges.
};

////////////////////////////////////////////////////////////////////////
// 
// random_flag 
//
// This is a class of scan management objects that operates by flipping
// a fair coin for each edge and setting the edge's flag accordingly.
//
// There is a single instance of this class, called my_random_flag.
//
////////////////////////////////////////////////////////////////////////

extern int flip_coin();

class random_flag_scan : AMI_scan_object {
public:
  AMI_err initialize(void);  
  AMI_err operate(const edge &in, AMI_SCAN_FLAG *sfin,
                  edge *out, AMI_SCAN_FLAG *sfout);
};

AMI_err random_flag_scan::operate(const edge &in, AMI_SCAN_FLAG *sfin,
                                  edge *out, AMI_SCAN_FLAG *sfout)
{ 
    if (!*sfin) return AMI_SCAN_DONE;
    *out = in;
    out->flag = flip_coin();
}

// Generate the proper function from the AMI_scan() function template.
AMI_err AMI_scan(AMI_base_stream<edge> *, random_flag_scan *,
                 AMI_base_stream<edge> *);

static random_flag_scan my_random_flag;

////////////////////////////////////////////////////////////////////////
//
// active_cancel
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
//
////////////////////////////////////////////////////////////////////////
class active_cancel : AMI_scan_object {
public:
  AMI_err initialize(void);  
  AMI_err operate(const edge &e1, const edge &e2, AMI_SCAN_FLAG *sfin,
                  edge *active, edge *cancel, AMI_SCAN_FLAG *sfout);
};

AMI_err active_cancel::operate(const edge &e1, const edge &e2, 
                               AMI_SCAN_FLAG *sfin,
                               edge *active, edge *cancel, 
                               AMI_SCAN_FLAG *sfout)
{ 
  if (!sfin[0]) return AMI_SCAN_DONE;

  sfout[0] = 1;
  if (e1.flag && !e2.flag) {
    active->from = e1.from;
    active->to = e2.to;
    active->weight += e2.weight;
    *cancel = e2;
    sfout[1] = 1;
  } else {
    *active = e1;
  }
  return AMI_SCAN_CONTINUE;
}

// Generate the proper function from the AMI_scan() function template.
AMI_err AMI_scan(AMI_base_stream<edge> *, AMI_base_stream<edge> *,
                 active_cancel *,
                 AMI_base_stream<edge> *, AMI_base_stream<edge> *);

static active_cancel my_active_cancel;

////////////////////////////////////////////////////////////////////////
//
// merge_active_cancel
//
// This is a class of merge object that merges two lists of edges
// based on their to fields.  The first list of edges should be active
// edges, while the second should be cancelled edges.  When we see two
// edges with the same to field, we know that the second was cancelled
// when the first was made active.  We then fix up the weights and
// output the two of them, one in the current call and one in the next
// call.
//
////////////////////////////////////////////////////////////////////////

class merge_active_cancel : AMI_merge_base<edge> {
private:
  int state;            // A finite amount of state.
  edge held_edge;       // An edge waiting to go in the output stream.
public:
  AMI_err initialize(arity_t arity, edge **in, AMI_merge_flag *taken_flags);
  AMI_err operate(const edge **in_edges, AMI_merge_flag *taken_flags,
                  edge *out);
};

AMI_err merge_active_cancel::initialize(arity_t arity, edge **in,
                                        AMI_merge_flag *taken_flags)
{
  state = 0;
  return AMI_ERROR_NO_ERROR;
}

AMI_err merge_active_cancel::operate(const edge **in_edges,
                                     AMI_merge_flag *taken_flags,
                                     edge *out_edge)
{
    long int lost_weight;

    // If we're holding an edge from last time, output it.
    if (state) {
        state = 0;
        *out_edge = held_edge;
        taken_flags[0] = taken_flags[1] = 0;
        return AMI_MERGE_OUTPUT;
    }

    if (!in_edges[0])
        return AMI_MERGE_DONE;

    // If the active edge goes to a smaller node than the cancelled one,
    // skip it.
    if (in_edges[0]->to < in_edges[1]->to) {
        *out_edge = *(in_edges[0]);
        taken_flags[0] = 1; taken_flags[1] = 0;
    } else {

        // The edges should go to the same node.
        tp_assert(in_edges[0]->to == in_edges[1]->to,
                  "Edges don't go the same place.");

        taken_flags[0] = taken_flags[1] = 1;
        held_edge = *(in_edges[0]);
        *out_edge = *(in_edges[1]);
        out_edge->to = held_edge.from;
        lost_weight = held_edge.weight;
        held_edge.weight = out_edge->weight;
        out_edge->weight -= lost_weight;
        state = 1;
    }
   
    return AMI_MERGE_OUTPUT;
}

merge_active_cancel my_merge_active_cancel;

////////////////////////////////////////////////////////////////////////
//
// edgefromcmp()
// edgetocmp()
//
// Helper functions used to compare to edges to sort them either by 
// the node they are from or the node they are to.
//
////////////////////////////////////////////////////////////////////////

int edgefromcmp(edge *s, edge *t)
{
  return (s->from < t->from) ? -1 : ((s->from < t->from) ? 1 : 0);
}
  
static int edgetocmp(edge *s, edge *t)
{
  return (s->to < t->to) ? -1 : ((s->to < t->to) ? 1 : 0);
}

////////////////////////////////////////////////////////////////////////
//
// Until the ANSI standards committee resolves the issue of type
// conversion in function template matching, these declarations are
// required to force the compiler to define the appropriate template
// functions.
//
// With GNU g++ 2.5.8, things are even worse.  We have to create a
// dummy function that actually makes each call in order for the
// corresponding template function to be generated in the object file.
// This dummy function should never be called.
//
////////////////////////////////////////////////////////////////////////

#if __GNUG__ // GNU g++
static void _____dummy_____DO_NOT_EVER_CALL_ME_(void)
{
    pp_AMI_bs<edge> ppb;
    arity_t arity;
    AMI_base_stream<edge> *amibs;
    merge_active_cancel *mac;
    int (*cmp)(edge *, edge *);
    
    tp_assert(0, "Warning, dummy function was called somehow."
              "seg fault is imminent.");
    
    AMI_merge(ppb, arity, amibs, mac);
    // Sorry, as of 2.5.8, g++ still does not support function types
    // in template unification.
    /* AMI_sort(amibs, cmp); */
    AMI_copy_stream(amibs, amibs);
}
#endif // __GNUG__

AMI_err AMI_merge(pp_AMI_bs<edge>, arity_t, AMI_base_stream<edge> *,
                  merge_active_cancel *);
AMI_err AMI_sort(AMI_base_stream<edge> *s, int (*cmp)(edge *, edge *));
AMI_err AMI_copy_stream(AMI_base_stream<edge> *, AMI_base_stream<edge> *);

////////////////////////////////////////////////////////////////////////
//
// main_mem_list_rank()
//
// This function ranks a list that can fit in main memory.  It is used
// when the recursion bottoms out.
//
// Details of this function are omitted for brevity
//
////////////////////////////////////////////////////////////////////////

AMI_STREAM<edge> *main_mem_list_rank(AMI_STREAM<edge> *edges)
{
  // Read the stream into memory;
  // Use your favorite main memory list ranking algorithm 
  //    (is there actually more than one?);
  // Write the results out to a stream;
  // Return the stream;
  return edges;  
}

////////////////////////////////////////////////////////////////////////
//
// list_rank()
//
// This is the actual recursive function that gets the job done.
//
// We assume that all weigths are 1 when the initial call is made to
// this function.
//
////////////////////////////////////////////////////////////////////////

AMI_STREAM<edge> *list_rank(AMI_STREAM<edge> *edges)
{
    AMI_STREAM<edge> *edges1; 		 // A copy of the input stream of edges
    AMI_STREAM<edge> *a_edges[2];        // An array to pass the two copies to
                                         // AMI_scan.
    AMI_STREAM<edge> *a_active_cancel[2];// Output of the scan.  
                                         // One list is the active 
                                         // list and one is the cancel 
                                         // list.

    // Check if the recursion has bottomed out.

    if (edges->stream_len() * sizeof(edge) < AMI_mem_size())
        return(main_mem_list_rank(edges));

    // Flip coins for each node, setting the flag to 0 or 1 with equal
    // probability.

    AMI_scan(edges, &my_random_flag, edges);

    // Make a copy of the stream.  

    AMI_copy_stream(edges, edges1);
    
    // Sort one stream by where the edge is from, the other by where it is
    // to.

    AMI_sort(edges, edgetocmp);
    AMI_sort(edges1, edgefromcmp);

    // Scan to produce and active list and a cancel list.

    a_edges[0] = edges; a_edges[1] = edges1;
    AMI_scan(edges, edges1, &my_active_cancel,
             a_active_cancel[0], a_active_cancel[1]);

    // Recurse on the active list.  It will return sorted by terminal (to)
    // vertex.

    a_active_cancel[0] = list_rank(a_active_cancel[0]);

    // Sort the cancel list by where the edges go to.
  
    AMI_sort(a_active_cancel[0], edgetocmp);

    // Now merge the two lists.

    AMI_merge(a_active_cancel, 2, edges1, &my_merge_active_cancel);

    return edges1;
}
