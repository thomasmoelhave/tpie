class compare_re_class {
public:
    int compare ( const complex &c1, const complex &c2 ) {
        return (c1.re() < c2.re()) ? -1 :
               ((c1.re() > c2.re()) ? 1 : 0);
    };
};

class compare_im_class {
public:
    int compare ( const complex &c1, const complex &c2 ) {
        return (c1.im() < c2.im()) ? -1 :
               ((c1.im() > c2.im()) ? 1 : 0);
    };
};

AMI_STREAM<complex> instream;
AMI_STREAM<complex> outstream_re;
AMI_STREAM<complex> outstream_im;

compare_re_class compare_re
compare_im_class compare_im

void f()
{
    AMI_sort(&instream, &outstream_re, &compare_re);
    AMI_sort(&instream, &outstream_im, &compare_im);
}
