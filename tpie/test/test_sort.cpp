#include "app_config.h"
#include <portability.h>
#include <ami_sort.h>
#include <tpie_log.h>
#include "getopts.h"
#include "quicksort.h"  //TPIE internal sort
#include <algorithm>    //STL internal sort
#include <stdlib.h>     //C internal sort
#include "cpu_timer.h"

#include <progress_indicator_arrow.h>

//snprintf is different on WIN/Unix platforms
#ifdef _WIN32
#define APP_SNPRINTF _snprintf
#else
#define APP_SNPRINTF snprintf
#endif

enum app_options { 
  APP_OPTION_PATH = 1,
  APP_OPTION_NUM_ITEMS,
  APP_OPTION_FILE_SIZE,
  APP_OPTION_MEM_SIZE,
  APP_OPTION_EMPTY,
};

enum test_type {
  APP_TEST_OBJ_OP = 1,
  APP_TEST_PTR_OP,
  APP_TEST_OBJ_CMPOBJ,
  APP_TEST_PTR_CMPOBJ,
  APP_TEST_KOBJ
};

const unsigned int APP_OPTION_NUM_OPTIONS=5; 
const TPIE_OS_LONGLONG APP_MEG = 1024*1024;
const TPIE_OS_LONGLONG APP_GIG = 1024*1024*1024;
const TPIE_OS_LONGLONG APP_DEFAULT_N_ITEMS = 10000;
const TPIE_OS_LONGLONG APP_DEFAULT_MEM_SIZE = 128*1024*1024;
const int  APP_ITEM_SIZE = 1;

const char* APP_FILE_BASE =  "TPIE_Test";

// Basic parameters for the test
// better than having several global variables
typedef struct app_info{
    char * path;
    int item_size;
    TPIE_OS_LONGLONG num_items;
    TPIE_OS_LONGLONG mem_size;
} appInfo;

// ********************************************************** 
// class SortItem
// **********************************************************

// A class that contains a simple fixed-size array of chars 
// The array can also be used as a counter in base-26 arithmetic
class SortItem{
  public:
    SortItem(long key_);
    ~SortItem(){};
    long key;        //key to sort on
    bool operator<(const SortItem& rhs) const {
      return (this->key<rhs.key);
    }
  private:
    char item[APP_ITEM_SIZE];  //extra space to make bigger items
};

// Constructor
// Initialize item 'aaaaaaa...'
SortItem::SortItem(long key_=0): key(key_) {
  int i;
  for(i=0; i<APP_ITEM_SIZE-1; i++) item[i]='a';
  //null-terminate so we can print array as string
  item[APP_ITEM_SIZE-1]='\0';
}

// ********************************************************** 
// End Class SortItem
// ********************************************************** 

// Comparison class for STL/TPIE sorting
class SortCompare{
   public:
     SortCompare(){};
     //For STL sort
     inline bool operator()(const SortItem &left, const SortItem &right) const{
       return left.key<right.key;
     }
     //For TPIE sort
     inline int compare(const SortItem &left, const SortItem &right){
       if(left.key<right.key){return -1;}
       else if (left.key>right.key){return 1;}
       else return 0;
     }
};

// class for extracting, comparing keys for key sorting
class KeySortCompare{
  public:
    KeySortCompare(){};
    
    //Extract key from sortItem and store in given key reference
    inline void copy(long* key, const SortItem& item){
      *key=item.key;
    }
    //compare sortItem keys (longs)
    inline int compare(const long& left, const long& right){
      if(left<right){ return -1; }
      else if(left>right) {return 1; }
      else return 0;
    }
};

int qcomp(const void* left, const void* right){
  return (int)(((SortItem*)left)->key-((SortItem*)right)->key);
}

