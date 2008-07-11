class scan_count : AMI_scan_object {
private:
    int maximum;
    int nextint;
public:
    scan_count(int max) : maximum(max), ii(0) {};

    AMI_err initialize(void) 
    {
        nextint = 0;
        return AMI_ERROR_NO_ERROR;
    };

    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf)
    {
        *out1 = ++nextint;
        return (*sf = (nextint <= maximum)) ? AMI_SCAN_CONTINUE : 
            AMI_SCAN_DONE;
    };
};

scan_count sc(10000);
AMI_STREAM<int> amis0;    

void f()
{
    AMI_scan(&sc, &amis0);
}

AMI_err AMI_scan(scan_count &sc, AMI_STREAM<int> *pamis)
{
    int nextint;
    AMI_err ae;    
    AMI_SCAN_FLAG sf;

    sc.initialize();    
    while ((ae = sc.operate(&nextint, &sf)) == AMI_SCAN_CONTINUE) {
        if (sf) {
            Write nextint to *pamis;
        }
    }

    if (ae != AMI_SCAN_DONE) {
        Handle error conditions;
    }

    return AMI_ERROR_NO_ERROR;
}
