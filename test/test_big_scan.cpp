#include "app_config.h"
#include <portability.h>
#include <ami_stream.h>
#include "getopts.h"

//snprintf is different on WIN/Unix platforms
#ifdef _WIN32
#define APP_SNPRINTF _snprintf
#else
#define APP_SNPRINTF snprintf
#endif

enum app_options { 
  APP_OPTION_PATH = 1,
  APP_OPTION_2GIG_TEST,
  APP_OPTION_4GIG_TEST,
  APP_OPTION_NUM_ITEMS,
  APP_OPTION_FILE_SIZE,
  APP_OPTION_EMPTY,
};

const unsigned int APP_OPTION_NUM_OPTIONS=6; 
const TPIE_OS_LONGLONG APP_MEG = 1024*1024;
const TPIE_OS_LONGLONG APP_GIG = 1024*1024*1024;
const TPIE_OS_LONGLONG APP_DEFAULT_N_ITEMS = 10000;
const int  APP_ITEM_SIZE = 64;

const char* APP_FILE_BASE =  "TPIE_Test";

// Basic parameters for the test
// better than having several global variables
typedef struct app_info{
    char * path;
    int item_size;
    TPIE_OS_LONGLONG num_items;
} appInfo;

// ********************************************************** 
// class Item
// **********************************************************

// A class that contains a simple fixed-size array of chars 
// The array can also be used as a counter in base-26 arithmetic
class Item{
  public:
    Item();
    ~Item(){};
    TPIE_OS_LONGLONG convert();
    Item operator ++(int j);
    friend ostream& operator << (ostream & out, const Item & it);
    friend bool operator == (const Item & i1, const Item & i2);
  private:
    char item[APP_ITEM_SIZE];
};

// Constructor
// Initialize item to base-26 equivalent of 0='....aaaaaaa'_26
Item::Item(){
  int i;
  for(i=0; i<APP_ITEM_SIZE-1; i++) item[i]='a';
  //null-terminate so we can print array as string
  item[APP_ITEM_SIZE-1]='\0';
}

//Converts ascii counter into actual numeric value
//assuming base 26 arithmetic
//assumes TPIE_OS_LONGLONG is 64 bits
TPIE_OS_LONGLONG Item::convert(){
  int i, stop;
  TPIE_OS_LONGLONG ans=0;
  //13 'digits' in base 26 ('zzzzzzzzzzzzz') is 
  //most digits that won't overflow a TPIE_OS_LONGLONG
  stop = (APP_ITEM_SIZE < 14) ? APP_ITEM_SIZE : 14;
  i=APP_ITEM_SIZE-stop;
  while(i < APP_ITEM_SIZE-1){
    ans = 26*ans+(item[i]-'a');
    i++;
  }
  return ans;
}

// A simple increment operator
Item Item::operator ++(int j){
  int i = APP_ITEM_SIZE-2;

  //carry 
  while( (i > 0) && (item[i]=='z') ){
    item[i]='a';
    i--;
  }
  
  //increment first non-carry
  if(i>=0){ item[i]++;}
  return *this;
}
 
bool operator == (const Item & it1, const Item & it2){
  int i=0;
  while( i<APP_ITEM_SIZE ){
    if(it1.item[i] != it2.item[i]){ return false; }
    i++;
  }
  return true;
}

