#ifndef _PARSE_ARGS_H
#define _PARSE_ARGS_H

#include "app_config.h"
#include <tpie/portability.h>
#include "getopts.h"

template <typename T>
T parse_number(char *s) { 
  T n = 0; 
  unsigned int mult = 1;
  size_t len = strlen(s);
  if(isalpha(s[len-1])) {
    switch(s[len-1]) {
    case 'G':
    case 'g':
      mult = (1u << 30);
      break;
    case 'M':
    case 'm':
      mult = (1u << 20);
      break;
    case 'K':
    case 'k':
      mult = (1u << 10);
      break;
    default:
      std::cerr << "Error parsing arguments: bad number format: " << s << "\n";
      exit(-1);
      break;
    }
    s[len-1] = '\0';
  }
  n = (T)(atof(s) * mult);
  return n;
}

// Parse arguments for flags common to all test apps, as well as those
// specific to this particular app.  aso points to a string of app
// specific options, and parse_app is a pointer to a function, tpyically
// just a big switch, to handle them.
void parse_args(int argc, char **argv, struct options *application_opts,
		void (*parse_app_opts)(int idx, char *opt_arg), 
		bool stop_if_no_args = true);              
#endif // _PARSE_ARGS_H 
