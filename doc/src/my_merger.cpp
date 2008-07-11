class my_merger : AMI_merge_base {
public:
    AMI_err initialize(arity_t arity, const T * const *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(const T * const *in, AMI_merge_flag *taken_flags,
                    int &taken_index, T *out);
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
    size_t space_usage_per_stream(void);
};

AMI_STREAM<T> instream, outstream;

void f() 
{
    my_merger mm;    
    AMI_partition_and_merge(&instream, &outstream, &mm);
}

AMI_err AMI_partition_and_merge(instream, outstream, mm)
{
    max_ss = max # of items that can fit in main memory;
    Partition instream into num_substreams substreams of size max_ss;

    Foreach substream[i] {
        Read substream[i] into main memory;
        mm->main_mem_operate(substream[i]);
        Write substream[i];
    }

    Call mm->space_usage_overhead() and mm->space_usage_per_stream;
    
    Compute merge_arity; // Maximum # of streams we can merge.     

    while (num_substreams > 1) {
        for (i = 0; i < num_substreams; i += merge_arity) {
            Merge substream[i] .. substream[i+merge_arity-1];
        }
        num_substreams /= merge_arity;
        max_ss *= merge_arity;
    }

    Write single remaining substream to outstream;
        
    return AMI_ERROR_NO_ERROR;
}

