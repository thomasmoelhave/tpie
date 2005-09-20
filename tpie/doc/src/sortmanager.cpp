// Here is the definition of the sort management class
class SortManager {
private:
    int result;
public:
   inline int compare (const double & k1, const double & k2) {
      return ((k1 < k2)? -1 : (k1 > k2) ? 1 : 0);
   }
   inline void copy (double *key, const rectangle &record) {
      *key = record.southwest_y;
   }
};

// create a sort management object
SortManager <rectangle,double> smo;

AMI_STREAM<rectangle> instream;
AMI_STREAM<rectangle> outstream;
double dummyKey;

void f()
{
    AMI_key_sort(&instream, &outstream, dummyKey, &smo );
}