// Just like atoi or atol, but should also work for 64bit numbers
// Also supports KMG suffixes (e.g. 2K = 2*1024)
TPIE_OS_LONGLONG ascii2longlong(char *s){

  int i, len, digit, multfactor;
  TPIE_OS_LONGLONG val;
  bool ok, neg;
  
  len = strlen(s);
  val = 0;
  i=0;
  ok=true;
  neg=false;
  multfactor=1;

  if(s[0]=='-'){ 
    neg = true;
    i++;
  }
  
  switch(s[len-1]){
    case 'K':
    case 'k':
      multfactor = 1024;
      break;
    case 'M':
    case 'm':
      multfactor = APP_MEG;
      break;
    case 'G':
    case 'g':
      multfactor = APP_GIG;
      break;
    default:
      break;
  }

  if(i<len){
    do {
      digit = s[i]-'0';
      if((digit< 0) || (digit > 9)){
        ok = false;
      }
      else{
        val = 10*val+digit;
      }
      i++;
    } while((i<len) && ok);
  }
 
  val *= multfactor;
  if(neg){ val = 0-val; }
  return val;
}

// init_opts sets up the options structures for use in getopts
void init_opts(struct options* & opts, int argc, char** argv){
  
  opts = new struct options[APP_OPTION_NUM_OPTIONS];

  const int bufsize = 160;
    
  int i=0;
  opts[i].number=APP_OPTION_PATH;
  opts[i].name = "path-name";
  opts[i].shortName = "p";
  //this option takes the requested path name as an argument
  opts[i].args = 1;
  opts[i].description = "Path for temp files";
  
  i++; 
  opts[i].number=APP_OPTION_NUM_ITEMS;
  opts[i].name = "numitems";
  opts[i].shortName = "n";
  //this option takes the requested item-size as an argument
  opts[i].args = 1;
  opts[i].description = "Number of items to write";

  i++;
  opts[i].number=APP_OPTION_FILE_SIZE;
  opts[i].name = "size";
  opts[i].shortName = "s";
  opts[i].args = 1;
  opts[i].description = "Create a file of size <args>";

  i++;
  opts[i].number=APP_OPTION_MEM_SIZE;
  opts[i].name = "memsize";
  opts[i].shortName = "m";
  opts[i].args = 1;
  opts[i].description = "Use no more memory than size <args>";

  // get opts looks for a structure with an empty description
  // not having one could lead to segfaults
  // is there a way for getopts to count options?
  i++;
  opts[i].number = APP_OPTION_EMPTY;
  opts[i].name = "";
  opts[i].shortName = "";
  opts[i].args=0;
  opts[i].description = 0;
}

// Print a warning when a user specifies multiple conflicting options. 
inline void print_warning(){
  static bool done=false;
  if(!done){
    cerr << "Warning: Only one of --numitems, or --size can be specified.\n"
         << "Ignoring extra options\n" << endl;
    done=true;
  }
}

