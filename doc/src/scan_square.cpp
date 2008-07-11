class scan_square : AMI_scan_object {
public:
    AMI_err initialize(void)
    {
        return AMI_ERROR_NO_ERROR;
    };

    AMI_err operate(const int &in, AMI_SCAN_FLAG *sfin,
                    int *out, AMI_SCAN_FLAG *sfout)
    {
        if (*sfout = *sfin) {
            *out = in * in;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};

scan_square ss;
AMI_STREAM<int> amis1;    

void g() 
{
    AMI_scan(&amis0, &ss, &amis1);
}

AMI_err AMI_scan(AMI_STREAM<int> *instream, scan_square &ss, 
        AMI_STREAM<int> *outstream)
{
    int in, out;
    AMI_err ae;    
    AMI_SCAN_FLAG sfin, sfout;

    sc.initialize();

    while (1) {
        {
             Read in from *instream;
             sfin = (read succeeded);
        }
        if ((ae = ss.operate(in, &sfin, &out, &sf)) == 
            AMI_SCAN_CONTINUE) {
            if (sfout) {
                Write out to *outstream;
            }
            if (ae == AMI_SCAN_DONE) {
                return AMI_ERROR_NO_ERROR;
            }
            if (ae != AMI_SCAN_CONTINUE) {
                Handle error conditions;
            }
        }
    }
}

void f()
{
    ifstream in_ascii("input_nums.txt");
    ofstream out_ascii("output_nums.txt");
    cxx_istream_scan<int> in_scan(in_ascii);
    cxx_ostream_scan<int> out_scan(out_ascii);
    AMI_STREAM<int> in_ami, out_ami;
    scan_square ss;    

    // Read them.
    AMI_scan(&in_scan, &in_ami);

    // Square them.
    AMI_scan(&in_ami, &ss, &out_scan);
    
    // Write them.
    AMI_scan(&out_ami, out_scan);

}    
