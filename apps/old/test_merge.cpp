#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>
#include <ami_merge.h>


class Compare_test
{
  public:

  AMI_err initialize(arity_t a, const int* const* in,
		     AMI_merge_flag *tf,
		     int& ti) {
    return AMI_ERROR_NO_ERROR;
  }
  AMI_err operate(const int* const *in, AMI_merge_flag* tf,
		  int& ti, int* out) {
    return AMI_ERROR_NO_ERROR;
  } 
  int compare(const int& v1, const int& v2)
  {
    if(v1 > v2)
      return -1;
    else if(v1 < v2)
      return 1;
    else
      return 0;
  }
};

/*

  int compare(const int& v1, const int& v2)
  {
    if(v1 > v2)
      return -1;
    else if(v1 < v2)
      return 1;
    else
      return 0;
  }
*/

int main()
{
  AMI_STREAM<int> **instream, outstream;
  instream = (AMI_STREAM<int>**)malloc(5*sizeof(AMI_STREAM<int>*));
  Compare_test *comp = new Compare_test();
  AMI_err ae;

  for(int j=0; j<5; j++)
  {
    for(int i=0; i<100; i++)
     instream[j]->write_item(100*j+i);
  }

  ae = AMI_merge(instream, (arity_t)5, &outstream, comp);
}