// Set up application parameters based on command line args
// Displays help if no parameters are specified
void get_app_info(int argc, char** argv, appInfo & Info){

  struct options* opts;
  char* optarg;
  int optidx, nopts;
  bool optset=false;
  TPIE_OS_LONGLONG tmp;

  init_opts(opts, argc, argv); 

  // get the dir
  char* base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
  if (base_dir == NULL) { base_dir = getenv(TMPDIR_ENV); }
  if (base_dir == NULL) { base_dir = TMP_DIR; }
  
  Info.path=base_dir;
  Info.item_size=sizeof(SortItem);
  Info.num_items=APP_DEFAULT_N_ITEMS;
  Info.mem_size=APP_DEFAULT_MEM_SIZE;

  nopts=0;
  while (optidx=getopts(argc, argv, opts, &optarg)){
    nopts++;
    if(optidx==-1){
      cerr << "Could not allocate space for arguments. Exiting...\n";
      exit(1);
    }
    if(optidx==APP_OPTION_PATH){
      Info.path=optarg;
    }
    else if(optidx==APP_OPTION_MEM_SIZE){
      tmp = ascii2longlong(optarg);
      if(tmp < 0){
        cerr << "Invalid memory size. Exiting...\n";
        exit(1);
      }
      else { Info.mem_size=tmp; }
    }
    else if(!optset){
      //set optset flag if applicable
      switch(optidx){
        case APP_OPTION_NUM_ITEMS:
        case APP_OPTION_FILE_SIZE:
          optset=true;
        default:
          break;
      }
      //do actual setup
      switch(optidx){
        case APP_OPTION_NUM_ITEMS:
          tmp = ascii2longlong(optarg);
          if(tmp < 0){
            cerr << "Invalid item count. Exiting...\n";
            exit(1);
          }
          else { Info.num_items=tmp; }
          break;
        case APP_OPTION_FILE_SIZE:
          tmp = ascii2longlong(optarg);
          if(tmp < 0){
            cerr << "Invalid file size. Exiting...\n";
            exit(1);
          }
          else { Info.num_items=(tmp/Info.item_size)+1; }
          break;
        default:
          cerr << "Warning: Unhandled option - " << optidx << endl;
          break;
      }
    } //else if(!optset)
    else {
      //optset=true, optidx != APP_OPTION_PATH
      //check for bad options
      switch(optidx){
        case APP_OPTION_NUM_ITEMS:
        case APP_OPTION_FILE_SIZE:
          print_warning();
          break;
        default:
          cerr << "Warning: Unhandled option - " << optidx << endl;
          break;
      } //switch
    } //else
  } //while options

  // if no command line options, display usage
  if(nopts == 0) { 
    // add more general comments here if neeeded
    // then display usage options
    printf("\nSummary: Writes a random stream of specified size\n"
           "to a temporary file, sorts the stream\n"
           "then deletes the input/output streams and exits.\n"
           "Useful for testing basic AMI_sort routine\n\n");
    getopts_usage(argv[0], opts);
    printf("\nEach item is %d bytes\n", APP_ITEM_SIZE);
    printf("--path-name is \"%s\" by default\n", TMP_DIR);
    printf("Suffixes K, M, and G can be appended to the\n"
           "--numitems and --size options to mean\n"
           "*1024, *1024*1024, and *2^30 respectively\n"
           "e.g., --size 128M creates a 128 MB stream\n\n");
    printf("Example runs\n\n");
    printf("Test a stream of size 2 GB\n");
    printf("%s -s 2G\n",argv[0]);
   
    exit(1);
  }  
  
  //check if path is valid
  char * tmpfname = tpie_tempnam(APP_FILE_BASE,Info.path);
  TPIE_OS_FILE_DESCRIPTOR fd;
  fd=TPIE_OS_OPEN_OEXCL(tmpfname);
  if(TPIE_OS_IS_VALID_FILE_DESCRIPTOR(fd)){
    TPIE_OS_CLOSE(fd);
    TPIE_OS_UNLINK(tmpfname);
  }
  else{
    cerr << "Unable to write to path " << Info.path 
         << ".  Exiting..." << endl;
    exit(1);
  }
  
  delete [] opts; 
  return; 
}

//converts numbers into strings
//with appropriate G, M, K suffixes
char* ll2size(TPIE_OS_LONGLONG n, char* buf){
  
  const int bufsize = 20;
  float size;
  if(n > APP_GIG ){
    size = (n*1.)/APP_GIG;
    APP_SNPRINTF(buf, bufsize, "%.2f G",size);
  }
  else if (n > APP_MEG ){
    size = (n*1.)/APP_MEG;
    APP_SNPRINTF(buf, bufsize, "%.2f M",size);
  } 
  else if (n > 1024 ){
    size = (n*1.)/1024.;
    APP_SNPRINTF(buf, bufsize, "%.2f K",size);
  }
  else {
    size = (n*1.);
    APP_SNPRINTF(buf,bufsize, "%.0f", size);
  }

  return buf;
}

