class MergeMgr : public AMI_merge_base<int> { 
private:
    arity_t input_arity;
    pqueue  *pq;
public:
    MergeMgr (void);
    virtual ~MergeMgr  (void);
    AMI_err initialize (arity_t arity, const int * const *in,
                       AMI_merge_flag *taken_flags, int &taken_index);
    AMI_err operate    (const int * const *in, 
                       AMI_merge_flag *taken_flags,
                       int &taken_index, int *out);
    AMI_err main_mem_operate      (int* mm_stream, size_t len);
    size_t  space_usage_overhead  (void);
    size_t  space_usage_per_stream(void);
};

MergeMgr::MergeMgr(void)
{
    pq = NULL;
}

MergeMgr::~MergeMgr(void)
{
    if (pq != NULL) {
        delete pq;
    }
}

size_t MergeMgr::space_usage_overhead(void)
{
    return sizeof(pqueue<arity_t,int>);
}


size_t MergeMgr::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(int);
}

AMI_err MergeMgr::main_mem_operate(int* mm_stream, size_t len)
{
    qsort(mm_stream, len, sizeof(int), c_int_cmp);
    return AMI_ERROR_NO_ERROR;
}

AMI_err MergeMgr::initialize(arity_t arity, const int * const *in,
                                    AMI_merge_flag *taken_flags,
                                    int &taken_index)
{
    input_arity = arity;

    if (pq != NULL) {
        delete pq;
    }

    // Construct a priority queue that can hold arity items.
    pq = new pqueue_heap_op(arity);

    for (arity_t ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            pq->insert(ii,*in[ii]);
        } else {
            taken_flags[ii] = 0;
        }
    }

    taken_index = -1;
    return AMI_MERGE_READ_MULTIPLE;
}

AMI_err MergeMgr::operate(const int * const *in,
                                 AMI_merge_flag *taken_flags,
                                 int &taken_index,
                                 int *out)
{
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!pq->num_elts()) {
        return AMI_MERGE_DONE;
    } else {
        arity_t min_source;
        int min_t;

        pq->extract_min(min_source,min_t);
        *out = min_t;
        if (in[min_source] != NULL) {
            pq->insert(min_source,*in[min_source]);
            taken_index = min_source;
        } else {
            taken_index = -1;
        }
        return AMI_MERGE_OUTPUT;
    }
}