// An output operator. For large arrays, displaying only the
// last few characters should suffice
ostream & operator << (ostream & out, const Item & it){
  
  //display only this many characters (at most)
  const int len = 10;
  
  if(APP_ITEM_SIZE < len){
   return out << it.item;
  }
  else{
   return out << &(it.item[APP_ITEM_SIZE-len]);
  }
}   
// ********************************************************** 
// End Class Item
// ********************************************************** 

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
  opts[i].number=APP_OPTION_2GIG_TEST;
  opts[i].name = "2gig-test";
  opts[i].shortName = "g";
  opts[i].args = 0;
  opts[i].description = "Test files larger than 2 GB";

  i++;
  opts[i].number=APP_OPTION_4GIG_TEST;
  opts[i].name = "4gig-test";
  opts[i].shortName = "G";
  opts[i].args = 0;
  opts[i].description = "Test files larger than 4 GB";
    
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
    cerr << "Warning: Only one of --2gig-test, --4gig-test\n"
         << "--numitems, or --size can be specified.\n"
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

  init_opts(opts, argc, argv); 
  
  Info.path=TMP_DIR;
  Info.item_size=APP_ITEM_SIZE;
  Info.num_items=APP_DEFAULT_N_ITEMS;

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
    else if(!optset){
      //set optset flag if applicable
      switch(optidx){
        case APP_OPTION_2GIG_TEST:
        case APP_OPTION_4GIG_TEST:
        case APP_OPTION_NUM_ITEMS:
        case APP_OPTION_FILE_SIZE:
          optset=true;
        default:
          break;
      }
      //do actual setup
      TPIE_OS_LONGLONG tmp;
      switch(optidx){
        case APP_OPTION_2GIG_TEST:
	  Info.num_items=2*((APP_GIG/Info.item_size)+1);
	  break;
        case APP_OPTION_4GIG_TEST:
	  Info.num_items=4*((APP_GIG/Info.item_size)+1);
          break;
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
        case APP_OPTION_2GIG_TEST:
        case APP_OPTION_4GIG_TEST:
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
    printf("\nSummary: Writes a stream of specified size\n"
           "to a temporary file, reads back the stream\n"
           "then deletes the stream and exits.\n"
           "Useful for testing support of\n"
           "TPIE files larger than 2GB and 4GB\n\n");
    getopts_usage(argv[0], opts);
    printf("\nEach item is %d bytes\n", APP_ITEM_SIZE);
    printf("--path-name is \"%s\" by default\n", TMP_DIR);
    printf("Suffixes K, M, and G can be appended to the\n"
           "--numitems and --size options to mean\n"
           "*1024, *1024*1024, and *2^30 respectively\n"
           "e.g., --size 128M creates a 128 MB stream\n\n");
    printf("Example runs\n\n");
    printf("Test a stream of size 2 GB\n");
    printf("%s -g\n",argv[0]);
   
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
char* ll2size(TPIE_OS_LONGLONG n){
  
  const int bufsize = 20;
  float size;
  char* buf = new char[bufsize];
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

// A simple progress bar that shows percentage complete
// and current file offset
void progress_bar(float pct, TPIE_OS_LONGLONG nbytes){

  //how long is progress bar?
  const int nticks=20;
  const int bufsize=30;
  
  int intpct, i;
  char* buf;
  
  //percent done as an integer
  intpct = (int)(100*pct);
  
  cout <<"\r[";
  i=1;
  while( i <= (nticks*pct) ){
    cout << ".";
    i++;
  }
  while( i <= nticks ){
    cout << " ";
    i++;
  }
  cout <<"] "<<intpct<<"%";
 
  buf = new char[bufsize];
  APP_SNPRINTF(buf, bufsize, " - %sB      ",ll2size(nbytes));
  cout << buf;
  cout.flush();
}

// Open a stream, write num_items, close stream
void write_test(char* fname, appInfo & info){
  
  TPIE_OS_LONGLONG i,n;
  Item x;
  AMI_err ae=AMI_ERROR_NO_ERROR;
  
  i=0;
  n=info.num_items;

  cout << "Starting write test." << endl;
  
  AMI_STREAM<Item>* str = new AMI_STREAM<Item>(fname);
  assert(str->is_valid());
  str->persist(PERSIST_PERSISTENT);
  
  cout << "Opened file " << fname 
       << "\nWriting "<< n << " items..." << endl;
  
  ae = str->truncate(((TPIE_OS_OFFSET)(sizeof (x)))*n);
  if(ae != AMI_ERROR_NO_ERROR){
    cout << "\nError truncating file"<<endl;
  }
  str->seek(0);

  float pct = 0.;
  progress_bar(pct, 0);
  while((i<n) && (ae==AMI_ERROR_NO_ERROR)){
    ae=str->write_item(x);   
    x++;
    i++;
    if( (( i/(n*1.) ) - pct) > 0.001 ){
      pct = i/(n*1.);
      progress_bar(pct, i*info.item_size);
    }
  }
  
  pct = i/(n*1.);
  progress_bar(pct, i*info.item_size);
  
  if(ae != AMI_ERROR_NO_ERROR){
    cout<< "\nWrite stopped early with AMI_ERROR: " << ae << endl;
  }
  
  cout << "\nWrote " << i 
       << " items\n" << "Closing file...";
  cout.flush();
  delete str;
  cout << "done" << endl;
  return;
}

void read_test(char * fname, appInfo & info){

  TPIE_OS_LONGLONG i,n;
  Item *x1, x2;
  AMI_err ae=AMI_ERROR_NO_ERROR;
   
  i=0;
  n=info.num_items;

  cout << "Starting read test." << endl;
  
  AMI_STREAM<Item>* str = new AMI_STREAM<Item>(fname);
  assert(str->is_valid());
  str->persist(PERSIST_PERSISTENT);
  str->seek(0);
  
  cout << "Opened file " << fname 
       << "\nReading "<< n << " items..." << endl;
  
  float pct = 0.;
  progress_bar(pct, 0);
  while((i<n) && (ae==AMI_ERROR_NO_ERROR)){
    ae=str->read_item(&x1);
    assert((*x1)==x2);
    x2++;
    i++;
    if( (( i/(n*1.) ) - pct) > 0.001 ){
      pct = i/(n*1.);
      progress_bar(pct, i*info.item_size);
    }
  }

  pct = i/(n*1.);
  progress_bar(pct, i*info.item_size);
  
  if(ae != AMI_ERROR_NO_ERROR){
    cout<< "\nRead stopped early with AMI_ERROR: " << ae << endl;
  }
  
  cout << "\nRead " << i 
       << " items\n" << "Closing file...";
  cout.flush();
  delete str;
  cout << "done" << endl;
  return;
}

int main(int argc, char **argv){
  appInfo info;
  Item x;

  //Set up TPIE memory manager
  MM_manager.set_memory_limit(64*APP_MEG);
  MM_manager.enforce_memory_limit();

  get_app_info(argc, argv, info);
 
  TPIE_OS_LONGLONG filesize = info.num_items*info.item_size;
  cout << "Path:  " << info.path 
       << "\nNum Items: " << info.num_items 
       << "\nItem Size: " << info.item_size
       << "\nFile Size: " << ll2size(filesize) << "B\n" <<endl;
 

  char * fname = tpie_tempnam(APP_FILE_BASE, info.path);
  write_test(fname, info);
  cout << endl;
  read_test(fname, info);

  //delete stream from disk
  cout << "\nDeleting stream " << fname << endl;
  TPIE_OS_UNLINK(fname);
  
  cout << "Test ran successfully " << endl;
  return 0;
}