// Open a stream, write num_items, close stream
void write_random_stream(char* fname, appInfo & info, progress_indicator_base* indicator=NULL){
  
  TPIE_OS_OFFSET i,n,trunc;
  AMI_err ae=AMI_ERROR_NO_ERROR;
  char buf[20];
  i=0;
  n=info.num_items;

  cout << "Writing random input data." << endl;
  TP_LOG_APP_DEBUG_ID("Writing stream"); 
  AMI_STREAM<SortItem>* str = new AMI_STREAM<SortItem>(fname);
  assert(str->is_valid());
  str->persist(PERSIST_PERSISTENT);
  
  cout << "Opened file " << fname 
       << "\nWriting "<< n << " items..." << endl;
  
  trunc=((TPIE_OS_OFFSET)(sizeof (SortItem)))*n;
  if(trunc<0 || trunc>(4*APP_GIG)){
    cout << "Initial file length computed as "<< trunc
         << "\nSetting to 4GB "<< endl;
    trunc=4*APP_GIG;
  }
  ae = str->truncate(n); //Truncate is based on item count, not bytes
  if(ae != AMI_ERROR_NO_ERROR){
    cout << "\nError truncating file"<<endl;
  }
  str->seek(0);

  if (indicator) {
      indicator->set_title("Generating stream of random elements.");
      indicator->set_percentage_range(0,n,1000);
      indicator->init("Items written:");
  }
  while((i<n) && (ae==AMI_ERROR_NO_ERROR)){
    ae=str->write_item(SortItem(TPIE_OS_RANDOM()));   
    i++;
    if (indicator) {
	indicator->step_percentage();
    }
  }

  if (indicator) {
      indicator->done();
  }
  
  if(ae != AMI_ERROR_NO_ERROR){
    cout<< "\nWrite stopped early with AMI_ERROR: " << ae << endl;
  }
  
  cout << "\nWrote " << i << " items\n" << endl;
  delete str;
  TP_LOG_APP_DEBUG_ID("Returning from write_random_stream"); 
  return;
}

// Read sorted stream from fname and check that its elements are sorted
void check_sorted(char * fname, appInfo & info, progress_indicator_base* indicator=NULL){

  TPIE_OS_LONGLONG i,n;
  SortItem *x, x_prev;
  AMI_err ae=AMI_ERROR_NO_ERROR;
   
  n=info.num_items;

  cout << "Checking that output is sorted." << endl;
  TP_LOG_APP_DEBUG_ID("Checking that output is sorted"); 
  
  AMI_STREAM<SortItem>* str = new AMI_STREAM<SortItem>(fname);
  assert(str->is_valid());
  str->persist(PERSIST_PERSISTENT);
  str->seek(0);
  
  cout << "Opened file " << fname 
       << "\nReading "<< n << " items..." << endl;
  
  if (indicator) {
      indicator->set_title("Checking that output is sorted.");
      indicator->set_percentage_range(0,n,1000);
      indicator->init("Items checked:");
  }
  i=0;
  while((i<n) && (ae==AMI_ERROR_NO_ERROR)){
    ae=str->read_item(&x);
    i++;
    if(i>1){ 
      if(x_prev.key > x->key){
        printf("prev = %d, curr = %d, i=%d\n", x_prev.key, x->key,
        i);
      }
      tp_assert(x_prev.key <= x->key, 
                       "List not sorted! Exiting");
    }
    x_prev=*x;
    if (indicator) {
	indicator->step_percentage();
    }
  }

  if (indicator) {
      indicator->done();
  }
  
  if(ae != AMI_ERROR_NO_ERROR){
    cout<< "\nRead stopped early with AMI_ERROR: " << ae << endl;
  }
  
  cout << "\nRead " << i << " items\n" << endl;
  delete str;
  TP_LOG_APP_DEBUG_ID("Done checking that output is sorted"); 
  return;
}

