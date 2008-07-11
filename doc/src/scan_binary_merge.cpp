class scan_binary_merge : AMI_scan_object {
public:
    AMI_err initialize(void) {};
    
    AMI_err operate(const int &in0, const int &in1, AMI_SCAN_FLAG *sfin,
                    int *out, AMI_SCAN_FLAG *sfout) 
    {
        if (sfin[0] && sfin[1]) {
            if (in0 < in1) {
                sfin[1] = false;
                *out = in0;
            } else {
                sfin[0] = false;
                *out = in1;
            }
        } else if (!sfin[0]) {
            if (!sfin[1]) {
                *sfout = false;
                return AMI_SCAN_DONE;
            } else {
                *out = in1;
            }
        } else {
            *out = in0;
        }
        *sfout = 1;
        return AMI_SCAN_CONTINUE;
    }
};