void load_list(AMI_STREAM<SortItem>* str, SortItem* list, int nitems){
  SortItem *s_item;
  str->seek(0);
  for(int i=0; i<nitems; i++){
    str->read_item(&s_item);
    list[i]=*s_item;
  }
}

// Internal sort tests
void internal_sort_test(const appInfo& info){  
  TPIE_OS_SIZE_T str_mem_usage;
  int i,nitems;
  SortItem *list;
  AMI_err ae;
  cpu_timer clk;
  SortCompare cmp;
  char buf[20];
  AMI_STREAM<SortItem>* Str = 
    new AMI_STREAM<SortItem>(tpie_tempnam(APP_FILE_BASE, info.path));

  Str->seek(0);
  Str->main_memory_usage(&str_mem_usage, MM_STREAM_USAGE_MAXIMUM);
  nitems=(MM_manager.memory_available()-str_mem_usage-16)/sizeof(SortItem);
  list=new SortItem[nitems];
  for(i=0; i<nitems; i++){
    Str->write_item(SortItem(TPIE_OS_RANDOM()));   
  }
  cout << "Number items: " << nitems << " size: "
       << ll2size(nitems*sizeof(SortItem),buf) << endl;
       
  load_list(Str, list, nitems);
  cout << "TPIE quick_sort_op: ";
  clk.reset();
  clk.start();
  quick_sort_op(list, nitems, 20);
  clk.stop();
  cout << clk.wall_time() << endl;
  
  load_list(Str, list, nitems);
  cout << "TPIE quick_sort_obj: ";
  clk.reset();
  clk.start();
  quick_sort_obj(list, nitems, &cmp, 20);
  clk.stop();
  cout << clk.wall_time() << endl;
  
  load_list(Str, list, nitems);
  cout << "STL sort: ";
  clk.reset();
  clk.start();
  sort(list, list+nitems);
  clk.stop();
  cout << clk.wall_time() << endl;
  
  load_list(Str, list, nitems);
  cout << "STL sort obj: ";
  clk.reset();
  clk.start();
  sort(list, list+nitems, cmp);
  clk.stop();
  cout << clk.wall_time() << endl;
  
  load_list(Str, list, nitems);
  cout << "STL stable sort: ";
  clk.reset();
  clk.start();
  stable_sort(list, list+nitems);
  clk.stop();
  cout << clk.wall_time() << endl;
  
  load_list(Str, list, nitems);
  cout << "STL stable sort obj: ";
  clk.reset();
  clk.start();
  stable_sort(list, list+nitems, cmp);
  clk.stop();
  cout << clk.wall_time() << endl;
 
  load_list(Str, list, nitems);
  cout << "Old school stdlib qsort: ";
  clk.reset();
  clk.start();
  qsort(list, nitems, sizeof(SortItem), qcomp);
  clk.stop();
  cout << clk.wall_time() << endl;
  
  Str->persist(PERSIST_DELETE);
  delete [] list;
  delete Str;
}

AMI_err test_3x_sort(appInfo& info, enum test_type ttype, progress_indicator_base* indicator=NULL){
  //Make up some temp filenames
  cout << "****TEST START****" << endl;
  char fname[BUFSIZ], fname2[BUFSIZ];
  strncpy(fname, tpie_tempnam(APP_FILE_BASE, info.path), BUFSIZ);
  strncpy(fname2, tpie_tempnam(APP_FILE_BASE, info.path), BUFSIZ);
  //Create the input stream
  write_random_stream(fname, info, indicator);
  
  //Sort
  AMI_err ae;
  AMI_STREAM<SortItem>* inStr = new AMI_STREAM<SortItem>(fname);
  AMI_STREAM<SortItem>* outStr = new AMI_STREAM<SortItem>(fname2);
  cout << "\nMem available: " << MM_manager.memory_available()
       << "\nSorting "<< fname << " to " << fname2 << endl; 
  TP_LOG_APP_DEBUG_ID("Starting sort"); 
  SortCompare cmp;
  KeySortCompare kcmp;
  long dummykey;
  switch(ttype){
    case APP_TEST_OBJ_OP:
      cout << "Using operator sorting and object heaps" << endl;
      ae=AMI_sort(inStr, outStr, indicator);
      break;
    case APP_TEST_PTR_OP:
      cout << "Using operator sorting and ptr heaps" << endl;
      ae=AMI_ptr_sort(inStr, outStr, indicator);
      break;
    case APP_TEST_OBJ_CMPOBJ: 
      cout << "Using comp obj sorting and object heaps" << endl;
      ae=AMI_sort(inStr, outStr, &cmp, indicator);
      break;
    case APP_TEST_PTR_CMPOBJ: 
      cout << "Using comp obj sorting and ptr heaps" << endl;
      ae=AMI_ptr_sort(inStr, outStr, &cmp, indicator);
      break;
    case APP_TEST_KOBJ: 
      cout << "Using key+obj sorting and object heaps" << endl;
      ae=AMI_key_sort(inStr, outStr, dummykey, &kcmp, indicator);
      break;
    default:
      ae=AMI_ERROR_GENERIC_ERROR;
      break;
  }
  TP_LOG_APP_DEBUG_ID("Done with sort"); 
  if(ae != AMI_ERROR_NO_ERROR){
    cout << "Error during sorting: ";
    switch(ae){
      case AMI_ERROR_INSUFFICIENT_MAIN_MEMORY:
        cout << "insufficient memory. Brother, can you spare a meg?";
        break;
      default:
        cout << "AE code " << ae << " look this number up in ami_err.h";
    }
    cout << endl;
  }
  cout << "Input stream length = " << inStr->stream_len() << endl;
  cout << "Output stream length = " << outStr->stream_len() << endl;
  inStr->persist(PERSIST_PERSISTENT);
  outStr->persist(PERSIST_PERSISTENT);
  delete inStr;
  delete outStr;

  //Check the output
  if(ae==AMI_ERROR_NO_ERROR){ check_sorted(fname2, info, indicator); }

  //delete stream from disk
  cout << "\nDeleting streams " << fname << " and " << fname2 << endl;
  TPIE_OS_UNLINK(fname);
  TPIE_OS_UNLINK(fname2);

  cout << "****TEST STOP****\n" << endl;
  return ae;
}

AMI_err test_2x_sort(appInfo& info, enum test_type ttype, progress_indicator_base* indicator=NULL){
  //Make up some temp filenames
  cout << "****TEST START****" << endl;
  char fname[BUFSIZ];
  strncpy(fname, tpie_tempnam(APP_FILE_BASE, info.path), BUFSIZ);
  //Create the input stream
  write_random_stream(fname, info, indicator);
  
  //Sort
  AMI_err ae;
  AMI_STREAM<SortItem>* inStr = new AMI_STREAM<SortItem>(fname);
  cout << "\nMem available: " << MM_manager.memory_available()
       << "\nSorting "<< fname << " to itself" << endl; 
  TP_LOG_APP_DEBUG_ID("Starting sort"); 
  SortCompare cmp;
  KeySortCompare kcmp;
  long dummykey;
  switch(ttype){
    case APP_TEST_OBJ_OP:
      cout << "Using operator sorting and object heaps" << endl;
      ae=AMI_sort(inStr, indicator);
      break;
    case APP_TEST_PTR_OP:
      cout << "Using operator sorting and ptr heaps" << endl;
      ae=AMI_ptr_sort(inStr, indicator);
      break;
    case APP_TEST_OBJ_CMPOBJ: 
      cout << "Using comp obj sorting and object heaps" << endl;
      ae=AMI_sort(inStr, &cmp, indicator);
      break;
    case APP_TEST_PTR_CMPOBJ: 
      cout << "Using comp obj sorting and ptr heaps" << endl;
      ae=AMI_ptr_sort(inStr, &cmp, indicator);
      break;
    case APP_TEST_KOBJ: 
      cout << "Using key+obj sorting and object heaps" << endl;
      ae=AMI_key_sort(inStr, dummykey, &kcmp, indicator);
      break;
    default:
      ae=AMI_ERROR_GENERIC_ERROR;
      break;
  }
  TP_LOG_APP_DEBUG_ID("Done with sort"); 
  if(ae != AMI_ERROR_NO_ERROR){
    cout << "Error during sorting: ";
    switch(ae){
      case AMI_ERROR_INSUFFICIENT_MAIN_MEMORY:
        cout << "insufficient memory. Brother, can you spare a meg?";
        break;
      default:
        cout << "AE code " << ae << " look this number up in ami_err.h";
    }
    cout << endl;
  }
  cout << "Input stream length = " << inStr->stream_len() << endl;
  inStr->persist(PERSIST_PERSISTENT);
  delete inStr;

  //Check the output
  if(ae==AMI_ERROR_NO_ERROR){ check_sorted(fname, info, indicator); }

  //delete stream from disk
  cout << "\nDeleting stream " << fname << endl;
  TPIE_OS_UNLINK(fname);
  cout << "****TEST STOP****\n" << endl;
  return ae;
}

int main(int argc, char **argv){
  appInfo info;
  TPIE_OS_SIZE_T mSize;
  char buf[20];
  TPIE_OS_SRANDOM(time(NULL));
  get_app_info(argc, argv, info);

  //Set up TPIE memory manager
  MM_manager.set_memory_limit(info.mem_size);
  
  //Sort has memory problems and may crash with enforce
  MM_manager.warn_memory_limit();

  //Set up TPIE logging. 
  //Log files will be written to /tmp/tpielog_XXXXXX.txt
  //where XXXXX is randomly generated

  //tpie_log_init(TPIE_LOG_MEM_DEBUG);
  //TP_LOG_SET_THRESHOLD(TPIE_LOG_MEM_DEBUG);
  //printf("Log file is %s\n", tpie_log_name());

  TPIE_OS_LONGLONG filesize = info.num_items*info.item_size;
  cout << "Path:  " << info.path 
       << "\nNum Items: " << info.num_items 
       << "\nItem Size: " << info.item_size
       << "\nFile Size: " << ll2size(filesize,buf) << "B\n" <<endl;

  progress_indicator_arrow* myIndicator = 
      new progress_indicator_arrow("Title", "Description", 0, 100, 1);
  
  AMI_err ae;
  /*
  cout << "++++start 3X space tests++++" << endl;
  ae=test_3x_sort(info, APP_TEST_OBJ_OP, true);
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_3x_sort(info, APP_TEST_PTR_OP, true);
  }
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_3x_sort(info, APP_TEST_OBJ_CMPOBJ, true);
  }
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_3x_sort(info, APP_TEST_PTR_CMPOBJ, true);
  }
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_3x_sort(info, APP_TEST_KOBJ, true);
  }
  cout << "++++end 3X space tests++++" << endl;

  cout << "++++start 2X space tests++++" << endl;
  ae=test_2x_sort(info, APP_TEST_OBJ_OP, myIndicator); 
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_2x_sort(info, APP_TEST_PTR_OP, true);
  }
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_2x_sort(info, APP_TEST_OBJ_CMPOBJ, true);
  }
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_2x_sort(info, APP_TEST_PTR_CMPOBJ, true);
  }
*/
  if(ae==AMI_ERROR_NO_ERROR){
    ae=test_2x_sort(info, APP_TEST_KOBJ, myIndicator);
  }
  cout << "++++end 2X space tests++++" << endl;
  
  //cout << "Internal sort testing..." << endl;
  //internal_sort_test(info);
  
  if(ae==AMI_ERROR_NO_ERROR){ cout << "Test ran successfully " << endl; }
  else { cout << "Test at least ran without crashing" << endl; }

  delete myIndicator;
  return 0;
}
